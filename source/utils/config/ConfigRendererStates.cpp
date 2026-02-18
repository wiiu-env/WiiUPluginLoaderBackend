#include "ConfigRendererStates.h"
#include "ConfigDisplayItem.h"
#include "ConfigRenderer.h"
#include "utils/DrawUtils.h"
#include "utils/StringTools.h"
#include "utils/input/Input.h"

std::function<bool(const ConfigDisplayItem &)> DefaultListState::GetConfigFilter() const {
    return [](const auto &item) { return item.isActivePlugin(); };
}

bool DefaultListState::IsMainView() const {
    return true;
}

bool DefaultListState::HandleInput(ConfigRenderer &renderer, const Input &input) {
    if (input.data.buttons_d & Input::eButtons::BUTTON_DOWN) {
        // Hidden Combo: L + R + Down
        constexpr auto COMBO_HOLD = Input::eButtons::BUTTON_L | Input::eButtons::BUTTON_R;
        if ((input.data.buttons_h & COMBO_HOLD) == COMBO_HOLD) {
            // Switch to Heap Tracking Mode
            if (!renderer.GetConfigItems().empty()) {
                renderer.SetListState(std::make_unique<HeapTrackingListState>());
                return true;
            }
        }
        return false; // Allow scrolling
    }

    if (input.data.buttons_d & Input::eButtons::BUTTON_A) {
        if (!renderer.GetConfigItems().empty()) {
            renderer.EnterSelectedCategory();
        }
        return true;
    }

    if (input.data.buttons_d & Input::eButtons::BUTTON_X) {
        if (!renderer.GetConfigItems().empty()) {
            renderer.SetListState(std::make_unique<ActivePluginsListState>());
        }
        return true;
    }

    if (input.data.buttons_d & (Input::eButtons::BUTTON_B | Input::eButtons::BUTTON_HOME)) {
        renderer.Exit();
        return true;
    }

    return false;
}

std::string DefaultListState::GetTitle() const {
    return "Wii U Plugin System Config Menu";
}

std::string DefaultListState::GetBottomBar(bool isWiimote) const {
    return string_format("\ue000 Select | %s Manage plugins", isWiimote ? "\uE048" : "\uE002");
}

bool DefaultListState::RenderItemIcon(const ConfigDisplayItem & /*item*/, int /*x*/, int /*y*/) const {
    return false;
}


bool ActivePluginsListState::HandleInput(ConfigRenderer &renderer, const Input &input) {
    if (input.data.buttons_d & Input::eButtons::BUTTON_A) {
        auto &items = renderer.GetFilteredConfigItems();
        int pos     = renderer.GetCursorPos();
        if (pos >= 0 && static_cast<size_t>(pos) < items.size()) {
            items[pos].get().toggleIsActivePlugin();
            renderer.SetPluginsListDirty(true);
            renderer.RequestRedraw();
        }
        return true;
    }

    if (input.data.buttons_d & Input::eButtons::BUTTON_PLUS) {
        // Apply and Reload
        renderer.ExitWithReload();
        return true;
    }

    if (input.data.buttons_d & (Input::eButtons::BUTTON_B | Input::eButtons::BUTTON_HOME)) {
        // Abort / Reset
        for (auto &item : renderer.GetFilteredConfigItems()) {
            item.get().resetIsActivePlugin();
        }
        renderer.SetPluginsListDirty(false);
        renderer.SetListState(std::make_unique<DefaultListState>());
        return true;
    }

    return false;
}

std::string ActivePluginsListState::GetTitle() const {
    return "Please select the plugins that should be active";
}

std::string ActivePluginsListState::GetBottomBar(bool /*isWiimote*/) const {
    return "\uE000 Toggle | \uE001 Abort | \uE045 Apply";
}

bool ActivePluginsListState::RenderItemIcon(const ConfigDisplayItem &item, int x, int y) const {
    DrawUtils::setFontSize(24);
    if (item.isActivePlugin()) {
        DrawUtils::print(x, y, "\u25C9");
    } else {
        DrawUtils::print(x, y, "\u25CE");
    }
    return true;
}

std::function<bool(const ConfigDisplayItem &)> ActivePluginsListState::GetConfigFilter() const {
    return [](const auto &) { return true; };
}

bool ActivePluginsListState::IsMainView() const {
    return false;
}

bool HeapTrackingListState::HandleInput(ConfigRenderer &renderer, const Input &input) {
    if (input.data.buttons_d & Input::eButtons::BUTTON_A) {
        auto &items = renderer.GetFilteredConfigItems();
        int pos     = renderer.GetCursorPos();
        if (pos >= 0 && static_cast<size_t>(pos) < items.size()) {
            items[pos].get().toggleIsHeapTrackingEnabled();
            renderer.SetPluginsListDirty(true);
            renderer.RequestRedraw();
        }
        return true;
    }

    if (input.data.buttons_d & Input::eButtons::BUTTON_PLUS) {
        // Apply and Reload
        renderer.ExitWithReload();
        return true;
    }

    if (input.data.buttons_d & (Input::eButtons::BUTTON_B | Input::eButtons::BUTTON_HOME)) {
        // Abort
        for (auto &item : renderer.GetFilteredConfigItems()) {
            item.get().resetIsHeapTrackingEnabled();
        }
        renderer.SetPluginsListDirty(false);
        renderer.SetListState(std::make_unique<DefaultListState>());
        return true;
    }

    return false;
}

std::string HeapTrackingListState::GetTitle() const {
    return "Select plugins to enable Heap Tracking";
}

std::string HeapTrackingListState::GetBottomBar(bool /*isWiimote*/) const {
    return "\uE000 Toggle | \uE001 Abort | \uE045 Apply";
}

bool HeapTrackingListState::RenderItemIcon(const ConfigDisplayItem &item, const int x, const int y) const {
    DrawUtils::setFontSize(24);
    if (item.isHeapTrackingEnabled()) {
        DrawUtils::print(x, y, "\u25C9");
    } else {
        DrawUtils::print(x, y, "\u25CE");
    }
    return true;
}

std::function<bool(const ConfigDisplayItem &)> HeapTrackingListState::GetConfigFilter() const {
    return [](const auto &item) { return item.isActivePlugin(); };
}

bool HeapTrackingListState::IsMainView() const {
    return false;
}
