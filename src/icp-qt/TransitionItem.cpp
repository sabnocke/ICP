#include "TransitionItem.h"
#include "StateItem.h"
#include "EditableTextItem.h"
#include <QPen>
#include <QtMath>
#include <qgraphicsscene.h>
#include <QTimer>


// Constructor initializes the path and arrowhead
TransitionItem::TransitionItem(StateItem* from, StateItem* to, QGraphicsItem* parent)
    : QGraphicsPathItem(parent), fromState(from), toState(to) {
  // Set the pen for the transition path
  setPen(QPen(Qt::white, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

  // Create arrowhead for the transition direction
  arrowHead = new QGraphicsPolygonItem(this);
  arrowHead->setBrush(Qt::white);
  arrowHead->setPen(QPen(Qt::black, 1));

  // Create label for the transition
  label = new QGraphicsTextItem(this);
  label->setDefaultTextColor(Qt::white);
  label->setZValue(2);

  // Initialize position and shape
  updatePosition();
}

// Calculates endpoint offset to ensure arrows start outside of the state's shape
QPointF TransitionItem::adjustEndpoint(QPointF from, QPointF to, qreal radius) {
  QLineF line(from, to);
  if (line.length() == 0.0) return from;
  line.setLength(radius);
  return line.p1() + (line.p2() - line.p1());
}

void TransitionItem::updatePosition() {
  if (!fromState || !toState) return;

  QPainterPath path;

  // Self-loop case
  if (fromState == toState) {
    path = makeSelfLoopPath(fromState->scenePos());
    setPath(path);

    QPointF end = path.pointAtPercent(1.0);
    QPointF beforeEnd = path.pointAtPercent(0.99);
    QLineF direction(beforeEnd, end);
    double angle = std::atan2(direction.dy(), direction.dx());

    const double arrowSize = 15.0;
    QPointF arrowP1 = end - QPointF(std::cos(angle - M_PI / 6) * arrowSize,
                                    std::sin(angle - M_PI / 6) * arrowSize);
    QPointF arrowP2 = end - QPointF(std::cos(angle + M_PI / 6) * arrowSize,
                                    std::sin(angle + M_PI / 6) * arrowSize);

    QPolygonF arrowPolygon;
    arrowPolygon << end << arrowP1 << arrowP2;
    arrowHead->setPolygon(arrowPolygon);
  } else {
    // Normal or curved transition
    QPointF from = fromState->scenePos();
    QPointF to = toState->scenePos();
    QPointF adjustedFrom = adjustEndpoint(from, to);
    QPointF adjustedTo = adjustEndpoint(to, from);

    path.moveTo(adjustedFrom);
    if (isCurved()) {
      QPointF mid = (adjustedFrom + adjustedTo) / 2;
      QLineF base(adjustedFrom, adjustedTo);
      QPointF offset(-base.dy(), base.dx());
      double len = std::hypot(offset.x(), offset.y());
      offset = len > 0.001 ? offset / len * 50.0 : QPointF(0, 0);
      path.quadTo(mid + offset, adjustedTo);
    } else {
      path.lineTo(adjustedTo);
    }

    setPath(path);

    // Arrowhead
    QPointF pointB = path.pointAtPercent(1.0);
    QPointF pointA = path.pointAtPercent(0.99);
    QLineF direction(pointA, pointB);
    double angle = std::atan2(direction.dy(), direction.dx());
    const double arrowSize = 15.0;

    QPointF arrowP1 = pointB - QPointF(std::cos(angle - M_PI / 6) * arrowSize,
                                       std::sin(angle - M_PI / 6) * arrowSize);
    QPointF arrowP2 = pointB - QPointF(std::cos(angle + M_PI / 6) * arrowSize,
                                       std::sin(angle + M_PI / 6) * arrowSize);

    QPolygonF arrowPolygon;
    arrowPolygon << pointB << arrowP1 << arrowP2;
    arrowHead->setPolygon(arrowPolygon);
  }

  // Update label position after path is set
  qreal labelYOffset = -30;  // Default offset

  if (fromState) {
    int similarCount = 0;

    for (TransitionItem* t : fromState->getAttachedTransitions()) {
      if (t == this) break; // Only count previous transitions
      if (t->getFromState() == fromState && t->getToState() == toState) {
        similarCount++;
      }
    }

    labelYOffset -= similarCount * 15; // Shift more for each previous transition
  }

  QPointF labelPos = path.pointAtPercent(0.5) + QPointF(0, labelYOffset);
  if (label) label->setPos(labelPos);
  if (labelEditor) labelEditor->setPos(labelPos);
}


// Creates a self-loop arc path based on the center of a state
QPainterPath TransitionItem::makeSelfLoopPath(QPointF center) {
  qreal radius = 40;
  QPointF pointA = center + QPointF(-radius, 0);
  QPointF pointB = center + QPointF(0, -radius);
  QPointF control1 = center + QPointF(-radius * 3.5, -radius * 1.2);
  QPointF control2 = center + QPointF(-radius * 1.0, -radius * 3.0);

  QPainterPath path(pointA);
  path.cubicTo(control1, control2, pointB);
  return path;
}

// Set whether the transition is rendered as curved
void TransitionItem::setCurved(bool curvedVal) {
  curved = curvedVal;
}

// Check if the transition is currently curved
bool TransitionItem::isCurved() const {
  return curved;
}

// Return source state
StateItem* TransitionItem::getFromState() const {
  return fromState;
}

// Return destination state
StateItem* TransitionItem::getToState() const {
  return toState;
}

// Sets the static visible label text of the transition
void TransitionItem::setLabel(const QString& text) {
  if (label) {
    label->setPlainText(text);
    label->setVisible(true);
  }
}

// Returns the current label text of the transition
QString TransitionItem::getLabel() const {
  return label ? label->toPlainText() : "";
}

// Opens an inline text editor to edit the transition's label
void TransitionItem::startLabelEditing() {
  // Create QLineEdit for user input
  QLineEdit* editor = new QLineEdit;
  const int inputWidth = 100;
  editor->setFixedWidth(inputWidth);
  editor->setStyleSheet(
      "background-color: transparent; "
      "color: white; "
      "border: 1px solid #2196f3;"
      "border-radius: 5px;"
      );

  // Wrap in QGraphicsProxyWidget
  labelEditor = new QGraphicsProxyWidget;
  labelEditor->setWidget(editor);
  labelEditor->setZValue(10);

  // Position the editor above the middle of the transition line
  QPointF midpoint = path().pointAtPercent(0.5);
  labelEditor->setPos(midpoint + QPointF(-inputWidth / 2.0, -30));

  // Add to scene
  if (scene()) {
    scene()->addItem(labelEditor);
  }

  // Give focus to editor
  editor->setFocus();

  // Handle Enter key (editing finished)
  QObject::connect(editor, &QLineEdit::editingFinished, [this, editor]() {
    QString text = editor->text().trimmed();
    if (text.isEmpty()) {
      text = "?";
    }

    // Set the new label text
    if (label) {
      label->setPlainText(text);
      label->setVisible(true);
    }

    // Remove the input field from the scene
    if (labelEditor) {
      scene()->removeItem(labelEditor);
      labelEditor->deleteLater();
      labelEditor = nullptr;
    }
  });

  // Hide the old label while editing
  if (label) {
    label->setVisible(false);
  }
}
