/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#ifndef IVW_GEOMETRYRENDERPROCESSORGL_H
#define IVW_GEOMETRYRENDERPROCESSORGL_H

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
#include <modules/opengl/openglmoduledefine.h>
#include <modules/opengl/glwrap/shader.h>
#include <vector>

namespace inviwo {

/** \docpage{org.inviwo.GeometryRenderGL, Mesh Renderer}
 * ![](org.inviwo.GeometryRenderGL.png?classIdentifier=org.inviwo.GeometryRenderGL)
 *
 * ...
 * 
 * ### Inports
 *   * __geometry.inport__ ...
 * 
 * ### Outports
 *   * __image.outport__ ...
 * 
 * ### Properties
 *   * __Line Width__ ...
 *   * __Normals (View space)__ ...
 *   * __Cull Face__ ...
 *   * __Geometry Rendering Properties__ ...
 *   * __Camera__ ...
 *   * __Layers__ ...
 *   * __Lighting__ ...
 *   * __Calculate Near and Far Plane__ ...
 *   * __Reset Camera__ ...
 *   * __Color__ ...
 *   * __Center view on geometry__ ...
 *   * __Point Size__ ...
 *   * __Texture Coordinates__ ...
 *   * __Polygon Mode__ ...
 *   * __Normals (World Space)__ ...
 *
 */
class IVW_MODULE_OPENGL_API MeshRenderProcessorGL : public Processor {
public:
    MeshRenderProcessorGL();

    MeshRenderProcessorGL(const MeshRenderProcessorGL&) = delete;
    MeshRenderProcessorGL& operator=(const MeshRenderProcessorGL&) = delete;

    ~MeshRenderProcessorGL();

    InviwoProcessorInfo();

    virtual void initializeResources();
    virtual void process(); 

protected:
    void centerViewOnGeometry();
    void setNearFarPlane();
    void changeRenderMode();
    void updateDrawers();
    void addCommonShaderDefines(Shader& shader);

    MeshFlatMultiInport inport_;
    ImageOutport outport_;

    CameraProperty camera_;
    ButtonProperty centerViewOnGeometry_;
    ButtonProperty setNearFarPlane_;
    ButtonProperty resetViewParams_;
    CameraTrackball trackball_;
    
    CompositeProperty geomProperties_;
    OptionPropertyInt cullFace_;
    OptionPropertyInt polygonMode_;
    FloatProperty renderPointSize_;
    FloatProperty renderLineWidth_;
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

} // namespace

#endif // IVW_GEOMETRYRENDERPROCESSORGL_H
