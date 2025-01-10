/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2025 Inviwo Foundation
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

#include <modules/plottinggl/plottingglmoduledefine.h>  // for IVW_MODULE_PLOTTINGGL_API

#include <inviwo/core/ports/imageport.h>                    // for ImageInport, ImageOutport
#include <inviwo/core/processors/processor.h>               // for Processor
#include <inviwo/core/processors/processorinfo.h>           // for ProcessorInfo
#include <inviwo/core/properties/boolproperty.h>            // for BoolProperty
#include <inviwo/core/properties/ordinalproperty.h>         // for FloatProperty
#include <inviwo/core/properties/marginproperty.h>          // for MarginProperty
#include <modules/plotting/properties/axisproperty.h>       // for AxisProperty
#include <modules/plotting/properties/axisstyleproperty.h>  // for AxisStyleProperty
#include <modules/plottinggl/utils/axisrenderer.h>          // for AxisRenderer

#include <array>

namespace inviwo {

namespace plot {

/** \docpage{org.inviwo.AxisRenderProcessor, Axis Render Processor}
 * ![](org.inviwo.AxisRenderProcessor.png?classIdentifier=org.inviwo.AxisRenderProcessor)
 * Test processor for rendering plot axes
 */

/**
 * \class AxisRenderProcessor
 * \brief Test processor for axis rendering
 */
class IVW_MODULE_PLOTTINGGL_API AxisRenderProcessor : public Processor {
public:
    AxisRenderProcessor();
    virtual ~AxisRenderProcessor() = default;

    virtual void process() override;

    virtual const ProcessorInfo& getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    ImageInport inport_;
    ImageOutport outport_;

    MarginProperty margins_;
    FloatProperty axisMargin_;
    BoolProperty antialiasing_;

    AxisStyleProperty style_;
    AxisProperty axis1_;
    AxisProperty axis2_;
    AxisProperty axis3_;

    std::array<AxisRenderer, 3> axisRenderers_;
};

}  // namespace plot

}  // namespace inviwo
