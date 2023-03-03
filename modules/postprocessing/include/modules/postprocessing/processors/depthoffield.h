/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2023 Inviwo Foundation
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

#include <modules/postprocessing/postprocessingmoduledefine.h>  // for IVW_MODULE_POSTPROCESSING...

#include <inviwo/core/datastructures/camera/camera.h>  // for Camera
#include <inviwo/core/ports/imageport.h>               // for BaseImageInport, ImageOut...
#include <inviwo/core/ports/meshport.h>                // for MeshInport
#include <inviwo/core/processors/processor.h>          // for Processor
#include <inviwo/core/processors/processorinfo.h>      // for ProcessorInfo
#include <inviwo/core/properties/boolproperty.h>       // for BoolProperty
#include <inviwo/core/properties/cameraproperty.h>     // for CameraProperty
#include <inviwo/core/properties/eventproperty.h>      // for EventProperty
#include <inviwo/core/properties/ordinalproperty.h>    // for IntSizeTProperty, DoubleP...
#include <inviwo/core/util/glmvec.h>                   // for vec2, size2_t, vec4
#include <modules/opengl/shader/shader.h>              // for Shader
#include <modules/opengl/shader/shaderutils.h>         // for ImageInport

#include <cstddef>  // for size_t
#include <memory>   // for shared_ptr, unique_ptr
#include <vector>   // for vector

namespace inviwo {
class Event;
class Image;
class SkewedPerspectiveCamera;
class TextureUnitContainer;
class Volume;
class VolumeRAM;

/** \docpage{org.inviwo.DepthOfField, Depth Of Field}
 * ![](org.inviwo.DepthOfField.png?classIdentifier=org.inviwo.DepthOfField)
 *
 * Applies a depth of field effect where out-of-focus regions appear blurred.
 * Use any pinhole camera-generated image as input and connect the camera property to the rendering
 * processor. The image should be generated with a perspective camera but may use any rendering
 * technique.
 *
 * Implementation is based on Depth of Field Rendering from Sparsely Sampled Pinhole Images (Master
 * thesis, 2020) by Natalie Axelsson.
 *
 * ### Inports
 *   * __ImageInport__ Input image.
 *   * __MeshInport__ World space point to focus on. The first point in the first position buffer
 * will be used.
 *
 * ### Outports
 *   * __ImageOutport__ Output image.
 *
 * ### Properties
 *   * __Aperture__ The diameter of the simulated lens. Affects blur strength.
 *   * __FocusDepth__ The depth in the scene that is in focus. Requires manual focus to be enabled.
 *   * __ManualFocus__ Determines whether the focus depth is determined by manual input or inport
 * data.
 *   * __Approximate__ If true, uses an approximative algorithm which renders
 * the scene fewer times. Requires a valid depth map. Note that the approximative algorithm does not
 * support transparency in the scene.
 *   * __ViewCount__ The number of times to render the scene. A higher view count leads to smoother
 * blur in the image, but also requires longer computation times.
 *   * __RenderedViewCount__ The number of times to render the scene when using the approximative
 * algorithm. A higher rendered view count improves approximations of partially occluded regions and
 * may reduce artifacts around object boundaries.
 *   * __SimulatedViewCount__ The number of additional views to approximate. The simulated view
 * count affects ghosting artifacts in the final image.
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
    MeshInport trackingInport_;
    ImageOutport outport_;

    DoubleProperty aperture_;
    DoubleProperty focusDepth_;
    BoolProperty manualFocus_;
    BoolProperty approximate_;
    IntSizeTProperty viewCountExact_;
    IntSizeTProperty viewCountApprox_;
    IntSizeTProperty simViewCountApprox_;
    EventProperty clickToFocus_;
    CameraProperty camera_;

    int evalCount_;
    const bool useComputeShaders_;
    std::unique_ptr<Camera> ogCamera_;

    std::vector<float> haltonX_;

    // Exact algorithm
    std::shared_ptr<Image> prevOutImg_;
    std::shared_ptr<Image> nextOutImg_;
    std::vector<float> haltonY_;
    Shader addSampleShader_;

    // Approximative algorithm
    std::shared_ptr<Volume> lightField_;
    std::shared_ptr<Volume> lightFieldDepth_;
    std::shared_ptr<Image> haltonImg_;
    Shader addToLightFieldShader_;
    Shader averageLightFieldShader_;

    void clickToFocus(Event* e);
    void setupRecursion(size2_t dim, size_t maxEvalCount, std::shared_ptr<const Image> img);
    double calculateFocusDepth();
    vec2 calculatePeripheralCameraPos(int evalCount, int maxEvalCount);
    void addToAccumulationBuffer(std::shared_ptr<const Image> img, TextureUnitContainer& cont);
    void warpToLightfieldGPU(TextureUnitContainer& cont, double fovy, double focusDepth,
                             size2_t dim, vec2 cameraPos);
    void warpToLightfieldCPU(std::shared_ptr<const Image> img, double fovy, double focusDepth,
                             size2_t dim, vec2 cameraPos);
    void warp(vec2 st, vec2 uv, vec4 color, double zWorld, size_t viewIndex, VolumeRAM* lightField,
              VolumeRAM* lightFieldDepth, double fovy, double focusDepth);
    void synthesizeLightfield(TextureUnitContainer& cont);
    void moveCamera(SkewedPerspectiveCamera* camera, int maxEvalCount, double focusDepth);
    double ndcToWorldDepth(double depthNdc);
};

}  // namespace inviwo
