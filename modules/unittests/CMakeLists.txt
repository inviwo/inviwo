
#--------------------------------------------------------------------
# InviwoUnitTests Module
ivw_module(UnitTests)

add_definitions(-DGTEST_HAS_TR1_TUPLE=0)

add_subdirectory(ext/gtest-1.7.0)


if(BUILD_SHARED_LIBS)
    add_definitions(-DGTEST_LINKED_AS_SHARED_LIBRARY=1)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ext/gtest-1.7.0/include)
include_directories(${CMAKE_BINARY_DIR}/modules/_generated)
ivw_link_directories(${LIBRARY_OUTPUT_PATH})


ivw_retrieve_all_modules(package_list)

foreach(module ${package_list})
    string(TOUPPER ${module} u_module)
    if(u_module MATCHES "QT+")
        list(REMOVE_ITEM package_list ${module})
    elseif(u_module MATCHES "VRN+")
        list(REMOVE_ITEM package_list ${module})
    elseif(u_module MATCHES "UNITTESTS+")
        list(REMOVE_ITEM package_list ${module})
    elseif(u_module MATCHES "GLFW+")
        list(REMOVE_ITEM package_list ${module})
    else()
        ivw_mod_name_to_dir(libraryname ${module})
        add_dependency_libs_to_module(inviwo-module-${libraryname})
    endif()
endforeach()
list(APPEND package_list InviwoCore)

#--------------------------------------------------------------------
# Register the use of modules
ivw_register_use_of_modules(${package_list})

#--------------------------------------------------------------------
# Need to add dependent directories before creating application
ivw_add_dependency_directories(${package_list})
ivw_register_use_of_modules(${package_list})


#configure_file(${CMAKE_BINARY_DIR}/modules/_generated/unittests_temp.h ${CMAKE_BINARY_DIR}/modules/_generated/unittests.h)


#--------------------------------------------------------------------
# Add header files

set(HEADER_FILES
#    ${CMAKE_BINARY_DIR}/modules/_generated/unittests.h
    ${CMAKE_CURRENT_SOURCE_DIR}/logerrorcounter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/unittestsmodule.h
    ${CMAKE_CURRENT_SOURCE_DIR}/unittestsmoduledefine.h
)

ivw_group("Header Files" ${HEADER_FILES})


## remove non-existing unit test files
foreach (item ${unittest_files})
    if (NOT EXISTS "${item}")
        # unit test does no longer exist, remove from list
        #ivw_message("  does not exist, removing ${item}")
        list(REMOVE_ITEM unittest_files ${item})
    endif()
endforeach()
## update cache internal variable
set(unittest_files ${unittest_files} CACHE INTERNAL "Unit test files")


#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    ${unittest_files}
    ${CMAKE_CURRENT_SOURCE_DIR}/logerrorcounter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/unittestsmodule.cpp
)

##ivw_add_tmp_file(${CMAKE_BINARY_DIR}/modules/_generated/unittests_temp.h)

ivw_group("Source Files" ${SOURCE_FILES})

if(IVW_MODULE_OPENCL)
    ivw_add_definition(__CL_ENABLE_EXCEPTIONS)
endif()


add_dependency_libs_to_module(gtest)

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES})


