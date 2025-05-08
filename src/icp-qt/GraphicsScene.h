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
  // Marks the given state as the initial state, replacing any existing one
  void setInitialState(StateItem* state);
  // Returns the current editing mode
  EditorMode getCurrentMode() const { return currentMode; }
  // Returns the current initial state (or nullptr if none)
  StateItem* getInitialState() const { return initialState; }
  // Notifies the scene that a state's name has been changed
  void notifyStateRenamed(const QString& oldName, const QString& newName);
  // Searches and returns a pointer to the state with the given name (or nullptr if not found)
  StateItem* findStateByName(const QString& name);

protected:
  // Handles mouse click events depending on current interaction mode
  void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

  // Handles double-clicks for label editing (states and transitions)
  void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;

private:
  EditorMode currentMode = EditorMode::AddState; // Current interaction mode
  StateItem* selectedState = nullptr;            // First state selected for transition creation
  StateItem* initialState = nullptr;            // The current initial state

  // Create a new state at a specific scene position
  void createStateAt(const QPointF& pos);

  // Handle the second click to complete a transition creation
  void handleTransitionClick(StateItem* clicked);

  // Reset any temporary selection used when creating transitions
  void resetTransitionSelection();

  // Enables or disables state movability based on the current mode
  void updateStateMovability();

signals:
  // Emitted when a new state is added (used to update action table)
  void stateAdded(const QString& name);
  // Emitted when a state is renamed (used to rename row in action table)
  void stateRenamed(const QString& oldName, const QString& newName);
};

#endif // GRAPHICSSCENE_H
