#include "config/ConfigSchema.hpp"

namespace csx::core {

bool ConfigSchema::validate(const nlohmann::json& config) {
    return config.is_object() && config.contains("application") && config.contains("camera");
}

nlohmann::json ConfigSchema::defaults() {
    return nlohmann::json{
        {"application", {{"name", "Cyber Sentinel X"}, {"target_fps", 60}}},
        {"camera", {{"width", 1280}, {"height", 720}, {"fps", 60}}},
        {"threat", {{"levels", {{"green_max", 20}, {"yellow_max", 40}, {"orange_max", 60}, {"red_max", 80}}}}},
        {"ui", {{"boot_sequence", true}, {"animation_speed", 1.0}}}
    };
}

}  // namespace csx::core
