#include "draggablewidget.h"

DraggableWidget::DraggableWidget(QWidget *parent)
    : QWidget{parent}
{
    setAcceptDrops(false);
    this->setVisible(true);
}


void DraggableWidget::mousePressEvent(QMouseEvent *event) {
    if (event->buttons() != Qt::LeftButton)
        return;

    QDrag *drag = new QDrag(this);
    QMimeData *mime = new QMimeData;

    mime->setText("draggable");
    drag->setMimeData(mime);

    drag->setPixmap(grab());

    drag->exec(Qt::MoveAction);
}
