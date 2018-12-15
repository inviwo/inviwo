add_subdirectory(fmt)

if(${IVW_PACKAGE_PROJECT}) # remove this check when we require 3.13
    # Possible to add install target from sub dirs from version 3.13
    # Avoids editing fmt's cmake file
    cmake_minimum_required(VERSION 3.13.0)
    ivw_default_install_comp_targets(core fmt)	
endif()

ivw_move_targets_in_dir_to_folder(fmt ext)