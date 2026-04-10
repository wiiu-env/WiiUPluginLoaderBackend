#include "reent.h"

#include "logger.h"
#include "plugin/PluginContainer.h"
#include "plugin/SectionInfo.h"

#include <coreinit/debug.h>
#include <coreinit/interrupts.h>
#include <coreinit/scheduler.h>
#include <coreinit/thread.h>
#include <forward_list>
#include <unordered_set>
#include <wups/reent_internal.h>

#define __WUPS_CONTEXT_THREAD_SPECIFIC_ID 0
#define WUPS_REENT_ALLOC_SENTINEL         ((__wups_reent_node *) 0xFFFFFFFF)
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
    std::unordered_set<__wups_reent_node *> sGlobalNodesSeen;
    std::vector<__wups_reent_node *> sGlobalNodes;
    std::recursive_mutex sGlobalNodesMutex;

    void removeNodeFromListsLocked(__wups_reent_node *curr) {
        std::lock_guard lock(sGlobalNodesMutex);
        if (const auto it = std::ranges::find(sGlobalNodes, curr); it != sGlobalNodes.end()) {
            *it = sGlobalNodes.back();
            sGlobalNodes.pop_back();
        }
        sGlobalNodesSeen.erase(curr);
    }
} // namespace


void ClearDanglingReentPtr() {
    std::lock_guard lock(sGlobalNodesMutex);

    DEBUG_FUNCTION_LINE_ERR("Before clean up having %d entries", sGlobalNodes.size());

    // This function is expected to be called exactly once at the start of each new application cycle.
    // It acts as a garbage collector for nodes left behind by the previous application.
    // Leftover nodes typically occur when threads are forcefully killed before they can execute
    // their cleanup callbacks, or if a thread's cleanup function was wrongly overridden.
    //
    // Mechanism: Since threads do not survive across application boundaries, any node we
    // observe across multiple cycles is guaranteed to be a dangling pointer from a dead thread.
    std::erase_if(sGlobalNodes, [](__wups_reent_node *ptr) {
        if (ptr == nullptr) {
            return true;
        }

        // Try to register the pointer in our historical "seen" tracker.
        auto [iterator, isNewValue] = sGlobalNodesSeen.insert(ptr);

        if (isNewValue) {
            // If it was newly inserted, it might be a valid node created during the current
            // application cycle (e.g., initialized via a hook in an RPL's init function).
            // We keep it in the vector for now.
            return false;
        }

        // Otherwise, we have already seen this address in a previous cycle.
        // This means the node belongs to a dead thread from an older application.
        // It is now safe to execute its payload cleanup and free the memory.
        auto *nodeToFree = *iterator;
        if (nodeToFree->cleanupFn) {
            nodeToFree->cleanupFn(nodeToFree->reentPtr);
        }
        free(nodeToFree);

        // Make to remove it from the "seen" list as well.
        sGlobalNodesSeen.erase(iterator);
        return true;
    });

    DEBUG_FUNCTION_LINE_ERR("After clean up having %d entries", sGlobalNodes.size());
}

static void __wups_thread_cleanup(OSThread *thread, void *stack) {
    auto *head = static_cast<__wups_reent_node *>(wups_get_thread_specific(__WUPS_CONTEXT_THREAD_SPECIFIC_ID));

    if (!head || head == WUPS_REENT_ALLOC_SENTINEL || head->magic != WUPS_REENT_NODE_MAGIC) {
        return;
    }

    OSThreadCleanupCallbackFn savedCleanup = head->savedCleanup;

    // Set to effective global during free to prevent malloc re-entrancy loops
    wups_set_thread_specific(__WUPS_CONTEXT_THREAD_SPECIFIC_ID, WUPS_REENT_ALLOC_SENTINEL);

    auto *curr = head;
    while (curr) {
        __wups_reent_node *next = curr->next;

        if (curr->cleanupFn) {
            curr->cleanupFn(curr->reentPtr);
        }

        removeNodeFromListsLocked(curr);

        free(curr);
        curr = next;
    }

    wups_set_thread_specific(__WUPS_CONTEXT_THREAD_SPECIFIC_ID, nullptr);

    // Chain to previous OS callback
    if (savedCleanup) {
        savedCleanup(thread, stack);
    }
}

void *wups_backend_get_context(const void *pluginId, wups_loader_init_reent_errors_t_ *outError) {
    if (!outError) {
        OSFatal("Called wups_backend_get_context with error nullptr");
        return nullptr;
    }

    if (!OSGetCurrentThread()) {
        *outError = WUPSReent_ERROR_NO_THREAD;
        return nullptr;
    }

    auto *head = static_cast<__wups_reent_node *>(wups_get_thread_specific(__WUPS_CONTEXT_THREAD_SPECIFIC_ID));

    if (head == WUPS_REENT_ALLOC_SENTINEL) {
        *outError = WUPSReent_ERROR_GLOBAL_REENT_REQUESTED;
        return nullptr;
    }
    if (head && head->magic != WUPS_REENT_NODE_MAGIC) {
        *outError = WUPSReent_ERROR_GLOBAL_REENT_REQUESTED;
        return nullptr;
    }

    const __wups_reent_node *curr = head;
    while (curr) {
        if (curr->version >= 1 && curr->pluginId == pluginId) {
            return curr->reentPtr;
        }
        curr = curr->next;
    }

    *outError = WUPSReent_ERROR_NONE;

    return nullptr;
}

void *wups_backend_set_sentinel() {
    auto *head = wups_get_thread_specific(__WUPS_CONTEXT_THREAD_SPECIFIC_ID);
    wups_set_thread_specific(__WUPS_CONTEXT_THREAD_SPECIFIC_ID, WUPS_REENT_ALLOC_SENTINEL);
    return head;
}

void wups_backend_restore_head(void *oldHead) {
    wups_set_thread_specific(__WUPS_CONTEXT_THREAD_SPECIFIC_ID, oldHead);
}

bool wups_backend_register_context(const void *pluginId, void *reentPtr, void (*cleanupFn)(void *), void *oldHeadVoid) {
    auto *oldHead = static_cast<__wups_reent_node *>(oldHeadVoid);

    auto *newNode = static_cast<__wups_reent_node *>(malloc(sizeof(__wups_reent_node)));
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

    if (oldHead == nullptr || oldHead == WUPS_REENT_ALLOC_SENTINEL || oldHead->magic != WUPS_REENT_NODE_MAGIC || oldHead->version < WUPS_REENT_NODE_VERSION) {
        newNode->savedCleanup = OSSetThreadCleanupCallback(OSGetCurrentThread(), &__wups_thread_cleanup);
    } else {
        newNode->savedCleanup = oldHead->savedCleanup;
        oldHead->savedCleanup = nullptr;
    }

    wups_set_thread_specific(__WUPS_CONTEXT_THREAD_SPECIFIC_ID, newNode);

    {
        std::lock_guard lock(sGlobalNodesMutex);
        sGlobalNodes.push_back(newNode);
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
                if (!head || head == WUPS_REENT_ALLOC_SENTINEL || head->magic != WUPS_REENT_NODE_MAGIC) {
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
        auto *oldHead                 = wups_backend_set_sentinel();
        __wups_reent_node *nodeToFree = deferredFreeHead;
        while (nodeToFree) {
            __wups_reent_node *nextNode = nodeToFree->next;
            if (nodeToFree->cleanupFn) {
                nodeToFree->cleanupFn(nodeToFree->reentPtr);
            }

            removeNodeFromListsLocked(nodeToFree);
            free(nodeToFree);

            nodeToFree = nextNode;
        }
        wups_backend_restore_head(oldHead);
    }
}
