#pragma once

#include <modules/opengl/shader/shadermanager.h>
#include <filesystem>

@ADD_INCLUDES@

namespace inviwo {

namespace @L_MODULE@ {

inline void addGeneratedShaderResources([[maybe_unused]] ShaderManager* manager) {
@ADD_RESOURCES@
}

inline void addShaderResources(ShaderManager* manager, std::vector<std::filesystem::path> includePaths) {
#ifdef @U_MODULE@_INCLUDE_SHADER_RESOURCES
    addGeneratedShaderResources(manager);
#else
    for (auto& includePath : includePaths) {
        manager->addShaderSearchPath(includePath);
    }
#endif
}

}  // namespace

}  // namespace
