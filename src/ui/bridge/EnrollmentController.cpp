#include "EnrollmentController.hpp"

#include "Application.hpp"

namespace csx::ui {

EnrollmentController::EnrollmentController(csx::Application* app, QObject* parent)
    : QObject(parent), app_(app)
{
    if (app_) {
        authorizedCount_ = static_cast<int>(app_->authorizedFaceCount());
    }
}

bool EnrollmentController::enrollFromFile(const QString& filePath, const QString& name,
                                          const QString& country, const QString& role) {
    if (!app_) {
        setStatus(QStringLiteral("Application unavailable"), false);
        return false;
    }

    const auto result = app_->enrollFromImage(
        filePath.toStdString(), name.toStdString(), country.toStdString(), role.toStdString());
    authorizedCount_ = static_cast<int>(app_->authorizedFaceCount());
    emit authorizedCountChanged();
    setStatus(QString::fromStdString(result.message), result.success);
    return result.success;
}

bool EnrollmentController::enrollFromCamera(const QString& name, const QString& country,
                                            const QString& role) {
    if (!app_) {
        setStatus(QStringLiteral("Application unavailable"), false);
        return false;
    }

    const auto result = app_->enrollFromLiveFrame(name.toStdString(), country.toStdString(),
                                                  role.toStdString());
    authorizedCount_ = static_cast<int>(app_->authorizedFaceCount());
    emit authorizedCountChanged();
    setStatus(QString::fromStdString(result.message), result.success);
    return result.success;
}

bool EnrollmentController::importFolder(const QString& folderPath) {
    if (!app_) {
        return false;
    }
    const auto count = app_->importAuthorizedFaces(folderPath.toStdString());
    authorizedCount_ = static_cast<int>(app_->authorizedFaceCount());
    emit authorizedCountChanged();
    setStatus(QStringLiteral("Imported %1 face(s) from folder").arg(static_cast<int>(count)), count > 0);
    return count > 0;
}

QString EnrollmentController::databaseSummary() {
    return app_ ? QString::fromStdString(app_->authorizedDatabaseSummary()) : QString();
}

void EnrollmentController::setStatus(const QString& message, const bool success) {
    statusMessage_ = message;
    emit statusMessageChanged();
    emit enrollmentCompleted(success, message);
}

}  // namespace csx::ui
