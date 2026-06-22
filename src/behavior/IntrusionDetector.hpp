#pragma once

#include "BehaviorSettings.hpp"
#include "types/Frame.hpp"

#include <vector>

namespace csx::behavior {

class IntrusionDetector {
public:
    explicit IntrusionDetector(BehaviorSettings settings);

    void detect(const std::vector<core::Track>& tracks,
                std::vector<core::BehaviorAnomaly>& outAnomalies) const;

private:
    BehaviorSettings settings_;
};

}  // namespace csx::behavior
