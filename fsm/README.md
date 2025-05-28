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
