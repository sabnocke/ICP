# ICP
Repo for ICP project

---
## 👤 Authors

- Denis Milistenfer `<xmilis00>`
- Robert Zelnice `<xzelni06>`
- Michal Hloch `<xhlochm00>`

## BUILD

### WINDOWS
```shell
$env:VCPKG_ROOT = "C:\path\to\vcpkg"  # unnecessary, set by cmake
$env:QT_PREFIX = "C:\Qt\6.9.0\msvc2022_64" # dont know about this, but could be set by cmake too
# technically unnecessary since cmake builds executable,
# however not sure how it works with QT.
mingw32-make
```

### LINUX
```shell
export VCPKG_ROOT=/absolute/path/to/vcpkg
export QT_PREFIX=/absolute/path/to/Qt/6.9.0/gcc_64
make
```

## Vcpkg Dependencies (more see fsm/readme.md)

To install dependencies using vcpkg:

```shell
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg

# Bootstrap (choose based on your platform)
.\bootstrap-vcpkg.bat     # for Windows PowerShell
./bootstrap-vcpkg.sh      # for Linux/macOS

# Integrate with CMake
./vcpkg.exe integrate install

# Install required packages
./vcpkg.exe install
# Installation is handled by manifest
```
# Dependencies
This project uses following external libraries:
 - [abseil](https://abseil.io)
 - [re2](https://github.com/google/re2/tree/main)
 - [fast_float](https://github.com/fastfloat/fast_float) 
 - [range-v3](https://github.com/ericniebler/range-v3)
 - [spdlog](https://github.com/gabime/spdlog)
 - [sol2](https://github.com/ThePhD/sol2)
 - [lua](https://www.lua.org)