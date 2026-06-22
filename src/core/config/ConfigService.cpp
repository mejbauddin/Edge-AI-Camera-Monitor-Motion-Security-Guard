#include "config/ConfigService.hpp"

#include "events/EventTypes.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>

namespace csx::core {

namespace {

std::vector<std::string> splitKey(const std::string& key) {
    std::vector<std::string> parts;
    std::stringstream stream(key);
    std::string segment;
    while (std::getline(stream, segment, '.')) {
        if (!segment.empty()) {
            parts.push_back(segment);
        }
    }
    return parts;
}

}  // namespace

ConfigService::ConfigService(std::shared_ptr<IEventBus> eventBus)
    : eventBus_(std::move(eventBus)) {}

bool ConfigService::load(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return false;
    }

    nlohmann::json loaded;
    try {
        file >> loaded;
    } catch (const nlohmann::json::exception&) {
        return false;
    }

    if (!ConfigSchema::validate(loaded)) {
        return false;
    }

    {
        std::unique_lock lock(mutex_);
        config_ = std::move(loaded);
        path_ = path;
    }

    if (eventBus_) {
        eventBus_->publish(events::kConfigReloaded, path);
    }
    return true;
}

bool ConfigService::reload() {
    if (path_.empty()) {
        return false;
    }
    return load(path_);
}

const nlohmann::json* ConfigService::resolve(const std::string& dottedKey) const {
    const auto parts = splitKey(dottedKey);
    const nlohmann::json* current = &config_;
    for (const auto& part : parts) {
        if (!current->is_object() || !current->contains(part)) {
            return nullptr;
        }
        current = &(*current)[part];
    }
    return current;
}

std::string ConfigService::getString(const std::string& key,
                                     const std::string& defaultValue) const {
    std::shared_lock lock(mutex_);
    const auto* node = resolve(key);
    if (node == nullptr || !node->is_string()) {
        return defaultValue;
    }
    return node->get<std::string>();
}

int ConfigService::getInt(const std::string& key, const int defaultValue) const {
    std::shared_lock lock(mutex_);
    const auto* node = resolve(key);
    if (node == nullptr || !node->is_number_integer()) {
        return defaultValue;
    }
    return node->get<int>();
}

float ConfigService::getFloat(const std::string& key, const float defaultValue) const {
    std::shared_lock lock(mutex_);
    const auto* node = resolve(key);
    if (node == nullptr || !node->is_number()) {
        return defaultValue;
    }
    return node->get<float>();
}

bool ConfigService::getBool(const std::string& key, const bool defaultValue) const {
    std::shared_lock lock(mutex_);
    const auto* node = resolve(key);
    if (node == nullptr || !node->is_boolean()) {
        return defaultValue;
    }
    return node->get<bool>();
}

const nlohmann::json& ConfigService::raw() const {
    std::shared_lock lock(mutex_);
    return config_;
}

std::string ConfigService::path() const {
    std::shared_lock lock(mutex_);
    return path_;
}

std::shared_ptr<IConfigService> createConfigService(std::shared_ptr<IEventBus> eventBus) {
    return std::make_shared<ConfigService>(std::move(eventBus));
}

}  // namespace csx::core
