ivw_module(Python3MCP)

set(HEADER_FILES
    include/inviwo/python3mcp/python3mcpmodule.h
    include/inviwo/python3mcp/python3mcpmoduledefine.h
)
ivw_group("Header Files" ${HEADER_FILES})

set(SOURCE_FILES
    src/python3mcpmodule.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

# Add script files
set(SCRIPT_FILES
    scripts/inviwomcp/__init__.py
    scripts/inviwomcp/inviwomcp.py
)
ivw_group("Script Files" ${SCRIPT_FILES})

set(TEST_FILES
    tests/unittests/python3mcp-unittest-main.cpp
)
ivw_add_unittest(${TEST_FILES})

ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SCRIPT_FILES})

# Add shader directory to install package
#ivw_add_to_module_pack(${CMAKE_CURRENT_SOURCE_DIR}/glsl)
set_property(GLOBAL APPEND PROPERTY IVW_PYTHON_ADDITIONAL_MODULES "mcp[cli]")
set_property(GLOBAL APPEND PROPERTY IVW_PYTHON_ADDITIONAL_MODULES fastmcp)
set_property(GLOBAL APPEND PROPERTY IVW_PYTHON_ADDITIONAL_MODULES pywin32)
set_property(GLOBAL APPEND PROPERTY IVW_PYTHON_ADDITIONAL_MODULES pypiwin32)
set_property(GLOBAL APPEND PROPERTY IVW_PYTHON_ADDITIONAL_MODULES anyio)