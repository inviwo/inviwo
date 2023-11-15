project(opencl)

# Add source files
set(SOURCE_FILES
    opencl.cpp
)

# Define libraries that should be linked
set(package_list InviwoCore InviwoOpenCLModule)

# Create test
ivw_create_test(${SOURCE_FILES})

# Add dependencies
ivw_add_module_dependencies(sometarget ${package_list})
