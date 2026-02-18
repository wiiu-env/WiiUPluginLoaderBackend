#pragma once

#include <functional>
#include <memory>
#include <string>

class ConfigRenderer;
class Input;
class ConfigDisplayItem;

class ConfigListState {
public:
    virtual ~ConfigListState() = default;

    virtual bool HandleInput(ConfigRenderer &renderer, const Input &input) = 0;

    virtual std::string GetTitle() const                                           = 0;
    virtual std::string GetBottomBar(bool isWiimote) const                         = 0;
    virtual bool RenderItemIcon(const ConfigDisplayItem &item, int x, int y) const = 0;

    virtual std::function<bool(const ConfigDisplayItem &)> GetConfigFilter() const = 0;
    virtual bool IsMainView() const                                                = 0;
};

class DefaultListState : public ConfigListState {
public:
    bool HandleInput(ConfigRenderer &renderer, const Input &input) override;
    std::string GetTitle() const override;
    std::string GetBottomBar(bool isWiimote) const override;
    bool RenderItemIcon(const ConfigDisplayItem &item, int x, int y) const override;
    std::function<bool(const ConfigDisplayItem &)> GetConfigFilter() const override;
    bool IsMainView() const override;
};

class ActivePluginsListState : public ConfigListState {
public:
    bool HandleInput(ConfigRenderer &renderer, const Input &input) override;
    std::string GetTitle() const override;
    std::string GetBottomBar(bool isWiimote) const override;
    bool RenderItemIcon(const ConfigDisplayItem &item, int x, int y) const override;
    std::function<bool(const ConfigDisplayItem &)> GetConfigFilter() const override;
    bool IsMainView() const override;
};

class HeapTrackingListState : public ConfigListState {
public:
    bool HandleInput(ConfigRenderer &renderer, const Input &input) override;
    std::string GetTitle() const override;
    std::string GetBottomBar(bool isWiimote) const override;
    bool RenderItemIcon(const ConfigDisplayItem &item, int x, int y) const override;
    std::function<bool(const ConfigDisplayItem &)> GetConfigFilter() const override;
    bool IsMainView() const override;
};