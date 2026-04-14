#include "reent.h"

#include "logger.h"
#include "plugin/PluginContainer.h"
#include "plugin/SectionInfo.h"

#include <coreinit/cache.h>
#include <coreinit/debug.h>
#include <coreinit/interrupts.h>
#include <coreinit/scheduler.h>
#include <coreinit/thread.h>
#include <forward_list>
#include <wups/reent_internal.h>

#define __WUPS_CONTEXT_THREAD_SPECIFIC_ID 0
#define WUPS_REENT_NODE_VERSION           1
#define WUPS_REENT_NODE_MAGIC             0x57555053 // WUPS

void wups_set_thread_specific_ex(const int id, void *value, OSThread *thread) {
    if (thread != nullptr) {
        if (id == 0) {
            thread->reserved[0] = reinterpret_cast<uint32_t>(value);
        } else {
            OSReport("wups_set_thread_specific: invalid id\n");
            OSFatal("wups_set_thread_specific: invalid id");
        }
    } else {
        OSReport("wups_set_thread_specific: invalid thread\n");
        OSFatal("wups_set_thread_specific: invalid thread");
    }
}

void *wups_get_thread_specific_ex(const int id, const OSThread *thread) {
    if (thread != nullptr) {
        if (id == 0) {
            return reinterpret_cast<void *>(thread->reserved[0]);
        } else {
            OSReport("wups_get_thread_specific: invalid id\n");
            OSFatal("wups_get_thread_specific: invalid id");
        }
    } else {
        OSReport("wups_get_thread_specific: invalid thread\n");
        OSFatal("wups_get_thread_specific: invalid thread\n");
    }
    return nullptr;
}

void wups_set_thread_specific(int id, void *value) {
    return wups_set_thread_specific_ex(id, value, OSGetCurrentThread());
}

void *wups_get_thread_specific(int id) {
    return wups_get_thread_specific_ex(id, OSGetCurrentThread());
}

struct __wups_reent_node {
    uint32_t magic;
    uint32_t version;
    __wups_reent_node *next;

    const void *pluginId;
    void *reentPtr;            // The ABI payload
    void (*cleanupFn)(void *); // The trampoline to clean up the payload
    OSThreadCleanupCallbackFn savedCleanup;
};

namespace {
    std::vector<__wups_reent_node *> sGlobalNodesCopy;
    std::vector<__wups_reent_node *> sGlobalNodes;
    std::recursive_mutex sGlobalNodesMutex;

    void removeNodeFromListsSafe(__wups_reent_node *curr) {
        std::lock_guard lock(sGlobalNodesMutex);
        if (const auto it = std::ranges::find(sGlobalNodes, curr); it != sGlobalNodes.end()) {
            *it = sGlobalNodes.back();
            sGlobalNodes.pop_back();
        }
    }
} // namespace

void MarkReentNodesForDeletion() {
    sGlobalNodesCopy = std::move(sGlobalNodes);
    sGlobalNodes.clear();
}

void ClearDanglingReentPtr() {
    for (auto nodeToFree : sGlobalNodesCopy) {
        if (nodeToFree->cleanupFn) {
            DEBUG_FUNCTION_LINE_VERBOSE("[%p] Call cleanupFn(%p) for node %p (dangling)", OSGetCurrentThread(), nodeToFree->reentPtr, nodeToFree);
            nodeToFree->cleanupFn(nodeToFree->reentPtr);
        }
        DEBUG_FUNCTION_LINE_VERBOSE("[%p] Free node %p (dangling)", OSGetCurrentThread(), nodeToFree);
        MEMFreeToDefaultHeap(nodeToFree);
    }
    sGlobalNodesCopy.clear();
}

static void __wups_thread_cleanup(OSThread *thread, void *stack) {
    auto *head = static_cast<__wups_reent_node *>(wups_get_thread_specific(__WUPS_CONTEXT_THREAD_SPECIFIC_ID));

    if (!head || head->magic != WUPS_REENT_NODE_MAGIC) {
        return;
    }

    OSThreadCleanupCallbackFn savedCleanup = head->savedCleanup;

    auto *curr = head;
    while (curr) {
        __wups_reent_node *next = curr->next;
        if (curr->magic == WUPS_REENT_NODE_MAGIC && curr->version >= 1) {
            if (curr->cleanupFn) {
                DEBUG_FUNCTION_LINE_VERBOSE("[%p] Call cleanupFn(%p) for node %p", thread, curr->reentPtr, curr);
                curr->cleanupFn(curr->reentPtr);
            }

            removeNodeFromListsSafe(curr);

            DEBUG_FUNCTION_LINE_VERBOSE("[%p] Free node %p", thread, curr);
            MEMFreeToDefaultHeap(curr);
        }

        curr = next;
    }

    wups_set_thread_specific(__WUPS_CONTEXT_THREAD_SPECIFIC_ID, nullptr);

    // Chain to previous OS callback
    if (savedCleanup) {
        DEBUG_FUNCTION_LINE_VERBOSE("[%p] Call saved cleanup function %p", thread, savedCleanup);
        savedCleanup(thread, stack);
    }
}

bool wups_backend_get_context(const void *pluginId, void **outPtr) {
    if (!outPtr) {
        return false;
    }

    *outPtr = nullptr;

    if (!OSGetCurrentThread()) {
        return false;
    }

    const auto *head = static_cast<__wups_reent_node *>(wups_get_thread_specific(__WUPS_CONTEXT_THREAD_SPECIFIC_ID));
    if (head && head->magic != WUPS_REENT_NODE_MAGIC) {
        return false;
    }

    const __wups_reent_node *curr = head;
    while (curr) {
        if (curr->version >= 1 && curr->pluginId == pluginId) {
            *outPtr = curr->reentPtr;
            break;
        }
        curr = curr->next;
    }

    return true;
}

bool wups_backend_register_context(const void *pluginId, void *reentPtr, void (*cleanupFn)(void *)) {
    auto *oldHead = static_cast<__wups_reent_node *>(wups_get_thread_specific(__WUPS_CONTEXT_THREAD_SPECIFIC_ID));
    if (oldHead && (oldHead->magic != WUPS_REENT_NODE_MAGIC || oldHead->version < WUPS_REENT_NODE_VERSION)) {
        return false;
    }

    auto *newNode = static_cast<__wups_reent_node *>(MEMAllocFromDefaultHeap(sizeof(__wups_reent_node)));
    if (!newNode) {
        return false;
    }

    newNode->magic        = WUPS_REENT_NODE_MAGIC;
    newNode->version      = WUPS_REENT_NODE_VERSION;
    newNode->next         = oldHead;
    newNode->pluginId     = pluginId;
    newNode->reentPtr     = reentPtr;
    newNode->cleanupFn    = cleanupFn;
    newNode->savedCleanup = nullptr;

    if (oldHead == nullptr || oldHead->magic != WUPS_REENT_NODE_MAGIC || oldHead->version < WUPS_REENT_NODE_VERSION) {
        DEBUG_FUNCTION_LINE_VERBOSE("[%p] Set OSSetThreadCleanupCallback for node %p", OSGetCurrentThread(), newNode);
        newNode->savedCleanup = OSSetThreadCleanupCallback(OSGetCurrentThread(), &__wups_thread_cleanup);
    } else {
        DEBUG_FUNCTION_LINE_VERBOSE("[%p] Add to existing cleanup chain for node %p", OSGetCurrentThread(), newNode);
        newNode->savedCleanup = oldHead->savedCleanup;
        oldHead->savedCleanup = nullptr;
    }
    OSMemoryBarrier();

    wups_set_thread_specific(__WUPS_CONTEXT_THREAD_SPECIFIC_ID, newNode);

    {
        std::lock_guard lock(sGlobalNodesMutex);
        sGlobalNodes.push_back(newNode);
        DEBUG_FUNCTION_LINE_VERBOSE("[%p] Registered reent ptr %p as node %p", OSGetCurrentThread(), reentPtr, newNode);
    }

    return true;
}

void ClearReentDataForPlugins(const std::vector<PluginContainer> &plugins) {
    auto *curThread = OSGetCurrentThread();

    for (const auto &cur : plugins) {
        if (!cur.isLinkedAndLoaded()) {
            continue;
        }

        if (cur.getMetaInformation().getWUPSVersion() <= WUPSVersion(0, 9, 0)) {
            continue;
        }

        const auto dataMemory   = cur.getPluginLinkInformation().getDataMemory();
        const auto startAddress = reinterpret_cast<uint32_t>(dataMemory.data());
        const auto endAddress   = reinterpret_cast<uint32_t>(dataMemory.data()) + dataMemory.size();

        // Zero-allocation deferred free list
        __wups_reent_node *deferredFreeHead = nullptr;

        struct PendingRestore {
            OSThread *thread;
            OSThreadCleanupCallbackFn callback;
        };
        std::vector<PendingRestore> pendingRestores;
        constexpr int PENDING_RESTORES_SIZE = 128;
        // Pre-allocate to prevent malloc() from firing while the scheduler is locked!
        pendingRestores.reserve(PENDING_RESTORES_SIZE);
        {
            // Acquire GLOBAL scheduler lock
            const int state = OSDisableInterrupts();
            __OSLockScheduler(curThread);

            OSThread *t = *reinterpret_cast<OSThread **>(0x100567F8);

            while (t) {
                auto *head = static_cast<__wups_reent_node *>(wups_get_thread_specific_ex(__WUPS_CONTEXT_THREAD_SPECIFIC_ID, t));

                // Safety checks with Sentinel/Magic
                if (!head || head->magic != WUPS_REENT_NODE_MAGIC) {
                    t = t->activeLink.next;
                    continue;
                }

                __wups_reent_node *prev = nullptr;
                auto *curr              = head;

                while (curr) {
                    __wups_reent_node *next = curr->next;
                    auto pluginIdAddr       = reinterpret_cast<uint32_t>(curr->pluginId);

                    // plugin id lives in the .data/.bss section of a plugin
                    if (pluginIdAddr >= startAddress && pluginIdAddr < endAddress) {
                        // remove from linked list
                        if (prev) {
                            prev->next = next;
                        } else {
                            head = next;
                            if (curr->savedCleanup) {
                                if (head) {
                                    head->savedCleanup = curr->savedCleanup;
                                } else {
                                    // No WUPS nodes left, mark for restoring.
                                    if (pendingRestores.size() == PENDING_RESTORES_SIZE) {
                                        OSFatal("WUPSBackend pendingRestores size limit hit");
                                    }
                                    pendingRestores.push_back({t, curr->savedCleanup});
                                }
                            }
                        }

                        curr->next       = deferredFreeHead;
                        deferredFreeHead = curr;
                    } else {
                        prev = curr;
                    }
                    curr = next;
                }

                // Restore the updated head to the thread
                wups_set_thread_specific_ex(__WUPS_CONTEXT_THREAD_SPECIFIC_ID, head, t);
                t = t->activeLink.next;
            }

            __OSUnlockScheduler(curThread);
            OSRestoreInterrupts(state);
        }

        for (const auto &restore : pendingRestores) {
            DEBUG_FUNCTION_LINE_VERBOSE("Set cleanup function for thread %p to %p", restore.thread, restore.callback);
            OSSetThreadCleanupCallback(restore.thread, restore.callback);
        }

        // Free removed entries
        __wups_reent_node *nodeToFree = deferredFreeHead;
        while (nodeToFree) {
            __wups_reent_node *nextNode = nodeToFree->next;
            if (nodeToFree->cleanupFn) {
                DEBUG_FUNCTION_LINE_VERBOSE("[%p] Call cleanupFn(%p) for node %p (cleanup)", OSGetCurrentThread(), nodeToFree->reentPtr, nodeToFree);
                nodeToFree->cleanupFn(nodeToFree->reentPtr);
            }

            removeNodeFromListsSafe(nodeToFree);
            DEBUG_FUNCTION_LINE_VERBOSE("[%p] Free node %p (cleanup)", OSGetCurrentThread(), nodeToFree);
            MEMFreeToDefaultHeap(nodeToFree);

            nodeToFree = nextNode;
        }
    }
}
