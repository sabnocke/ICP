#ifndef GRAPHICSSCENE_H
#define GRAPHICSSCENE_H

#include <QGraphicsScene>
#include "StateItem.h"
#include "EditorMode.h"

// Custom scene class for the FSM editor
// Handles all mouse interaction and state/transition creation logic
class GraphicsScene : public QGraphicsScene {
  Q_OBJECT

public:
  explicit GraphicsScene(QObject* parent = nullptr);

  // Change current editor interaction mode (AddState, AddTransition, Move)
  void setMode(EditorMode mode);

  void setInitialState(StateItem* state);

  EditorMode getCurrentMode() const { return currentMode; }

protected:
  // Handles mouse click events depending on current interaction mode
  void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

  // Handles double-clicks for label editing (states and transitions)
  void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;

private:
  EditorMode currentMode = EditorMode::AddState; // Current interaction mode
  StateItem* selectedState = nullptr;            // First state selected for transition creation
  StateItem* initialState = nullptr;

  // Create a new state at a specific scene position
  void createStateAt(const QPointF& pos);

  // Handle the second click to complete a transition creation
  void handleTransitionClick(StateItem* clicked);

  // Reset any temporary selection used when creating transitions
  void resetTransitionSelection();

  // Enables or disables state movability based on the current mode
  void updateStateMovability();
};

#endif // GRAPHICSSCENE_H
