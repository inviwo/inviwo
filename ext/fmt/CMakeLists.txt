add_subdirectory(fmt)

ivw_default_install_targets(fmt)	
ivw_move_targets_in_dir_to_folder(fmt ext)
ivw_register_package(fmt fmt)
