/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2022 Inviwo Foundation
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

#include <modules/basegl/baseglmoduledefine.h>  // for IVW_MODULE_BASEGL_API

#include <inviwo/core/ports/imageport.h>                     // for ImageMultiInport, ImageOutport
#include <inviwo/core/processors/processor.h>                // for Processor
#include <inviwo/core/processors/processorinfo.h>            // for ProcessorInfo
#include <inviwo/core/properties/compositeproperty.h>        // for CompositeProperty
#include <inviwo/core/properties/ordinalproperty.h>          // for IntProperty
#include <inviwo/core/util/glmvec.h>                         // for ivec2
#include <modules/basegl/datastructures/splittersettings.h>  // for Direction
#include <modules/basegl/properties/splitterproperty.h>      // for SplitterProperty
#include <modules/basegl/rendering/splitterrenderer.h>       // for SplitterRenderer
#include <modules/basegl/viewmanager.h>                      // for ViewManager
#include <modules/opengl/shader/shader.h>                    // for Shader

namespace inviwo {
class Deserializer;
class Event;
class Inport;
class Outport;

class IVW_MODULE_BASEGL_API Layout : public Processor {
public:
    Layout(splitter::Direction direction);
    virtual ~Layout() = default;

    virtual void process() override;
    virtual void propagateEvent(Event* event, Outport* source) override;
    virtual void deserialize(Deserializer& d) override;
    virtual bool isConnectionActive([[maybe_unused]] Inport* from, Outport* to) const override;

protected:
    virtual void updateViewports(ivec2 dim) = 0;

    void updateSplitters(bool connect);
    float getSplitPosition(int i);
    void updateSliders(int i);

    ImageMultiInport inport_;
    ImageOutport outport_;

    SplitterProperty splitterSettings_;
    IntProperty minWidth_;
    CompositeProperty splitters_;
    ViewManager viewManager_;
    SplitterRenderer renderer_;
    splitter::Direction direction_;
    ivec2 currentDim_;
    Shader shader_;
    bool deserialized_;
};

/** \docpage{org.inviwo.ColumnLayout, Column Layout}
 * ![](org.inviwo.ColumnLayout.png?classIdentifier=org.inviwo.ColumnLayout)
 * Horizontal layout which puts multiple input images next to each other. Interactions are forwarded
 * to the respective areas.
 */
class IVW_MODULE_BASEGL_API ColumnLayout : public Layout {
public:
    ColumnLayout();
    virtual ~ColumnLayout() = default;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    virtual void updateViewports(ivec2 dim) override;
};

/** \docpage{org.inviwo.RowLayout, Column Layout}
 * ![](org.inviwo.RowLayout.png?classIdentifier=org.inviwo.RowLayout)
 * Horizontal layout which puts multiple input images next to each other. Interactions are forwarded
 * to the respective areas.
 */
class IVW_MODULE_BASEGL_API RowLayout : public Layout {
public:
    RowLayout();
    virtual ~RowLayout() = default;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    virtual void updateViewports(ivec2 dim) override;
};

}  // namespace inviwo
