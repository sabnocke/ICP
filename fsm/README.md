- In order to run this, [vcpkg](https://learn.microsoft.com/en-us/vcpkg/get_started/get-started?pivots=shell-powershell) is required
> The slash/backslash might be incorrect
```shell
    git clone https://github.com/microsoft/vcpkg.git
    cd vcpkg && .\bootstrap-vcpkg.bat # for powershell
    cd vcpkg && ./bootstrap-vcpkg.sh  # for linux
    ./vcpkg.exe integrate install
    ./vcpkg.exe install abseil
    ./vcpkg.exe install re2
    ./vcpkg.exe install lua
    ./vcpkg.exe install range-v3
```
> That should be all, if it doesn't work I will get angry.

- vcpkg installs packages based on operating system, which can be problem for windows (*MSVC* vs *MINGW*)
- as such it is important to set correct triplets (linux vs windows doesn't matter, vcpkg can detect that)

> In the future, I should make a script for that.

1.  Building and compiling of the project is done via presets `CMakePresets.json`.
    - The only working preset is for windows, as of now.
2. `cmake --preset windows-ucrt-debug` then
3. `cmake --build --preset build-windows-ucrt-debug`.
4. This, if all goes well, creates compiled project in directory `./build/windows-ucrt-debug`.

For linux the idea is same but preset is `linux-gcc-debug` and build one is `build-linux-gcc-debug`


> the CMAKE_TOOLCHAIN_FILE is important
> the path should look something like:
>  `/vcpkg/scripts/buildsystems/vcpkg.cmake`
> 
> It is currently set by `CMakeLists.txt`, but in future might be set by preset.

> Using correct target (fsm) is handled by preset.

---
