#include "ThreatSettings.hpp"

#include <algorithm>
#include <cctype>

namespace csx::threat {

ThreatSettings defaultThreatSettings() {
    return ThreatSettings{};
}

core::ThreatLevel threatLevelFromName(const std::string& name) {
    std::string upper = name;
    std::transform(upper.begin(), upper.end(), upper.begin(),
                   [](unsigned char c) { return static_cast<char>(std::toupper(c)); });

    if (upper == "GREEN") {
        return core::ThreatLevel::Green;
    }
    if (upper == "YELLOW") {
        return core::ThreatLevel::Yellow;
    }
    if (upper == "ORANGE") {
        return core::ThreatLevel::Orange;
    }
    if (upper == "RED") {
        return core::ThreatLevel::Red;
    }
    if (upper == "CRITICAL") {
        return core::ThreatLevel::Critical;
    }
    return core::ThreatLevel::Orange;
}

}  // namespace csx::threat
