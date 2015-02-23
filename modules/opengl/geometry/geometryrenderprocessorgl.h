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
#include <inviwo/core/rendering/geometryrenderer.h>
#include <modules/opengl/openglmoduledefine.h>
#include <vector>

namespace inviwo {

class Shader;
class CameraTrackball;

/** \docpage{org.inviwo.GeometryRenderProcessorGL, Geometry Renderer}
 * Renders a geometry with optional shading onto an image. 
 * 
 * 
 * ### Inports
 *   * __GeometryMultiInport__ Input geometry to render.
 *   * __ImageInport__ Color texture for color mapping (optional).
 *   * __ImageInport__ Normal map input (optional). Normals in texture are assumed to lie in [0 1]^3, data space.
 *
 * ### Outports
 *   * __ImageOutport__ The rendered geometry.
 * 
 * ### Properties
 *   * __Cull Face__ Option to remove geometry facing away from the camera.
 *   * __Polygon Mode__ Specifies how polygons will be rasterized.
 *   * __Point Size__ Size of renderered points in pixels (only for Point polygon mode). 
 *   * __Line Width__ Thickness of renderered lines in pixels (only for Lines polygon mode). 
 *
 */

/**
 * \brief Renders a geometry with optional texture and normal mapping to an image.
 *
 */
class IVW_MODULE_OPENGL_API GeometryRenderProcessorGL : public Processor {
public:
    GeometryRenderProcessorGL();
    ~GeometryRenderProcessorGL();

    InviwoProcessorInfo();

    virtual void initialize();
    virtual void deinitialize();
    virtual void initializeResources();

    virtual bool isReady() const {
        // only the mesh input port is mandatory
        return inport_.isReady(); 
    }

    virtual void process();
protected:

    void centerViewOnGeometry();
    void resetViewParams();
    void changeRenderMode();

    void updateRenderers();

    GeometryMultiInport inport_;
    ImageInport colors_; //!< inport for the 2D color texture (optional)
    ImageInport normals_; //!< inport for the 2D normal map texture (optional)
    ImageOutport outport_;

    CameraProperty camera_;
    ButtonProperty centerViewOnGeometry_;
    ButtonProperty resetViewParams_;
    CameraTrackball trackball_;

    std::vector<GeometryRenderer*> renderers_;
    
    CompositeProperty geomProperties_;
    OptionPropertyInt cullFace_;
    OptionPropertyInt polygonMode_;
    FloatProperty renderPointSize_;
    FloatProperty renderLineWidth_;
    SimpleLightingProperty lightingProperty_;

    Shader* shader_;
};

} // namespace

#endif // IVW_GEOMETRYRENDERPROCESSORGL_H
