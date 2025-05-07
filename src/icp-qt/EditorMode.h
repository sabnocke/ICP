#ifndef EDITORMODE_H
#define EDITORMODE_H

// Represents the editor's current interaction mode
// Used to determine what mouse actions do (for add state, create transition, move)
enum class EditorMode {
  AddState,      // Clicking on the canvas creates a new state
  AddTransition, // Clicking two states in sequence creates a transition
  Move           // States can be moved by dragging
};

#endif // EDITORMODE_H
