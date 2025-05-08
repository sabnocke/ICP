#include "GraphicsScene.h"
#include "TransitionItem.h"
#include <QGraphicsSceneMouseEvent>
#include <QInputDialog>
#include <QEvent>
#include <QGraphicsSceneMouseEvent>

// Constructor for the custom FSM editor scene
GraphicsScene::GraphicsScene(QObject* parent)
    : QGraphicsScene(parent) {}

// Sets the current editing mode (AddState, AddTransition, Move)
// Also updates whether states are movable based on mode
void GraphicsScene::setMode(EditorMode mode) {
  currentMode = mode;
  updateStateMovability();
}

// Main mouse event handler
// Depending on current mode, creates new state or starts/completes a transition
void GraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent* event) {
  switch (currentMode) {
    case EditorMode::AddState:
      createStateAt(event->scenePos());
      break;

    case EditorMode::AddTransition: {
      auto* item = itemAt(event->scenePos(), QTransform());
      auto* state = qgraphicsitem_cast<StateItem*>(item);
      if (state) handleTransitionClick(state);
      break;
    }

    default:
      break;
  }
  QGraphicsScene::mousePressEvent(event); // Call base implementation
}

// Creates and adds a new state to the scene
// In default states are named State1, State2, etc.
// After creation, emits stateAdded() so actions table can be updated
void GraphicsScene::createStateAt(const QPointF& pos) {
  static int counter = 1; // Counter for automatic naming
  auto* state = new StateItem("State" + QString::number(counter++));
  state->setPos(pos);     // Position where user clicked
  addItem(state);         // Add to the scene

  emit stateAdded(state->getName());
}

// Handles click on a second state while in AddTransition mode
// Creates a transition between two states
void GraphicsScene::handleTransitionClick(StateItem* clicked) {
  if (!selectedState) {
    selectedState = clicked;
    selectedState->setSelectedVisual(true); // Highlight the first selected state
    return;
  }

  // Create new transition from selectedState to clicked
  auto* transition = new TransitionItem(selectedState, clicked);

  // Check if a reverse transition already exists, and if so, curve both lines
  for (QGraphicsItem* item : items()) {
    if (auto* existing = dynamic_cast<TransitionItem*>(item)) {
      if (existing->getFromState() == clicked && existing->getToState() == selectedState && clicked != selectedState) {
        transition->setCurved(true);
        existing->setCurved(true);
        existing->updatePosition();
      }
    }
  }

  // Link transitions to both states so they can update when states move
  selectedState->addTransition(transition);
  clicked->addTransition(transition);

  addItem(transition);       // Add to scene
  transition->updatePosition(); // Position line properly

  // Ask user for label
  transition->startLabelEditing();

  resetTransitionSelection(); // Reset for next transition
}

// Clears selection of a previously selected state after creating a transition
void GraphicsScene::resetTransitionSelection() {
  if (selectedState) selectedState->setSelectedVisual(false);
  selectedState = nullptr;
}

// Handles double-clicks for "inplace" editing
// Only enabled in Move mode
// Supports editing transition labels and state names
void GraphicsScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) {
  if (currentMode != EditorMode::Move) {
    // Ignore double-clicks in other modes (AddState or AddTransition)
    QGraphicsScene::mouseDoubleClickEvent(event);
    return;
  }

  QGraphicsItem* clickedItem = itemAt(event->scenePos(), QTransform());

  // Transition label editing
  if (auto* label = qgraphicsitem_cast<QGraphicsTextItem*>(clickedItem)) {
    if (auto* transition = qgraphicsitem_cast<TransitionItem*>(label->parentItem())) {
      transition->startLabelEditing();
      return;
    }
  }

  // State name editing
  if (auto* state = qgraphicsitem_cast<StateItem*>(clickedItem)) {
    state->startNameEditing();
    return;
  }

  QGraphicsScene::mouseDoubleClickEvent(event); // fallback
}

// Updates movability of states based on current mode
// States are movable only in Move mode
void GraphicsScene::updateStateMovability() {
  for (QGraphicsItem* item : items()) {
    if (auto* state = dynamic_cast<StateItem*>(item)) {
      state->setMovable(currentMode == EditorMode::Move);
    }
  }
}

// Marks the given state as the initial state
// Ensures only one state can be initial at a time
void GraphicsScene::setInitialState(StateItem* state) {
  if (initialState) {
    initialState->setInitial(false);  // Unmark previous
  }
  state->setInitial(true);
  initialState = state;
}

// Notifies that a state has been renamed
// This allows action table to update the corresponding row to the new name
void GraphicsScene::notifyStateRenamed(const QString& oldName, const QString& newName) {
  emit stateRenamed(oldName, newName);
}
