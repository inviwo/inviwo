project(opencl)

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    opencl.cpp
)

#--------------------------------------------------------------------
# Define libraries that should be linked
ivw_retrieve_all_modules(package_list)
list(APPEND package_list InviwoCore)

#--------------------------------------------------------------------
# Create test
ivw_create_test(${SOURCE_FILES})

#--------------------------------------------------------------------
# Add dependencies
ivw_add_module_dependencies(sometarget ${package_list})
