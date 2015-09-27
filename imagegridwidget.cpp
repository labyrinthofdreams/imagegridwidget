#include <algorithm>
#include <iterator>
#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include "imagegridwidget.hpp"

namespace {

template <class T>
T calculateHeight(const QPixmap &img, const T newWidth) {
    return static_cast<double>(img.height()) / img.width() * newWidth;
}

enum Side {
    Top, Right, Bottom, Left
};

Side getWidth(const QPoint &needle, const QPoint &haystack) {
    const auto midW = haystack.x() / 2;
    if(needle.x() < midW) {
        return Left;
    }
    else {
        return Right;
    }
}

Side getHeight(const QPoint &needle, const QPoint &haystack) {
    const auto midH = haystack.y() / 2;
    if(needle.y() < midH) {
        return Top;
    }
    else {
        return Bottom;
    }
}


Side getSide(const QPoint &needle, const QPoint &haystack) {
    const auto x = getWidth(needle, haystack);
    const auto y = getHeight(needle, haystack);
    if(x == Left && y == Top) {
        // top-left
        const auto diagonalPixelHorizontal = needle.y() * 2;
        if(needle.x() > diagonalPixelHorizontal) {
            return Top;
        }
        else {
            return Left;
        }
    }
    else if(x == Right && y == Top) {
        // top-right
        QPoint tmp(haystack.x() - needle.x(), needle.y());
        const auto diagonalPixelVertical = tmp.x() / 2;
        if(tmp.y() < diagonalPixelVertical) {
            return Top;
        }
        else {
            return Right;
        }
    }
    else if(x == Left && y == Bottom) {
        // bottom-left
        QPoint tmp(haystack.x() - needle.x(), needle.y());
        const auto diagonalPixelVertical = tmp.x() / 2;
        if(tmp.y() < diagonalPixelVertical) {
            return Left;
        }
        else {
            return Bottom;
        }
    }
    if(x == Right && y == Bottom) {
        // bottom-right
        const auto diagonalPixelHorizontal = needle.y() * 2;
        if(needle.x() > diagonalPixelHorizontal) {
            return Right;
        }
        else {
            return Bottom;
        }
    }
}

} // namespace

// TODO: See if you can combine the image edge detection in
// dropevent and paintevent into one function
// so that we'd get 100% equal detection in both functions

ImageGridWidget::ImageGridWidget(QWidget *parent) :
    QWidget(parent),
    point_(),
    layout_(new QVBoxLayout),
    isDragging_(false)
{
    layout_->setSpacing(10);
    layout_->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding));

    setAcceptDrops(true);
    setLayout(layout_);
    setMaximumHeight(800);
    setMouseTracking(true);
}



int ImageGridWidget::getRowCount() const
{
    const QList<Index> keys = grid_.keys();
    if(keys.isEmpty()) {
        return 0;
    }

    const auto l = [](const Index &lhs, const Index &rhs){ return lhs.first < rhs.first; };
    const Index max = *std::max_element(keys.cbegin(), keys.cend(), l);
    return max.first + 1;
}

int ImageGridWidget::getColumnsForRow(const int row) const
{
    const QList<Index> keys = grid_.keys();
    return std::count_if(keys.cbegin(), keys.cend(),
                         [&row](const Index &key){ return key.first == row; });
}

QMap<int, QSize> ImageGridWidget::calculateRowSizes() const
{
    // 1. Calculate how many columns each row has
    const QList<Index> keys = grid_.keys();
    QHash<int, int> counts;
    for(const Index &key : keys) {
        counts[key.first]++;
    }

    // 2. Minimum width will be used to calculate the new sizes
    // for each row where the column size differs
    const QPixmap pmIcon = grid_.first().pixmap(grid_.first().availableSizes().first());
    const auto minWidth = pmIcon.width();

    // 3. Calculate the image widths for each row
    QMap<int, QSize> sizes;
    for(auto it = counts.cbegin(); it != counts.cend(); ++it) {
        const auto numberOfImages = it.value();
        const auto rowSpacing = (numberOfImages - 1) * layout_->spacing();
        const auto widthWithoutSpacing = minWidth - rowSpacing;
        const auto rowImgWidth = widthWithoutSpacing / numberOfImages;
        sizes.insert(it.key(), QSize(rowImgWidth,
                                     calculateHeight(pmIcon, rowImgWidth)));
    }

    return sizes;
}

void ImageGridWidget::insertBefore(const int row, const QIcon &icon)
{
    if(row < 0) {
        return;
    }

    // Insert icon into the layout
    auto lo = new QHBoxLayout;
    const QPixmap pm = icon.pixmap(icon.availableSizes().first());
    auto label = new QLabel;
    label->setPixmap(pm);
    lo->addWidget(label);
    lo->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding));
    layout_->insertLayout(row, lo);

    QMap<Index, QIcon> newGrid;
    // 1. Copy icons above it with their current position
    auto it = grid_.begin();
    for(; it != grid_.end(); ++it) {
        const Index current = it.key();
        if(current.first < row) {
            newGrid.insert(qMakePair(current.first, current.second), it.value());
        }
        else {
            break;
        }
    }

    // 2. Copy icons below it with their position moved down by one row
    for(; it != grid_.end(); ++it) {
        const Index current = it.key();
        newGrid.insert(qMakePair(current.first + 1, current.second), it.value());
    }

    // 3. Insert the new icon where there's an empty row now
    newGrid.insert(qMakePair(row, 0), icon);
    grid_.swap(newGrid);
}

void ImageGridWidget::insertBefore(const Index index, const QIcon &icon)
{
    if(index.first < 0 || index.second < 0) {
        return;
    }

    QMap<Index, QIcon> newGrid;
    for(auto it = grid_.begin(); it != grid_.end(); ++it) {
        const Index current = it.key();
        if(current.first != index.first) {
            newGrid.insert(current, it.value());
        }
        else {
            if(current.second < index.second) {
                newGrid.insert(current, it.value());
            }
            else {
                newGrid.insert(qMakePair(current.first, current.second + 1), it.value());
            }
        }
    }

    newGrid.insert(index, icon);
    grid_.swap(newGrid);

    // Insert icon into the layout
    const QPixmap pm = icon.pixmap(icon.availableSizes().first());
    auto label = new QLabel;
    label->setPixmap(pm);
    auto lo = qobject_cast<QHBoxLayout *>(layout_->itemAt(index.first)->layout());
    lo->insertWidget(index.second, label);

    // Resize widgets
    const QMap<int, QSize> sizes = calculateRowSizes();
    for(auto it = sizes.cbegin(); it != sizes.cend(); ++it) {
        const auto row = it.key();
        auto lo = qobject_cast<QHBoxLayout *>(layout_->itemAt(row)->layout());
        // count() - 1 skips the spacer item
        for(auto idx = 0; idx < lo->count() - 1; ++idx) {
            const QSize size = it.value();
            const QPixmap pm = grid_.value(qMakePair(row, idx)).pixmap(size);
            qobject_cast<QLabel *>(lo->itemAt(idx)->widget())->setPixmap(pm);
        }
    }
}

void ImageGridWidget::dragEnterEvent(QDragEnterEvent *event)
{
    event->accept();

    isDragging_ = true;
}

void ImageGridWidget::dragLeaveEvent(QDragLeaveEvent *event)
{
    Q_UNUSED(event);

    isDragging_ = false;

    repaint();
}

void ImageGridWidget::dragMoveEvent(QDragMoveEvent *event)
{
    point_ = event->pos();

    repaint();
}

void ImageGridWidget::dropEvent(QDropEvent *event)
{
    event->accept();

    isDragging_ = false;

    const auto list = qobject_cast<QListWidget *>(event->source());
    const auto icon = qvariant_cast<QIcon>(list->currentItem()->data(Qt::DecorationRole));

    // Decide where to put the widget...
    if(layout_->isEmpty()) {
        //grid_.insert(qMakePair(0, 0), icon);
        insertBefore(0, icon);
    }
    else {
        auto y = 0;
        auto idx = 0;
        // count() - 1 skips the QSpacerItem
        for(; idx != layout_->count() - 1; ++idx) {
            const auto layoutSize = layout_->itemAt(idx)->sizeHint();
            const auto spacing = layout_->spacing();
            const auto height = layoutSize.height() + spacing;
            y += height;
            if(point_.y() > y) {
                continue;
            }

            const QPoint imageArea(layoutSize.width(), height);

            // Point relative to the current image
            auto adjusted = point_;
            adjusted -= QPoint(0, idx == 0 ? 0 : y - height);
            const auto side = getSide(adjusted, imageArea);
            if(side == Top) {
                insertBefore(idx, icon);
                break;
            }
            else if(side == Bottom) {
                insertBefore(idx + 1, icon);
                break;
            }
            else if(side == Left || side == Right) {
                // Find x
                QLayoutItem *item = layout_->itemAt(idx);
                auto l = item->layout();
                auto x = 0;
                auto xIdx = 0;
                // count() - 1 skips the QSpacerItem
                // We need to store the value because insertBefore modifies layout
                const auto count = l->count() - 1;
                for(; xIdx < count; ++xIdx) {
                    const auto size = l->itemAt(xIdx)->sizeHint();
                    const auto spacing = l->spacing();
                    const auto width = size.width() + spacing;
                    x += width;
                    if(point_.x() > x) {
                        continue;
                    }

                    if(side == Left) {
                        insertBefore(qMakePair(idx, xIdx), icon);
                        break;
                    }
                    else {
                        insertBefore(qMakePair(idx, xIdx + 1), icon);
                        break;
                    }
                }

                if(point_.x() > x) {
                    insertBefore(qMakePair(idx, xIdx), icon);
                }

                break;
            }
        }

        if(point_.y() > y) {
            // Insert icon after the last icon
            //grid_.insert(qMakePair(idx, 0), icon);
            insertBefore(idx, icon);
        }
    }

    repaint();
}

void ImageGridWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    if(!isDragging_) {
        return;
    }

    QPainter painter(this);
    painter.setPen(Qt::blue);
    if(layout_->isEmpty()) {
        painter.drawLine(0, 0, width(), 0);
    }
    else {
        auto y = 0;
        auto idx = 0;
        // count() - 1 skips the QSpacerItem
        for(; idx != layout_->count() - 1; ++idx) {
            const auto layoutSize = layout_->itemAt(idx)->sizeHint();
            const auto spacing = layout_->spacing();
            const auto height = layoutSize.height() + spacing;
            y += height;
            if(point_.y() > y) {
                continue;
            }

            const QPoint imageArea(layoutSize.width(), height);

            // Point relative to the current image
            auto adjusted = point_;
            adjusted -= QPoint(0, idx == 0 ? 0 : y - height);
            const auto side = getSide(adjusted, imageArea);
            if(side == Top) {
                painter.drawLine(0, y - height, layoutSize.width(), y - height);
                break;
            }
            else if(side == Bottom) {
                painter.drawLine(0, y, layoutSize.width(), y);
                break;
            }
            else if(side == Left) {
                painter.drawLine(0, y - height, 0, y);
                break;
            }
            else if(side == Right) {
                painter.drawLine(layoutSize.width() + spacing, y - height,
                                 layoutSize.width() + spacing, y);
                break;
            }
        }

        if(point_.y() > y) {
            painter.drawLine(0, y, layout_->sizeHint().width(), y);
        }
    }
}
