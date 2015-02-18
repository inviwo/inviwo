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

#ifndef IVW_POSITIONWIDGETPROCESSOR_H
#define IVW_POSITIONWIDGETPROCESSOR_H

#include <modules/basegl/baseglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <modules/opengl/inviwoopengl.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/ports/geometryport.h>
#include <modules/opengl/image/compositeprocessorgl.h>
#include <inviwo/core/interaction/pickingobject.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/interaction/cameratrackball.h>


namespace inviwo {
class Shader;

/** \docpage{org.inviwo.GeometryPicking, Geometry Picking}
* Composite image with geometry where geometry repositioned through picking
*
* Use Left Mouse Button to move the geomtry around in the scene 
*
* ### Inports
*   * __GeometryInport__ The input geometry.
*   * __ImageInport__ The input image.
*
* ### Outports
*   * __ImageOutport__ The output image.
*
* ### Properties
*   * __Position_ Defines size of all lines.
*   * __Camera__ Camera of the scene.
*   * __Trackball__ Camera trackball.
*/

/**
* \brief Composite image with geometry where geometry repositioned through picking
*
*/

class IVW_MODULE_BASEGL_API GeometryPicking : public CompositeProcessorGL {
public:
    InviwoProcessorInfo();

    GeometryPicking();
    virtual ~GeometryPicking();

    void initialize();
    void deinitialize();

    virtual void process();
    
    bool isReady() const { return geometryInport_.isReady(); }

    void updateWidgetPositionFromPicking(const PickingObject*);

private:
    GeometryInport geometryInport_;
    ImageInport imageInport_;
    ImageOutport outport_;

    FloatVec3Property position_;

    CameraProperty camera_;
    CameraTrackball trackball_;

    const PickingObject* widgetPickingObject_;

    Shader* shader_;
};

} // namespace

#endif // IVW_POSITIONWIDGETPROCESSOR_H
