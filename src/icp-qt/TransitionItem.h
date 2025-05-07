#ifndef TRANSITIONITEM_H
#define TRANSITIONITEM_H

#include <QGraphicsPathItem>
#include <QGraphicsPolygonItem>
#include <QGraphicsTextItem>
#include <QGraphicsProxyWidget>
#include <QLineEdit>

class StateItem;

// Represents a visual transition between two state items
// Can be either a straight or curved line or self-loops
// Draws an arrowhead at the end to indicate direction
class TransitionItem : public QGraphicsPathItem {
public:
  // Constructor: takes source and destination StateItem pointers
  TransitionItem(StateItem* from, StateItem* to, QGraphicsItem* parent = nullptr);

  void updatePosition();       // Recalculate path when states move
  void setCurved(bool curved); // Enable curved style for transitions
  bool isCurved() const;       // Returns true if curved mode is on

  void setLabel(const QString& text); // Set static label text
  QString getLabel() const;           // Get current label
  void startLabelEditing();           // Show inline QLineEdit for editing

  QGraphicsTextItem* getLabelItem() const { return label; }  // Returns the QGraphicsTextItem used as label

  // Accessors to source and destination state pointers
  StateItem* getFromState() const;
  StateItem* getToState() const;

private:
  // Adjusts the endpoint inward from the node center to avoid overlap
  QPointF adjustEndpoint(QPointF from, QPointF to, qreal radius = 40.0);
  // Constructs a cubic Bézier path for self-loop transitions
  QPainterPath makeSelfLoopPath(QPointF center);

  StateItem* fromState;               // Source state node
  StateItem* toState;                 // Target state node
  QGraphicsPolygonItem* arrowHead;    // Arrowhead graphics object
  QGraphicsTextItem* label = nullptr;          // Permanent visible label
  QGraphicsProxyWidget* labelEditor = nullptr; // Input field for editing
  bool curved = false;                // Whether this is a curved reverse transition
};

#endif // TRANSITIONITEM_H
