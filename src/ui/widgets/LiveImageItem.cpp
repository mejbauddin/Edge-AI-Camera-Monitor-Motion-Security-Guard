#include "LiveImageItem.hpp"

#include <algorithm>
#include <QPainter>

namespace csx::ui {

LiveImageItem::LiveImageItem(QQuickItem* parent)
    : QQuickPaintedItem(parent)
{
    setRenderTarget(QQuickPaintedItem::FramebufferObject);
    setAntialiasing(true);
}

void LiveImageItem::setImageSupplier(ImageSupplier supplier) {
    supplier_ = std::move(supplier);
}

void LiveImageItem::refresh() {
    if (!supplier_) {
        return;
    }

    const QSize targetSize(static_cast<int>(width()), static_cast<int>(height()));
    const QImage next = supplier_(targetSize);
    if (next.isNull()) {
        return;
    }

    image_ = next;
    update();
}

void LiveImageItem::paint(QPainter* painter) {
    painter->fillRect(boundingRect(), QColor(5, 10, 18));

    if (image_.isNull()) {
        return;
    }

    const QRectF target = boundingRect();
    const QSizeF sourceSize = image_.size();
    if (sourceSize.isEmpty()) {
        return;
    }

    const qreal scale = std::min(target.width() / sourceSize.width(),
                                 target.height() / sourceSize.height());
    const QSizeF scaled(sourceSize.width() * scale, sourceSize.height() * scale);
    const QRectF dest((target.width() - scaled.width()) * 0.5,
                      (target.height() - scaled.height()) * 0.5,
                      scaled.width(), scaled.height());

    painter->drawImage(dest, image_);
}

}  // namespace csx::ui
