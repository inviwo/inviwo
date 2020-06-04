# Adapted settings from openexr/CMakeLists.txt and others, such as /openexr/OpenEXR/config/OpenEXRSetup.cmake

set(PYILMBASE_ENABLE             OFF CACHE INTERNAL "Enables configuration of the PyIlmBase module" FORCE)
set(OPENEXR_VIEWERS_ENABLE       OFF CACHE INTERNAL "Enables configuration of the viewers module" FORCE)
set(BUILD_TESTING          	     OFF CACHE INTERNAL "Enable the tests" FORCE)
set(OPENEXR_BUILD_UTILS          OFF CACHE INTERNAL "Enables building of utility programs" FORCE)
set(INSTALL_OPENEXR_EXAMPLES     OFF CACHE INTERNAL "Install OpenEXR examples" FORCE)
set(INSTALL_OPENEXR_DOCS         OFF CACHE INTERNAL "Install OpenEXR documentation" FORCE)

add_subdirectory(openexr)

ivw_move_targets_in_dir_to_folder(${CMAKE_CURRENT_SOURCE_DIR} ext/openexr)
ivw_suppress_warnings_for_targets_in_dir(${CMAKE_CURRENT_SOURCE_DIR})

#--------------------------------------------------------------------
# Make package (for other projects to find)
ivw_make_package(IlmBase IlmBase::Half
                         IlmBase::Iex
                         IlmBase::IexMath
                         IlmBase::Imath
                         IlmBase::IlmThread)
ivw_make_package(OpenEXR OpenEXR::IlmImf
                         OpenEXR::IlmImfUtil)

