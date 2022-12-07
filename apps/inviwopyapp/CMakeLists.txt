# Inviwo Qt Application
project(inviwopyapp)

# Add source files
set(PYTHON_FILES
    inviwo.py
    richdoc.py
)
ivw_group("Python Files" ${PYTHON_FILES})

set(CMAKE_FILES
    CMakeLists.txt
)
ivw_group("CMake Files" ${CMAKE_FILES})

ivw_retrieve_all_modules(enabled_modules)
# Only include one OpenGL context creation module
list(REMOVE_ITEM enabled_modules InviwoGLFWModule)

# Create application
add_custom_target(inviwopyapp SOURCES ${PYTHON_FILES} ${CMAKE_FILES})

# Move to folder
ivw_folder(inviwopyapp minimals)