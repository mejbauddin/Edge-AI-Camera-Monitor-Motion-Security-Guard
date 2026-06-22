#pragma once

#include "types/Frame.hpp"

#include <cstdint>
#include <deque>
#include <string>
#include <vector>

namespace csx::recognition {

class FaceLockTracker {
public:
    void configure(std::uint32_t holdFrames, float smoothAlpha, std::uint32_t voteWindow);

    void update(const std::vector<core::FaceMatch>& detections);
    void tickWithoutDetection();

    [[nodiscard]] std::vector<core::FaceMatch> stableMatches() const;
    [[nodiscard]] std::string lockPhase() const;
    [[nodiscard]] int lockStrength() const noexcept;

private:
    struct LockedTrack {
        std::uint32_t id{0};
        core::Rect2f bbox{};
        core::FaceMatch match{};
        int missedFrames{0};
        int lockFrames{0};
        bool holding{false};
        std::deque<core::IdentityClassification> classVotes;
    };

    [[nodiscard]] static float computeIoU(const core::Rect2f& a, const core::Rect2f& b) noexcept;
    [[nodiscard]] static core::Rect2f smoothBox(const core::Rect2f& prev, const core::Rect2f& next,
                                                float alpha) noexcept;
    [[nodiscard]] core::IdentityClassification voteClassification(const LockedTrack& track) const;
    void applyVotedIdentity(LockedTrack& track);

    std::vector<LockedTrack> tracks_;
    std::uint32_t nextId_{1};
    std::uint32_t holdFrames_{15};
    float smoothAlpha_{0.38F};
    std::uint32_t voteWindow_{8};
};

}  // namespace csx::recognition
