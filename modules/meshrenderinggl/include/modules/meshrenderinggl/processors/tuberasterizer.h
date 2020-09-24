/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020 Inviwo Foundation
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

#include <modules/meshrenderinggl/meshrenderingglmoduledefine.h>
#include <modules/meshrenderinggl/datastructures/rasterization.h>
#include <modules/meshrenderinggl/ports/rasterizationport.h>
#include <inviwo/core/ports/meshport.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/simplelightingproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/transferfunctionproperty.h>
#include <modules/opengl/shader/shader.h>
#include <modules/base/properties/transformlistproperty.h>
#include <modules/basegl/datastructures/meshshadercache.h>

namespace inviwo {

/** \docpage{org.inviwo.TubeRasterizer, Tube Rasterizer}
 * ![](org.inviwo.TubeRasterizer.png?classIdentifier=org.inviwo.TubeRasterizer)
 * Rendering lines as tubes, supports oit.
 *
 * ### Inports
 *   * __Meshes__ Meshes to render
 *
 * ### Outports
 *   * __Rasterization__ Rasterization object, holds all data necessary to render.
 *
 * ### Properties
 *   * __<Prop1>__ <description>.
 *   * __<Prop2>__ <description>
 */
class IVW_MODULE_MESHRENDERINGGL_API TubeRasterizer : public Processor {
    friend class TubeRasterization;

public:
    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;
    TubeRasterizer();
    virtual ~TubeRasterizer() = default;

    virtual void initializeResources() override;
    virtual void process() override;

protected:
    void configureShader(Shader& shader);
    void setUniforms(Shader& shader) const;

    MeshFlatMultiInport inport_;
    RasterizationOutport outport_;

    CompositeProperty tubeProperties_;
    BoolProperty forceRadius_;
    FloatProperty defaultRadius_;
    BoolProperty forceColor_;
    FloatVec4Property defaultColor_;
    BoolProperty useMetaColor_;
    TransferFunctionProperty metaColor_;
    BoolProperty forceOpaque_;
    TransformListProperty transformSetting_;

    SimpleLightingProperty lighting_;
    std::vector<std::pair<ShaderType, std::string>> shaderItems_;
    std::vector<MeshShaderCache::Requirement> shaderRequirements_;
    std::shared_ptr<MeshShaderCache> adjacencyShaders_, shaders_;
};

/**
 * \brief Functor object that will render linesas tubes into a fragment list.
 */
class IVW_MODULE_MESHRENDERINGGL_API TubeRasterization : public Rasterization {
public:
    /**
     * \brief Copy all settings and the shader to hand to a renderer.
     */
    TubeRasterization(const TubeRasterizer& rasterizerProcessor);
    virtual void rasterize(const ivec2& imageSize, const mat4& worldMatrixTransform,
                           std::function<void(Shader&)> setUniforms) const override;
    virtual bool usesFragmentLists() const override;
    virtual Document getInfo() const override;
    virtual Rasterization* clone() const override;

protected:
    std::shared_ptr<MeshShaderCache> adjacencyShaders_, shaders_;
    std::vector<std::shared_ptr<const Mesh>> meshes_;

    const Layer* tfTexture_;
    const bool forceOpaque_;
};

}  // namespace inviwo
