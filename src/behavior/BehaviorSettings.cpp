#include "BehaviorSettings.hpp"

namespace csx::behavior {

BehaviorSettings defaultBehaviorSettings(const std::uint32_t frameWidth,
                                         const std::uint32_t frameHeight) {
    BehaviorSettings settings;

    const float width = static_cast<float>(frameWidth);
    const float height = static_cast<float>(frameHeight);

    RestrictedZone perimeter;
    perimeter.id = "perimeter_center";
    perimeter.restricted = true;
    perimeter.polygon = {
        {width * 0.25F, height * 0.25F},
        {width * 0.75F, height * 0.25F},
        {width * 0.75F, height * 0.75F},
        {width * 0.25F, height * 0.75F},
    };
    settings.zones.push_back(std::move(perimeter));

    return settings;
}

}  // namespace csx::behavior
