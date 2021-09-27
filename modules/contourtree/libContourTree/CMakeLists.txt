#--------------------------------------------------------------------
project(contour-tree)

#--------------------------------------------------------------------

# Add header files
set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/constants.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ContourTree.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ContourTreeData.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/DisjointSets.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Grid3D.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/HyperVolume.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MergeTree.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Persistence.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ScalarFunction.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SimFunction.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SimplifyCT.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/TopologicalFeatures.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/TriMesh.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ContourTree.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ContourTreeData.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/HyperVolume.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MergeTree.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Persistence.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/SimplifyCT.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TopologicalFeatures.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TriMesh.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

find_package(OpenMP REQUIRED)

#--------------------------------------------------------------------
# Create library
add_library(contour-tree STATIC ${SOURCE_FILES} ${HEADER_FILES})
add_library(inviwo::contour-tree ALIAS contour-tree)

target_include_directories(contour-tree PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(contour-tree INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

#--------------------------------------------------------------------
# Define defintions
ivw_define_standard_definitions(contour-tree contour-tree)
ivw_define_standard_properties(contour-tree)

#--------------------------------------------------------------------
# Creates VS folder structure
ivw_folder(contour-tree ext)

#--------------------------------------------------------------------
# Make package (for other projects to find)
ivw_default_install_comp_targets(core contour-tree)
ivw_make_package(ContourTree contour-tree)

ivw_suppress_compiler_warnings(contour-tree)

