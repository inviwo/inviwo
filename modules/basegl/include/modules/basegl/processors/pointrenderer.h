/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2022 Inviwo Foundation
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

#include <modules/basegl/baseglmoduledefine.h>        // for IVW_MODULE_BASEGL_API

#include <inviwo/core/interaction/cameratrackball.h>  // for CameraTrackball
#include <inviwo/core/ports/imageport.h>              // for ImageInport, ImageOutport
#include <inviwo/core/ports/meshport.h>               // for MeshFlatMultiInport
#include <inviwo/core/processors/processor.h>         // for Processor
#include <inviwo/core/processors/processorinfo.h>     // for ProcessorInfo
#include <inviwo/core/properties/cameraproperty.h>    // for CameraProperty
#include <inviwo/core/properties/ordinalproperty.h>   // for FloatProperty, FloatVec4Property
#include <modules/opengl/shader/shader.h>             // for Shader

namespace inviwo {

/** \docpage{org.inviwo.PointRenderer, Point Renderer}
 * ![](org.inviwo.PointRenderer.png?classIdentifier=org.inviwo.PointRenderer)
 * This processor renders a set of meshes as points using OpenGL.
 *
 * ### Inports
 *   * __geometry__ Input meshes
 *   * __imageInport__ Optional background image
 *
 * ### Outports
 *   * __image__ output image containing the rendered mesh and the optional input image
 *
 * ### Properties
 *   * __Point Size__  size of the rendered points (in pixel)
 *   * __Border Width__  width of the border
 *   * __Border Color__  color of the border
 *   * __Antialising__ width of the antialised point edge (in pixel), this determines the
 *                     softness along the outer edge of the point
 */

/**
 * \class PointRenderer
 * \brief Renders input geometry with 2D points
 */
class IVW_MODULE_BASEGL_API PointRenderer : public Processor {
public:
    PointRenderer();
    virtual ~PointRenderer() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    void drawMeshes();

    MeshFlatMultiInport inport_;
    ImageInport imageInport_;
    ImageOutport outport_;

    FloatProperty pointSize_;
    FloatProperty borderWidth_;
    FloatVec4Property borderColor_;
    FloatProperty antialising_;

    CameraProperty camera_;
    CameraTrackball trackball_;

    Shader shader_;
};

}  // namespace inviwo
