set(UNITS_ENABLE_TESTS OFF)
set(UNITS_BUILD_CONVERTER_APP OFF)
set(UNITS_BUILD_WEB_SERVER OFF)
set(UNITS_BUILD_FUZZ_TARGETS OFF)
set(UNITS_ENABLE_ERROR_ON_WARNINGS OFF)
set(UNITS_ENABLE_EXTRA_COMPILER_WARNINGS ON)

add_subdirectory(units)

target_compile_definitions(units PUBLIC ENABLE_UNIT_MAP_ACCESS)
ivw_suppress_compiler_warnings(units)
ivw_move_targets_in_dir_to_folder(units ext)

ivw_register_package(units units)
ivw_install_helper(TARGET units NAMESPACE units DESTINATION inviwo)