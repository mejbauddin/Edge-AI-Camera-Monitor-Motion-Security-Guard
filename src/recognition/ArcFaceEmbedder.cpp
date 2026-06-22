#include "ArcFaceEmbedder.hpp"

#include "CvHelpers.hpp"
#include "EmbeddingMath.hpp"

#include <cstring>
#include <filesystem>

#if defined(CSX_HAS_OPENCV)
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect/face.hpp>
#endif

namespace csx::recognition {

#if defined(CSX_HAS_OPENCV)
class ArcFaceEmbedder::RecognizerImpl {
public:
    cv::Ptr<cv::FaceRecognizerSF> recognizer;
};
#endif

ArcFaceEmbedder::ArcFaceEmbedder(RecognitionSettings settings) : settings_(std::move(settings)) {
#if defined(CSX_HAS_OPENCV)
    impl_ = std::make_unique<RecognizerImpl>();
#endif
}

ArcFaceEmbedder::~ArcFaceEmbedder() = default;

bool ArcFaceEmbedder::loadModel(const std::string& modelPath) {
#if defined(CSX_HAS_OPENCV)
    if (!std::filesystem::exists(modelPath)) {
        modelLoaded_ = false;
        return false;
    }
    try {
        impl_->recognizer = cv::FaceRecognizerSF::create(modelPath, "");
        modelLoaded_ = impl_->recognizer != nullptr && !impl_->recognizer.empty();
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

std::vector<float> ArcFaceEmbedder::embedWithCpuFallback(const core::Frame& frame,
                                                         const core::Rect2f& faceBox) const {
    constexpr std::uint32_t kSize = 32;

    const auto x = static_cast<std::uint32_t>(std::max(0.0F, faceBox.x));
    const auto y = static_cast<std::uint32_t>(std::max(0.0F, faceBox.y));
    const auto width = static_cast<std::uint32_t>(
        std::min(faceBox.width, static_cast<float>(frame.width - x)));
    const auto height = static_cast<std::uint32_t>(
        std::min(faceBox.height, static_cast<float>(frame.height - y)));

    if (width == 0 || height == 0) {
        return {};
    }

    std::vector<std::uint8_t> region;
    csx::utils::copyBgrRegion(frame.bgrData->data(), frame.width, frame.height, x, y, width, height,
                              region);

    std::vector<std::uint8_t> patch(csx::utils::bgrBufferSize(kSize, kSize));
    csx::utils::resizeBgrNearestNeighbor(region.data(), width, height, patch.data(), kSize, kSize);

    std::vector<float> embedding;
    embedding.reserve(patch.size());
    for (const std::uint8_t value : patch) {
        embedding.push_back(static_cast<float>(value) / 255.0F);
    }
    if (embedding.size() > settings_.embeddingSize) {
        embedding.resize(settings_.embeddingSize);
    } else if (embedding.size() < settings_.embeddingSize) {
        embedding.resize(settings_.embeddingSize, 0.0F);
    }

    normalizeEmbedding(embedding);
    return embedding;
}

#if defined(CSX_HAS_OPENCV)
std::vector<float> ArcFaceEmbedder::embedWithOpenCv(const core::Frame& frame,
                                                    const FaceDetection& detection) const {
    try {
        std::lock_guard<std::mutex> lock(csx::utils::openCvMutex());

        const cv::Mat inputView(static_cast<int>(frame.height), static_cast<int>(frame.width), CV_8UC3,
                                const_cast<std::uint8_t*>(frame.bgrData->data()));
        const cv::Mat input = inputView.clone();

        cv::Mat faceBox(1, 15, CV_32FC1);
        float* row = faceBox.ptr<float>(0);
        row[0] = detection.bbox.x;
        row[1] = detection.bbox.y;
        row[2] = detection.bbox.width;
        row[3] = detection.bbox.height;
        if (detection.hasLandmarks) {
            for (int k = 0; k < 5; ++k) {
                row[4 + k * 2] = detection.landmarks[static_cast<std::size_t>(k)].x;
                row[4 + k * 2 + 1] = detection.landmarks[static_cast<std::size_t>(k)].y;
            }
        } else {
            const float cx = detection.bbox.x + detection.bbox.width * 0.5F;
            const float cy = detection.bbox.y + detection.bbox.height * 0.5F;
            const float eyeY = detection.bbox.y + detection.bbox.height * 0.35F;
            const float mouthY = detection.bbox.y + detection.bbox.height * 0.72F;
            row[4] = detection.bbox.x + detection.bbox.width * 0.3F;
            row[5] = eyeY;
            row[6] = detection.bbox.x + detection.bbox.width * 0.7F;
            row[7] = eyeY;
            row[8] = cx;
            row[9] = cy;
            row[10] = detection.bbox.x + detection.bbox.width * 0.35F;
            row[11] = mouthY;
            row[12] = detection.bbox.x + detection.bbox.width * 0.65F;
            row[13] = mouthY;
        }
        row[14] = detection.score;

        cv::Mat aligned;
        impl_->recognizer->alignCrop(input, faceBox, aligned);
        cv::Mat feature;
        impl_->recognizer->feature(aligned, feature);

        std::vector<float> embedding(static_cast<std::size_t>(feature.total()));
        std::memcpy(embedding.data(), feature.ptr<float>(), embedding.size() * sizeof(float));
        if (embedding.size() > settings_.embeddingSize) {
            embedding.resize(settings_.embeddingSize);
        } else if (embedding.size() < settings_.embeddingSize) {
            embedding.resize(settings_.embeddingSize, 0.0F);
        }
        normalizeEmbedding(embedding);
        return embedding;
    } catch (const cv::Exception&) {
        return embedWithCpuFallback(frame, detection.bbox);
    }
}
#endif

std::vector<float> ArcFaceEmbedder::embed(const core::Frame& frame,
                                          const FaceDetection& detection) const {
    if (!frame.valid()) {
        return {};
    }

#if defined(CSX_HAS_OPENCV)
    if (modelLoaded_ && settings_.runtimeDnn) {
        return embedWithOpenCv(frame, detection);
    }
#endif
    return embedWithCpuFallback(frame, detection.bbox);
}

}  // namespace csx::recognition
