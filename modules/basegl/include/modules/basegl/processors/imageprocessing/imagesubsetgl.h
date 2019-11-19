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

#pragma once

#include <modules/basegl/baseglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/eventproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>

#include <modules/opengl/shader/shader.h>

namespace inviwo {

/** \docpage{org.inviwo.ImageSubsetGL, Image Subset}
 * ![](org.inviwo.ImageSubsetGL.png?classIdentifier=org.inviwo.ImageSubsetGL)
 *
 * Extracts a region of an image. First color layer, depth layer, and picking layer are
 * copied.
 *
 * ### Inports
 *   * __image.inport__ Image to extract region from.
 *
 * ### Outports
 *   * __image.outport__ Extracted region.
 *
 * ### Properties
 *   * __Range X__ Min/max pixel along horizontal axis.
 *   * __Range Y__ Min/max pixel along vertical axis.
 *
 */
class IVW_MODULE_BASEGL_API ImageSubsetGL : public Processor {
public:
    ImageSubsetGL();
    ~ImageSubsetGL();

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    virtual void process() override;
    

    void pan(ivec2 diff);
    ImageInport inport_;
    ImageOutport outport_;
    
    IntSizeTMinMaxProperty rangeX_;
    IntSizeTMinMaxProperty rangeY_;
private:
    void handlePan(Event* event);
    void touchGesture(Event* event);

    EventProperty mouseZoom_;
    EventProperty mousePan_;
    EventProperty touchGesture_;
    
    dvec2 lastInteractionPos_;
    size2_t rangePressedX_;
    size2_t rangePressedY_;
    bool mousePressed_ = false;
    size2_t dims_;
    
    Shader shader_;
    Mesh rect_;
    std::shared_ptr<Buffer<vec2>> texCoordsBuffer_;
};

}  // namespace inviwo

