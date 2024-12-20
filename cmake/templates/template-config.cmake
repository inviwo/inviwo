@PACKAGE_INIT@

include(CMakeFindDependencyMacro)

set_and_check(IVW_PREFIX @PACKAGE_IVW_INSTALL_PREFIX@)
set_and_check(IVW_INCLUDE_DIR @PACKAGE_IVW_INCLUDE_INSTALL_DIR@)
set_and_check(IVW_SHARE_DIR @PACKAGE_IVW_SHARE_INSTALL_DIR@)
set_and_check(IVW_RESOURCE_PREFIX @PACKAGE_IVW_RESOURCE_INSTALL_PREFIX@)

@PRECONFIG@

@PACKAGES@

include("${CMAKE_CURRENT_LIST_DIR}/@NAME@-targets.cmake")

@POSTCONFIG@

check_required_components(@NAME@)
