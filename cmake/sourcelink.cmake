function(ivw_target_add_sourcelink)
    cmake_parse_arguments("ARG" "" "TARGET;REPO;REF;ROOT" "" ${ARGN})
    if(DEFINED ARG_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "ivw_target_add_sourcelink was passed extra arguments: ${ARG_UNPARSED_ARGUMENTS}")
    endif()
    foreach(required_arg IN ITEMS TARGET REPO REF)
        if(NOT DEFINED ARG_${required_arg})
            message(FATAL_ERROR "${required_arg} must be set")
        endif()
    endforeach()

    if(MSVC)
        string(SUBSTRING ${ARG_REPO} 0 19 test)
        if(NOT ${test} STREQUAL "https://github.com/")
            return()
        endif()
        # We need the the org/repo part.
        string(LENGTH "${ARG_REPO}" length)
        math(EXPR size "${length} - 4 - 19")
        string(SUBSTRING ${ARG_REPO} 19 ${size} repo)

        set(base_url "https://raw.githubusercontent.com/${repo}/${ARG_REF}/*")
        file(TO_NATIVE_PATH "${ARG_ROOT}/*" local_base)
        string(REPLACE [[\]] [[\\]] local_base "${local_base}")
        set(output_json_filename "${CMAKE_CURRENT_BINARY_DIR}/vcpkgsourcelink-${ARG_TARGET}.json")
        file(WRITE "${output_json_filename}"
"{
  \"documents\": {
    \"${local_base}\": \"${base_url}\"
  }
}"
        )
        file(TO_NATIVE_PATH "${output_json_filename}" native_json_filename)
        target_link_options(${ARG_TARGET} PRIVATE "/SOURCELINK:${native_json_filename}")
    endif()
endfunction()
