/**
 * @file GraphicsScene.cpp
 * @brief Implements the interactive scene logic for the FSM editor, including mouse handling for creating, editing, and deleting states and transitions in a QGraphicsScene environment. Handles visual logic for state interactions and user events from the canvas area of the application.
 * @author Denis Milistenfer <xmilis00@stud.fit.vutbr.cz>
 * @date 11.05.2025
 */

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
// Depending on current mode, creates new state or starts/completes a transition or deletes them
void GraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent* event) {
  switch (currentMode) {
    case EditorMode::AddState: {
      createStateAt(event->scenePos());
      break;
    }

    case EditorMode::AddTransition: {
      auto* item = itemAt(event->scenePos(), QTransform());
      auto* state = qgraphicsitem_cast<StateItem*>(item);
      if (state) handleTransitionClick(state);
      break;
    }

    case EditorMode::Delete: {
      handleDeleteClick(event->scenePos());
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

// Finds a state item in the scene by its name
StateItem* GraphicsScene::findStateByName(const QString& name) {
  for (QGraphicsItem* item : items()) {
    if (auto* state = qgraphicsitem_cast<StateItem*>(item)) {
      if (state->getName() == name)
        return state;
    }
  }
  return nullptr;
}

// Handles deletion of a state or transition based on click position
void GraphicsScene::handleDeleteClick(const QPointF& pos) {
  QGraphicsItem* item = itemAt(pos, QTransform());

  // If its a transition, remove it directly
  if (auto* transition = dynamic_cast<TransitionItem*>(item)) {
    // Detach from states
    if (auto* from = transition->getFromState())
      from->removeTransition(transition);
    if (auto* to = transition->getToState())
      to->removeTransition(transition);

    removeItem(transition);
    delete transition;
    return;
  }

  // If its a state, remove it and all attached transitions also
  if (auto* state = dynamic_cast<StateItem*>(item)) {
    QSet<TransitionItem*> transitions = state->getAttachedTransitions();
    for (TransitionItem* t : transitions) {
      if (auto* from = t->getFromState())
        from->removeTransition(t);
      if (auto* to = t->getToState())
        to->removeTransition(t);

      removeItem(t);
      delete t;
    }

    // Notify MainWindow to remove action row
    emit stateDeleted(state->getName());
    removeItem(state);
    delete state;
    return;
  }
}

// Clears the scene of all items (states and transitions)
// Resets the initial state reference as well
void GraphicsScene::clearScene() {
  for (auto* item : items())
    removeItem(item);
  clear(); // Remove all items from the scene
  initialState = nullptr; // Reset initial state
}

// Creates a new transition between two states by references
// Adds the transition to both states and updates its position in the scene
TransitionItem* GraphicsScene::createTransitionByNames(StateItem* from, StateItem* to) {
  auto* transition = new TransitionItem(from, to);

  // Check if a reverse transition exists and curve both
  for (QGraphicsItem* item : items()) {
    if (auto* existing = dynamic_cast<TransitionItem*>(item)) {
      if (existing->getFromState() == to && existing->getToState() == from && from != to) {
        transition->setCurved(true);
        existing->setCurved(true);
        existing->updatePosition();
      }
    }
  }

  from->addTransition(transition);
  to->addTransition(transition);
  addItem(transition);  // Add transition line to scene
  transition->updatePosition(); // Recalculate its position based on current state positions
  return transition;
}

// Creates and adds a named state at a given position
// Used primarily during import or model restoration
StateItem* GraphicsScene::createState(const QPointF& pos, const QString& name) {
  auto* state = new StateItem(name); // Create new state with given name
  state->setPos(pos);  // Set its position on the canvas
  addItem(state);  // Add to scene
  return state;
}
