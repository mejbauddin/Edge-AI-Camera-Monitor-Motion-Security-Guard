#pragma once

#include "config/ConfigSchema.hpp"
#include "interfaces/Interfaces.hpp"

#include <nlohmann/json.hpp>
#include <shared_mutex>
#include <string>

namespace csx::core {

class ConfigService final : public IConfigService {
public:
    explicit ConfigService(std::shared_ptr<IEventBus> eventBus = nullptr);

    bool load(const std::string& path) override;
    bool reload() override;

    [[nodiscard]] std::string getString(const std::string& key,
                                        const std::string& defaultValue) const override;
    [[nodiscard]] int getInt(const std::string& key, int defaultValue) const override;
    [[nodiscard]] float getFloat(const std::string& key, float defaultValue) const override;
    [[nodiscard]] bool getBool(const std::string& key, bool defaultValue) const override;

    [[nodiscard]] const nlohmann::json& raw() const;
    [[nodiscard]] std::string path() const;

private:
    [[nodiscard]] const nlohmann::json* resolve(const std::string& dottedKey) const;

    std::shared_ptr<IEventBus> eventBus_;
    mutable std::shared_mutex mutex_;
    nlohmann::json config_{ConfigSchema::defaults()};
    std::string path_;
};

std::shared_ptr<IConfigService> createConfigService(std::shared_ptr<IEventBus> eventBus = nullptr);

}  // namespace csx::core
