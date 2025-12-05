# WARP.md

This file provides guidance to WARP (warp.dev) when working with code in this repository.

## Build System and Common Commands

Inviwo uses CMake as its build system with vcpkg for dependency management. The project supports cross-platform development on Windows, Linux, and macOS.

### Basic Build Commands
```bash
# Configure with default settings
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake

# Configure with specific options (common configuration)
cmake -B build -S . \
  -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake \
  -DIVW_ENABLE_PYTHON=ON \
  -DIVW_ENABLE_QT=ON \
  -DBUILD_SHARED_LIBS=ON

# Build all targets
cmake --build build

# Build specific target
cmake --build build --target inviwo

# Install
cmake --install build
```

### Testing Commands
```bash
# Build and run all tests
cmake --build build --target test

# Run specific integration tests
./build/bin/inviwo-integrationtests

# Run unit tests for a specific module (example)
ctest --test-dir build -R "base-unittest"

# Run tests with verbose output
ctest --test-dir build --verbose
```

### Development Tools
```bash
# Generate new module using meta tools
./build/bin/inviwo-meta-cli --template module --name MyModule

# Format code (if clang-format is available)
find . -name "*.h" -o -name "*.cpp" | grep -v build | xargs clang-format -i

# Update module registration
cmake --build build --target inviwo-module-registration
```

## High-Level Architecture

Inviwo is a data visualization framework built around a modular, component-based architecture with visual programming capabilities.

### Core Architectural Concepts

**Visual Programming Model**: Inviwo uses a data flow network where **processors** are connected via **ports** to form visualization pipelines. Each processor performs a specific task (data loading, transformation, rendering) and can be connected visually in the GUI.

**Modular System**: Functionality is organized into modules that can be enabled/disabled at compile time:
- **Core modules** (`src/core/`): Fundamental data structures, properties, processors
- **Base module** (`modules/base/`): Essential processors for data I/O and basic operations  
- **Rendering modules** (`modules/basegl/`, `modules/opengl/`): OpenGL-based rendering
- **Python integration** (`modules/python3/`, `modules/python3qt/`): Python scripting support
- **Qt integration** (`modules/qtwidgets/`): GUI components and widgets

**Key Data Structures**:
- **Processors**: Fundamental processing units that transform data
- **Properties**: Configurable parameters that control processor behavior
- **Ports**: Typed connection points for data flow (inports receive, outports send)
- **Data objects**: Volume, Image, Mesh, Buffer - core data types for visualization

### Module System

Modules are self-contained units that extend Inviwo's functionality. Each module:
- Has its own `CMakeLists.txt` using `ivw_module()` macro
- Registers processors, properties, and data readers/writers
- Can depend on other modules via CMake dependencies
- Contains processors, algorithms, data structures, and I/O functionality

**Module Dependencies**: The build system automatically handles module dependencies. Use `ivw_enable_modules_if()` to conditionally enable modules based on build options.

### Build System Architecture

**CMake Configuration**: 
- Main configuration in root `CMakeLists.txt`
- Module-specific configuration via `ivw_module()` macro
- Custom CMake utilities in `cmake/` directory for module registration, compilation options

**External Dependencies**: Managed through vcpkg with `vcpkg.json` manifest. Dependencies are automatically installed during configure step.

**Applications**: Multiple application targets are built:
- `inviwo`: Main Qt-based GUI application for visual programming
- `inviwo-glfwminimum`: Minimal GLFW example
- `inviwopyapp`: Python-based application
- Various utility tools in `tools/`

### Code Organization

```
src/core/          # Framework core (data structures, properties, processors base)
src/qt/            # Qt GUI implementation
src/py/            # Python bindings and integration
modules/           # Extension modules (base, rendering, domain-specific)
apps/             # Application entry points
tools/meta/       # Code generation and module creation tools  
tests/            # Integration and unit tests
```

**Development Workflow**: 
1. Create processors by inheriting from base processor classes
2. Define properties using the property system  
3. Implement `process()` method for data transformation
4. Register new processors in module's registration function
5. Use meta tools to generate boilerplate code and module structure

### Testing Architecture

- **Integration tests**: Full application tests in `tests/integrationtests/`
- **Unit tests**: Module-specific tests, typically in `modules/[name]/tests/`
- **Regression tests**: Image-based testing for visual output validation

The testing system uses Google Test framework and includes utilities in `tests/testutil/` for common test functionality.