#pragma once
// Automatically generated file do not change!

#include <inviwo/core/common/inviwomodule.h>

#define RESOURCE_ARRAY_SIZE @IVW_RESOURCE_ARRAY_SIZE@
std::string @IVW_RESOURCE_ARRAY_NAME@[RESOURCE_ARRAY_SIZE] = { @IVW_RESOURCE_PATHS@ };

@SHADER_HEADERS@
namespace inviwo {
void @SHADER_RESOURCES_REGISTER_FUNCTION@(InviwoModule* module) {
    @SHADER_RESOURCES_PATHS@
}
}
