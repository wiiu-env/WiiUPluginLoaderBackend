#include "NotificationsUtils.h"
#include "globals.h"
#include "utils/logger.h"
#include <coreinit/cache.h>
#include <coreinit/messagequeue.h>
#include <coreinit/thread.h>
#include <cstring>
#include <notifications/notifications.h>
#include <thread>

std::unique_ptr<std::thread> sNotificationsThread;
static bool sShutdownNotificationsThread = false;

OSMessageQueue sNotificationQueue;
OSMessage sNotificationMessages[0x10];

#define NOTIFICATION_QUEUE_COMMAND_STOP  0
#define NOTIFICATION_QUEUE_COMMAND_ERROR 1

struct NotificationMessageWrapper {
    NotificationModuleNotificationType type = NOTIFICATION_MODULE_NOTIFICATION_TYPE_INFO;
    char text[512]                          = {};
    float duration                          = 10.0f;
};

void NotificationMainLoop() {
    bool isOverlayReady = false;
    while (!sShutdownNotificationsThread &&
           NotificationModule_IsOverlayReady(&isOverlayReady) == NOTIFICATION_MODULE_RESULT_SUCCESS && !isOverlayReady) {
        OSSleepTicks(OSMillisecondsToTicks(16));
    }
    if (sShutdownNotificationsThread || !isOverlayReady) {
        return;
    }

    OSMessage recv;
    while (OSReceiveMessage(&sNotificationQueue, &recv, OS_MESSAGE_FLAGS_BLOCKING)) {
        if (recv.args[0] == NOTIFICATION_QUEUE_COMMAND_STOP) {
            break;
        }

        if (recv.args[0] == NOTIFICATION_QUEUE_COMMAND_ERROR) {
            auto *param = (NotificationMessageWrapper *) recv.message;
            if (param->type == NOTIFICATION_MODULE_NOTIFICATION_TYPE_INFO) {
                NotificationModule_SetDefaultValue(NOTIFICATION_MODULE_NOTIFICATION_TYPE_INFO, NOTIFICATION_MODULE_DEFAULT_OPTION_DURATION_BEFORE_FADE_OUT, param->duration);
                NotificationModule_AddInfoNotification(param->text);
            } else if (param->type == NOTIFICATION_MODULE_NOTIFICATION_TYPE_ERROR) {
                NotificationModule_SetDefaultValue(NOTIFICATION_MODULE_NOTIFICATION_TYPE_ERROR, NOTIFICATION_MODULE_DEFAULT_OPTION_DURATION_BEFORE_FADE_OUT, param->duration);
                NotificationModule_AddErrorNotification(param->text);
            } else {
                DEBUG_FUNCTION_LINE_WARN("Unsupported notification type: %d", param->type);
            }

            free(param);
            continue;
        }
    }
}

bool DisplayNotificationMessage(std::string_view text, NotificationModuleNotificationType type, float duration) {
    if (!gNotificationModuleLoaded) {
        return false;
    }
    if (type == NOTIFICATION_MODULE_NOTIFICATION_TYPE_DYNAMIC) {
        return false;
    }
    auto *param = (NotificationMessageWrapper *) malloc(sizeof(NotificationMessageWrapper));
    if (!param) {
        return false;
    }
    strncpy(param->text, text.data(), sizeof(param->text) - 1);
    param->type     = type;
    param->duration = duration;

    OSMessage send;
    send.message = param;
    send.args[0] = NOTIFICATION_QUEUE_COMMAND_ERROR;
    auto res     = OSSendMessage(&sNotificationQueue, &send, OS_MESSAGE_FLAGS_NONE);
    if (!res) {
        DEBUG_FUNCTION_LINE_ERR("Failed to add Error Notification: Queue full");
        free(param);
        return false;
    }
    return true;
}

bool DisplayInfoNotificationMessage(std::string_view text, float duration) {
    return DisplayNotificationMessage(text, NOTIFICATION_MODULE_NOTIFICATION_TYPE_INFO, duration);
}

bool DisplayErrorNotificationMessage(std::string_view text, float duration) {
    return DisplayNotificationMessage(text, NOTIFICATION_MODULE_NOTIFICATION_TYPE_ERROR, duration);
}

void StartNotificationThread() {
    sNotificationsThread.reset();
    sShutdownNotificationsThread = false;
    if (!gNotificationModuleLoaded) {
        return;
    }

    constexpr int32_t messageSize = sizeof(sNotificationMessages) / sizeof(sNotificationMessages[0]);
    OSInitMessageQueue(&sNotificationQueue, sNotificationMessages, messageSize);
    sNotificationsThread = make_unique_nothrow<std::thread>(NotificationMainLoop);
}

void StopNotificationThread() {
    if (sNotificationsThread != nullptr) {
        OSMessage message;
        message.args[0] = NOTIFICATION_QUEUE_COMMAND_STOP;
        OSSendMessage(&sNotificationQueue, &message, OS_MESSAGE_FLAGS_NONE);
        sShutdownNotificationsThread = true;
        OSMemoryBarrier();
        sNotificationsThread->join();
        sNotificationsThread.reset();
    }
}