/******************************************************************************
The MIT License (MIT)

Copyright (c) 2015 https://github.com/labyrinthofdreams

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
******************************************************************************/

#include <QBrush>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLayoutItem>
#include <QListWidget>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QPen>
#include <QPixmap>
#include <QPoint>
#include <QSize>
#include <QSpacerItem>
#include <QVBoxLayout>
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

// TODO: Implement undo
// TODO: Implement changing spacing
// TODO: Drag n' drop existing images in the layout
// so that we don't have to undo the whole thing to reset
// TODO: Drawing drop helper lines only work with 10px spacing
// so we need to make it work with more or less pixels as well

ImageGridWidget::ImageGridWidget(QWidget *parent) :
    ImageGridWidget(0, parent)
{

}

ImageGridWidget::ImageGridWidget(const int spacing, QWidget *parent) :
    QWidget(parent),
    point_(),
    layout_(new QVBoxLayout),
    isDragging_(false),
    grid_(),
    width_(0),
    pen_(QPen(QBrush(Qt::blue, Qt::SolidPattern), 1)),
    backgroundColor_(Qt::transparent)
{
    layout_->setSpacing(spacing);
    layout_->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding));

    setAcceptDrops(true);
    setLayout(layout_);
    setMouseTracking(true);
}

int ImageGridWidget::getRowCount() const
{
    if(grid_.isEmpty()) {
        return 0;
    }

    return grid_.lastKey().first + 1;
}

int ImageGridWidget::getColumnCount(const int row) const
{
    if(row < 0) {
        qWarning("ImageGridWidget::getColumnCount: Negative row: %d", row);
        return 0;
    }

    const QList<Index> keys = grid_.keys();
    return std::count_if(keys.cbegin(), keys.cend(),
                         [&row](const Index &key){ return key.first == row; });
}

QIcon ImageGridWidget::iconAt(const int row, const int column) const
{
    return iconAt(qMakePair(row, column));
}

QIcon ImageGridWidget::iconAt(const ImageGridWidget::Index index) const
{
    if(!grid_.contains(index)) {
        return {};
    }

    return grid_.value(index);
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
    const auto minWidth = width_ > 0 ? width_ : pmIcon.width();

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
        qWarning("ImageGridWidget::insertBefore: Negative row: %d", row);
        return;
    }

    if(icon.isNull()) {
        qWarning("ImageGridWidget::insertBefore: Null icon");
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

    resizeWidgets();
}

void ImageGridWidget::insertBefore(const Index index, const QIcon &icon)
{
    if(index.first < 0 || index.second < 0) {
        qWarning("ImageGridWidget::insertBefore: Negative index: %dx%d",
                 index.first, index.second);
        return;
    }

    if(icon.isNull()) {
        qWarning("ImageGridWidget::insertBefore: Null icon");
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

    resizeWidgets();
}

void ImageGridWidget::resizeWidgets()
{
    if(grid_.isEmpty()) {
        return;
    }

    const auto minWidth = width_ > 0 ? width_ :
        grid_.first().pixmap(grid_.first().availableSizes().first()).width();

    const auto spacing = layout_->spacing();
    const QMap<int, QSize> sizes = calculateRowSizes();
    for(auto it = sizes.cbegin(); it != sizes.cend(); ++it) {
        const auto row = it.key();
        auto lo = qobject_cast<QHBoxLayout *>(layout_->itemAt(row)->layout());
        // count() - 1 skips the spacer item
        const auto count = lo->count() - 1;
        for(auto idx = 0; idx < count; ++idx) {
            QSize size = it.value();
            if(idx + 1 == count) {
                // If last widget, resize width to fill the minimum width
                const auto pixelsTaken = ((idx + 1) * size.width()) + (idx * spacing);
                const auto extraPixels = minWidth - pixelsTaken;
                size.setWidth(size.width() + extraPixels);
            }

            const QPixmap pm = grid_.value(qMakePair(row, idx)).pixmap(size).scaled(size);
            qobject_cast<QLabel *>(lo->itemAt(idx)->widget())->setPixmap(pm);
        }
    }
}

void ImageGridWidget::removeAt(const ImageGridWidget::Index index)
{
    const auto cols = getColumnCount(index.first);
    grid_.remove(index);
    for(auto c = index.second + 1; c < cols; ++c) {
        const QIcon icon = grid_.take(qMakePair(index.first, c));
        grid_.insert(qMakePair(index.first, c - 1), icon);
    }
}

void ImageGridWidget::removeAt(const int row)
{
    // Remove everything from row row
    const auto cols = getColumnCount(row);
    for(auto idx = 0; idx < cols; ++idx) {
        grid_.remove(qMakePair(row, idx));
    }

    // Then move the other rows down by one
    const auto rows = getRowCount();
    for(auto r = row; r < rows; ++r) {
        const auto cols = getColumnCount(r);
        for(auto c = 0; c < cols; ++c) {
            const QIcon icon = grid_.take(qMakePair(r, c));
            grid_.insert(qMakePair(r - 1, c), icon);
        }
    }
}

void ImageGridWidget::setSpacing(const int spacing)
{
    if(spacing < 0) {
        qWarning("ImageGridWidget::setSpacing: Negative spacing: %d", spacing);
        return;
    }

    layout_->setSpacing(spacing);

    resizeWidgets();
}

void ImageGridWidget::setWidth(const int width)
{
    if(width < 0) {
        qWarning("ImageGridWidget::setWidth: Negative width: %d", width);
        return;
    }

    width_ = width;

    resizeWidgets();
}

void ImageGridWidget::setPen(const QPen &pen)
{
    pen_ = pen;
}

void ImageGridWidget::setBackgroundColor(const QColor &color)
{
    backgroundColor_ = color;

    repaint();
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
        insertBefore(0, icon);
        return;
    }

    const auto spacing = layout_->spacing();

    const QPair<int, int> v = getVertical();
    auto y = v.first;
    const auto idx = v.second;
    if(point_.y() > y) {
        insertBefore(idx, icon);
        repaint();
        return;
    }

    const auto layoutSize = layout_->itemAt(idx)->sizeHint();
    const auto height = layoutSize.height() + spacing;
    // Point relative to the current image
    auto adjusted = point_;
    adjusted -= QPoint(0, idx == 0 ? 0 : y - height);

    const QPair<int, int> h = getHorizontal(idx);
    auto x = h.first;
    const auto xIdx = h.second;
    if(point_.x() > x) {
        insertBefore(qMakePair(idx, xIdx), icon);
        repaint();
        return;
    }

    // Get image size for current image
    const QSize imageSize = layout_->itemAt(idx)->layout()->itemAt(xIdx)->sizeHint();
    const auto width = imageSize.width() + spacing;

    adjusted -= QPoint(xIdx == 0 ? 0 : x - width, 0);

    const auto side = getSide(adjusted, QPoint(imageSize.width(), imageSize.height()));
    if(side == Top) {
        insertBefore(idx, icon);
    }
    else if(side == Bottom) {
        insertBefore(idx + 1, icon);
    }
    else if(side == Left) {
        insertBefore(qMakePair(idx, xIdx), icon);
    }
    else if(side == Right) {
        insertBefore(qMakePair(idx, xIdx + 1), icon);
    }

    repaint();
}

void ImageGridWidget::mousePressEvent(QMouseEvent *event)
{
    const auto rowCount = layout_->count() - 1;
    if(rowCount == 0) {
        return;
    }

    const QPoint pos = event->pos();
    QLayoutItem *li = nullptr;
    auto height = 0;
    auto yIdx = 0;
    for(; yIdx < rowCount; ++yIdx) {
        li = layout_->itemAt(yIdx);
        height += li->sizeHint().height() + layout_->spacing();
        if(pos.y() <= height) {
            break;
        }
    }

    if(pos.y() > height) {
        // We don't want to remove the last widget
        // if the mouse press happens below it
        return;
    }

    auto width = 0;
    QLayoutItem *li2 = nullptr;
    QHBoxLayout *lo = qobject_cast<QHBoxLayout*>(li->layout());
    const auto colCount = lo->count() - 1;
    auto xIdx = 0;
    for(; xIdx < colCount; ++xIdx) {
        li2 = lo->itemAt(xIdx);
        width += li2->sizeHint().width() + lo->spacing();
        if(pos.x() <= width) {
            break;
        }
    }

    if(pos.x() > width) {
        return;
    }

    if(lo && colCount == 1) {
        // Remove widget and spacer item
        for(auto idx = 0; idx < lo->count(); ++idx) {
            lo->itemAt(idx)->widget()->deleteLater();
            lo->removeItem(lo->itemAt(idx));
        }

        // Then remove the layout
        layout_->removeItem(lo);

        removeAt(yIdx);
    }
    else if(lo) {
        li2->widget()->deleteLater();
        lo->removeItem(li2);

        removeAt(qMakePair(yIdx, xIdx));
    }

    resizeWidgets();
}

void ImageGridWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    QPainter painter(this);
    if(!layout_->isEmpty() && layout_->spacing() > 0
            && backgroundColor_ != Qt::transparent) {
        painter.setBrush(QBrush(backgroundColor_));
        painter.drawRect(QRect(0, 0, width(), height()));
    }

    if(!isDragging_) {
        return;
    }

    painter.setPen(pen_);
    if(layout_->isEmpty()) {
        painter.drawLine(0, 0, width(), 0);
        return;
    }

    const auto spacing = layout_->spacing();
    const auto halfSpacing = spacing / 2 - 2;

    const QPair<int, int> v = getVertical();
    auto y = v.first;
    const auto idx = v.second;
    if(point_.y() > y) {
        y--;
        painter.drawLine(spacing - 1, y + halfSpacing,
                         layout_->sizeHint().width() - spacing, y + halfSpacing);
        return;
    }

    const auto layoutSize = layout_->itemAt(idx)->sizeHint();
    const auto height = layoutSize.height() + spacing;
    // Point relative to the current image
    auto adjusted = point_;
    adjusted -= QPoint(0, idx == 0 ? 0 : y - height);

    const QPair<int, int> h = getHorizontal(idx);
    auto x = h.first;
    const auto xIdx = h.second;

    // Get image size for current image
    const QSize imageSize = layout_->itemAt(idx)->layout()->itemAt(xIdx)->sizeHint();
    const auto width = imageSize.width() + spacing;

    adjusted -= QPoint(xIdx == 0 ? 0 : x - width, 0);

    x--;
    y--;

    const auto side = getSide(adjusted, QPoint(imageSize.width(), imageSize.height()));
    if(side == Top) {
        painter.drawLine(x - width + spacing, y - height + halfSpacing, x, y - height + halfSpacing);
    }
    else if(side == Bottom) {
        painter.drawLine(x - width + spacing, y + halfSpacing, x, y + halfSpacing);
    }
    else if(side == Left) {
        painter.drawLine(x - width + halfSpacing, y - height + spacing, x - width + halfSpacing, y);
    }
    else if(side == Right) {
        painter.drawLine(x + halfSpacing, y - height + spacing, x + halfSpacing, y);
    }
}

QPair<int, int> ImageGridWidget::getVertical() const
{
    auto y = 0;
    auto idx = 0;
    const auto spacing = layout_->spacing();
    // count() - 1 skips the QSpacerItem
    for(; idx != layout_->count() - 1; ++idx) {
        const auto layoutSize = layout_->itemAt(idx)->sizeHint();
        const auto height = layoutSize.height() + spacing;
        y += height;
        if(point_.y() > y) {
            continue;
        }
        else {
            break;
        }
    }

    return qMakePair(y, idx);
}

QPair<int, int> ImageGridWidget::getHorizontal(const int yIndex) const
{
    if(yIndex < 0) {
        qWarning("ImageGridWidget::getHorizontal: Negative index: %d", yIndex);
        return {};
    }

    const QLayout *layout = layout_->itemAt(yIndex)->layout();
    const auto spacing = layout_->spacing();
    auto x = 0;
    auto xIdx = 0;
    auto width = 0;
    for(; xIdx < layout->count() - 1; ++xIdx) {
        const QSize imageArea = layout->itemAt(xIdx)->sizeHint();
        width = imageArea.width() + spacing;
        x += width;
        if(point_.x() > x) {
            continue;
        }
        else {
            break;
        }
    }

    return qMakePair(x, xIdx);
}
