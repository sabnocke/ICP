Issues found in the repo by Gemini

## Build Configuration - Redundant Source Compilation / ODR Violation Risk:

Source files for the FSM logic (e.g., ParserLib.cpp, Interpret.cpp, Utils.cpp) are included in both the fsm target in the main CMakeLists.txt and the icp-qt target in src/icp-qt/CMakeLists.txt. 
This will lead to them being compiled twice. 
If the fsm target is a library meant to be used by icp-qt, then icp-qt should link against the fsm library, not compile its sources directly. 
Otherwise, this can lead to One Definition Rule (ODR) violations or linker errors.

## Build Configuration - Lua Linking Inconsistency:

The main CMakeLists.txt finds Lua using find_package(Lua) and links via Lua::lua or LUA_LIBRARIES. 
The src/icp-qt/CMakeLists.txt links against lua (lowercase). While CMake's find_package might create an alias, this inconsistency should be verified and potentially standardized.
