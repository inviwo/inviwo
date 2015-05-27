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

#ifndef IVW_IMAGELAYOUTGL_H
#define IVW_IMAGELAYOUTGL_H

#include <modules/basegl/baseglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/interaction/events/mouseevent.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/baseoptionproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/interaction/interactionhandler.h>
#include <inviwo/core/ports/imageport.h>
#include <modules/opengl/inviwoopengl.h>
#include <modules/opengl/glwrap/shader.h>
#include <modules/basegl/viewmanager.h>

namespace inviwo {

// Mouse click activates the area for mouse/key interactions.
/** \docpage{org.inviwo.ImageLayoutGL, Image Layout}
 * ![](org.inviwo.ImageLayoutGL.png?classIdentifier=org.inviwo.ImageLayoutGL)
 *
 * ...
 * 
 * ### Inports
 *   * __multiinport__ ...
 * 
 * ### Outports
 *   * __outport__ ...
 * 
 * ### Properties
 *   * __Vertical Split__ ...
 *   * __Split Position__ ...
 *   * __Split Position__ ...
 *   * __Horizontal Split__ ...
 *
 */
class IVW_MODULE_BASEGL_API ImageLayoutGL : public Processor {
public:
    InviwoProcessorInfo();

    enum class Layout {
        Single,
        HorizontalSplit,
        VerticalSplit,
        CrossSplit,
        ThreeLeftOneRight,
        ThreeRightOneLeft,
    };

    ImageLayoutGL();
    virtual ~ImageLayoutGL();

    virtual bool propagateResizeEvent(ResizeEvent* event, Outport* source) override;
    virtual void propagateEvent(Event*) override;

protected:
    virtual void process() override;

    void updateViewports(ivec2 size, bool force = false);
    void onStatusChange();

private:
    ImageMultiInport multiinport_;
    ImageOutport outport_;

    TemplateOptionProperty<Layout> layout_;
    FloatProperty horizontalSplitter_;
    FloatProperty verticalSplitter_;
    FloatProperty vertical3Left1RightSplitter_;
    FloatProperty vertical3Right1LeftSplitter_;

    Shader shader_;
    ViewManager viewManager_;
    Layout currentLayout_;
    ivec2 currentDim_;
};

}  // namespace

#endif  // IVW_IMAGELAYOUTGL_H
