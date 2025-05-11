# Build output directory
BUILD_DIR := build

# Executable name
TARGET := fsm_editor

# CMake configuration
CMAKE_TOOLCHAIN ?= $(VCPKG_ROOT)/scripts/buildsystems/vcpkg.cmake
CMAKE_PREFIX_PATH ?= $(QT_PREFIX)
CMAKE_BUILD_TYPE ?= Release

# Doxygen configuration
DOXYFILE := Doxyfile
DOC_DIR := doc

# Compiler flags
CXX_STANDARD := -DCMAKE_CXX_STANDARD=17

# Targets
.PHONY: all build doxygen clean run

all: build

# Configure and build the project
build:
	@echo "[Build] Creating build directory and running CMake..."
	@mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && cmake .. \
		$(CXX_STANDARD) \
		-DCMAKE_TOOLCHAIN_FILE=$(CMAKE_TOOLCHAIN) \
		-DCMAKE_PREFIX_PATH="$(CMAKE_PREFIX_PATH)" \
		-DCMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE)
	cd $(BUILD_DIR) && cmake --build . --config $(CMAKE_BUILD_TYPE)

# Run the compiled application
run:
	@echo "[Run] Executing application..."
	@./$(BUILD_DIR)/$(TARGET)

# Generate HTML documentation via Doxygen
doxygen:
	@echo "[Doxygen] Generating documentation..."
	doxygen $(DOXYFILE)

# Clean build and documentation directories
clean:
	@echo "[Clean] Removing build and doc output..."
	@rm -rf $(BUILD_DIR) $(DOC_DIR)