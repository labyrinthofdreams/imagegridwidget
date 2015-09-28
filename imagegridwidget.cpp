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

#include <QDebug>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QPaintEvent>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QListWidget>
#include <QPainter>
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

ImageGridWidget::ImageGridWidget(QWidget *parent) :
    QWidget(parent),
    point_(),
    layout_(new QVBoxLayout),
    isDragging_(false),
    grid_()
{
    layout_->setSpacing(10);
    layout_->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding));

    setAcceptDrops(true);
    setLayout(layout_);
    setMouseTracking(true);
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

    resizeWidgets();
}

void ImageGridWidget::resizeWidgets()
{
    if(grid_.isEmpty()) {
        return;
    }

    const QPixmap pmIcon = grid_.first().pixmap(grid_.first().availableSizes().first());
    const auto minWidth = pmIcon.width();
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
                qDebug() << size << minWidth << pixelsTaken << extraPixels;
                size.setWidth(size.width() + extraPixels);
            }

            const QPixmap pm = grid_.value(qMakePair(row, idx)).pixmap(size).scaled(size);
            qobject_cast<QLabel *>(lo->itemAt(idx)->widget())->setPixmap(pm);
        }
    }
}

void ImageGridWidget::setSpacing(const int spacing)
{
    layout_->setSpacing(spacing);

    resizeWidgets();
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
        insertBefore(qMakePair(idx, xIdx + 1), icon);
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
