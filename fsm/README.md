- In order to run this, [vcpkg](https://learn.microsoft.com/en-us/vcpkg/get_started/get-started?pivots=shell-powershell) is required
> The slash/backslash might be incorrect
```shell
    git clone https://github.com/microsoft/vcpkg.git
    cd vcpkg && .\bootstrap-vcpkg.bat # for powershell
    cd vcpkg && ./bootstrap-vcpkg.sh  # for linux
    ./vcpkg.exe integrate install
    ./vcpkg.exe install abseil
    ./vcpkg.exe install re2
    ./vcpkg.exe install rxcpp
    ./vcpkg.exe install lua
```
> That should be all, if it doesn't work I will get angry.
> 
> vcpkg installs packages based on operating system, which can be problem for windows (*MSVC* vs *MINGW*)
> 
> In the future, I should make a script for that.

> Building and compiling of the project is done via presets `CMakePresets.json`.
> The only working preset is for windows, as of now.
> 
> `cmake --preset windows-ucrt-debug` then
> 
> `cmake --build --preset build-windows-ucrt-debug`.
> 
> This, if all goes well, creates compiled project in directory `./build/windows-ucrt-debug`.

> Linux should be done similarly (albeit there is no preset for it), since ucrt is almost linux.
> 
> So, TODO, I guess.

> the CMAKE_TOOLCHAIN_FILE is important
> the path should look something like:
>  `/libs/vcpkg/scripts/buildsystems/vcpkg.cmake`
> 
> It is currently set by `CMakeLists.txt`, but in future might be set by preset.

> Using correct target (fsm) is handled by preset.

---

# Communication
The current idea of execution is to create a new cpp file at runtime and then
execute it as a separate process (via *reproc*).

However, that requires handling communication of inputs/outputs from __FSM__ to __GUI__.


- [cpp-ipc](https://github.com/mutouyun/cpp-ipc)
- [Flow-ipc](https://github.com/Flow-IPC/ipc)
- [grpc](https://grpc.io)
- [Boost.Interprocess](https://www.boost.org/doc/libs/1_84_0/doc/html/interprocess.html)
- [ZeroMQ](https://zeromq.org)

---
