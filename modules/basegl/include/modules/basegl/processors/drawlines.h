/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2018 Inviwo Foundation
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

#ifndef IVW_DRAWLINES_H
#define IVW_DRAWLINES_H

#include <modules/basegl/baseglmoduledefine.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/processors/processor.h>
#include <modules/opengl/image/imagecompositor.h>
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <modules/opengl/rendering/meshdrawergl.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/eventproperty.h>
#include <modules/opengl/shader/shader.h>

namespace inviwo {

/** \docpage{org.inviwo.DrawLines, Draw Lines}
 * ![](org.inviwo.DrawLines.png?classIdentifier=org.inviwo.DrawLines)
 *
 * Interactive 2D line drawing
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
 *   * __LineSize__ Defines size of all lines.
 *   * __LineColor__ Defines color of all lines.
 *   * __ClearButton__ Button to clear all lines.
 */

/**
 * \brief Interactive 2D line drawing
 *
 * Hold Ctrl+D and click/move Left Mouse Button to Draw
 */
class IVW_MODULE_BASEGL_API DrawLines : public Processor {
public:
    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    DrawLines();
    virtual ~DrawLines();

protected:
    void addPoint(vec2);
    void clearLines();

    virtual void process() override;

private:
    void eventDraw(Event*);
    void eventEnableDraw(Event*);

    ImageInport inport_;
    ImageOutport outport_;
    ImageCompositor compositor_;

    FloatProperty lineSize_;
    FloatVec4Property lineColor_;
    ButtonProperty clearButton_;

    EventProperty mouseDraw_;
    EventProperty keyEnableDraw_;

    Mesh lines_;
    MeshDrawerGL lineDrawer_;

    Shader lineShader_;

    bool drawModeEnabled_;
};

}  // namespace inviwo

#endif  // IVW_DRAWLINES_H
