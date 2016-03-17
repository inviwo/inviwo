#ifndef @U_MODULE@_SHADER_RESOURCE
#define @U_MODULE@_SHADER_RESOURCE

#include <modules/opengl/shader/shadermanager.h>

@ADD_INCLUDES@

namespace inviwo {

inline void @U_MODULE@_addGeneratedShaderResources(ShaderManager* manager) {
@ADD_RESOURCES@
}

inline void @U_MODULE@_addShaderResources(ShaderManager* manager, std::vector<std::string> includePaths) {
#ifdef @U_MODULE@_INCLUDE_SHADER_RESOURCES
    @U_MODULE@_addGeneratedShaderResources(manager);
#else
    for (auto& includePath : includePaths) {
        manager->addShaderSearchPath(includePath);
    }
#endif
}

}

#endif
