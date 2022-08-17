/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2022 Inviwo Foundation
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

#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <inviwo/core/datastructures/datamapper.h>

#include <inviwo/core/util/formats.h>
#include <inviwo/core/util/foreacharg.h>

namespace inviwo {

namespace detail {

struct Helper {
    template <typename Format>
    void operator()(std::vector<OptionPropertyOption<DataFormatId>>& formats) {
        formats.emplace_back(Format::str(), Format::str(), Format::id());
    }
};

#include <warn/push>
#include <warn/ignore/conversion>

struct CopyVol {
    template <typename Result, typename Format>
    Result operator()(std::shared_ptr<const Volume> src, bool mapData, dvec2 dstRange) {
        return src->getRepresentation<VolumeRAM>()->dispatch<std::shared_ptr<Volume>>(
            [&](auto vrprecision, bool mapData, dvec2 dstRange) {
                using ValueType = util::PrecisionValueType<decltype(vrprecision)>;
                const auto dims = vrprecision->getDimensions();
                const ValueType* srcData = vrprecision->getDataTyped();
                using T = typename util::same_extent<ValueType, typename Format::type>::type;
                using Tdouble = typename util::same_extent<ValueType, double>::type;

                auto dstVol = std::make_shared<VolumeRAMPrecision<T>>(
                    dims, src->getSwizzleMask(), src->getInterpolation(), src->getWrapping());
                T* dstData = dstVol->getDataTyped();

                if (mapData) {
                    const dvec2 srcRange{
                        (src->getDataFormat()->getNumericType() != NumericType::Float)
                            ? src->dataMap_.dataRange
                            : dvec2{0.0, 1.0}};
                    std::transform(srcData, srcData + glm::compMul(dims), dstData,
                                   [srcRange, dstRange](auto& v) {
                                       const auto result = (static_cast<Tdouble>(v) - srcRange.x) /
                                                               (srcRange.y - srcRange.x) *
                                                               (dstRange.y - dstRange.x) +
                                                           dstRange.x;
                                       return static_cast<T>(result);
                                   });
                } else {
                    std::transform(srcData, srcData + glm::compMul(dims), dstData,
                                   [](auto& v) { return static_cast<T>(v); });
                }

                auto vol = std::make_shared<Volume>(dstVol);
                vol->setBasis(src->getBasis());
                vol->setOffset(src->getOffset());
                vol->copyMetaDataFrom(*src.get());

                return vol;
            },
            mapData, dstRange);
    }
};

#include <warn/pop>

}  // namespace detail

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo VolumeConverter::processorInfo_{
    "org.inviwo.VolumeConverter",  // Class identifier
    "Volume Converter",            // Display name
    "Volume Operation",            // Category
    CodeState::Stable,             // Code state
    "CPU, Volume",                 // Tags
};
const ProcessorInfo VolumeConverter::getProcessorInfo() const { return processorInfo_; }

VolumeConverter::VolumeConverter()
    : Processor{}
    , inport_{"inport"}
    , outport_{"outport"}
    , inputFormat_{"inputFormat", "Input Format"}
    , format_{"outputFormat", "Output Format",
              [&]() {
                  std::vector<OptionPropertyOption<DataFormatId>> formats;
                  util::for_each_type<std::tuple<DataFloat16, DataFloat32, DataFloat64, DataInt8,
                                                 DataInt16, DataInt32, DataInt64, DataUInt8,
                                                 DataUInt16, DataUInt32, DataUInt64>>{}(
                      detail::Helper(), formats);
                  return formats;
              }(),
              1}
    , enableDataMapping_{"enableDataMapping", "Use Data Mapping", false}
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

    inputFormat_.setReadOnly(true);
    outputDataRange_.setReadOnly(true);
    outputDataRange_.setSerializationMode(PropertySerializationMode::All);
    dataRange_.insertProperty(2, outputDataRange_);
    addProperties(inputFormat_, format_, enableDataMapping_, dataRange_);

    auto updateOutputRange = [this]() {
        const auto* format = DataFormatBase::get(format_);
        if (enableDataMapping_ && (format->getNumericType() != NumericType::Float)) {
            outputDataRange_.set(DataMapper{format}.dataRange);
        } else if (inport_.hasData()) {
            outputDataRange_.set(inport_.getData()->dataMap_.dataRange);
        }
    };
    enableDataMapping_.onChange(updateOutputRange);
    format_.onChange(updateOutputRange);

    inport_.onChange([this, updateOutputRange]() {
        if (inport_.hasData()) {
            inputFormat_.set(inport_.getData()->getDataFormat()->getString());
            updateOutputRange();
        }
    });
}

void VolumeConverter::process() {
    if ((inport_.getData()->getDataFormat()->getId() == format_.get()) &&
        !dataRange_.getCustomRangeEnabled()) {
        outport_.setData(inport_.getData());
        return;
    }

    const auto dstRange = [&]() -> std::pair<dvec2, dvec2> {
        if (dataRange_.getCustomRangeEnabled()) {
            return {dataRange_.getCustomDataRange(), dataRange_.getCustomValueRange()};
        } else {
            if (enableDataMapping_) {
                if (auto format = DataFormatBase::get(format_);
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

    auto volume = [&]() {
        if (inport_.getData()->getDataFormat()->getId() == format_.get()) {
            return std::shared_ptr<Volume>(inport_.getData()->clone());
        } else {
            return dispatching::dispatch<std::shared_ptr<Volume>, dispatching::filter::Scalars>(
                format_.get(), detail::CopyVol{}, inport_.getData(), enableDataMapping_,
                dstRange.first);
        }
    }();
    volume->dataMap_.dataRange = dstRange.first;
    volume->dataMap_.valueRange = dstRange.second;

    outport_.setData(volume);
}

}  // namespace inviwo
