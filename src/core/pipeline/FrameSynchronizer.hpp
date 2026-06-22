#pragma once

#include "types/Frame.hpp"

#include <cstdint>
#include <mutex>
#include <optional>
#include <unordered_map>

namespace csx::core {

class FrameSynchronizer {
public:
    void registerFrame(const Frame& frame);
    void registerTracks(std::uint64_t frameSequence, std::vector<Track> tracks);
    void registerFaces(std::uint64_t frameSequence, std::vector<FaceMatch> faces);

    struct SyncedSnapshot {
        Frame frame;
        std::vector<Track> tracks;
        std::vector<FaceMatch> faces;
    };

    [[nodiscard]] std::optional<SyncedSnapshot> latestSynced() const;

private:
    mutable std::mutex mutex_;
    Frame latestFrame_;
    std::unordered_map<std::uint64_t, std::vector<Track>> tracksByFrame_;
    std::unordered_map<std::uint64_t, std::vector<FaceMatch>> facesByFrame_;
};

}  // namespace csx::core
