#include "NotificationsUtils.h"

#include "globals.h"
#include "utils/logger.h"

#include <notifications/notifications.h>

#include <cstring>

bool DisplayNotificationMessage(std::string_view text, NotificationModuleNotificationType type, float duration) {
    if (!gNotificationModuleLoaded) {
        return false;
    }
    if (type == NOTIFICATION_MODULE_NOTIFICATION_TYPE_DYNAMIC) {
        return false;
    }

    if (type == NOTIFICATION_MODULE_NOTIFICATION_TYPE_INFO) {
        NotificationModule_SetDefaultValue(NOTIFICATION_MODULE_NOTIFICATION_TYPE_INFO, NOTIFICATION_MODULE_DEFAULT_OPTION_DURATION_BEFORE_FADE_OUT, duration);
        NotificationModule_AddInfoNotification(text.data());
    } else if (type == NOTIFICATION_MODULE_NOTIFICATION_TYPE_ERROR) {
        NotificationModule_SetDefaultValue(NOTIFICATION_MODULE_NOTIFICATION_TYPE_ERROR, NOTIFICATION_MODULE_DEFAULT_OPTION_KEEP_UNTIL_SHOWN, true);
        NotificationModule_SetDefaultValue(NOTIFICATION_MODULE_NOTIFICATION_TYPE_ERROR, NOTIFICATION_MODULE_DEFAULT_OPTION_DURATION_BEFORE_FADE_OUT, duration);
        NotificationModule_AddErrorNotification(text.data());
    } else {
        DEBUG_FUNCTION_LINE_WARN("Unsupported notification type: %d", type);
    }

    return true;
}

bool DisplayInfoNotificationMessage(std::string_view text, float duration) {
    return DisplayNotificationMessage(text, NOTIFICATION_MODULE_NOTIFICATION_TYPE_INFO, duration);
}

bool DisplayErrorNotificationMessage(std::string_view text, float duration) {
    return DisplayNotificationMessage(text, NOTIFICATION_MODULE_NOTIFICATION_TYPE_ERROR, duration);
}