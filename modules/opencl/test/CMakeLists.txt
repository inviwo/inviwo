ivw_project(opencl)

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/test/opencl.cpp
)

#--------------------------------------------------------------------
# Define libraries that should be linked
ivw_retrieve_all_modules(enabled_modules)
list(APPEND enabled_modules InviwoCore)

#--------------------------------------------------------------------
# Need to add dependent directories before creating application
ivw_add_dependency_directories(${enabled_modules})

#--------------------------------------------------------------------
# Create test
ivw_create_test(${SOURCE_FILES})

#--------------------------------------------------------------------
# Add dependencies
ivw_add_dependencies(${enabled_modules})
