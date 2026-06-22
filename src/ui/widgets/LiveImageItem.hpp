#pragma once

#include <QImage>
#include <QQuickPaintedItem>

#include <functional>

namespace csx::ui {

class LiveImageItem : public QQuickPaintedItem {
    Q_OBJECT

public:
    using ImageSupplier = std::function<QImage(const QSize& requestedSize)>;

    explicit LiveImageItem(QQuickItem* parent = nullptr);

    void setImageSupplier(ImageSupplier supplier);

public slots:
    void refresh();

protected:
    void paint(QPainter* painter) override;

private:
    ImageSupplier supplier_;
    QImage image_;
};

}  // namespace csx::ui
