/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2023 Inviwo Foundation
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

#include <inviwo/core/datastructures/light/lightingstate.h>        // for LightingState
#include <inviwo/core/interaction/cameratrackball.h>               // for CameraTrackball
#include <inviwo/core/ports/meshport.h>                            // for MeshFlatMultiInport
#include <inviwo/core/processors/processor.h>                      // for Processor
#include <inviwo/core/processors/processorinfo.h>                  // for ProcessorInfo
#include <inviwo/core/properties/boolproperty.h>                   // for BoolProperty
#include <inviwo/core/properties/cameraproperty.h>                 // for CameraProperty
#include <inviwo/core/properties/compositeproperty.h>              // for CompositeProperty
#include <inviwo/core/properties/optionproperty.h>                 // for OptionProperty
#include <inviwo/core/properties/ordinalproperty.h>                // for FloatProperty, FloatVe...
#include <inviwo/core/properties/simplelightingproperty.h>         // for SimpleLightingProperty
#include <inviwo/core/properties/transferfunctionproperty.h>       // for TransferFunctionProperty
#include <inviwo/core/util/document.h>                             // for Document
#include <inviwo/core/util/glmmat.h>                               // for mat4
#include <inviwo/core/util/glmvec.h>                               // for ivec2, vec4
#include <modules/base/properties/transformlistproperty.h>         // for TransformListProperty
#include <modules/meshrenderinggl/datastructures/rasterization.h>  // for Rasterization
#include <modules/meshrenderinggl/ports/rasterizationport.h>       // for RasterizationOutport

#include <modules/basegl/util/meshbnlgl.h>
#include <modules/basegl/util/uniformlabelatlasgl.h>
#include <modules/basegl/util/periodicitygl.h>
#include <modules/basegl/util/sphereconfig.h>
#include <modules/basegl/util/glyphclipping.h>
#include <modules/basegl/util/meshtexturing.h>

#include <functional>   // for __base, function
#include <memory>       // for shared_ptr
#include <string>       // for operator==, operator+
#include <string_view>  // for operator==
#include <vector>       // for operator!=, vector

namespace inviwo {
class Layer;
class Mesh;
class MeshShaderCache;
class Shader;

class IVW_MODULE_MESHRENDERINGGL_API SphereRasterizer : public RasterizationProcessor {
    friend class SphereRasterization;

public:
    SphereRasterizer();
    virtual ~SphereRasterizer() = default;

    virtual void initializeResources() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    virtual Document getInfo() const override;
    virtual std::optional<mat4> boundingBox() const override;
    virtual bool usesFragmentLists() const override;
    virtual void rasterize(const ivec2& imageSize, const mat4& worldMatrixTransform,
                           std::function<void(Shader&)> setUniforms,
                           std::function<void(Shader&)> initializeShader) override;

private:
    void configureShader(Shader& shader);
    void configureOITShader(Shader& shader);

    enum class RenderMode {
        EntireMesh,  //!< render all vertices of the input mesh as glyphs
        PointsOnly,  //!< render only parts of mesh with DrawType::Points
    };

    MeshFlatMultiInport inport_;

    OptionProperty<RenderMode> renderMode_;

    BoolProperty forceOpaque_;

    MeshBnLGL bnl_;
    GlyphClipping clip_;
    SphereConfig config_;
    UniformLabelAtlasGL labels_;
    PeriodicityGL periodic_;
    MeshTexturing texture_;

    MeshShaderCache shaders_;
};

}  // namespace inviwo
