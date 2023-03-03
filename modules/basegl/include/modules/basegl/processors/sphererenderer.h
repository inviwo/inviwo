/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2023 Inviwo Foundation
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

#include <modules/basegl/baseglmoduledefine.h>  // for IVW_MODULE_BASEGL...

#include <inviwo/core/interaction/cameratrackball.h>        // for CameraTrackball
#include <inviwo/core/ports/imageport.h>                    // for ImageInport, Imag...
#include <inviwo/core/ports/meshport.h>                     // for MeshFlatMultiInport
#include <inviwo/core/processors/processor.h>               // for Processor
#include <inviwo/core/processors/processorinfo.h>           // for ProcessorInfo
#include <inviwo/core/properties/cameraproperty.h>          // for CameraProperty
#include <inviwo/core/properties/optionproperty.h>          // for OptionProperty
#include <inviwo/core/properties/simplelightingproperty.h>  // for SimpleLightingPro...

#include <modules/basegl/datastructures/meshshadercache.h>  // for MeshShaderCache
#include <modules/basegl/util/meshbnlgl.h>
#include <modules/basegl/util/uniformlabelatlasgl.h>
#include <modules/basegl/util/periodicitygl.h>
#include <modules/basegl/util/sphereconfig.h>
#include <modules/basegl/util/glyphclipping.h>
#include <modules/basegl/util/meshtexturing.h>

namespace inviwo {

class Shader;

/**
 * \brief Renders input geometry with 3D sphere glyphs using OpenGL shaders
 */
class IVW_MODULE_BASEGL_API SphereRenderer : public Processor {
public:
    SphereRenderer();
    virtual ~SphereRenderer() = default;
    SphereRenderer(const SphereRenderer&) = delete;
    SphereRenderer(SphereRenderer&&) = delete;
    SphereRenderer& operator=(const SphereRenderer&) = delete;
    SphereRenderer& operator=(SphereRenderer&&) = delete;

    virtual void process() override;
    virtual void initializeResources() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    void configureShader(Shader& shader);

    enum class RenderMode {
        EntireMesh,  //!< render all vertices of the input mesh as glyphs
        PointsOnly,  //!< render only parts of mesh with DrawType::Points
    };

    MeshFlatMultiInport inport_;
    ImageInport imageInport_;
    ImageOutport outport_;

    OptionProperty<RenderMode> renderMode_;
    MeshBnLGL bnl_;
    GlyphClipping clip_;
    SphereConfig config_;
    UniformLabelAtlasGL labels_;
    PeriodicityGL periodic_;
    MeshTexturing texture_;

    CameraProperty camera_;
    CameraTrackball trackball_;
    SimpleLightingProperty lighting_;

    MeshShaderCache shaders_;
};

}  // namespace inviwo
