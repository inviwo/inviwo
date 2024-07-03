# Inviwo BrushingAndLinking Module
ivw_module(BrushingAndLinking)

# Add header files
set(HEADER_FILES
    include/modules/brushingandlinking/brushingandlinkingmanager.h
    include/modules/brushingandlinking/brushingandlinkingmodule.h
    include/modules/brushingandlinking/brushingandlinkingmoduledefine.h
    include/modules/brushingandlinking/datastructures/brushingaction.h
    include/modules/brushingandlinking/datastructures/indexlist.h
    include/modules/brushingandlinking/ports/brushingandlinkingports.h
    include/modules/brushingandlinking/processors/brushingandlinkingprocessor.h
)
ivw_group("Header Files" ${HEADER_FILES})

# Add source files
set(SOURCE_FILES
    src/brushingandlinkingmanager.cpp
    src/brushingandlinkingmodule.cpp
    src/datastructures/brushingaction.cpp
    src/datastructures/indexlist.cpp
    src/ports/brushingandlinkingports.cpp
    src/processors/brushingandlinkingprocessor.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

# Add Unittests
set(TEST_FILES
	#${CMAKE_CURRENT_SOURCE_DIR}/tests/brushingandlinking-test.cpp
)
ivw_add_unittest(${TEST_FILES})

# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES})

if(IVW_ENABLE_PYTHON AND IVW_MODULE_PYTHON3)
    add_subdirectory(bindings)
endif()