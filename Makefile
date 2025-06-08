# build artifacts
BUILD_DIR := build

# CMake target name of GUI
TARGET := icp-qt
# subfolder under build where that target is generated
TARGET_SUBDIR := src/icp-qt

# on multi-config generators -> build/.../Debug
# single-config (Makefiles, Ninja) -> ignore
BUILD_CFG ?= $(CMAKE_BUILD_TYPE)

# users can override these from the env or command-line
QT_DIR     ?= /opt/Qt/6.5.0   # if Qt was not detected by qmake change this path
VCPKG_ROOT ?= $(CURDIR)/vcpkg # if you have installed vcpkg in different location then suggested change this path

CMAKE_BUILD_TYPE ?= Debug

# Qt detection using qmake
ifeq ($(OS),Windows_NT)
  QMAKE := $(shell where qmake 2>nul | head -n1)
else
  QMAKE := $(shell which qmake 2>/dev/null)
endif

ifneq ($(QMAKE),)
  QT_PREFIX := $(shell $(QMAKE) -query QT_INSTALL_PREFIX)
else
  QT_PREFIX := $(QT_DIR)
endif

# feed into CMake
CMAKE_TOOLCHAIN_FILE := $(VCPKG_ROOT)/scripts/buildsystems/vcpkg.cmake
CMAKE_PREFIX_PATH    := $(QT_PREFIX)

CMAKE_FLAGS := \
  -DCMAKE_TOOLCHAIN_FILE=$(CMAKE_TOOLCHAIN_FILE) \
  -DCMAKE_PREFIX_PATH=$(CMAKE_PREFIX_PATH)       \
  -DCMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE)

# Doxygen configuration
DOXYFILE := Doxyfile
DOC_DIR  := doc

.PHONY: all configure build run doxygen clean

# default: configure + build
all: configure build

# generate your build system
configure:
	@echo "[cmake] Configuring in $(BUILD_DIR)..."
	cmake -S . -B $(BUILD_DIR) $(CMAKE_FLAGS)

# actually compile
build:
	@echo "[cmake] Building $(TARGET)..."
	cmake --build $(BUILD_DIR)

# build + run
run: all
	@echo "[run] Launching $(TARGET)..."
ifeq ($(OS),Windows_NT)
	@"$(BUILD_DIR)/$(TARGET_SUBDIR)/$(BUILD_CFG)/$(TARGET).exe"
else
	@"$(BUILD_DIR)/$(TARGET_SUBDIR)/$(TARGET)"
endif

# generate doxygen documentation
doxygen:
	@echo "[doxygen] Generating documentation..."
	doxygen $(DOXYFILE)

# wipe everything
clean:
	@echo "[clean] Removing build directory and doxy documentation..."
	rm -rf $(BUILD_DIR) $(DOC_DIR)
