/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2024 Inviwo Foundation
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

#include <modules/base/processors/volumeconverter.h>

#include <inviwo/core/datastructures/datamapper.h>                      // for DataMapper
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/datastructures/volume/volume.h>                   // for Volume
#include <inviwo/core/datastructures/volume/volumeram.h>                // for VolumeRAM
#include <inviwo/core/ports/volumeport.h>                               // for VolumeInport, Vol...
#include <inviwo/core/processors/processor.h>                           // for Processor
#include <inviwo/core/processors/processorinfo.h>                       // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                      // for CodeState, CodeSt...
#include <inviwo/core/processors/processortags.h>                       // for Tags
#include <inviwo/core/properties/boolproperty.h>                        // for BoolProperty
#include <inviwo/core/properties/invalidationlevel.h>                   // for InvalidationLevel
#include <inviwo/core/properties/minmaxproperty.h>                      // for DoubleMinMaxProperty
#include <inviwo/core/properties/optionproperty.h>                      // for OptionPropertyOption
#include <inviwo/core/properties/propertysemantics.h>                   // for PropertySemantics
#include <inviwo/core/properties/stringproperty.h>                      // for StringProperty
#include <inviwo/core/properties/valuewrapper.h>                        // for PropertySerializa...
#include <inviwo/core/util/foreacharg.h>                                // for for_each_type
#include <inviwo/core/util/formatdispatching.h>                         // for dispatch, Precisi...
#include <inviwo/core/util/formats.h>                                   // for DataFormatBase
#include <inviwo/core/util/glmutils.h>                                  // for same_extent
#include <inviwo/core/util/glmvec.h>                                    // for dvec2
#include <inviwo/core/util/staticstring.h>                              // for operator+
#include <modules/base/properties/datarangeproperty.h>                  // for DataRangeProperty

#include <algorithm>      // for transform
#include <limits>         // for numeric_limits
#include <memory>         // for shared_ptr, share...
#include <tuple>          // for tuple
#include <type_traits>    // for remove_extent_t
#include <unordered_set>  // for unordered_set
#include <utility>        // for pair

#include <glm/gtx/component_wise.hpp>  // for compMul
#include <glm/vec2.hpp>                // for vec<>::(anonymous)

namespace inviwo {

namespace {

struct Helper {
    template <typename Format>
    void operator()(std::vector<OptionPropertyOption<DataFormatId>>& formats) {
        formats.emplace_back(Format::str(), Format::str(), Format::id());
    }
};

}  // namespace

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo VolumeConverter::processorInfo_{
    "org.inviwo.VolumeConverter",  // Class identifier
    "Volume Converter",            // Display name
    "Volume Operation",            // Category
    CodeState::Stable,             // Code state
    "CPU, Volume",                 // Tags
    R"(Converts the data type of a volume to a given output data format.
    The number of channels remains unchanged.)"_unindentHelp};

const ProcessorInfo VolumeConverter::getProcessorInfo() const { return processorInfo_; }

VolumeConverter::VolumeConverter()
    : Processor{}
    , inport_{"inport", "volume input"_help}
    , outport_{"outport", "converted output volume"_help}
    , srcFormat_{"inputFormat", "Input Format"}
    , dstFormat_{"outputFormat", "Output Format", R"(Data format of the output volume.
        The number of channels remains the same.
        If identical to input data, no conversion will be performed.)"_unindentHelp,
                 [&]() {
                     std::vector<OptionPropertyOption<DataFormatId>> formats;
                     util::for_each_type<std::tuple<DataFloat16, DataFloat32, DataFloat64, DataInt8,
                                                    DataInt16, DataInt32, DataInt64, DataUInt8,
                                                    DataUInt16, DataUInt32, DataUInt64>>{}(Helper(),
                                                                                           formats);
                     return formats;
                 }(),
                 1}
    , enableDataMapping_{"enableDataMapping", "Use Data Mapping",
                         R"(If enabled, the processor will utilize data mapping between different
        integer formats. That is, each data value is normalized using the data range of
        the input volume before being adjusted to the target format (using the range of
        the data type). For example in a conversion from `uint8 [0 255]` to
        `uint16 [0 65536]`, a value of `255` will be mapped to `65536`. If the target
        format is floating point, then data values are only normalized.
        Float formats are __not__ normalized!)"_unindentHelp,
                         false}
    , dataRange_{"dataRange", "Data Range", inport_, true}
    , outputDataRange_{"outputDataRange",
                       "Output Data Range",
                       0.0,
                       1.0,
                       std::numeric_limits<double>::lowest(),
                       std::numeric_limits<double>::max(),
                       0.01,
                       0.0,
                       InvalidationLevel::InvalidOutput,
                       PropertySemantics::Text} {

    addPort(inport_);
    addPort(outport_);

    srcFormat_.setReadOnly(true);
    outputDataRange_.setReadOnly(true);
    outputDataRange_.setSerializationMode(PropertySerializationMode::All);
    dataRange_.insertProperty(2, outputDataRange_);
    addProperties(srcFormat_, dstFormat_, enableDataMapping_, dataRange_);

    auto updateOutputRange = [this]() {
        const auto* format = DataFormatBase::get(dstFormat_);
        if (enableDataMapping_ && (format->getNumericType() != NumericType::Float)) {
            outputDataRange_.set(DataMapper{format}.dataRange);
        } else if (inport_.hasData()) {
            outputDataRange_.set(inport_.getData()->dataMap_.dataRange);
        }
    };
    enableDataMapping_.onChange(updateOutputRange);
    dstFormat_.onChange(updateOutputRange);

    inport_.onChange([this, updateOutputRange]() {
        if (inport_.hasData()) {
            srcFormat_.set(inport_.getData()->getDataFormat()->getString());
            updateOutputRange();
        }
    });
}

namespace {

#include <warn/push>
#include <warn/ignore/conversion>

std::shared_ptr<Volume> convertVolume(DataFormatId dstScalarFormatId, const Volume& src,
                                      bool mapData, dvec2 dstRange) {

    auto ramSrc = src.getRepresentation<VolumeRAM>();

    auto ramDst = dispatching::doubleDispatch<
        std::shared_ptr<VolumeRAM>, dispatching::filter::Scalars, dispatching::filter::All>(
        dstScalarFormatId, ramSrc->getDataFormatId(),
        [&]<typename DstScalarFormat, typename SrcFormat>() {
            const auto* ramSrcTyped = static_cast<const VolumeRAMPrecision<SrcFormat>*>(ramSrc);
            std::span<const SrcFormat> srcData = ramSrcTyped->getView();

            using DstFormat = util::same_extent_t<SrcFormat, DstScalarFormat>;
            auto ramDst = std::make_shared<VolumeRAMPrecision<DstFormat>>(noData, *ramSrcTyped);
            std::span<DstFormat> dstData = ramDst->getView();

            if (mapData) {
                const dvec2 srcRange{(src.getDataFormat()->getNumericType() != NumericType::Float)
                                         ? src.dataMap_.dataRange
                                         : dvec2{0.0, 1.0}};

                using DstDoubleFormat = util::same_extent_t<SrcFormat, double>;
                std::transform(srcData.begin(), srcData.end(), dstData.begin(),
                               [srcRange, dstRange](const auto& v) {
                                   const auto result =
                                       (static_cast<DstDoubleFormat>(v) - srcRange.x) /
                                           (srcRange.y - srcRange.x) * (dstRange.y - dstRange.x) +
                                       dstRange.x;
                                   return static_cast<DstFormat>(result);
                               });
            } else {
                std::transform(srcData.begin(), srcData.end(), dstData.begin(),
                               [](const auto& v) { return static_cast<DstFormat>(v); });
            }

            return ramDst;
        });

    auto vol = std::make_shared<Volume>(ramDst);
    vol->setBasis(src.getBasis());
    vol->setOffset(src.getOffset());
    vol->copyMetaDataFrom(src);
    return vol;
}

#include <warn/pop>

}  // namespace

void VolumeConverter::process() {
    if ((inport_.getData()->getDataFormat()->getId() == dstFormat_.get()) &&
        !dataRange_.getCustomRangeEnabled()) {
        outport_.setData(inport_.getData());
        return;
    }

    const auto dstRange = [&]() -> std::pair<dvec2, dvec2> {
        if (dataRange_.getCustomRangeEnabled()) {
            return {dataRange_.getCustomDataRange(), dataRange_.getCustomValueRange()};
        } else {
            if (enableDataMapping_) {
                if (auto format = DataFormatBase::get(dstFormat_);
                    format->getNumericType() != NumericType::Float) {
                    DataMapper mapper{format};
                    return {mapper.dataRange, mapper.valueRange};
                } else {
                    return {dvec2{0.0, 1.0}, dvec2{0.0, 1.0}};
                }
            } else {
                return {inport_.getData()->dataMap_.dataRange,
                        inport_.getData()->dataMap_.valueRange};
            }
        }
    }();

    auto dstVolume = [&]() {
        auto srcVolume = inport_.getData();
        if (srcVolume->getDataFormat()->getId() == dstFormat_.get()) {
            return std::shared_ptr<Volume>(srcVolume->clone());
        } else {
            return convertVolume(dstFormat_.get(), *srcVolume, enableDataMapping_, dstRange.first);
        }
    }();
    dstVolume->dataMap_.dataRange = dstRange.first;
    dstVolume->dataMap_.valueRange = dstRange.second;

    outport_.setData(dstVolume);
}

}  // namespace inviwo
