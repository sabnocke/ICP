#include "Draggable.h"
#include "draggablewidget.h"

Draggable::Draggable(QWidget *parent)
    : QWidget{parent}
{
    setMouseTracking(true);
    setAcceptDrops(true);
    layout = new QVBoxLayout(this);
    layout->addStretch(); // Keeps items stacked at the top
}

void Draggable::mousePressEvent(QMouseEvent *event) {
    if(event->button() == Qt::LeftButton)
        lastMousePos = event->pos();
}

void Draggable::mouseMoveEvent(QMouseEvent *event) {
    //TODO maybe invert the if?
    if(event->buttons() == Qt::LeftButton) {

        QPoint delta = event->pos() - lastMousePos;

        if (QScrollArea *scrollArea = findScrollArea()) {
            QScrollBar *hBar = scrollArea->horizontalScrollBar();
            QScrollBar *vBar = scrollArea->verticalScrollBar();

            hBar->setValue(hBar->value() - delta.x());
            vBar->setValue(vBar->value() - delta.y());
        }

        lastMousePos = event->pos();
    }
}

QScrollArea * Draggable::findScrollArea() {
    QWidget *p = parentWidget();
    while(p) {
        if(auto *scroll = qobject_cast<QScrollArea *>(p))
            return scroll;

        p = p->parentWidget();
    }
    return nullptr;
}


void Draggable::dragEnterEvent(QDragEnterEvent *event) {
    if(event->mimeData()->hasText() && event->mimeData()->text() == "draggable")
        event->acceptProposedAction();
}

void Draggable::dropEvent(QDropEvent *event) {
    if(!event->mimeData()->hasText() || event->mimeData()->text() != "draggable") return;

    qDebug() << "Drop at position:" << event->position();
    qDebug() << "Source:" << event->source();

    auto *source = qobject_cast<DraggableWidget *>(event->source());
    if (!source) return;

    source->setParent(this);

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QPoint pos = event->position().toPoint();
#else
    QPoint pos = event->pos();
#endif

    source->move(pos);
    source->show();

    qDebug() << "isEnabled: " << source->isEnabled() << Qt::endl << "isVisible: " << source->isVisible();
    qDebug() << "parent name: " << source->parent()->objectName();

    event->acceptProposedAction();
}
