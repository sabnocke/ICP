#ifndef DRAGGABLEWIDGET_H
#define DRAGGABLEWIDGET_H

#include <QWidget>
#include <QMouseEvent>
#include <QDrag>
#include <QMimeData>

class DraggableWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DraggableWidget(QWidget *parent = nullptr);
protected:
    void mousePressEvent(QMouseEvent *event) override;

signals:
};

#endif // DRAGGABLEWIDGET_H
