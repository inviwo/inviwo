/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2022 Inviwo Foundation
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

#include <inviwo/dataframe/dataframemoduledefine.h>       // for IVW_MODULE_DATAFRAME_API

#include <inviwo/core/datastructures/image/imagetypes.h>  // for LayerType, operator<<
#include <inviwo/core/ports/dataoutport.h>                // for DataOutport
#include <inviwo/core/ports/imageport.h>                  // for ImageInport
#include <inviwo/core/processors/processor.h>             // for Processor
#include <inviwo/core/processors/processorinfo.h>         // for ProcessorInfo
#include <inviwo/core/properties/minmaxproperty.h>        // for IntSizeTMinMaxProperty
#include <inviwo/core/properties/optionproperty.h>        // for OptionProperty
#include <inviwo/core/properties/ordinalproperty.h>       // for IntSizeTProperty
#include <inviwo/core/util/staticstring.h>                // for operator+
#include <inviwo/dataframe/datastructures/dataframe.h>    // for DataFrame

#include <functional>                                     // for __base
#include <string>                                         // for operator==, operator+, string
#include <string_view>                                    // for operator==
#include <vector>                                         // for operator!=, vector, operator==

#include <fmt/core.h>                                     // for format

namespace inviwo {

/** \docpage{org.inviwo.ImageToDataFrame, Image To DataFrame}
 * ![](org.inviwo.ImageToDataFrame.png?classIdentifier=org.inviwo.ImageToDataFrame)
 * This processor converts an image into a DataFrame.
 *
 * ### Inports
 *   * __image__  source image
 *
 * ### Outports
 *   * __outport__  generated DataFrame
 *
 * ### Properties
 *   * __Mode__  The processor can operate in 3 modes: Analytics, where data for each pixel is
 *               outputted, or Rows/Columns where one column for each line of pixel in the
 *               specified direction is outputted.
 *   * __Layer__ The image layer to use
 *   * __Color Index__ The color layer index
 *   * __Range__ range of rows/columns to use.
 */

class IVW_MODULE_DATAFRAME_API ImageToDataFrame : public Processor {
public:
    ImageToDataFrame();
    virtual ~ImageToDataFrame() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    enum class Mode { Analytics, Rows, Columns };

    ImageInport inport_;
    DataOutport<DataFrame> outport_;
    OptionProperty<Mode> mode_;
    OptionProperty<LayerType> layer_;
    IntSizeTProperty layerIndex_;

    IntSizeTMinMaxProperty range_;
};

}  // namespace inviwo
