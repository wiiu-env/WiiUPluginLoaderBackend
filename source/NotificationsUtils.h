#pragma once

#include <string>

void StartNotificationThread();

void StopNotificationThread();

bool DisplayInfoNotificationMessage(std::string &text, float duration);

bool DisplayErrorNotificationMessage(std::string &text, float duration);