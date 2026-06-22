#include "YuNetDetector.hpp"

#include "CvHelpers.hpp"

#include <filesystem>
#include <algorithm>

#if defined(CSX_HAS_OPENCV)
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect/face.hpp>
#endif

namespace csx::recognition {

#if defined(CSX_HAS_OPENCV)
class YuNetDetector::FaceDetectorImpl {
public:
    cv::Ptr<cv::FaceDetectorYN> detector;
};
#endif

YuNetDetector::YuNetDetector(RecognitionSettings settings) : settings_(std::move(settings)) {
#if defined(CSX_HAS_OPENCV)
    impl_ = std::make_unique<FaceDetectorImpl>();
#endif
}

YuNetDetector::~YuNetDetector() = default;

bool YuNetDetector::loadModel(const std::string& modelPath) {
#if defined(CSX_HAS_OPENCV)
    if (!std::filesystem::exists(modelPath)) {
        modelLoaded_ = false;
        return false;
    }
    try {
        impl_->detector = cv::FaceDetectorYN::create(modelPath, "", cv::Size(320, 320), 0.45F, 0.35F, 5000);
        modelLoaded_ = impl_->detector != nullptr && !impl_->detector.empty();
        return modelLoaded_;
    } catch (const cv::Exception&) {
        modelLoaded_ = false;
        return false;
    }
#else
    (void)modelPath;
    return false;
#endif
}

std::vector<FaceDetection> YuNetDetector::detectFromTracks(
    const std::vector<core::Track>& tracks) const {
    std::vector<const core::Track*> candidates;
    candidates.reserve(tracks.size());
    for (const auto& track : tracks) {
        if (!track.active || track.bbox.width < 24.0F || track.bbox.height < 24.0F) {
            continue;
        }
        candidates.push_back(&track);
    }

    std::sort(candidates.begin(), candidates.end(),
              [](const core::Track* lhs, const core::Track* rhs) {
                  const float lhsArea = lhs->bbox.width * lhs->bbox.height;
                  const float rhsArea = rhs->bbox.width * rhs->bbox.height;
                  return lhsArea > rhsArea;
              });
    if (candidates.size() > 2) {
        candidates.resize(2);
    }

    std::vector<FaceDetection> detections;
    detections.reserve(candidates.size());
    for (const auto* track : candidates) {
        FaceDetection detection;
        detection.trackId = track->id;
        detection.score = track->confidence;
        detection.bbox = track->bbox;
        detection.bbox.height = std::max(1.0F, detection.bbox.height * 0.45F);
        detections.push_back(detection);
    }
    return detections;
}

#if defined(CSX_HAS_OPENCV)
std::vector<FaceDetection> YuNetDetector::detectWithFaceDetectorYn(const core::Frame& frame) const {
    std::vector<FaceDetection> detections;
    if (!modelLoaded_ || !frame.valid() || !impl_ || !impl_->detector) {
        return detections;
    }

    try {
        std::lock_guard<std::mutex> lock(csx::utils::openCvMutex());

        if (frame.width != lastWidth_ || frame.height != lastHeight_) {
            impl_->detector->setInputSize(cv::Size(static_cast<int>(frame.width),
                                                   static_cast<int>(frame.height)));
            lastWidth_ = frame.width;
            lastHeight_ = frame.height;
        }

        const cv::Mat inputView(static_cast<int>(frame.height), static_cast<int>(frame.width), CV_8UC3,
                                const_cast<std::uint8_t*>(frame.bgrData->data()));
        const cv::Mat input = inputView.clone();

        cv::Mat faces;
        impl_->detector->detect(input, faces);

        if (faces.empty() || faces.type() != CV_32FC1 || faces.cols < 15) {
            return detections;
        }

        for (int i = 0; i < faces.rows; ++i) {
            const float* row = faces.ptr<float>(i);
            const float score = row[14];
            if (score < 0.45F) {
                continue;
            }

            FaceDetection detection;
            detection.bbox.x = row[0];
            detection.bbox.y = row[1];
            detection.bbox.width = row[2];
            detection.bbox.height = row[3];
            detection.score = score;
            for (int k = 0; k < 5; ++k) {
                detection.landmarks[static_cast<std::size_t>(k)] = core::Point2f{
                    row[4 + k * 2], row[4 + k * 2 + 1]};
            }
            detection.hasLandmarks = true;
            detections.push_back(detection);
        }
    } catch (const cv::Exception&) {
        return {};
    }

    return detections;
}
#endif

std::vector<FaceDetection> YuNetDetector::detect(const core::Frame& frame,
                                                 const std::vector<core::Track>& tracks) const {
#if defined(CSX_HAS_OPENCV)
    if (modelLoaded_ && settings_.runtimeDnn) {
        auto detections = detectWithFaceDetectorYn(frame);
        if (!detections.empty()) {
            return detections;
        }
    }
#endif
    return detectFromTracks(tracks);
}

}  // namespace csx::recognition
