/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2024 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#pragma once

#include <modules/basegl/baseglmoduledefine.h>  // for IVW_MODULE_BASEGL_API

#include <inviwo/core/util/glmvec.h>                        // for size2_t
#include <modules/basegl/datastructures/linesettings.h>     // for LineSettings
#include <modules/basegl/datastructures/meshshadercache.h>  // for MeshShaderCache
#include <modules/opengl/shader/shaderutils.h>
#include <inviwo/core/datastructures/tflookuptable.h>

namespace inviwo {

class Camera;
class LineSettingsInterface;
class Mesh;
class Shader;

namespace algorithm {

/**
 * \class LineRenderer
 * \brief Helper class for rendering a mesh as lines.
 * Only renders Mesh with DrawType::Lines
 */
class IVW_MODULE_BASEGL_API LineRenderer {
public:
    explicit LineRenderer(const LineSettingsInterface* settings);
    LineRenderer(const std::vector<MeshShaderCache::Requirement>& requirements,
                 const LineSettingsInterface* settings);
    ~LineRenderer() = default;

    /**
     * \brief Render lines according to currently set LineSettingsInterface settings
     * Only meshes with DrawType::Lines will be rendered.
     *
     * @param mesh to render as lines, must
     * @param camera for projection
     * @param screenDim width, height in pixels
     * @param settings @see LineSettingsInterface
     */
    void render(const Mesh& mesh, const Camera& camera, size2_t screenDim,
                const LineSettingsInterface* settings);

    template <typename... T>
    void renderWithUniforms(const Mesh& mesh, const Camera& camera, size2_t screenDim,
                            const LineSettingsInterface* settings, const T&... args) {
        render(mesh, camera, screenDim, settings,
               [&](Shader& shader) { utilgl::setUniforms(shader, args...); });
    }

protected:
    void render(const Mesh& mesh, const Camera& camera, size2_t screenDim,
                const LineSettingsInterface* settings, const std::function<void(Shader&)>& func);
    // Call whenever PseudoLighting or RoundDepthProfile, or Stippling mode change
    void configureShaders();
    void setUniforms(Shader& shader, const Mesh& mesh, const Camera& camera, size2_t screenDim,
                     const std::function<void(Shader&)>& func);
    void configureShader(Shader& shader);
    LineSettings settings_;  //!< Local cache
    MeshShaderCache lineShaders_;
    TFLookupTable tfLookup_;
};

}  // namespace algorithm

}  // namespace inviwo
