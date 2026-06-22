#include "FaceLockTracker.hpp"

#include <algorithm>
#include <array>
#include <cmath>

namespace csx::recognition {

namespace {

constexpr float kIoUThreshold = 0.25F;
constexpr std::size_t kMaxLockedTracks = 6;

bool validBox(const core::Rect2f& box) noexcept {
    return std::isfinite(box.x) && std::isfinite(box.y) && std::isfinite(box.width) &&
           std::isfinite(box.height) && box.width >= 1.0F && box.height >= 1.0F;
}

}  // namespace

void FaceLockTracker::configure(const std::uint32_t holdFrames, const float smoothAlpha,
                                const std::uint32_t voteWindow) {
    holdFrames_ = holdFrames;
    smoothAlpha_ = smoothAlpha;
    voteWindow_ = voteWindow;
}

float FaceLockTracker::computeIoU(const core::Rect2f& a, const core::Rect2f& b) noexcept {
    const float x1 = std::max(a.x, b.x);
    const float y1 = std::max(a.y, b.y);
    const float x2 = std::min(a.x + a.width, b.x + b.width);
    const float y2 = std::min(a.y + a.height, b.y + b.height);

    const float intersection = std::max(0.0F, x2 - x1) * std::max(0.0F, y2 - y1);
    const float unionArea = a.width * a.height + b.width * b.height - intersection;
    if (unionArea <= 0.0F) {
        return 0.0F;
    }
    return intersection / unionArea;
}

core::Rect2f FaceLockTracker::smoothBox(const core::Rect2f& prev, const core::Rect2f& next,
                                        const float alpha) noexcept {
    const float beta = 1.0F - alpha;
    return core::Rect2f{prev.x * beta + next.x * alpha, prev.y * beta + next.y * alpha,
                        prev.width * beta + next.width * alpha,
                        prev.height * beta + next.height * alpha};
}

core::IdentityClassification FaceLockTracker::voteClassification(const LockedTrack& track) const {
    if (track.classVotes.empty()) {
        return core::IdentityClassification::Unknown;
    }

    std::array<int, 3> counts{0, 0, 0};
    for (const auto cls : track.classVotes) {
        switch (cls) {
            case core::IdentityClassification::Authorized:
                ++counts[0];
                break;
            case core::IdentityClassification::Foe:
                ++counts[1];
                break;
            default:
                ++counts[2];
                break;
        }
    }

    const int total = static_cast<int>(track.classVotes.size());
    if (counts[0] >= 2 && counts[0] >= counts[1]) {
        return core::IdentityClassification::Authorized;
    }
    if (counts[1] >= 3 && counts[1] > counts[0]) {
        return core::IdentityClassification::Foe;
    }
    if (counts[2] >= total / 2) {
        return core::IdentityClassification::Unknown;
    }
    return track.match.classification;
}

void FaceLockTracker::applyVotedIdentity(LockedTrack& track) {
    const auto voted = voteClassification(track);
    track.match.classification = voted;
    if (voted == core::IdentityClassification::Foe) {
        track.match.identityName = "UNAUTHORIZED";
    } else if (voted == core::IdentityClassification::Unknown) {
        track.match.identityName = "ANALYZING";
    }
}

void FaceLockTracker::update(const std::vector<core::FaceMatch>& detections) {
    std::vector<core::FaceMatch> filtered;
    filtered.reserve(detections.size());
    for (const auto& detection : detections) {
        if (validBox(detection.bbox)) {
            filtered.push_back(detection);
        }
    }
    if (filtered.size() > 3) {
        filtered.resize(3);
    }

    struct MatchPair {
        std::size_t detectionIndex{0};
        std::size_t trackIndex{0};
        float iou{0.0F};
    };

    std::vector<MatchPair> pairs;
    pairs.reserve(tracks_.size() * filtered.size());

    for (std::size_t d = 0; d < filtered.size(); ++d) {
        for (std::size_t t = 0; t < tracks_.size(); ++t) {
            const float iou = computeIoU(filtered[d].bbox, tracks_[t].bbox);
            if (iou >= kIoUThreshold) {
                pairs.push_back({d, t, iou});
            }
        }
    }

    std::sort(pairs.begin(), pairs.end(),
              [](const MatchPair& lhs, const MatchPair& rhs) { return lhs.iou > rhs.iou; });

    std::vector<bool> detectionUsed(filtered.size(), false);
    std::vector<bool> trackUsed(tracks_.size(), false);

    for (const auto& pair : pairs) {
        if (detectionUsed[pair.detectionIndex] || trackUsed[pair.trackIndex]) {
            continue;
        }
        detectionUsed[pair.detectionIndex] = true;
        trackUsed[pair.trackIndex] = true;

        auto& track = tracks_[pair.trackIndex];
        const auto& detection = filtered[pair.detectionIndex];

        track.bbox = smoothBox(track.bbox, detection.bbox, smoothAlpha_);
        track.match.bbox = track.bbox;
        track.match.confidence =
            track.match.confidence * 0.35F + detection.confidence * 0.65F;
        track.match.embeddingDistance = detection.embeddingDistance;
        track.match.userId = detection.userId;
        track.match.country = detection.country;
        track.match.role = detection.role;
        track.match.clearance = detection.clearance;
        track.match.identityName = detection.identityName;
        track.match.classification = detection.classification;

        track.classVotes.push_back(detection.classification);
        while (track.classVotes.size() > voteWindow_) {
            track.classVotes.pop_front();
        }
        applyVotedIdentity(track);

        track.missedFrames = 0;
        track.holding = false;
        ++track.lockFrames;
    }

    for (std::size_t d = 0; d < filtered.size(); ++d) {
        if (detectionUsed[d]) {
            continue;
        }
        LockedTrack track;
        track.id = nextId_++;
        track.bbox = filtered[d].bbox;
        track.match = filtered[d];
        track.match.trackId = track.id;
        track.classVotes.push_back(filtered[d].classification);
        applyVotedIdentity(track);
        track.lockFrames = 1;
        tracks_.push_back(std::move(track));
    }

    for (std::size_t t = 0; t < tracks_.size(); ++t) {
        if (trackUsed[t]) {
            continue;
        }
        auto& track = tracks_[t];
        ++track.missedFrames;
        track.holding = track.missedFrames > 0 && track.missedFrames <= static_cast<int>(holdFrames_);
        if (track.missedFrames > static_cast<int>(holdFrames_)) {
            track.lockFrames = 0;
        }
    }

    tracks_.erase(std::remove_if(tracks_.begin(), tracks_.end(),
                                 [this](const LockedTrack& track) {
                                     return track.missedFrames > static_cast<int>(holdFrames_);
                                 }),
                  tracks_.end());

    while (tracks_.size() > kMaxLockedTracks) {
        tracks_.erase(tracks_.begin());
    }

    for (auto& track : tracks_) {
        track.match.trackId = track.id;
        track.match.bbox = track.bbox;
    }
}

void FaceLockTracker::tickWithoutDetection() {
    for (auto& track : tracks_) {
        ++track.missedFrames;
        track.holding = track.missedFrames > 0 && track.missedFrames <= static_cast<int>(holdFrames_);
        if (track.missedFrames > static_cast<int>(holdFrames_)) {
            track.lockFrames = 0;
        }
    }

    tracks_.erase(std::remove_if(tracks_.begin(), tracks_.end(),
                                 [this](const LockedTrack& track) {
                                     return track.missedFrames > static_cast<int>(holdFrames_);
                                 }),
                  tracks_.end());
}

std::vector<core::FaceMatch> FaceLockTracker::stableMatches() const {
    std::vector<core::FaceMatch> matches;
    matches.reserve(tracks_.size());
    for (const auto& track : tracks_) {
        matches.push_back(track.match);
    }
    return matches;
}

std::string FaceLockTracker::lockPhase() const {
    if (tracks_.empty()) {
        return "SCANNING";
    }

    bool hasFreshLock = false;
    bool hasHolding = false;
    for (const auto& track : tracks_) {
        if (track.missedFrames == 0 && track.lockFrames >= 3) {
            hasFreshLock = true;
        }
        if (track.holding) {
            hasHolding = true;
        }
        if (track.missedFrames == 0 && track.lockFrames < 3) {
            return "ACQUIRING";
        }
    }

    if (hasHolding && !hasFreshLock) {
        return "HOLDING";
    }
    if (hasFreshLock) {
        return "LOCKED";
    }
    return "ACQUIRING";
}

int FaceLockTracker::lockStrength() const noexcept {
    if (tracks_.empty()) {
        return 0;
    }

    int best = 0;
    for (const auto& track : tracks_) {
        const int strength =
            std::min(100, track.lockFrames * 8 + (track.holding ? 20 : 40) - track.missedFrames * 5);
        best = std::max(best, strength);
    }
    return std::max(0, best);
}

}  // namespace csx::recognition
