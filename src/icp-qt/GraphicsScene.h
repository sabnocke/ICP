/**
 * @file GraphicsScene.h
 * @brief Custom scene class for the FSM editor. Handles all mouse interaction and state/transition creation logic.
 * @author Denis Milistenfer <xmilis00@stud.fit.vutbr.cz>
 * @date 11.05.2025
 */

#ifndef GRAPHICSSCENE_H
#define GRAPHICSSCENE_H

#include <QGraphicsScene>
#include "StateItem.h"
#include "EditorMode.h"

/**
 * @class GraphicsScene
 * @brief Custom scene class for the FSM editor.
 *
 * Handles all mouse interaction and logic related to adding, moving, editing, and deleting states and transitions.
 */
class GraphicsScene : public QGraphicsScene {
  Q_OBJECT

public:
/**
   * @brief Constructs the GraphicsScene.
   * @param parent Optional parent object.
   */
  explicit GraphicsScene(QObject* parent = nullptr);

  /**
   * @brief Change current editor interaction mode (AddState, AddTransition, Move).
   * @param mode New editor mode to apply.
   */
  void setMode(EditorMode mode);

  /**
   * @brief Marks the given state as the initial state, replacing any existing one.
   * @param state Pointer to the state to mark as initial.
   */
  void setInitialState(StateItem* state);

  /**
   * @brief Returns the current editing mode.
   * @return The current EditorMode.
   */
  EditorMode getCurrentMode() const { return currentMode; }

  /**
   * @brief Returns the current initial state (or nullptr if none is set).
   * @return Pointer to the initial state or nullptr.
   */
  StateItem* getInitialState() const { return initialState; }

  /**
   * @brief Notifies the scene that a state's name has been changed.
   * @param oldName The old name of the state.
   * @param newName The new name of the state.
   */
  void notifyStateRenamed(const QString& oldName, const QString& newName);

  /**
   * @brief Searches and returns a pointer to the state with the given name.
   * @param name Name of the state to search for.
   * @return Pointer to the matching state or nullptr if not found.
   */
  StateItem* findStateByName(const QString& name);

  /**
   * @brief Create a new unnamed state at a specific scene position.
   * @param pos Position in the scene where the state should be placed.
   */
  void createStateAt(const QPointF& pos);

  /**
   * @brief Creates and returns a transition between two existing states.
   * @param from Pointer to the source state.
   * @param to Pointer to the destination state.
   * @return Pointer to the created TransitionItem.
   */
  TransitionItem* createTransitionByNames(StateItem* from, StateItem* to);

  /**
   * @brief Clears all items (states and transitions) from the scene.
   */
  void clearScene();

  /**
   * @brief Creates and returns a new named state at a specific position.
   * @param pos Position in the scene where the state should be placed.
   * @param name Name to assign to the new state.
   * @return Pointer to the created StateItem.
   */
  StateItem* createState(const QPointF& pos, const QString& name);

protected:
  /**
   * @brief Handles mouse click events depending on current interaction mode.
   * @param event Mouse press event.
   */
  void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

  /**
   * @brief Handles double-clicks for label editing (states and transitions).
   * @param event Mouse double-click event.
   */
  void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;

private:
  EditorMode currentMode = EditorMode::AddState; ///< Current interaction mode
  StateItem* selectedState = nullptr;            ///< First state selected for transition creation
  StateItem* initialState = nullptr;             ///< The current initial state

  /**
   * @brief Handles the second click to complete a transition creation.
   * @param clicked The second clicked state.
   */
  void handleTransitionClick(StateItem* clicked);

  /**
   * @brief Resets temporary state selection used during transition creation.
   */
  void resetTransitionSelection();

  /**
   * @brief Enables or disables state movability based on the current mode.
   */
  void updateStateMovability();

  /**
   * @brief Deletes a clicked item (state with attached transitions or just a transition).
   * @param pos Scene position of the click.
   */
  void handleDeleteClick(const QPointF& pos);

signals:
  /**
   * @brief Emitted when a new state is added (used to update action table).
   * @param name Name of the newly added state.
   */
  void stateAdded(const QString& name);

  /**
   * @brief Emitted when a state is deleted (used to update action table).
   * @param stateName Name of the deleted state.
   */
  void stateDeleted(const QString& stateName);

  /**
   * @brief Emitted when a state is renamed (used to rename row in action table).
   * @param oldName Previous name of the state.
   * @param newName New name of the state.
   */
  void stateRenamed(const QString& oldName, const QString& newName);
};

#endif // GRAPHICSSCENE_H
