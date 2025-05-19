/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2025 Inviwo Foundation
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

#include <modules/oit/oitmoduledefine.h>  // for IVW_MODULE_MESHRENDERI...

#include <inviwo/core/ports/meshport.h>                      // for MeshFlatMultiInport
#include <inviwo/core/processors/processor.h>                // for Processor
#include <inviwo/core/processors/processorinfo.h>            // for ProcessorInfo
#include <inviwo/core/properties/boolproperty.h>             // for BoolProperty
#include <inviwo/core/properties/ordinalproperty.h>          // for FloatProperty, FloatVe...
#include <inviwo/core/util/document.h>                       // for Document
#include <inviwo/core/util/glmmat.h>                         // for mat4
#include <inviwo/core/util/glmvec.h>                         // for ivec2
#include <modules/base/properties/transformlistproperty.h>   // for TransformListProperty
#include <modules/basegl/properties/linesettingsproperty.h>  // for LineSettingsProperty
#include <modules/basegl/datastructures/meshshadercache.h>   // for MeshShaderC...
#include <modules/oit/datastructures/rasterization.h>        // for Rasterization
#include <modules/oit/ports/rasterizationport.h>             // for RasterizationOutport
#include <modules/oit/processors/rasterizer.h>

#include <functional>  // for function
#include <memory>      // for shared_ptr
#include <vector>      // for vector

namespace inviwo {
class Mesh;
class MeshShaderCache;
class Shader;

/**
 * @brief Renders input geometry with 2D lines
 */
class IVW_MODULE_OIT_API LineRasterizer : public Rasterizer {
    friend class LineRasterization;

public:
    LineRasterizer();
    virtual ~LineRasterizer() = default;

    virtual void initializeResources() override;

    virtual const ProcessorInfo& getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    virtual void rasterize(const ivec2& imageSize, const mat4& worldMatrixTransform) override;
    virtual UseFragmentList usesFragmentLists() const override;

    virtual std::optional<mat4> boundingBox() const override;

    virtual Document getInfo() const override;

private:
    virtual void configureShader(Shader& shader) override;
    virtual void setUniforms(Shader& shader) override;

    MeshFlatMultiInport inport_;
    LineSettingsProperty lineSettings_;
    BoolProperty forceOpaque_;

    BoolProperty overwriteColor_;
    FloatVec4Property constantColor_;
    BoolProperty useUniformAlpha_;
    FloatProperty uniformAlpha_;

    MeshShaderCache lineShaders_;
};

}  // namespace inviwo
