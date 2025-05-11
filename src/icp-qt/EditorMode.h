/**
 * @file EditorMode.h
 * @brief Represents the editor's current interaction mode used for mouse input handling in the FSM editor.
 * @author Denis Milistenfer <xmilis00@stud.fit.vutbr.cz>
 * @date 11.05.2025
 */

#ifndef EDITORMODE_H
#define EDITORMODE_H

/**
 * @enum EditorMode
 * @brief Represents the editor's current interaction mode.
 *
 * Used to determine what mouse actions do (e.g., add state, create transition, move states).
 */
enum class EditorMode {
  AddState,      ///< Clicking on the canvas creates a new state
  AddTransition, ///< Clicking two states in sequence creates a transition
  Move,          ///< States can be moved by dragging
  Delete         ///< Clicking on a state or transition deletes it
};

#endif // EDITORMODE_H
