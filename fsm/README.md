- In order to run this, [vcpkg](https://learn.microsoft.com/en-us/vcpkg/get_started/get-started?pivots=shell-powershell) is required

```shell
    git clone https://github.com/microsoft/vcpkg.git
    cd vcpkg && ./bootstrap-vcpkg.bat # for powershell
    cd vcpkg && ./bootstrap-vcpkg.sh  # for linux
    ./vcpkg.exe integrate install
    ./vcpkg.exe install
```
> The current version uses manifest (`vcpkg.json` and `vcpkg-configuration.json`) to download correct versions of packages

1.  Building and compiling of the project is done via presets (`CMakePresets.json`).
2. `cmake --preset windows-ucrt-debug` then
3. `cmake --build --preset build-windows-ucrt-debug`.
    - could the 3 be called without calling 2 first?
4. This, if all goes well, creates compiled project in directory `./build/windows-ucrt-debug`.

For linux the idea is the same, but the preset is `linux-gcc-debug` and build one is `build-linux-gcc-debug`.
> While there is preset for that, I don't remember if it works, so TODO


- the `CMAKE_TOOLCHAIN_FILE` is important; the path should look something like:
  - `/vcpkg/scripts/buildsystems/vcpkg.cmake` 
- It is currently set by `CMakeLists.txt`, ~~but in future might be set by preset.~~ it is still is set by `CMakeLists.txt`.
- Correct target (fsm) is handled by preset.

---
