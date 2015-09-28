#ifndef IMAGEGRIDWIDGET_HPP
#define IMAGEGRIDWIDGET_HPP

#include <QIcon>
#include <QList>
#include <QMap>
#include <QPair>
#include <QPoint>
#include <QSize>
#include <QWidget>

class QDragEnterEvent;
class QDragLeaveEvent;
class QDragMoveEvent;
class QDropEvent;
class QLayoutItem;
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

    using Index = QPair<int, int>;

    //! Grid will be used to calculate the row sizes
    QMap<Index, QIcon> grid_;

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

public:
    explicit ImageGridWidget(QWidget *parent = 0);

signals:

public slots:

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;

    void dragLeaveEvent(QDragLeaveEvent *event) override;

    void dragMoveEvent(QDragMoveEvent *event) override;

    void dropEvent(QDropEvent *event) override;

    void paintEvent(QPaintEvent *event) override;
};

#endif // IMAGEGRIDWIDGET_HPP
