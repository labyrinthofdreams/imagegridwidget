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
template <class I, class U> class QPair;

class ImageGridWidget : public QWidget
{
    Q_OBJECT

    QPoint point_;

    QVBoxLayout *layout_;

    bool isDragging_;

    using Index = QPair<int, int>;
    QMap<Index, QIcon> grid_;

    QList<QLayoutItem *> widgets_;

    void buildLayout();

    void insertBefore(int row, const QIcon &icon);

    void insertBefore(Index index, const QIcon &icon);

    int getRowCount() const;
    int getColumnsForRow(int row) const;

    QMap<int, QSize> calculateRowSizes() const;

public:
    explicit ImageGridWidget(QWidget *parent = 0);

    void clear();

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
