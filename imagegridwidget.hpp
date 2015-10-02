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

#ifndef IMAGEGRIDWIDGET_HPP
#define IMAGEGRIDWIDGET_HPP

#include <QColor>
#include <QIcon>
#include <QMap>
#include <QPair>
#include <QPen>
#include <QPoint>
#include <QSize>
#include <QWidget>

class QDragEnterEvent;
class QDragLeaveEvent;
class QDragMoveEvent;
class QDropEvent;
class QMouseEvent;
class QPaintEvent;
class QVBoxLayout;

class ImageGridWidget : public QWidget
{
    Q_OBJECT

    //! Keeps track of the cursor position when drag 'n dropping
    QPoint point_;

    //! Current layout
    QVBoxLayout *layout_;

    //! If dragging
    bool isDragging_;

    //! Represents a position in the grid
    using Index = QPair<int, int>;

    //! Grid will be used to calculate the row sizes
    QMap<Index, QIcon> grid_;

    //! Layout width
    int width_;

    //! Pen for drawing helper lines
    QPen pen_;

    //! Background color
    QColor backgroundColor_;

    /**
     * @brief Insert icon as a new row before row
     * @param row Row to insert before
     * @param icon Icon to add
     */
    void insertBefore(int row, const QIcon &icon);

    /**
     * @brief Insert icon into an existing row before index
     * @param index Index to insert before
     * @param icon Icon to add
     */
    void insertBefore(Index index, const QIcon &icon);

    /**
     * @brief Calculate row sizes
     * @return New row sizes
     */
    QMap<int, QSize> calculateRowSizes() const;

    /**
     * @brief Get vertical data (height, index) for current cursor position
     * @return Vertical data
     */
    QPair<int, int> getVertical() const;

    /**
     * @brief Get horizontal data (width, index) for current cursor position
     * @param yIndex Vertical index to get data for
     * @return Horizontal data
     */
    QPair<int, int> getHorizontal(int yIndex) const;

    /**
     * @brief Resize widgets
     */
    void resizeWidgets();

    /**
     * @brief Remove icon at row row
     * @param row Row to remove
     */
    void removeAt(int row);

    /**
     * @brief Remove icon at index index
     * @param index Index to remove
     */
    void removeAt(Index index);

public:
    /**
     * @brief Constructor
     *
     * Sets default spacing to 0
     * Sets default width to 0
     * @param parent Owner of the widget
     */
    explicit ImageGridWidget(QWidget *parent = 0);

    /**
     * @brief Constructor
     *
     * Sets default width to 0
     * @param spacing Space between images in pixels
     * @param parent Owner of the widget
     */
    explicit ImageGridWidget(int spacing = 0, QWidget *parent = 0);

    /**
     * @brief Get number of rows
     * @return Number of rows
     */
    int getRowCount() const;

    /**
     * @brief Get number of columns for a row
     * @param row Row to get columns for
     * @return Number of columns
     */
    int getColumnCount(int row) const;

    /**
     * @brief Get icon at index
     * @param row Row
     * @param column Column
     * @return Icon or null icon if index is invalid
     */
    QIcon iconAt(int row, int column) const;

    /**
     * @brief iconAt Get icon at index
     * @param index Index
     * @return Icon or null icon if index is invalid
     */
    QIcon iconAt(Index index) const;

signals:

public slots:
    /**
     * @brief Set space between images in pixels
     * @param spacing Space between images
     */
    void setSpacing(int spacing);

    /**
     * @brief Set layout width
     *
     * A value of zero will use image width as width
     * @param width Width in pixels
     */
    void setWidth(int width);

    /**
     * @brief Set pen for styles
     * @param pen New pen
     */
    void setPen(const QPen &pen);

    /**
     * @brief Set background color
     * @param color New color
     */
    void setBackgroundColor(const QColor &color);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;

    void dragLeaveEvent(QDragLeaveEvent *event) override;

    void dragMoveEvent(QDragMoveEvent *event) override;

    void dropEvent(QDropEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;

    void paintEvent(QPaintEvent *event) override;
};

#endif // IMAGEGRIDWIDGET_HPP
