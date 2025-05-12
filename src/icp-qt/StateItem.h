/**
 * @file StateItem.h
 * @brief Represents a draggable state node with an internal name and a visible label for FSM editing canvas.
 *        Supports highlighting, editing, and connection to transitions. Used in a QGraphicsScene environment.
 * @author Denis Milistenfer <xmilis00@stud.fit.vutbr.cz>
 * @date 11.05.2025
 */

#ifndef STATEITEM_H
#define STATEITEM_H

#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>
#include <QString>
#include <QSet>

class TransitionItem; // Forward declaration

/**
 * @class StateItem
 * @brief Represents a draggable state node with an internal name and a visible label.
 *
 * Used in the FSM editor as a visual and logical representation of a state. Supports renaming,
 * editing, movement, and storing references to connected transitions.
 */
class StateItem : public QGraphicsEllipseItem {
public:
/**
   * @brief Constructs a StateItem with a given name.
   * @param name Logical name of the state.
   * @param parent Optional parent graphics item.
   */
  explicit StateItem(const QString& name, QGraphicsItem* parent = nullptr);

  /**
   * @brief Sets a new name for the state.
   * @param newName New name to assign.
   */
  void setName(const QString& newName);

  /**
   * @brief Returns the current name of the state.
   * @return Name of the state.
   */
  QString getName() const;

  /**
   * @brief Highlights or unhighlights the state visually.
   * @param selected True to highlight, false to unhighlight.
   */
  void setSelectedVisual(bool selected);

  /**
   * @brief Adds a transition connected to this state.
   * @param t Pointer to the transition to add.
   */
  void addTransition(TransitionItem* t);

  /**
   * @brief Removes a transition from this state's connection list.
   * @param t Pointer to the transition to remove.
   */
  void removeTransition(TransitionItem* t);

  /**
   * @brief Sets whether the state is movable by dragging.
   * @param movable True to allow movement, false to lock position.
   */
  void setMovable(bool movable);

  /**
   * @brief Highlights or removes highlight for currently active state.
   * @param highlight True to highlight, false to remove.
   */
  void setHighlighted(bool highlight);

  /**
   * @brief Initiates "inplace" editing of the state name.
   */
  void startNameEditing();

  /**
   * @brief Marks or unmarks the state as the initial state.
   * @param isInitial True to mark as initial, false otherwise.
   */
  void setInitial(bool isInitial);

  /**
   * @brief Checks if this state is marked as initial.
   * @return True if marked as initial, false otherwise.
   */
  bool isInitialState() const;

  /**
   * @brief Returns a set of transitions connected to this state.
   * @return Set of TransitionItem pointers.
   */
  const QSet<TransitionItem*>& getAttachedTransitions() const;

protected:
  /**
   * @brief Handles notifications when the state item is moved.
   *        Updates connected transitions accordingly.
   * @param change Type of change.
   * @param value Associated value.
   * @return Possibly modified value.
   */
  QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

  /**
   * @brief Handles right-click context menu events for setting initial state.
   * @param event The context menu event.
   */
  void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

private:
  QGraphicsTextItem* label;                     ///< Visual label for the state
  QString stateName;                            ///< Name of the state
  QSet<TransitionItem*> attachedTransitions;    ///< Transitions connected to this state
  bool isInitial = false;                       ///< True if this state is the initial one
  QGraphicsPolygonItem* initialArrow = nullptr; ///< Visual arrow marker for initial state
};

#endif // STATEITEM_H
