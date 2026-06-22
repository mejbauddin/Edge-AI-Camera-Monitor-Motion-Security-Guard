#include "RadarImageProvider.hpp"

#include "radar/RadarModel.hpp"

#include <cmath>
#include <numbers>

#include <QBrush>
#include <QLinearGradient>
#include <QPen>
#include <QRadialGradient>

namespace csx::ui {

RadarImageProvider::RadarImageProvider(radar::RadarModel* radarModel)
    : QQuickImageProvider(QQuickImageProvider::Image)
    , radarModel_(radarModel)
{
}

RadarImageProvider::~RadarImageProvider() = default;

QImage RadarImageProvider::requestImage(const QString& id, QSize* size, const QSize& requestedSize) {
    (void)id;
    const QImage image = snapshot(requestedSize);
    if (size) {
        *size = image.size();
    }
    return image;
}

QImage RadarImageProvider::snapshot(const QSize& requestedSize) const {
    const int width = requestedSize.width() > 0 ? requestedSize.width() : 520;
    const int height = requestedSize.height() > 0 ? requestedSize.height() : 520;

    QImage image(width, height, QImage::Format_ARGB32);
    image.fill(QColor(10, 14, 23));

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);

    renderRadar(painter, width, height);

    return image;
}

void RadarImageProvider::renderRadar(QPainter& painter, int width, int height) const {
    const int centerX = width / 2;
    const int centerY = height / 2;
    const int radius = std::min(width, height) / 2 - 16;
    constexpr float kPi = std::numbers::pi_v<float>;
    constexpr float kHalfPi = kPi * 0.5f;
    constexpr float kSectorArc = kPi * 0.25f;

    QRadialGradient backdrop(centerX, centerY, radius);
    backdrop.setColorAt(0, QColor(12, 28, 42, 220));
    backdrop.setColorAt(0.65, QColor(8, 16, 28, 200));
    backdrop.setColorAt(1, QColor(4, 8, 14, 180));
    painter.setPen(Qt::NoPen);
    painter.setBrush(backdrop);
    painter.drawEllipse(centerX - radius, centerY - radius, radius * 2, radius * 2);

    // Eight perimeter sectors
    for (int sector = 0; sector < 8; ++sector) {
        const float start = static_cast<float>(sector) * kSectorArc - kHalfPi;
        const float end = start + kSectorArc;
        const float mid = (start + end) * 0.5f;
        const float x1 = centerX + radius * std::cos(start);
        const float y1 = centerY + radius * std::sin(start);
        const float x2 = centerX + radius * std::cos(end);
        const float y2 = centerY + radius * std::sin(end);

        const QColor sectorColor =
            sector % 2 == 0 ? QColor(0, 229, 255, 28) : QColor(224, 64, 251, 22);
        painter.setPen(QPen(sectorColor, 1));
        painter.drawLine(centerX, centerY, static_cast<int>(x1), static_cast<int>(y1));
        painter.drawLine(centerX, centerY, static_cast<int>(x2), static_cast<int>(y2));

        const float labelR = radius * 0.88f;
        const float lx = centerX + labelR * std::cos(mid);
        const float ly = centerY + labelR * std::sin(mid);
        static const char* kLabels[] = {"N", "NE", "E", "SE", "S", "SW", "W", "NW"};
        painter.setPen(QColor(0, 229, 255, 140));
        painter.drawText(static_cast<int>(lx) - 10, static_cast<int>(ly) + 4, kLabels[sector]);
    }

    // Distance rings
    painter.setPen(QPen(QColor(0, 229, 255, 70), 1));
    for (int i = 1; i <= 5; ++i) {
        const int r = radius * i / 5;
        painter.drawEllipse(centerX - r, centerY - r, r * 2, r * 2);
    }

    painter.setPen(QPen(QColor(0, 229, 255, 90), 1));
    painter.drawLine(centerX, centerY - radius, centerX, centerY + radius);
    painter.drawLine(centerX - radius, centerY, centerX + radius, centerY);

    if (radarModel_) {
        const float sweepAngle = radarModel_->getSweepAngle();
        const float sweepX = centerX + radius * std::cos(sweepAngle);
        const float sweepY = centerY + radius * std::sin(sweepAngle);

        QLinearGradient gradient(centerX, centerY, sweepX, sweepY);
        gradient.setColorAt(0, QColor(0, 229, 255, 0));
        gradient.setColorAt(0.7, QColor(0, 229, 255, 120));
        gradient.setColorAt(1, QColor(0, 230, 118, 200));

        painter.setPen(QPen(QBrush(gradient), 3));
        painter.drawLine(centerX, centerY, static_cast<int>(sweepX), static_cast<int>(sweepY));
    }

    renderBlips(painter, centerX, centerY, radius);
}

void RadarImageProvider::renderBlips(QPainter& painter, int centerX, int centerY, int radius) const {
    if (!radarModel_) return;
    
    const auto blips = radarModel_->getBlips();
    
    for (const auto& blip : blips) {
        if (blip.isPrediction) continue;  // Skip prediction ghosts for now
        
        const float distance = blip.coord.distance * radius;
        const float angle = blip.coord.angle;
        
        const float x = centerX + distance * std::cos(angle);
        const float y = centerY + distance * std::sin(angle);
        
        // Color based on classification
        QColor color;
        switch (blip.classification) {
            case core::IdentityClassification::Authorized:
                color = QColor(0, 230, 118);  // Green
                break;
            case core::IdentityClassification::Foe:
                color = QColor(255, 23, 71);  // Red
                break;
            default:
                color = QColor(0, 229, 255);  // Cyan
                break;
        }
        
        painter.setPen(Qt::NoPen);
        painter.setBrush(color);
        painter.drawEllipse(static_cast<int>(x) - 3, static_cast<int>(y) - 3, 6, 6);
        
        // Glow effect
        QRadialGradient glow(x, y, 10);
        glow.setColorAt(0, QColor(color.red(), color.green(), color.blue(), 100));
        glow.setColorAt(1, QColor(color.red(), color.green(), color.blue(), 0));
        painter.setBrush(QBrush(glow));
        painter.drawEllipse(static_cast<int>(x) - 10, static_cast<int>(y) - 10, 20, 20);
    }
}

}  // namespace csx::ui
