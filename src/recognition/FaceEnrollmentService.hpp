#pragma once

#include "FacePipeline.hpp"
#include "RecognitionSettings.hpp"
#include "types/Frame.hpp"

#include <memory>
#include <string>

namespace csx::database {
class Database;
}

namespace csx::recognition {

struct EnrollmentResult {
    bool success{false};
    std::string message;
    std::string userName;
    std::string country;
};

class FaceEnrollmentService {
public:
    FaceEnrollmentService(std::shared_ptr<csx::database::Database> database, FacePipeline* pipeline,
                          RecognitionSettings settings);

    EnrollmentResult enrollFromImageFile(const std::string& imagePath, const std::string& name,
                                         const std::string& country, const std::string& role = "operator",
                                         const std::string& clearance = "STANDARD");
    EnrollmentResult enrollFromFrame(const core::Frame& frame, const std::string& name,
                                     const std::string& country, const std::string& role = "operator",
                                     const std::string& clearance = "STANDARD");
    std::size_t importAuthorizedFolder(const std::string& folderPath);
    [[nodiscard]] std::size_t authorizedCount() const;

private:
    [[nodiscard]] static core::Frame loadImageFrame(const std::string& path);
    [[nodiscard]] static bool parseFilenameMetadata(const std::string& stem, std::string& name,
                                                    std::string& country, std::string& role);

    std::shared_ptr<csx::database::Database> database_;
    FacePipeline* pipeline_{nullptr};
    RecognitionSettings settings_;
};

}  // namespace csx::recognition
