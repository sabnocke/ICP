#
# @file Makefile
# @brief Makefile for building, running, cleaning program and generating documentation
# @author Denis Milistenfer <xmilis00@stud.fit.vutbr.cz>
# @date 09.06.2025
#

# where to put all the build artifacts
BUILD_DIR       := build

# CMake target (GUI executable)
TARGET          := icp-qt
# subfolder under build where target is generated
TARGET_SUBDIR   := src/icp-qt

# on multi-config generators -> build/.../Debug
# single-config (Makefiles/Ninja) -> ignore
BUILD_CFG       ?= $(CMAKE_BUILD_TYPE)

# users can override these from the env or command-line
QT_DIR           ?= /opt/Qt/6.5.0          # only used if qmake isn't found
VCPKG_ROOT       ?= $(CURDIR)/vcpkg        # adjust if you installed vcpkg elsewhere

CMAKE_BUILD_TYPE ?= Debug

# Qt detection via qmake
ifeq ($(OS),Windows_NT)
  QMAKE := $(shell where qmake 2>nul)
else
  QMAKE := $(shell which qmake 2>/dev/null)
endif

ifneq ($(QMAKE),)
  QT_PREFIX := $(shell "$(QMAKE)" -query QT_INSTALL_PREFIX)
else
  QT_PREFIX := $(QT_DIR)
endif

# CMake flags
CMAKE_PREFIX_PATH    := $(QT_PREFIX)
CMAKE_FLAGS := \
  -DCMAKE_PREFIX_PATH=$(CMAKE_PREFIX_PATH)       \
  -DCMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE)

# Doxygen configuration
DOXYFILE := Doxyfile
DOC_DIR  := doc


# Windows-specific path adjustment
ifeq ($(OS),Windows_NT)
  # turn "src/icp-qt" into "src\icp-qt"
  WIN_SUBDIR := $(subst /,\\,$(TARGET_SUBDIR))
  RUN_TARGET := run-win
else
  RUN_TARGET := run-unix
endif

.PHONY: all configure build run run-win run-unix doxygen clean

# default: configure + build
all: configure build

# generate build system
configure:
	@echo "[cmake] Configuring in $(BUILD_DIR)..."
	@cmake -S . -B $(BUILD_DIR) $(CMAKE_FLAGS)

# actually compile
build:
	@echo "[cmake] Building $(TARGET)..."
ifeq ($(OS),Windows_NT)
	@cmake --build $(BUILD_DIR) --config $(BUILD_CFG)
else
	@cmake --build $(BUILD_DIR)
endif

# pick the right run-target
run: $(RUN_TARGET)

# Windows:
run-win: all
	@echo "[run] Launching $(TARGET)..."
	@start /b "" "$(BUILD_DIR)\$(WIN_SUBDIR)\$(BUILD_CFG)\$(TARGET).exe"

# Linux/Unix:
run-unix: all
	@echo "[run] Launching $(TARGET)..."
	@$(BUILD_DIR)/$(TARGET_SUBDIR)/$(TARGET)

# generate doxygen documentation
doxygen:
	@echo "[doxygen] Generating documentation into $(DOC_DIR)..."
	@doxygen $(DOXYFILE)

# wipe everything
clean:
	@echo "[clean] Removing build directory and docs..."
ifeq ($(OS),Windows_NT)
	@cmd /C "if exist $(BUILD_DIR) rmdir /S /Q $(BUILD_DIR)"
	@cmd /C "if exist $(DOC_DIR)    rmdir /S /Q $(DOC_DIR)"
else
	@rm -rf $(BUILD_DIR) $(DOC_DIR)
endif
