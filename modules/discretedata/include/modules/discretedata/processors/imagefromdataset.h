/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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

#include <modules/discretedata/discretedatamoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/ports/imageport.h>
#include <modules/discretedata/properties/datachannelproperty.h>
#include <modules/discretedata/sampling/datasetspatialsampler.h>
#include <type_traits>
// #include <inviwo/core/datastructures/image/imageramprecision.h>
#include <inviwo/core/datastructures/image/layerramprecision.h>

#include <modules/discretedata/ports/datasetport.h>

#include <inviwo/core/properties/boolproperty.h>

namespace inviwo {
namespace discretedata {

/** \docpage{org.inviwo.ImageFromDataSet, Image From Data Set}
    ![](org.inviwo.ImageFromDataSet.png?classIdentifier=org.inviwo.ImageFromDataSet)

    Converts a DataSet into a Image.

    ### Inports
      * __InDataSet__ Input DataSet to be converted.

    ### Outports
      * __OutImage__ Converted image.
*/

/** \class ImageFromDataSet
    \brief Converts a DataSet to a Image
*/
class IVW_MODULE_DISCRETEDATA_API ImageFromDataSet : public Processor {
    // Construction / Deconstruction
public:
    ImageFromDataSet();
    virtual ~ImageFromDataSet() = default;

    // Methods
public:
    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    /// Our main computation function
    virtual void process() override;

    // template <typename T, ind N>
    std::shared_ptr<inviwo::Image> sampleImage(
        const std::shared_ptr<const Channel>& channel,
        const std::shared_ptr<const DataSetSamplerBase>& samplerBase) const;

    // template <typename T, typename To = T>
    // inviwo::Image* convertTo(const Channel& channel, const StructuredGrid<2>& grid);

    // template <typename T>
    // inviwo::Image* sample(const Channel& channel, const StructuredGrid<2>& grid);

    // Ports
public:
    /// Input dataset
    DataSetInport dataIn_;

    /// Output Image
    ImageOutport imageOut_;

    // DataChannelProperty dataChannel_;

    // TemplateOptionProperty<InterpolationType> interpolationType_;
    // // BoolProperty floatImageOutput;

    OptionPropertyString datasetSamplerName_;
    TemplateOptionProperty<InterpolationType> interpolationType_;
    DataChannelProperty dataChannel_;

    IntSize2Property imageSize_;
};

}  // namespace discretedata
}  // namespace inviwo
