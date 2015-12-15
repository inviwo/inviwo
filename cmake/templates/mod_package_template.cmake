# Package file for @package_name@, part of Inviwo projects
# It defines the following variables
#
# @package_name@_FOUND
# @package_name@_INCLUDE_DIR
# @package_name@_LIBRARY_DIR
# @package_name@_LIBRARIES
# @package_name@_DEFINITIONS
# @package_name@_LINK_FLAGS
# 

set(BUILD_@package_name@ 1)
set(@package_name@_PROJECT @_project_name@)
set(@package_name@_FOUND 1)
set(@package_name@_USE_FILE @_allIncludes@)
set(@package_name@_INCLUDE_DIR "@_allIncludeDirs@")
set(@package_name@_LIBRARY_DIR "@_allLibsDir@")
set(@package_name@_LIBRARIES "@_allLibs@")
set(@package_name@_DEFINITIONS @_allDefinitions@)
set(@package_name@_LINK_FLAGS @_allLinkFlags@)

mark_as_advanced(FORCE  @package_name@_FOUND)
mark_as_advanced(FORCE  @package_name@_USE_FILE)
mark_as_advanced(FORCE  @package_name@_INCLUDE_DIR)
mark_as_advanced(FORCE  @package_name@_LIBRARY_DIR)
mark_as_advanced(FORCE  @package_name@_LIBRARIES)
mark_as_advanced(FORCE  @package_name@_DEFINITIONS)
mark_as_advanced(FORCE  @package_name@_LINK_FLAGS)