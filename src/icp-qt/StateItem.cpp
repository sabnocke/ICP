#include "StateItem.h"
#include "TransitionItem.h"
#include "GraphicsScene.h"
#include <QBrush>
#include <QPen>
#include <QTimer>
#include <QTextCursor>
#include <QGraphicsScene>
#include <QMenu>
#include <QGraphicsSceneContextMenuEvent>

// Constructor: Initializes the visual circle and label
StateItem::StateItem(const QString& name, QGraphicsItem* parent)
    : QGraphicsEllipseItem(-40, -40, 80, 80, parent), stateName(name) {

  // Set default appearance and interactivity
  setBrush(Qt::white);
  setPen(QPen(Qt::black, 2));
  setFlags(ItemIsMovable | ItemIsSelectable);
  setFlag(QGraphicsItem::ItemSendsGeometryChanges);
  setZValue(1);

  // Create label for the state name
  label = new QGraphicsTextItem(name, this);
  label->setDefaultTextColor(Qt::black);
  label->setPos(-label->boundingRect().width() / 2, -label->boundingRect().height() / 2);
}

// Change the state's internal name
void StateItem::setName(const QString& newName) {
  stateName = newName;
  if (label) {
    label->setPlainText(newName);
    label->setPos(-label->boundingRect().width() / 2, -label->boundingRect().height() / 2);
  }
}

// Return the name of the state
QString StateItem::getName() const {
  return stateName;
}

// Visually highlight this state to show selection
void StateItem::setSelectedVisual(bool selected) {
  QColor customBlue("#2196f3");
  setPen(QPen(selected ? customBlue : Qt::black, 3));
}

// Add a transition to the list of attached transitions
void StateItem::addTransition(TransitionItem* t) {
  attachedTransitions.insert(t);
}

// Enable or disable whether the state can be moved
void StateItem::setMovable(bool movable) {
  setFlag(ItemIsMovable, movable);
}

// Called automatically when the item's position changes
// Notifies connected transitions to reposition themselves
QVariant StateItem::itemChange(GraphicsItemChange change, const QVariant &value) {
  if (change == QGraphicsItem::ItemPositionHasChanged) {
    for (TransitionItem* transition : attachedTransitions)
      transition->updatePosition();
  }
  return QGraphicsEllipseItem::itemChange(change, value);
}

// Allows user to rename the state by placing a QLineEdit on top of the node
void StateItem::startNameEditing() {
  if (!scene()) return;

  const int inputWidth = 100;
  auto* editor = new QLineEdit;
  editor->setFixedWidth(inputWidth);
  editor->setText(stateName);
  editor->setStyleSheet(
      "background-color: transparent; "
      "color: black; "
      "border: 1px solid #2196f3; "
      "border-radius: 5px;"
      );

  // Create proxy and position it centered in the state
  auto* proxy = new QGraphicsProxyWidget;
  proxy->setWidget(editor);
  proxy->setZValue(10);

  QPointF center = this->scenePos();
  proxy->setPos(center - QPointF(inputWidth / 2.0, 10));

  scene()->addItem(proxy);
  editor->setFocus();
  editor->selectAll();

  // Handle Enter key (editing finished)
  QObject::connect(editor, &QLineEdit::editingFinished, [this, editor, proxy]() {
    QString text = editor->text().trimmed();
    if (text.isEmpty()) {
      text = "?";
    }

    setName(text);

    if (scene()) {
      scene()->removeItem(proxy);
    }
    proxy->deleteLater();
  });
}

// Sets this state as the initial state and adds an arrow before
void StateItem::setInitial(bool isInitialFlag) {
  if (isInitialFlag) {
    if (!initialArrow) {
      QPolygonF triangle;
      triangle << QPointF(-40, 0) << QPointF(-60, -20) << QPointF(-60, 20);
      initialArrow = new QGraphicsPolygonItem(triangle, this);
      initialArrow->setBrush(Qt::white);
      initialArrow->setPen(Qt::NoPen);
      initialArrow->setZValue(0);
    }
  } else {
    if (initialArrow) {
      delete initialArrow;
      initialArrow = nullptr;
    }
  }
  isInitial = isInitialFlag;
}

// Returns true if this state is marked as the initial state
bool StateItem::isInitialState() const {
  return isInitial;
}

// Context menu handler for state node
// Allows marking the state as initial, only in Move mode
void StateItem::contextMenuEvent(QGraphicsSceneContextMenuEvent* event) {
  if (auto* scene = dynamic_cast<GraphicsScene*>(this->scene())) {
    if (scene->getCurrentMode() != EditorMode::Move) {
      return; // Do nothing if not in Move mode
    }

    QMenu menu;
    QAction* markInitialAction = menu.addAction("Mark as Initial");

    QAction* selectedAction = menu.exec(event->screenPos());
    if (selectedAction == markInitialAction) {
      scene->setInitialState(this);
    }
  }
}

// Returns a set of all transitions connected to this state
const QSet<TransitionItem*>& StateItem::getAttachedTransitions() const {
  return attachedTransitions;
}
