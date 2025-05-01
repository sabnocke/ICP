# ICP
Repo for ICP project

---
## TODO for parse
- [ ] Finish initial parsing
- [ ] Start with new file creation, i.e., the file containing runnable version of automat
- [ ] Need to create a way communicating between processes (one parses and creates the automat and other runs it)
- [ ] Process shenanigans

---
- [ ] Background draggable area motion visibility
- [ ] Create dragging for widgets (whatever I am constructing the automata of)
- [ ] For dragging of widgets maybe spawn them within the area first and make them draggable there?
---
- [ ] Beware of backwards compatibility (Qt 6.9 vs Qt 5.5)
```cpp
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	// do something
#else
	// do something differently
#endif
```
---
- [ ] Change the name of `Draggable.[cpp|h]` to match with other files

# Parsing

1. Name (static)
2. Comment (static)
3. IN variables
    - Considered defined when having value, otherwise not
    - Are always string, but can be string of other type
4. OUT variables
    - Always string, intended for displaying
5. Internal variables
    - Need to have type (as defined in language that handles them)
    - `<type> <name> = <value>`
6. States and their actions
    - Each state can have an action
    - Action is block of code written in inscription language
    - Can be variably long
    - Will need verification and run engine
7. Transitions and their conditions
    - Each transition can have condition
    - `<input_event>[<bool condition>]@<delay>`
    - Each part can be on its own or together with the rest
        - `<input_event>` defines which IN variable to read to transit
        - `[<bool condition>]` defines if transition can happen
        - `@<delay>` defines delay before transition
    - Might be a lot simpler than it looks (at least compared to 6.)

# Execution

1. If terminated, then exit.
2. If there is reachable state from current position: (reachable means its transition condition is true)
    - If no delay, then move there and do its action.
    - Else set timer for this transition. (this implies there can be multiple timers)
3. Remove unused timers (most likely for other delayed states from following step).
4. Wait for timer to run out or input event, then:
    - If it is timer, remove it, move to its state and do state's action.
    - If it is input, find the correct state, move there and so on...
    - If it is immediate transition move there and do its action.
    - If it is delayed transition, set timer for it.
    - If it is termination command, set terminated = true. 

# Dependencies
This project uses following external libraries
 - [ctre](https://github.com/hanickadot/compile-time-regular-expressions/tree/main) 