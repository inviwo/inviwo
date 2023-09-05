if (NOT BUILD_SHARED_LIBS)
	message(STATUS "tidds is published under LGPL. Shared Liberary build will be enforced for tidds")
endif()

# needs to be always used as a shared library due to LGPL of TinyDDS
set(BUILD_SHARED_LIBS ON) 
add_subdirectory(tidds)

ivw_suppress_compiler_warnings(tidds)
ivw_define_standard_definitions(tidds tidds)

# Creates VS folder structure
ivw_folder(tidds ext)

# Make package (for other projects to find)
add_library(tidds::tidds ALIAS tidds)
ivw_register_package(tidds tidds)
ivw_install_helper(TARGET tidds NAMESPACE tidds DESTINATION inviwo)
