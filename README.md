# ICP
Repo for ICP project

---
## 👤 Authors

- Denis Milistenfer `<xmilis00>`
- Robert Zelnice `<xzelni06>`
- Michal Hloch `<xhlochm00>`

## BUILD

### WINDOWS
$env:VCPKG_ROOT = "C:\path\to\vcpkg"
$env:QT_PREFIX = "C:\Qt\6.9.0\msvc2022_64"
mingw32-make

### LINUX
export VCPKG_ROOT=/absolute/path/to/vcpkg
export QT_PREFIX=/absolute/path/to/Qt/6.9.0/gcc_64
make

## Vcpkg Dependencies (more see fsm/readme.md)

To install dependencies using vcpkg:

```bash
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg

# Bootstrap (choose based on your platform)
.\bootstrap-vcpkg.bat     # for Windows PowerShell
./bootstrap-vcpkg.sh      # for Linux/macOS

# Integrate with CMake
./vcpkg.exe integrate install

# Install required packages
./vcpkg.exe install abseil
./vcpkg.exe install re2
./vcpkg.exe install lua
./vcpkg.exe install range-v3

# Dependencies
This project uses following external libraries
 - ~~[ctre](https://github.com/hanickadot/compile-time-regular-expressions/tree/main)~~
 - [reproc](https://github.com/DaanDeMeyer/reproc)
 - [abseil](https://abseil.io)
 - [re2](https://github.com/google/re2/tree/main)
 - [fast_float](https://github.com/fastfloat/fast_float)
 - [range-v3](https://github.com/ericniebler/range-v3)
 - [libzmq](https://github.com/zeromq/libzmq/tree/master)  << might get removed later
 - [zmgpp](https://github.com/zeromq/zmqpp/tree/develop)   << might get removed later
