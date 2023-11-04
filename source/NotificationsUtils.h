#pragma once

#include <string>

void StartNotificationThread();

void StopNotificationThread();

bool DisplayInfoNotificationMessage(std::string_view text, float duration);

bool DisplayErrorNotificationMessage(std::string_view text, float duration);