/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#ifndef IVW_DRAWPOINTS_H
#define IVW_DRAWPOINTS_H

#include <modules/basegl/baseglmoduledefine.h>
#include <inviwo/core/ports/imageport.h>
#include <modules/opengl/image/compositeprocessorgl.h>
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <modules/opengl/rendering/meshdrawergl.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/eventproperty.h>

namespace inviwo {

/** \docpage{org.inviwo.DrawPoints, Draw Points}
 * ![](org.inviwo.DrawPoints.png?classIdentifier=org.inviwo.DrawPoints)
 * 
 * Interactive 2D point drawing 
 * 
 * Hold Ctrl+D and click/move Left Mouse Button to Draw
 * 
 * ### Inports
 *   * __ImageInport__ The input image.
 *
 * ### Outports
 *   * __ImageOutport__ The output image.
 * 
 * ### Properties
 *   * __PointSize__ Defines size of all points.
 *   * __PointColor__ Defines color of all points.
 *   * __ClearButton__ Button to clear all points.
 */

/**
 * \brief Interactive 2D point drawing
 *
 * Hold Ctrl+D and click/move Left Mouse Button to Draw
 */
class IVW_MODULE_BASEGL_API DrawPoints : public CompositeProcessorGL {
public:
    DrawPoints();
    ~DrawPoints();

    InviwoProcessorInfo();

    void initialize();
    void deinitialize();

protected:
    void addPoint(vec2);
    void clearPoints();

    void process();

private:
    void eventDraw(Event*);
    void eventEnableDraw(Event*);

    ImageInport inport_;
    ImageOutport outport_;

    IntProperty pointSize_;
    FloatVec4Property pointColor_;
    ButtonProperty clearButton_;

    EventProperty mouseDraw_;
    EventProperty keyEnableDraw_;


    Mesh* points_;
    MeshDrawerGL* pointDrawer_;

    Shader* pointShader_;

    bool drawModeEnabled_;
};

}

#endif //IVW_DRAWPOINTS_H
