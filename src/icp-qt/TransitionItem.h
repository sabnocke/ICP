/**
 * @file TransitionItem.h
 * @brief Represents a visual transition between two state items in the FSM editor. Supports straight, curved, and self-loop paths with direction arrows and editable labels.
 * @author Denis Milistenfer <xmilis00@stud.fit.vutbr.cz>
 * @date 11.05.2025
 */

#ifndef TRANSITIONITEM_H
#define TRANSITIONITEM_H

#include <QGraphicsPathItem>
#include <QGraphicsPolygonItem>
#include <QGraphicsTextItem>
#include <QGraphicsProxyWidget>
#include <QLineEdit>

class StateItem;

/**
 * @class TransitionItem
 * @brief Represents a visual transition between two state items.
 *
 * Can render straight or curved lines, self-loops, and an arrowhead to indicate direction.
 * Supports label editing directly within the scene.
 */
class TransitionItem : public QGraphicsPathItem {
public:
  /**
   * @brief Constructs a transition between two state items.
   * @param from Pointer to the source state.
   * @param to Pointer to the destination state.
   * @param parent Optional parent graphics item.
   */
  TransitionItem(StateItem* from, StateItem* to, QGraphicsItem* parent = nullptr);

  /**
   * @brief Recalculates the transition path based on current state positions.
   */
  void updatePosition();

  /**
   * @brief Enables or disables curved style for reverse transitions.
   * @param curved True to enable curved style.
   */
  void setCurved(bool curved);

  /**
   * @brief Checks if the transition is currently rendered as curved.
   * @return True if curved mode is enabled.
   */
  bool isCurved() const;

  /**
   * @brief Sets the label text displayed on the transition.
   * @param text The text to display.
   */
  void setLabel(const QString& text);

  /**
   * @brief Returns the current label text.
   * @return The label string.
   */
  QString getLabel() const;

  /**
   * @brief Begins inline label editing using a QLineEdit widget.
   */
  void startLabelEditing();

  /**
   * @brief Returns the QGraphicsTextItem used as the label.
   * @return Pointer to the label item.
   */
  QGraphicsTextItem* getLabelItem() const { return label; }

  /**
   * @brief Returns the source (from) state.
   * @return Pointer to the source StateItem.
   */
  StateItem* getFromState() const;

  /**
   * @brief Returns the destination (to) state.
   * @return Pointer to the destination StateItem.
   */
  StateItem* getToState() const;

private:
  /**
   * @brief Computes a visually adjusted endpoint to prevent overlap with state nodes.
   * @param from Start point.
   * @param to End point.
   * @param radius Adjustment radius (default 40).
   * @return Adjusted end position.
   */
  QPointF adjustEndpoint(QPointF from, QPointF to, qreal radius = 40.0);

  /**
   * @brief Creates a cubic Bézier path for rendering a self-loop transition.
   * @param center Center position of the associated state.
   * @return Path representing the self-loop.
   */
  QPainterPath makeSelfLoopPath(QPointF center);

  StateItem* fromState;                         ///< Source state node
  StateItem* toState;                           ///< Destination state node
  QGraphicsPolygonItem* arrowHead;              ///< Arrowhead graphics object
  QGraphicsTextItem* label = nullptr;           ///< Static label for the transition
  QGraphicsProxyWidget* labelEditor = nullptr;  ///< Inline QLineEdit editor for label
  bool curved = false;                          ///< Whether the transition is curved (e.g., reverse)
};

#endif // TRANSITIONITEM_H
