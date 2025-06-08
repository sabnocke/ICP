# ICP project

## State of examples

|Name|Working|Issues
|--|:--:|--|
|BuildPipeline| ❌ | Incorrect syntax
|CoffeeMachine|✅| Stops after `STOPPED`, throws error but expected
|Hello|**❌**| Incorrect state action
|Toggle|✅|
|TrafficLight|**❌**| None of output signals are set at any time
|WashingMachine|**❌**| `status` signal is never set

---
## 👤 Authors

- Denis Milistenfer `<xmilis00>`
- Robert Zelnice `<xzelni06>`
- Michal Hloch `<xhlochm00>`

## BUILD

### 1. Vcpkg Dependencies

You need to install dependencies using vcpkg first.

Inside root of this project:
```shell
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg

# Bootstrap
.\bootstrap-vcpkg.bat     # for Windows PowerShell
./bootstrap-vcpkg.sh      # for Linux/macOS

# Integrate with CMake
./vcpkg.exe integrate install   # for Windows PowerShell
./vcpkg integrate install       # for Linux/macOS

# Install required packages
./vcpkg.exe install # for Windows PowerShell
./vcpkg install     # for Linux/macOS

# Installation is handled by manifest
```
### 2. Make
```shell
make            # build
make run        # build + run
make clean      # clean everything
make doxygen    # generate documentation
```
<span style="color:orange">You need to have cmake and qmake available for this
</span>

# Dependencies
This project uses following external libraries (_header-only_ are contained in fsm/external):
 - [abseil](https://abseil.io)
 - [re2](https://github.com/google/re2/tree/main)
 - [fast_float](https://github.com/fastfloat/fast_float) header-only
 - [range-v3](https://github.com/ericniebler/range-v3)
 - [sol2](https://github.com/ThePhD/sol2) header-only
 - [lua](https://www.lua.org)
