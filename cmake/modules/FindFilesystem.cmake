include(FindPackageHandleStandardArgs)
include(CMakePushCheckState)
include(CheckIncludeFileCXX)
include(CheckCXXSourceCompiles)

cmake_push_check_state(RESET)
set(CMAKE_CXX_STANDARD 17)
check_include_file_cxx("filesystem" filesystem_exists)
check_include_file_cxx("experimental/filesystem" experimental_filesystem_exists)
if(filesystem_exists)
    set(filesystem_headers_found TRUE)
    set(filesystem_header filesystem)
    set(filesystem_namespace std::filesystem)
elseif(experimental_filesystem_exists)
    if("experimental" IN_LIST Filesystem_FIND_COMPONENTS)
        set(filesystem_headers_found TRUE)
        set(filesystem_header experimental/filesystem)
        set(filesystem_namespace std::experimental::filesystem)
    endif()
endif()

if(filesystem_headers_found)
    string(CONFIGURE [[
        #include <@filesystem_header@>
        int main() {
            auto cwd = @filesystem_namespace@::current_path();
            return cwd.string().size();
        }
        ]] code @ONLY
    )

    check_cxx_source_compiles("${code}" filesystem_no_link_needed)
    set(CMAKE_REQUIRED_LIBRARIES -lstdc++fs)
    check_cxx_source_compiles("${code}" filesystem_stdcppfs_needed)
    set(CMAKE_REQUIRED_LIBRARIES -lc++fs)
    check_cxx_source_compiles("${code}" filesystem_cppfs_needed)

    if(filesystem_no_link_needed OR filesystem_stdcppfs_needed OR filesystem_cppfs_needed)
        set(filesystem_can_link TRUE)
        add_library(std::filesystem INTERFACE IMPORTED GLOBAL)
        target_compile_definitions(std::filesystem
            INTERFACE STD_FS_IS_EXPERIMENTAL=$<NOT:$<BOOL:${filesystem_exists}>>
        )
        if(filesystem_no_link_needed)
            # do nothing (avoid adding links it multiple works)
        elseif(filesystem_stdcppfs_needed)
            target_link_libraries(std::filesystem INTERFACE -lstdc++fs)
        elseif(filesystem_cppfs_needed)
            target_link_libraries(std::filesystem INTERFACE -lc++fs)
        endif()
    endif()
endif()
cmake_pop_check_state()

find_package_handle_standard_args(Filesystem
    REQUIRED_VARS
        filesystem_headers_found
        filesystem_can_link
)
