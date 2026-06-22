#pragma once

#include <QObject>
#include <QString>

namespace csx {
class Application;
}

namespace csx::ui {

class EnrollmentController : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)
    Q_PROPERTY(int authorizedCount READ authorizedCount NOTIFY authorizedCountChanged)

public:
    explicit EnrollmentController(csx::Application* app, QObject* parent = nullptr);

    QString statusMessage() const { return statusMessage_; }
    int authorizedCount() const { return authorizedCount_; }

    Q_INVOKABLE bool enrollFromFile(const QString& filePath, const QString& name,
                                    const QString& country, const QString& role);
    Q_INVOKABLE bool enrollFromCamera(const QString& name, const QString& country,
                                      const QString& role);
    Q_INVOKABLE bool importFolder(const QString& folderPath);
    Q_INVOKABLE QString databaseSummary();

signals:
    void statusMessageChanged();
    void authorizedCountChanged();
    void enrollmentCompleted(bool success, const QString& message);

private:
    void setStatus(const QString& message, bool success);

    csx::Application* app_{nullptr};
    QString statusMessage_;
    int authorizedCount_{0};
};

}  // namespace csx::ui
