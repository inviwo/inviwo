if(NOT(IVW_INTEGRATION_TESTS OR IVW_UNITTESTS))
    return()
endif()

project(inviwo-testutil)

# Add source files
set(headers
	include/inviwo/testutil/configurablegtesteventlistener.h
	include/inviwo/testutil/zipmatcher.h
)
ivw_group("Header Files" BASE include/inviwo/testutil ${headers})

# Add source files
set(sources
    src/configurablegtesteventlistener.cpp
    src/zipmatcher.cpp
)
ivw_group("Source Files" BASE src ${sources})

# gtest does not link dynamically so only handle static lib
add_library(inviwo-testutil STATIC ${headers} ${sources})
add_library(inviwo::testutil ALIAS inviwo-testutil)
target_include_directories(inviwo-testutil PUBLIC 
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(inviwo-testutil PUBLIC
	inviwo::warn 
    gtest
    gmock
)

ivw_define_standard_properties(inviwo-testutil)
ivw_define_standard_definitions(inviwo-testutil inviwo-testutil)

ivw_default_install_comp_targets(core inviwo-testutil)
