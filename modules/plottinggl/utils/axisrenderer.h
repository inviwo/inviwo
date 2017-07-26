/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017 Inviwo Foundation
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

#ifndef IVW_AXISRENDERER_H
#define IVW_AXISRENDERER_H

#include <modules/plottinggl/plottingglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <inviwo/core/properties/optionproperty.h>

#include <modules/plotting/properties/axisproperty.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/rendering/texturequadrenderer.h>
#include <modules/fontrendering/textrenderer.h>
#include <modules/fontrendering/util/textureatlas.h>

#include <map>

namespace inviwo {

class Camera;
class Mesh;
class Texture2D;

namespace plot {

class IVW_MODULE_PLOTTINGGL_API AxisRendererBase {
public:
    AxisRendererBase(const AxisProperty& property);
    AxisRendererBase(const AxisRendererBase& rhs);
    AxisRendererBase& operator=(const AxisRendererBase& rhs) = default;
    virtual ~AxisRendererBase() = default;

    std::shared_ptr<Mesh> getMesh() const;
    std::shared_ptr<Texture2D> getLabelAtlasTexture() const;

protected:
    void renderAxis(Camera* camera, const size2_t& outputDims, bool antialiasing);

    void invalidateInternalState(bool positionChange);

    void updateCaptionTexture();
    void updateLabelAtlas();

    virtual void invalidateLabelPositions() = 0;

    const AxisProperty& property_;

    TextRenderer textRenderer_;
    TextureQuadRenderer quadRenderer_;

    Shader lineShader_;

    std::shared_ptr<Mesh> axisMesh_;
    std::shared_ptr<Mesh> majorTicksMesh_;
    std::shared_ptr<Mesh> minorTicksMesh_;

    std::shared_ptr<Texture2D> axisCaptionTex_;
    
    util::TextureAtlas labelTexAtlas_;
};

class IVW_MODULE_PLOTTINGGL_API AxisRenderer : public AxisRendererBase {
public:
    AxisRenderer(const AxisProperty& property);
    AxisRenderer(const AxisRenderer& rhs);
    AxisRenderer& operator=(const AxisRenderer& rhs) = default;
    virtual ~AxisRenderer() = default;

    void render(const size2_t& outputDims, const size2_t& startPos, const size2_t& endPos,
                bool antialiasing = true);

private:
    void updateMeshes(const size2_t& startPos, const size2_t& endPos);

    void renderText(const size2_t& outputDims, const size2_t& startPos, const size2_t& endPos);

    virtual void invalidateLabelPositions() override;
    void updateLabelPositions(const size2_t& startPos, const size2_t& endPos);

    std::vector<ivec2> labelPos_;

    size2_t prevStartPos_;
    size2_t prevEndPos_;
};

class IVW_MODULE_PLOTTINGGL_API AxisRenderer3D : public AxisRendererBase {
public:
    AxisRenderer3D(const AxisProperty& property);
    AxisRenderer3D(const AxisRenderer3D& rhs);
    AxisRenderer3D& operator=(const AxisRenderer3D& rhs) = default;
    virtual ~AxisRenderer3D() = default;

    void render(Camera* camera, const size2_t& outputDims, const vec3& startPos, const vec3& endPos,
                const vec3& tickDirection, bool antialiasing = true);

private:
    void updateMeshes(const vec3& startPos, const vec3& endPos, const vec3& tickDirection);

    void renderText(Camera* camera, const size2_t& outputDims, const vec3& startPos,
                    const vec3& endPos, const vec3& tickDirection);

    virtual void invalidateLabelPositions() override;
    void updateLabelPositions(const vec3& startPos, const vec3& endPos, const vec3& tickDirection);

    std::vector<vec3> labelPos_;

    vec3 prevStartPos_;
    vec3 prevEndPos_;
    vec3 prevTickDirection_;
};

}  // namespace plot

}  // namespace inviwo

#endif  // IVW_AXISRENDERER_H
