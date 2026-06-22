#include "FaceEnrollmentService.hpp"

#include "ArcFaceEmbedder.hpp"
#include "Database.hpp"
#include "YuNetDetector.hpp"

#include "utils/CvHelpers.hpp"

#include <algorithm>
#include <chrono>
#include <cstring>
#include <filesystem>
#include <mutex>
#include <sstream>

#if defined(CSX_HAS_OPENCV)
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#endif

namespace csx::recognition {

namespace {

std::string joinParts(const std::vector<std::string>& parts, const std::size_t begin,
                      const std::size_t end, const char sep = ' ') {
    std::ostringstream oss;
    for (std::size_t i = begin; i < end && i < parts.size(); ++i) {
        if (i > begin) {
            oss << sep;
        }
        oss << parts[i];
    }
    return oss.str();
}

std::vector<std::string> split(const std::string& text, const char delimiter) {
    std::vector<std::string> parts;
    std::string current;
    for (const char ch : text) {
        if (ch == delimiter) {
            if (!current.empty()) {
                parts.push_back(current);
                current.clear();
            }
        } else {
            current.push_back(ch);
        }
    }
    if (!current.empty()) {
        parts.push_back(current);
    }
    return parts;
}

}  // namespace

FaceEnrollmentService::FaceEnrollmentService(std::shared_ptr<csx::database::Database> database,
                                           FacePipeline* pipeline, RecognitionSettings settings)
    : database_(std::move(database)), pipeline_(pipeline), settings_(std::move(settings)) {}

core::Frame FaceEnrollmentService::loadImageFrame(const std::string& path) {
    core::Frame frame;
#if defined(CSX_HAS_OPENCV)
    cv::Mat bgr;
    {
        std::lock_guard<std::mutex> lock(csx::utils::openCvMutex());
        bgr = cv::imread(path, cv::IMREAD_COLOR);
    }
    if (bgr.empty()) {
        return frame;
    }

    cv::Mat resized;
    {
        std::lock_guard<std::mutex> lock(csx::utils::openCvMutex());
        if (bgr.cols > 1280) {
            const double scale = 1280.0 / static_cast<double>(bgr.cols);
            cv::resize(bgr, resized, cv::Size(), scale, scale, cv::INTER_AREA);
        } else {
            resized = bgr;
        }
    }

    frame.width = static_cast<std::uint32_t>(resized.cols);
    frame.height = static_cast<std::uint32_t>(resized.rows);
    frame.cameraId = "enrollment";
    frame.captureTime = std::chrono::steady_clock::now();

    const auto byteCount = static_cast<std::size_t>(frame.width) * static_cast<std::size_t>(frame.height) * 3U;
    auto buffer = std::make_shared<std::vector<std::uint8_t>>(byteCount);
    std::memcpy(buffer->data(), resized.data, byteCount);
    frame.bgrData = std::move(buffer);
#else
    (void)path;
#endif
    return frame;
}

bool FaceEnrollmentService::parseFilenameMetadata(const std::string& stem, std::string& name,
                                                  std::string& country, std::string& role) {
    const auto parts = split(stem, '_');
    if (parts.size() < 2) {
        name = stem;
        return false;
    }

    if (parts.size() >= 3) {
        role = parts.back();
        country = parts[parts.size() - 2];
        name = joinParts(parts, 0, parts.size() - 2);
    } else {
        country = parts.back();
        name = joinParts(parts, 0, parts.size() - 1);
        role = "operator";
    }
    return true;
}

EnrollmentResult FaceEnrollmentService::enrollFromFrame(const core::Frame& frame,
                                                        const std::string& name,
                                                        const std::string& country,
                                                        const std::string& role,
                                                        const std::string& clearance) {
    EnrollmentResult result;
    if (!pipeline_ || !database_ || !database_->isOpen()) {
        result.message = "Database or pipeline unavailable";
        return result;
    }
    if (!frame.valid()) {
        result.message = "Invalid frame for enrollment";
        return result;
    }

    RecognitionSettings enrollSettings = settings_;
    enrollSettings.runtimeDnn = true;

    YuNetDetector detector(enrollSettings);
    if (!detector.loadModel(enrollSettings.detectorModel)) {
        result.message = "YuNet model not loaded at " + enrollSettings.detectorModel;
        return result;
    }

    const auto detections = detector.detect(frame, {});
    if (detections.empty()) {
        result.message = "No face detected — center your face and try again";
        return result;
    }

    const auto& best = *std::max_element(
        detections.begin(), detections.end(),
        [](const FaceDetection& a, const FaceDetection& b) { return a.score < b.score; });

    ArcFaceEmbedder embedder(enrollSettings);
    if (!embedder.loadModel(enrollSettings.embedderModel)) {
        result.message = "SFace model not loaded at " + enrollSettings.embedderModel;
        return result;
    }
    const auto embedding = embedder.embed(frame, best);
    if (embedding.empty()) {
        result.message = "Failed to compute face embedding";
        return result;
    }

    const auto userId = database_->users().createUser(name, role, country, clearance);
    if (!userId.has_value()) {
        result.message = "Failed to create user record";
        return result;
    }

    const auto faceId = database_->faces().addAuthorizedFace(*userId, embedding);
    if (!faceId.has_value()) {
        result.message = "Failed to store authorized face";
        return result;
    }

    StoredFaceIdentity identity;
    identity.faceId = *faceId;
    identity.userId = *userId;
    identity.userName = name;
    identity.role = role;
    identity.country = country;
    identity.clearance = clearance;
    identity.embedding = embedding;
    pipeline_->faceDatabase().addAuthorized(identity);
    pipeline_->reloadAuthorizedFaces();

    result.success = true;
    result.userName = name;
    result.country = country;
    result.message = "Enrolled " + name + " (" + country + ")";
    return result;
}

EnrollmentResult FaceEnrollmentService::enrollFromImageFile(const std::string& imagePath,
                                                            const std::string& name,
                                                            const std::string& country,
                                                            const std::string& role,
                                                            const std::string& clearance) {
    const auto frame = loadImageFrame(imagePath);
    return enrollFromFrame(frame, name, country, role, clearance);
}

std::size_t FaceEnrollmentService::importAuthorizedFolder(const std::string& folderPath) {
    if (!std::filesystem::exists(folderPath)) {
        return 0;
    }

    std::size_t imported = 0;
    for (const auto& entry : std::filesystem::directory_iterator(folderPath)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        const auto ext = entry.path().extension().string();
        if (ext != ".jpg" && ext != ".jpeg" && ext != ".png" && ext != ".bmp") {
            continue;
        }

        std::string name;
        std::string country;
        std::string role;
        parseFilenameMetadata(entry.path().stem().string(), name, country, role);

        const auto result = enrollFromImageFile(entry.path().string(), name, country, role);
        if (result.success) {
            ++imported;
        }
    }
    return imported;
}

std::size_t FaceEnrollmentService::authorizedCount() const {
    return pipeline_ ? pipeline_->faceDatabase().authorizedCount() : 0;
}

}  // namespace csx::recognition
