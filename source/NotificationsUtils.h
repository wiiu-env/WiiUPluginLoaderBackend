#pragma once

#include <string_view>

bool DisplayInfoNotificationMessage(std::string_view text, float duration);

bool DisplayErrorNotificationMessage(std::string_view text, float duration);