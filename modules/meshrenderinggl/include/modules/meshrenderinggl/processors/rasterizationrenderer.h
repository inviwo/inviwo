/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2020 Inviwo Foundation
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
#include <modules/meshrenderinggl/datastructures/halfedges.h>
#include <modules/meshrenderinggl/ports/rasterizationport.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/interaction/cameratrackball.h>
#include <inviwo/core/ports/meshport.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/boolcompositeproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/transferfunctionproperty.h>
#include <inviwo/core/properties/simplelightingproperty.h>
#include <inviwo/core/rendering/meshdrawer.h>
#include <modules/opengl/shader/shader.h>

#include <modules/meshrenderinggl/rendering/fragmentlistrenderer.h>
#include <modules/meshrenderinggl/algorithm/calcnormals.h>

#include <string_view>
#include <memory>

namespace inviwo {

/** \docpage{org.inviwo.RasterizationRenderer, Rasterization Renderer}
 * ![](org.inviwo.RasterizationRenderer.png?classIdentifier=org.inviwo.RasterizationRenderer)
 * Renderer bringing together several kinds of rasterizations objects.
 *
 * Fragment lists are used to render the transparent pixels with correct alpha blending.
 * Illustration effects can be applied as a post-process.
 *
 * ### Inports
 *   * __rasterizations__ Input rasterizations filling the fragment lists/render target
 *   * __imageInport__ Optional background image
 *
 * ### Outports
 *   * __image__ output image containing the rendered objects and the optional input image
 *
 *   * __Force Opaque__ Draw the mesh opaquly instead of transparent. Disables all transparency
 * settings
 *   * __Alpha__ Assemble construction of the alpha value out of many factors (which are summed up)
 *       + __Uniform__ uniform alpha value
 *       + __Angle-based__ based on the angle between the pixel normal and the direction to the
 * camera
 *       + __Normal variation__ based on the variation (norm of the derivative) of the pixel normal
 *       + __Density-based__ based on the size of the triangle / density of the smoke volume inside
 * the triangle
 *       + __Shape-based__ based on the shape of the triangle. The more stretched, the more
 * transparent
 *   * __Edges__ Settings for the display of triangle edges
 *       + __Thickness__ The thickness of the edges
 *       + __Depth dependent__ If checked, the thickness also depends on the depth.
 *           If unchecked, every edge has the same size in screen space regardless of the distance
 * to the camera
 *       + __Smooth edges__ If checked, a simple anti-alising is used
 *   * __Front Face__ Settings for the front face
 *       + __Show__ Shows or hides that face (culling)
 *       + __Color Source__ The source of the color: vertex color, transfer function, or external
 * constant color
 *       + __Separate Uniform Alpha__ Overwrite alpha settings from above with a constant alpha
 * value
 *       + __Normal Source__ Source of the pixel normal: interpolated or not
 *       + __Shading Mode__ The shading that is applied to the pixel color
 *       + __Show Edges__ Show triangle edges
 *       + __Edge Color__ The color of the edges
 *       + __Edge Opacity__ Blending of the edge color:
 *           0-1: blending factor of the edge color into the triangle color, alpha unmodified;
 *           1-2: full edge color and alpha is increased to fully opaque
 *   * __Back Face__ Settings for the back face
 *       + __Show__ Shows or hides that face (culling)
 *       + __Same as front face__ use the settings from the front face, disables all other settings
 * for the back face
 *       + __Copy Front to Back__ Copies all settings from the front face to the back face
 *       + __Color Source__ The source of the color: vertex color, transfer function, or external
 * constant color
 *       + __Separate Uniform Alpha__ Overwrite alpha settings from above with a constant alpha
 * value
 *       + __Normal Source__ Source of the pixel normal: interpolated or not
 *       + __Shading Mode__ The shading that is applied to the pixel color
 *       + __Show Edges__ Show triangle edges
 *       + __Edge Color__ The color of the edges
 *       + __Edge Opacity__ Blending of the edge color:
 *           0-1: blending factor of the edge color into the triangle color, alpha unmodified;
 *           1-2: full edge color and alpha is increased to fully opaque
 */

/**
 * \class RasterizationRenderer
 * \brief Mesh Renderer specialized for rendering highly layered and transparent surfaces.
 *
 * It uses the FragmentListRenderer for the rendering of the transparent mesh.
 * Many alpha computation modes, shading modes, color modes can be combined
 * and even selected individually for the front- and back face.
 */
class IVW_MODULE_MESHRENDERINGGL_API RasterizationRenderer : public Processor {
public:
    RasterizationRenderer();
    virtual ~RasterizationRenderer() = default;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    /**
     * \brief Performs the rendering.
     */
    virtual void process() override;

protected:
    RasterizationInport rasterizations_;

    std::shared_ptr<ImageInport> imageInport_;
    ImageOutport outport_;
    Image intermediateImage_;

    CameraProperty camera_;
    CameraTrackball trackball_;

    struct IllustrationSettings {
        IllustrationSettings();

        BoolCompositeProperty enabled_;
        FloatVec3Property edgeColor_;
        FloatProperty edgeStrength_;
        FloatProperty haloStrength_;
        IntProperty smoothingSteps_;
        FloatProperty edgeSmoothing_;
        FloatProperty haloSmoothing_;

        FragmentListRenderer::IllustrationSettings getSettings() const;
    };
    IllustrationSettings illustrationSettings_;

    std::unique_ptr<FragmentListRenderer> flr_;
    typename Dispatcher<void()>::Handle flrReload_;
    bool supportesIllustration_;
};

}  // namespace inviwo
