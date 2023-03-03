/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2023 Inviwo Foundation
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

#include <inviwo/core/ports/imageport.h>                     // for ImageInport, ImageOutport
#include <inviwo/core/processors/processor.h>                // for Processor
#include <inviwo/core/processors/processorinfo.h>            // for ProcessorInfo
#include <inviwo/core/properties/optionproperty.h>           // for OptionProperty
#include <inviwo/core/properties/ordinalproperty.h>          // for FloatProperty
#include <inviwo/core/util/staticstring.h>                   // for operator+
#include <modules/basegl/datastructures/splittersettings.h>  // for Direction
#include <modules/basegl/properties/splitterproperty.h>      // for SplitterProperty
#include <modules/basegl/rendering/splitterrenderer.h>       // for SplitterRenderer

#include <functional>   // for __base
#include <string>       // for operator==, string
#include <string_view>  // for operator==
#include <vector>       // for operator!=, vector, operator==

namespace inviwo {

/** \docpage{org.inviwo.SplitImage, Split Image}
 * ![](org.inviwo.SplitImage.png?classIdentifier=org.inviwo.SplitImage)
 * Split screen of two input images. The images are split in the middle either horizontally or
 * vertically.
 *
 * ### Inports
 *   * __inputA__  first image (left/top)
 *   * __inputB__  second image (right/bottom)
 *
 * ### Outports
 *   * __outport__  resulting image where the two input images are split in the middle
 *
 * ### Properties
 *   * __Split Direction__       split direction, i.e. either vertical or horizontal
 *   * __Split Position__        normalized split position [0,1]
 */

/**
 * \class SplitImage
 * \brief Processor providing split screen functionality for two images
 */
class IVW_MODULE_BASEGL_API SplitImage : public Processor {
public:
    SplitImage();
    virtual ~SplitImage() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    ImageInport inport0_;
    ImageInport inport1_;
    ImageOutport outport_;

    OptionProperty<splitter::Direction> splitDirection_;
    FloatProperty splitPosition_;

    SplitterProperty splitterSettings_;
    SplitterRenderer renderer_;
};

}  // namespace inviwo
