/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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

#ifndef IVW_MESHRENDERPROCESSORGL_H
#define IVW_MESHRENDERPROCESSORGL_H

#include <modules/basegl/baseglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/interaction/cameratrackball.h>
#include <inviwo/core/ports/meshport.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/simplelightingproperty.h>
#include <inviwo/core/rendering/meshdrawer.h>

#include <modules/opengl/shader/shader.h>

#include <vector>

namespace inviwo {

/** \docpage{org.inviwo.GeometryRenderGL, Mesh Renderer}
 * ![](org.inviwo.GeometryRenderGL.png?classIdentifier=org.inviwo.GeometryRenderGL)
 * Renders a set of meshes using OpenGL on top of an image. Different rendering modes can be
 * selected.
 *
 * ### Inports
 *   * __geometry__ Input meshes
 *   * __imageInport__ Optional background image
 *
 * ### Outports
 *   * __image__ output image containing the rendered mesh and the optional input image
 *
 * ### Properties
 *   * __Camera__ Camera used for rendering the mesh
 *   * __Fit View to Mesh__ Contains button properties to set the view of the camera to contain all
 * input meshes
 *   * __Reset Camera__ Reset the camera to its default state
 *   * __Geometry Rendering Properties__
 *       + __Cull Face__ (None, Front, Back, Back and Front)
 *       + __Polygon Mode__  (Points, Lines, Fill)
 *       + __Point Size__ Defines the point size when polygon mode is set to "Points"
 *       + __Line Width__ Defines the line width when polygon mode is set to "Lines" (not supported)
 *       + __Enable Depth Test__ Toggles the depth test during rendering
 *   * __Lighting__ Standard lighting settings
 *   * __Output Layers__
 *       + __Color__ Toggle output of color layer
 *       + __Texture Coordinates__ Toggle output of texture coordinates
 *       + __Normals (World Space)__ Toggle output of normals (world space)
 *       + __Normals (View space)__ Toggle output of view space normals
 *
 */
class IVW_MODULE_BASEGL_API MeshRenderProcessorGL : public Processor {
public:
    MeshRenderProcessorGL();

    MeshRenderProcessorGL(const MeshRenderProcessorGL&) = delete;
    MeshRenderProcessorGL& operator=(const MeshRenderProcessorGL&) = delete;

    virtual ~MeshRenderProcessorGL();

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    virtual void initializeResources() override;
    virtual void process() override;

protected:
    void updateDrawers();

    MeshFlatMultiInport inport_;
    ImageInport imageInport_;
    ImageOutport outport_;

    CameraProperty camera_;
    CameraTrackball trackball_;

    BoolProperty overrideColorBuffer_;
    FloatVec4Property overrideColor_;

    CompositeProperty geomProperties_;
    OptionPropertyInt cullFace_;
    BoolProperty enableDepthTest_;
    SimpleLightingProperty lightingProperty_;

    CompositeProperty layers_;
    BoolProperty colorLayer_;
    BoolProperty texCoordLayer_;
    BoolProperty normalsLayer_;
    BoolProperty viewNormalsLayer_;

    Shader shader_;

    using DrawerMap = std::multimap<const Outport*, std::unique_ptr<MeshDrawer>>;
    DrawerMap drawers_;
};

}  // namespace inviwo

#endif  // IVW_GEOMETRYRENDERPROCESSORGL_H
