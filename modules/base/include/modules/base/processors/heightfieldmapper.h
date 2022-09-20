/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2022 Inviwo Foundation
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

#include <modules/base/basemoduledefine.h>  // for IVW_MODULE_BASE_API

#include <inviwo/core/ports/imageport.h>             // for ImageInport, ImageOutport
#include <inviwo/core/processors/processor.h>        // for Processor
#include <inviwo/core/processors/processorinfo.h>    // for ProcessorInfo
#include <inviwo/core/properties/minmaxproperty.h>   // for FloatMinMaxProperty
#include <inviwo/core/properties/optionproperty.h>   // for OptionPropertyInt
#include <inviwo/core/properties/ordinalproperty.h>  // for FloatProperty

namespace inviwo {

namespace HeightFieldScaling {
enum Mode {
    FixedRange,  //!< scale to [0:1]
    DataRange,   //!< scale heights given min/max values
    SeaLevel,    //!< scale heights around sea level to fit in maxHeight
};
}

/**
 * \brief Maps a 2D input texture to a single channel heightfield and scales the data values.
 *
 * Maps a heightfield onto a geometry and renders it to an image.
 * The Height Field Mapper converts an arbitrary 2D input image to a grayscale float
 * image to be used in the Height Field Renderer processor. Additionally, data values
 * are mapped to either an user-defined range or are scaled to fit in a given maximum
 * height based on the sea level.
 */
class IVW_MODULE_BASE_API HeightFieldMapper : public Processor {
public:
    HeightFieldMapper();
    ~HeightFieldMapper();

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    virtual void process() override;

private:
    ImageInport inport_;    //!< inport for the 2D heightfield texture
    ImageOutport outport_;  //!< outport for the scaled heightfield

    OptionPropertyInt scalingModeProp_;  //!< scaling mode
    FloatMinMaxProperty heightRange_;    //!< min/max range for data range scaling
    FloatProperty maxHeight_;            //!< max height used in sea level scaling
    FloatProperty seaLevel_;             //!< sea level around which the heightfield is scaled
};

}  // namespace inviwo
