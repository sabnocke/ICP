- In order to run this, [vcpkg](https://learn.microsoft.com/en-us/vcpkg/get_started/get-started?pivots=shell-powershell) is required

```shell
    git clone https://github.com/microsoft/vcpkg.git
    cd vcpkg && ./bootstrap-vcpkg.bat # for powershell
    cd vcpkg && ./bootstrap-vcpkg.sh  # for linux
    ./vcpkg.exe integrate install
    ./vcpkg.exe install
```
> The current version uses manifest (`vcpkg.json` and `vcpkg-configuration.json`) to download correct versions of packages

### Windows
1. `mkdir build && cd build`
2. `cmake .. -G "Visual Studio 17 2022" -A x64` 
   - `-G "Visual Studio 16 2019"` can be used alternatively
   - `-A Win32` can be used for x32 version
3. `cmake --build . --config Debug`

### Linux

1.  `mkdir build && cd build`
2. `cmake .. -G "Ninja"`
3. `cmake --build . --config Debug`


- the `CMAKE_TOOLCHAIN_FILE` is important; the path should look something like:
  - `/vcpkg/scripts/buildsystems/vcpkg.cmake` 
- It is currently set by `CMakeLists.txt`, ~~but in future might be set by preset.~~ it is still is set by `CMakeLists.txt`.
- Correct target (fsm) is handled by preset.

---
# Syntax
- Any keyword is case-insensitive
- There are no comments, beside `comment:`
- No section is required, but for the automat to work correctly, 
 _states_ and _transitions_ are **required** and *signals* with *variables* are **optional**

## Name
- `name(:) <name>`
- The colon is optional

## Comment
- Ideally should be on single line
- `comment(:) <comment>`
- The colon is optional

## Signals
- `inputs: <name>, <name1>, ...`
- `inputs` can also be `outputs`

## Variables
- `<type> <name> = <value>`
- type should be one of
  - int
  - double
  - string
  - bool
- all variables must be initialized (assigned a value)
- lua on its own doesn't have types, so these act more as a constraint

## States
- `state <name> [<action>]`
- _action_ can be empty and can be on multiple lines
  - Additionally, _action_ can be anything that basic lua can compile or uses predefined functions
  - These are: `valueof(name)`, `defined(name)` or `output(name, value)`
- _name_ should be unique
- automat doesn't verify reachability of any state

## Transitions
- Whole section needs to start with `Transitions:` line (maybe remove that?)
- `<from> --> <to>: <input>? [<condition>]? @ <delay>?`
