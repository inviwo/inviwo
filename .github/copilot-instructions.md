# Inviwo Coding Agent Instructions

## Project Overview

**Inviwo** (Interactive Visualization Workshop) is an open-source visualization framework written in **C++23** with **Python** bindings. It enables scientists and developers to build data processing and visualization pipelines through a visual programming (node-graph) interface.

- **License**: Simplified BSD (free for commercial use)
- **Current version**: see the root `CMakeLists.txt` (project declared via `project(${IVW_CFG_PROJECT_NAME} ... VERSION ...)`, with the canonical version string assembled as `IVW_VERSION` from `IVW_MAJOR_VERSION`, `IVW_MINOR_VERSION`, etc.)
- **Cross Platform**: Windows, Linux, macOS
- **Main website**: https://inviwo.org
- **API docs**: https://inviwo.org/inviwo/doc
- **GitHub**: https://github.com/inviwo/inviwo
---

## Repository Layout

```
inviwo/
├── .github/              # GitHub Actions workflows, issue/PR templates
├── apps/                 # Executable applications (inviwo GUI, inviwopyapp, minimal examples)
├── cmake/                # Reusable CMake helper functions and macros
├── data/                 # Example datasets (volumes, meshes, transfer functions)
├── docs/                 # Sphinx/Doxygen documentation sources, style guides
├── ext/                  # Vendored external dependencies (flags, ticpp)
├── include/inviwo/       # Public C++ headers for the core framework
│   ├── core/             # Framework core: processors, data structures, properties, algorithms
│   └── qt/               # Qt-based GUI: network editor, properties panel, canvas
├── modules/              # 40+ extension modules (rendering, I/O, UI, Python, etc.)
├── resources/            # Application resources (icons, images, stylesheets)
├── src/                  # Source files for the core framework (mainly for internal implementation)
├── tests/                # Unit, integration, regression tests, and benchmarks
├── tools/
│   └── meta/             # Code-generation tool for scaffolding modules and processors
├── CMakeLists.txt        # Root CMake configuration
├── CMakePresets.json     # Named build presets
├── vcpkg.json            # vcpkg dependency manifest
├── .clang-format         # C++ code formatting rules
├── .clang-tidy           # Static analysis configuration
└── .editorconfig         # Editor settings
```

---

## Build System

### Requirements
- **CMake** ≥ 4.0
- **vcpkg** for dependency management (bootstrapped automatically via CMake presets)
- **Compiler**: MSVC 2026 (Windows), Clang (macOS), GCC or Clang (Linux)
- **Qt 6** (required for GUI applications)
- **Python 3** (required for Python bindings)

### Key CMake Presets (`CMakePresets.json`)

| Preset | Purpose |
|--------|---------|
| `user` | Standard release build, shared libs, basic apps |
| `developer` | Full development: assertions, profiling, all tests, all apps |
| `msvc-developer` | Windows MSVC developer build |
| `xcode-developer` | macOS Xcode developer build |
| `ninja-developer` | Ninja-based developer build (Linux/macOS) |
| `gha-dynamic` / `gha-static` | CI presets used by GitHub Actions defined in `.github/presets/*.json`|

### Building

```bash
# Clone
git clone https://github.com/inviwo/inviwo
git clone https://github.com/microsoft/vcpkg

# Configure
cmake -S inviwo --preset ninja-developer

# Build all targets
cmake --build builds/ninja-developer --parallel

# Run tests
ctest --test-dir builds/ninja-developer --output-on-failure

# Build a single module target
cmake --build builds/ninja-developer --target inviwo-module-base
```

### Common CMake Options

| Option | Default | Description |
|--------|---------|-------------|
| `IVW_ENABLE_PYTHON` | ON | Python 3 bindings via pybind11 |
| `IVW_ENABLE_QT` | ON | Qt6-based GUI |
| `BUILD_SHARED_LIBS` | ON | Shared vs. static linking |
| `IVW_TEST_UNIT_TESTS` | ON | GTest unit tests |
| `IVW_TEST_INTEGRATION_TESTS` | ON | Integration tests (requires GLFW + Base module) |
| `IVW_TEST_BENCHMARKS` | OFF | Google Benchmark tests |
| `IVW_CFG_PROFILING` | OFF | Tracy/profiling instrumentation |
| `IVW_EXTERNAL_MODULES` | - | Semicolon-separated paths to extra modules |

---

## Code Style & Conventions

### C++ Style (enforced by `.clang-format`)
- **Based on**: Google C++ style with customizations
- **Indentation**: 4 spaces (no tabs)
- **Line length**: 100 characters
- **Pointer alignment**: Left — `int* ptr;` not `int *ptr;`
- **Qualifier alignment**: Left — `const int` not `int const`
- **Constructor initializers**: Break before comma
- `SortIncludes: false` — manage include order manually

Run formatter: `clang-format -i <file>`

### General Conventions
- UTF-8 source files with Unix line endings
- Trailing whitespace trimmed, final newline required (see `.editorconfig`)
- Use C++23 features freely (structured bindings, `if constexpr`, `std::optional`, etc.)
- Prefer `std::string_view` and `std::span` over raw pointers
- Use `fmt::format` (bundled) for string formatting, not `std::sprintf`
- Exceptions are used for error reporting; use `Exception` from `<inviwo/core/util/exception.h>`
- Use `log::error`, `log::warn`, `log::info` functions for logging, they use fmt-style formatting
- All public API symbols need a DLL export macro (e.g., `IVW_CORE_API`, `IVW_MODULE_BASE_API`)

### Include Order (manual, not auto-sorted)
1. Corresponding header (for `.cpp` files)
2. Inviwo core headers
3. Module/component headers
4. Third-party headers
5. Standard library headers

---

## Architecture: Processors, Ports, Properties, and Modules

### Core Concepts

- **Processor**: A node in the visualization pipeline. Has `Inport`s, `Outport`s, and `Property`s. Override `process()` to implement logic.
- **Port**: Typed connection point (e.g., `VolumeOutport`/`VolumeInport`, `MeshOutport`/`MeshInport`, `ImageOutport`/`ImageInport`).
- **Property**: A configurable parameter exposed in the UI (e.g., `FloatProperty`, `BoolProperty`, `TransferFunctionProperty`).
- **Module**: A collection of processors, data structures, and utilities. Registered via `InviwoModule` subclass.

### Processor Pattern

```cpp
// myprocessor.h
#pragma once
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/mymodule/mymoduledefine.h>

namespace inviwo {

class IVW_MODULE_MYMODULE_API MyProcessor : public Processor {
public:
    MyProcessor();
    virtual const ProcessorInfo& getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    virtual void process() override;

private:
    VolumeInport inport_;
    VolumeOutport outport_;
    FloatProperty threshold_;
};

}  // namespace inviwo
```

```cpp
// myprocessor.cpp
#include <inviwo/mymodule/processors/myprocessor.h>

namespace inviwo {

const ProcessorInfo MyProcessor::processorInfo_{
    "org.inviwo.MyProcessor",  // Unique class identifier (reverse-domain style)
    "My Processor",            // Human-readable display name
    "My Category",             // Category shown in processor list
    CodeState::Experimental,   // CodeState::Stable | Experimental | Deprecated
    Tags::CPU,                 // Tags::GL, Tags::CPU, Tags::CL, Tags::PY, Tags::None
};

MyProcessor::MyProcessor()
    : Processor()
    , inport_{"inputVolume"}
    , outport_{"outputVolume"}
    , threshold_{"threshold", "Threshold", 0.5f, 0.0f, 1.0f} {
    addPort(inport_);
    addPort(outport_);
    addProperty(threshold_);
}

const ProcessorInfo& MyProcessor::getProcessorInfo() const { return processorInfo_; }

void MyProcessor::process() {
    auto volume = inport_.getData();
    // ... process and produce output ...
    auto result = std::make_shared<Volume>();
    outport_.setData(result);
}

}  // namespace inviwo
```

### Module Structure

```
inviwo/mymodule/
├── CMakeLists.txt                        # ivw_module() + ivw_create_module()
├── depends.cmake                         # Optional: list(APPEND dependencies Core Base)
├── include/inviwo/mymodule/
│   ├── mymoduledefine.h                  # DLL export macro (generated or hand-written)
│   ├── mymodule.h
│   └── processors                        # Module class declaration
│       └── myprocessor.h
└── src/
│   ├── mymodule.cpp
│   └── processors                        # Module registration
        └── myprocessor.cpp
```

**`CMakeLists.txt` template:**
```cmake
ivw_module(MyModule)

set(HEADER_FILES
    include/inviwo/mymodule/mymodule.h
    include/inviwo/mymodule/mymoduledefine.h
    include/inviwo/mymodule/processors/myprocessor.h
)
ivw_group("Header Files" ${HEADER_FILES})

set(SOURCE_FILES
    src/mymodule.cpp
    src/processors/myprocessor.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

ivw_create_module(${SOURCE_FILES} ${HEADER_FILES})
```

**Module registration (`src/mymodule.cpp`):**
```cpp
#include <inviwo/mymodule/mymodule.h>
#include <inviwo/mymodule/processors/myprocessor.h>

namespace inviwo {

MyModule::MyModule(InviwoApplication* app) : InviwoModule(app, "MyModule") {
    registerProcessor<MyProcessor>();
    // Also available: registerRepresentation, registerDataReader, registerDataWriter, etc.
}

}  // namespace inviwo
```

### Code Generation (Scaffolding Tool)

The `tools/meta` binary scaffolds new modules and processors:

```bash
# After building, the meta tool is at:
./build/tools/meta/inviwo-meta

# Create a new module
./inviwo-meta --modules MyModule --path ./modules/

# Create a new processor
./inviwo-meta --processors MyProcessor --path ./modules/
```

---

## Testing

### Test Types

| Type | Location | Run with |
|------|----------|----------|
| Unit tests | `modules/*/tests/unittests/` | `ctest -R unittests` |
| Integration tests | `tests/integrationtests/` | `ctest -R integrationtests` |
| Regression tests | `tests/regression/` | Requires full build + app |
| Benchmarks | `tests/benchmarks/` | `ctest -R benchmarks` |

### Writing Unit Tests

```cmake
# In module CMakeLists.txt
ivw_add_unittest(tests/unittests/mytest.cpp)
```

```cpp
// tests/unittests/mytest.cpp
#include <gtest/gtest.h>
#include <modules/mymodule/myprocessor.h>

TEST(MyProcessorTest, BasicConstruction) {
    // ...
}
```

### Running Tests

```bash
# All tests
ctest --test-dir build --output-on-failure

# By name pattern
ctest --test-dir build -R "inviwo-unittests-base" --output-on-failure

# With verbose output
ctest --test-dir build -V
```

---

## Common Patterns

### Data Structures

```cpp
// Shared ownership via std::shared_ptr (convention in Inviwo)
auto volume = std::make_shared<Volume>(dims, format);
auto mesh = std::make_shared<Mesh>(DrawType::Triangles, ConnectivityType::None);
outport_.setData(mesh);                 // Pass shared_ptr

// Port data access
auto vol = volumeInport_.getData();     // std::shared_ptr<const Volume>  
auto mesh = meshInport_.getData();      // std::shared_ptr<const Mesh>  
```

### Properties

```cpp
// Scalar
FloatProperty threshold_{"id", "Display Name", defaultValue, min, max};
IntProperty count_{"count", "Count", 10, 1, 100};

// Boolean toggle
BoolProperty enable_{"enable", "Enable Feature", true};

// String
StringProperty name_{"name", "Name", "default"};

// Transfer function
TransferFunctionProperty tf_{"tf", "Transfer Function"};

// Color
FloatVec4Property color_{"color", "Color", vec4(1.0f)};

// Option list
OptionProperty<int> method_{"method", "Method",
    {{"nearest", "Nearest", 0}, {"linear", "Linear", 1}}, 0};
```

### Invalidation & Callbacks

```cpp
// Invalidate when property changes (default behavior)
addProperty(threshold_);

// Custom callback on change
threshold_.onChange([this]() { invalidate(InvalidationLevel::InvalidOutput); });
```

### OpenGL Processors

```cpp
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/shader/shader.h>

class MyGLProcessor : public Processor {
    Shader shader_{"myshader.frag", Shader::Build::No};
    // ...
};
```

---

## CI / GitHub Actions

The workflow is defined in `.github/workflows/inviwo.yml`.

**Matrix**:  
- Windows (MSVC): {Dynamic, Static} × {Release} (no Debug)  
- macOS (arm64): {Dynamic, Static} × {Release, Debug}  
- Linux (GCC): Dynamic × {Release, Debug}, Static × {Release} (no Static Debug)  

**Key steps**:
1. Bootstrap vcpkg and restore dependency cache
2. Configure with CMake using `gha-dynamic` or `gha-static` preset
3. Build all targets
4. Run `ctest` for unit tests
5. Launch the app with `--logconsole --quit` to verify it starts without errors
6. Package installers with CPack (`.exe`, `.dmg`, `.AppImage`)

**Investigating CI failures**:
- Use GitHub Actions "Re-run failed jobs" to reproduce
- On Windows: if app launch fails, cdb.exe debugger is automatically invoked
- On macOS: if app launch fails, lldb is automatically invoked
- Check `ccache` hit rate in logs for slow builds

---

## Dependency Management (vcpkg)

Dependencies are declared in `vcpkg.json`. Features group optional dependencies:

```json
{
  "dependencies": ["eigen3", "glm", "nlohmann-json"],
  "features": {
    "qt": { "dependencies": ["qtbase", "qtsvg"] },
    "python3": { "dependencies": ["python3"] },
    "hdf5": { "dependencies": ["hdf5"] }
  }
}
```

Add new dependencies by editing `vcpkg.json`. CMake will automatically bootstrap vcpkg and install missing dependencies when you configure the project with a preset that includes `vcpkg` (e.g., `ninja-developer`).

---

## External Modules

Third-party modules can be added without modifying this repository:

```bash
cmake -DIVW_EXTERNAL_MODULES="/path/to/mymodules" ...
```

Community modules: https://github.com/inviwo/modules

---

## Known Errors and Workarounds

### OpenGL context not available in tests
Integration tests require a display. On Linux CI, use `QT_QPA_PLATFORM=offscreen` or `Xvfb`.

```bash
QT_QPA_PLATFORM=offscreen ./inviwo --logconsole --quit
```

### Module not found at runtime
Ensure the module's CMake target was built and the module identifier matches the directory name (case-insensitive). Use `INVIWO_DISABLE_MODULES` env var to isolate modules:

```bash
INVIWO_DISABLE_MODULES="animation;animationqt" ./inviwo
```

### Linker errors with DLL exports
Every public class in a module needs the module's export macro (e.g., `IVW_MODULE_BASE_API`). The macro is defined in the module's `*moduledefine.h` file.

---

## Useful Links

- Build guide: https://inviwo.org/manual-gettingstarted-build.html
- Developer guide: https://inviwo.org/manual_index.html
- Python processor guide: https://inviwo.org/manual-devguide-python-processors.html
- C++ processor guide: https://inviwo.org/manual-devguide-build-processor.html
- API reference: https://inviwo.org/inviwo/doc
- Community modules: https://github.com/inviwo/modules
- Slack: https://join.slack.com/t/inviwo/...
- Changelog: [CHANGELOG.md](../CHANGELOG.md)
