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

#ifndef IVW_SIMPLECROSSHAIROVERLAY_H
#define IVW_SIMPLECROSSHAIROVERLAY_H

// general
#include <modules/basegl/baseglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <memory>

// properties
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/eventproperty.h>

// ports
#include <inviwo/core/ports/imageport.h>

namespace inviwo {
/**
 * \class
 * \brief
 */
class IVW_MODULE_BASEGL_API SimpleCrosshairOverlay : public Processor {
public:
    SimpleCrosshairOverlay();
    virtual ~SimpleCrosshairOverlay() = default;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

	void process() override;

private:

    enum class InteractionState { NONE, MOVE, ROTATE };

    void updateMouse(Event* e);

    ImageInport imageIn_;
    ImageOutport imageOut_;

    FloatProperty cursorAngle_;
    FloatVec2Property cursorPos_;
    FloatProperty cursorRadius_;

    EventProperty mouseEvent_;
    InteractionState interactionState_;
    FloatVec2Property lastMousePos_;

    FloatVec4Property color1_;
    FloatVec4Property color2_;
    FloatVec4Property color3_;

    IntSizeTProperty thickness1_;
    IntSizeTProperty thickness2_;

    std::shared_ptr<Mesh> crosshairMesh_;
    std::shared_ptr<Mesh> outlineMesh_;
    Shader shader_;
};
}  // namespace inviwo

#endif  // IVW_MATHPROCESSOR_H
