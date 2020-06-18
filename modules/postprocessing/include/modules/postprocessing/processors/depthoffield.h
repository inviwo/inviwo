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

#include <modules/postprocessing/postprocessingmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/camera/skewedperspectivecamera.h>
#include <inviwo/core/datastructures/image/imageram.h>
#include <inviwo/core/datastructures/image/layerramprecision.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <modules/base/algorithm/randomutils.h>
#include <modules/opengl/image/imagegl.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/volume/volumeutils.h>
#include <modules/opengl/volume/volumegl.h>

namespace inviwo {

/** \docpage{org.inviwo.DepthOfField, Depth Of Field}
 * ![](org.inviwo.DepthOfField.png?classIdentifier=org.inviwo.DepthOfField)
 * Use any pinhole image as input and connect the camera property to the rendering processor. 
 * Applies a depth of field effect where out-of-focus regions appear blurred.
 *
 * ### Inports
 *   * __ImageInport__ Input image.
 *
 * ### Outports
 *   * __ImageOutport__ Output image.
 *
 * ### Properties
 *   * __Aperture__ The diameter of the simulated lens. Affects blur strength.
 *   * __FocusDepth__ The depth in the scene that is in focus.
 *   * __Approximate__ If true, uses an approximative algorithm which renders 
 * the scene fewer times. Requires a valid depth map.
 *   * __ViewCount__ The number of times to render the scene.
 *   * __SimulatedViewCount__ The number of additional views to approximate.
 *   * __Camera__ The camera should be linked with the rendering processor. 
 */
class IVW_MODULE_POSTPROCESSING_API DepthOfField : public Processor {
public:
    DepthOfField();
    virtual ~DepthOfField() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
	ImageInport inport_;
    ImageOutport outport_;

    FloatProperty aperture_;
    FloatProperty focusDepth_;
    BoolProperty approximate_;
    IntSizeTProperty viewCountExact_;
    IntSizeTProperty viewCountApprox_;
    IntSizeTProperty simViewCountApprox_;

    CameraProperty camera_;

    int evalCount_;
    std::unique_ptr<Camera> ogCamera_;

    std::vector<float> haltonX_;

    // Exact algorithm
    std::shared_ptr<Image> prevOutImg_;
    std::shared_ptr<Image> nextOutImg_;
    std::vector<float> haltonY_;
    Shader addSampleShader_;

    // Approximative algorithm
    size3_t dimLightField_;
    std::shared_ptr<Volume> lightField_;
    std::shared_ptr<Volume> lightFieldDepth_;
    std::shared_ptr<Image> haltonImg_;
    Shader addToLightFieldShader_;
    Shader averageLightfieldShader_;

    void warp(vec2 st, vec2 uv, vec4 color, double zWorld, size_t viewIndex, vec4* lightFieldData,
              float* lightFieldDepthData, double fovy);
};

}  // namespace inviwo
