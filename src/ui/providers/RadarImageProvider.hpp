#pragma once

#include <QQuickImageProvider>
#include <QImage>
#include <QPainter>
#include <QColor>
#include <memory>

namespace csx::radar {
    struct RadarBlip;
    class RadarModel;
}

namespace csx::ui {

// ──────────────────────────────────────────────────────────────────────────────
// RadarImageProvider — renders radar display to QML
// ──────────────────────────────────────────────────────────────────────────────
class RadarImageProvider : public QQuickImageProvider {
public:
    explicit RadarImageProvider(radar::RadarModel* radarModel);
    ~RadarImageProvider() override;

    QImage requestImage(const QString& id, QSize* size, const QSize& requestedSize) override;

    [[nodiscard]] QImage snapshot(const QSize& requestedSize = QSize(400, 400)) const;

private:
    void renderRadar(QPainter& painter, int width, int height) const;
    void renderBlips(QPainter& painter, int centerX, int centerY, int radius) const;

    radar::RadarModel* radarModel_;
};

}  // namespace csx::ui
