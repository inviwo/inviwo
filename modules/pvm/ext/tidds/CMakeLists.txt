if (BUILD_SHARED_LIBS)
else()
	message(WARNING "tidds is published under LGPL. Shared Liberary build will be enforced for tidds")
endif()

# needs to be always used as a shared library due to LGPL of TinyDDS
set(BUILD_SHARED_LIBS ON) 
add_subdirectory(tidds)

#--------------------------------------------------------------------
# Supress warnings
ivw_suppress_compiler_warnings(tidds)

#--------------------------------------------------------------------
# Define defintions
ivw_define_standard_definitions(tidds tidds)

#--------------------------------------------------------------------
# Creates VS folder structure
ivw_folder(tidds ext)

#--------------------------------------------------------------------
# Make package (for other projects to find)
ivw_default_install_targets(tidds)
ivw_make_package(TiDDS tidds)
