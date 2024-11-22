/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2024 Inviwo Foundation
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
#include <inviwo/core/properties/compositeproperty.h>       // for CompositeProperty
#include <inviwo/core/properties/minmaxproperty.h>          // for DoubleMinMaxProperty
#include <inviwo/core/properties/optionproperty.h>          // for OptionProperty
#include <inviwo/core/properties/ordinalproperty.h>         // for FloatProperty
#include <inviwo/core/properties/marginproperty.h>          // for MarginProperty
#include <inviwo/core/util/glmvec.h>                        // for size2_t, ivec2, ivec4
#include <inviwo/core/util/staticstring.h>                  // for operator+
#include <modules/basegl/viewmanager.h>                     // for ViewManager
#include <modules/opengl/rendering/texturequadrenderer.h>   // for TextureQuadRenderer
#include <modules/plotting/properties/axisproperty.h>       // for AxisProperty
#include <modules/plotting/properties/axisstyleproperty.h>  // for AxisStyleProperty
#include <modules/plottinggl/utils/axisrenderer.h>          // for AxisRenderer

#include <array>        // for array
#include <functional>   // for __base
#include <string>       // for operator==, operator+, string
#include <string_view>  // for operator==
#include <vector>       // for operator!=, vector, operator==

namespace inviwo {
class Event;
class Outport;

namespace plot {

/**
 * \brief plot an image with an x and y axis
 * Event handling based on ViewManager
 * \see ViewManager, ImageOverlayGL
 */
class IVW_MODULE_PLOTTINGGL_API ImagePlotProcessor : public Processor {
public:
    enum class AxisRangeMode { ImageDims, ImageBasis, ImageBasisOffset, Custom };

    ImagePlotProcessor();
    virtual ~ImagePlotProcessor() = default;

    virtual void process() override;

    virtual void propagateEvent(Event*, Outport* source) override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    void updateViewport();
    void updateViewport(size2_t dim, bool force = false);
    void onStatusChange();

private:
    struct ImageBounds {
        ivec2 pos;
        size2_t extent;
    };

    ImageBounds calcImageBounds(const size2_t& dims) const;
    void adjustRanges();

    ImageInport imgInport_;
    ImageInport bgInport_;
    ImageOutport outport_;

    MarginProperty margins_;
    FloatProperty axisMargin_;

    IntVec2Property plotImageSize_;

    OptionProperty<AxisRangeMode> rangeMode_;

    CompositeProperty customRanges_;
    DoubleMinMaxProperty rangeXaxis_;
    DoubleMinMaxProperty rangeYaxis_;

    AxisStyleProperty axisStyle_;
    AxisProperty xAxis_;
    AxisProperty yAxis_;

    BoolProperty imageInteraction_;  //<! allows to interact with the images, otherwise only
                                     // the background image will receive interaction events

    std::array<AxisRenderer, 2> axisRenderers_;
    TextureQuadRenderer imgRenderer_;

    ViewManager viewManager_;
    ivec4 viewport_;
    size2_t imgDims_;
    bool propertyUpdate_ = false;
};

}  // namespace plot

}  // namespace inviwo
