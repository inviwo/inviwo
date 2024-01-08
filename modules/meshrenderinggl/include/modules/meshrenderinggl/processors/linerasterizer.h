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

#include <modules/meshrenderinggl/meshrenderingglmoduledefine.h>  // for IVW_MODULE_MESHRENDERI...

#include <inviwo/core/ports/meshport.h>                            // for MeshFlatMultiInport
#include <inviwo/core/processors/processor.h>                      // for Processor
#include <inviwo/core/processors/processorinfo.h>                  // for ProcessorInfo
#include <inviwo/core/properties/boolproperty.h>                   // for BoolProperty
#include <inviwo/core/properties/ordinalproperty.h>                // for FloatProperty, FloatVe...
#include <inviwo/core/util/document.h>                             // for Document
#include <inviwo/core/util/glmmat.h>                               // for mat4
#include <inviwo/core/util/glmvec.h>                               // for ivec2
#include <modules/base/properties/transformlistproperty.h>         // for TransformListProperty
#include <modules/basegl/properties/linesettingsproperty.h>        // for LineSettingsProperty
#include <modules/meshrenderinggl/datastructures/rasterization.h>  // for Rasterization
#include <modules/meshrenderinggl/ports/rasterizationport.h>       // for RasterizationOutport

#include <functional>  // for function
#include <memory>      // for shared_ptr
#include <vector>      // for vector

namespace inviwo {
class Mesh;
class MeshShaderCache;
class Shader;

/** \docpage{org.inviwo.LineRasterizer, Line Rasterizer}
 * ![](org.inviwo.LineRasterizer.png?classIdentifier=org.inviwo.LineRasterizer)
 * Render input meshes as lines, allows for order-independent transparency.
 *
 * ### Inports
 *   * __geometry__ Input meshes
 *
 * ### Outports
 *   * __rasterization__ rasterization functor rendering either opaquely or into fragment buffer
 *
 * ### Properties
 *   * __Mesh Transform__ Additional world/model transform applied to all input lines
 *   * __Line Width__  width of the rendered lines (in pixel)
 *   * __Antialising__ width of the antialiased line edge (in pixel), this determines the
 *                     softness along the edge
 *   * __Miter Limit__ limit for cutting of sharp corners
 *   * __Round Caps__  if enabled, round caps are drawn at the end of each line
 *   * __Pseudo Lighting__      enables radial shading as depth cue, i.e. tube like appearance
 *   * __Round Depth Profile__  modify line depth matching a round depth profile
 *   * __Shade opaque__    use simple depth checks instead of fragment lists
 */

/**
 * \class LineRasterizer
 * \brief Renders input geometry with 2D lines
 */
class IVW_MODULE_MESHRENDERINGGL_API LineRasterizer : public Processor {
    friend class LineRasterization;

public:
    LineRasterizer();
    virtual ~LineRasterizer() = default;

    virtual void initializeResources() override;
    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    // Call whenever PseudoLighting or RoundDepthProfile, or Stippling mode change
    void configureAllShaders();
    void configureShader(Shader& shader);
    void setUniforms(Shader& shader) const;

    MeshFlatMultiInport inport_;
    RasterizationOutport outport_;
    LineSettingsProperty lineSettings_;
    BoolProperty forceOpaque_;

    BoolProperty overwriteColor_;
    FloatVec4Property constantColor_;
    BoolProperty useUniformAlpha_;
    FloatProperty uniformAlpha_;

    TransformListProperty transformSetting_;
    std::shared_ptr<MeshShaderCache> lineShaders_;
};

/**
 * \brief Functor object that will render lines into a fragment list.
 */
class IVW_MODULE_MESHRENDERINGGL_API LineRasterization : public Rasterization {
public:
    /**
     * \brief Copy all settings and the shader to hand to a renderer.
     */
    LineRasterization(const LineRasterizer& rasterizerProcessor);
    virtual void rasterize(const ivec2& imageSize, const mat4& worldMatrixTransform,
                           std::function<void(Shader&)> setUniforms) const override;
    virtual bool usesFragmentLists() const override;
    virtual Document getInfo() const override;

protected:
    std::shared_ptr<MeshShaderCache> lineShaders_;

    std::vector<std::shared_ptr<const Mesh>> meshes_;

    const bool forceOpaque_;
};

}  // namespace inviwo
