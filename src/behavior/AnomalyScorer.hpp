#pragma once

#include "BehaviorSettings.hpp"
#include "types/Frame.hpp"

#include <vector>

namespace csx::behavior {

class AnomalyScorer {
public:
    explicit AnomalyScorer(BehaviorSettings settings);

    void finalize(std::vector<core::BehaviorAnomaly>& anomalies) const;

private:
    BehaviorSettings settings_;
};

}  // namespace csx::behavior
