/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2018 Inviwo Foundation
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

#ifndef IVW_CUBERENDERER_H
#define IVW_CUBERENDERER_H

#include <modules/basegl/baseglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/interaction/cameratrackball.h>
#include <inviwo/core/ports/meshport.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/simplelightingproperty.h>
#include <modules/opengl/shader/shader.h>

namespace inviwo {

/** \docpage{org.inviwo.CubeRenderer, Cube Renderer}
 * ![](org.inviwo.CubeRenderer.png?classIdentifier=org.inviwo.CubeRenderer)
 * This processor renders a set of point meshes using cubical glyphs in OpenGL.
 * The glyphs are resolution independent and consist only of a single point.
 * The size of each point is given in the w coordinate of the vertex position unless
 * globally overwritten by the property.
 *
 * ### Inports
 *   * __geometry__ Input meshes
 *   * __imageInport__ Optional background image
 *
 * ### Outports
 *   * __image__    output image containing the rendered cubes and the optional input image
 *
 * ### Properties
 *   * __Render Mode__               render only input meshes marked as points or everything
 *   * __Clip Mode__                 defines the handling of cubes clipped at the camera
 *   * __Clip Surface Adjustment__   brighten/darken glyph color on clip surface
 *   * __Shaded Clipped Area__       enable illumination computations for the clipped surface
 *   * __Overwrite Cube Size__   enable a fixed user-defined size for all cubes
 *   * __Custom Size__          size of the rendered cubes (in world coordinates)
 *   * __Overwrite Color__     if enabled, all cubes will share the same custom color
 *   * __Custom Color__        custom color when overwriting the input colors
 */

/**
 * \class CubeRenderer
 * \brief Renders input geometry with 3D cube glyphs using OpenGL shaders
 */
class IVW_MODULE_BASEGL_API CubeRenderer : public Processor {
public:
    CubeRenderer();
    virtual ~CubeRenderer() = default;

    virtual void process() override;

    virtual void initializeResources() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    void drawMeshes();

    enum class RenderMode {
        EntireMesh,  //!< render all vertices of the input mesh as glyphs
        PointsOnly,  //!< render only parts of mesh with DrawType::Points
    };
    /**
     * \enum GlyphClippingMode
     * defines how glyphs are rendering if the first intersection, i.e. the front side,
     * lies behind the near clip plane of the camera.
     */
    enum class GlyphClippingMode {
        Discard,  //!< glyph is not rendered
        Cut,      //!< the cut surface is visible
    };

    MeshFlatMultiInport inport_;
    ImageInport imageInport_;
    ImageOutport outport_;

    CompositeProperty cubeProperties_;
    BoolProperty overrideCubeSize_;
    FloatProperty customSize_;
    BoolProperty overrideCubeColor_;
    FloatVec4Property customColor_;
	TemplateOptionProperty<GLint> culling_;
    CameraProperty camera_;
    SimpleLightingProperty lighting_;

    CameraTrackball trackball_;

    Shader shader_;
};

}  // namespace inviwo

#endif  // IVW_CUBERENDERER_H
