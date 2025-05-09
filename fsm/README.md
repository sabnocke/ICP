- In order to run this, [vcpkg](https://learn.microsoft.com/en-us/vcpkg/get_started/get-started?pivots=shell-powershell) is required
> The slash/backslash might be incorrect
```shell
    git clone https://github.com/microsoft/vcpkg.git
    cd vcpkg && .\bootstrap-vcpkg.bat # for powershell
    cd vcpkg && ./bootstrap-vcpkg.sh  # for linux
    ./vcpkg.exe integrate install
    ./vcpkg.exe install abseil
    ./vcpkg.exe install re2
```

- That should be all, if it doesn't work I will get angry.
- In the future, I should make script for that
- CMake is the run with

```shell
    mkdir build && cd build
    cmake -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/buildsystem -G something --target fsm
```

> the CMAKE_TOOLCHAIN_FILE is important
> the path should look something like:
>  `/libs/vcpkg/scripts/buildsystems/vcpkg.cmake`

> It is important to set CMAKE_TOOLCHAIN_FILE correctly otherwise vcpkg won't

> -G should also be correctly set, vcpkg by default downloads packages for current os, 
> this can cause issues if cmake is run from wsl, mingw or anywhere os mismatch can happen (linux vs win)

> Using correct target (fsm) is necessary for correct linking of packages

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
- 