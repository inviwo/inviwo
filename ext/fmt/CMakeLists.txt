set(FMT_INSTALL OFF)
add_subdirectory(fmt)

ivw_default_install_targets(fmt)
ivw_register_package(fmt fmt)

ivw_move_targets_in_dir_to_folder(fmt ext)