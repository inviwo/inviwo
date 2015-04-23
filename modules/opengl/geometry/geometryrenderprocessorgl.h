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
#include <inviwo/core/ports/geometryport.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/simplelightingproperty.h>
#include <inviwo/core/rendering/geometrydrawer.h>
#include <modules/opengl/openglmoduledefine.h>
#include <vector>

namespace inviwo {

class Shader;
class CameraTrackball;

class IVW_MODULE_OPENGL_API GeometryRenderProcessorGL : public Processor {
public:
    GeometryRenderProcessorGL();
    ~GeometryRenderProcessorGL();

    InviwoProcessorInfo();

    virtual void initialize();
    virtual void deinitialize();

    virtual void initializeResources();

protected:
    virtual void process(); 
    void centerViewOnGeometry();
    void setNearFarPlane();
    void resetViewParams();
    void changeRenderMode();

    void updateDrawers();

    GeometryMultiInport inport_;
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
    BoolProperty veiwNormalsLayer_;

    Shader* shader_;

    std::vector<GeometryDrawer*> drawers_;
    std::vector<Inport*> drawersPort_;
};

} // namespace

#endif // IVW_GEOMETRYRENDERPROCESSORGL_H
