#pragma once

#include <nlohmann/json.hpp>
#include <optional>
#include <string>

namespace csx::core {

class ConfigSchema {
public:
    static bool validate(const nlohmann::json& config);
    static nlohmann::json defaults();
};

}  // namespace csx::core
