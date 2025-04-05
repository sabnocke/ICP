#ifndef DRAGGABLE_H
#define DRAGGABLE_H

#include <QWidget>
#include <QScrollArea>
#include <QScrollBar>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QMimeData>

class Draggable : public QWidget
{
    Q_OBJECT
public:
    explicit Draggable(QWidget *parent = nullptr);
    QVBoxLayout *layout;
protected:
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    QPoint lastMousePos;
    QScrollArea *findScrollArea();

signals:
};

#endif // DRAGGABLE_H
