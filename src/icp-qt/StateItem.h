#ifndef STATEITEM_H
#define STATEITEM_H

#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>
#include <QString>
#include <QSet>

class TransitionItem; // Forward declaration

// Represents a draggable state node with an internal name and a visible label
class StateItem : public QGraphicsEllipseItem {
public:
  explicit StateItem(const QString& name, QGraphicsItem* parent = nullptr);

  void setName(const QString& newName); // Change state name
  QString getName() const;              // Return state name

  void setSelectedVisual(bool selected); // Visually highlight/unhighlight
  void addTransition(TransitionItem* t); // Store transition references
  void setMovable(bool movable);         // Enable/disable dragging
  void setHighlighted(bool highlight);   // Highlight current active state

  void startNameEditing();          // Start "inplace" editing of the state name
  void setInitial(bool isInitial);  // Mark/unmark this state as initial state
  bool isInitialState() const;      // Check if this state is marked as initial
  const QSet<TransitionItem*>& getAttachedTransitions() const;  // Return all connected transitions to this state

protected:
  QVariant itemChange(GraphicsItemChange change, const QVariant& value) override; // Notify transitions on move
  void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;          // Handle right-click menu for "initial state" marking

private:
  QGraphicsTextItem* label;                     // Visual label for the state
  QString stateName;                            // Logical name
  QSet<TransitionItem*> attachedTransitions;    // Connected transitions
  bool isInitial = false;                       // Bool if this state is initial state
  QGraphicsPolygonItem* initialArrow = nullptr; // Arrow marker indicating initial state
};

#endif // STATEITEM_H
