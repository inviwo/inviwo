/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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

#include <inviwo/dataframe/processors/imagetodataframe.h>

#include <inviwo/core/datastructures/image/image.h>
#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/datastructures/buffer/bufferram.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/datastructures/image/layerramprecision.h>
#include <inviwo/core/util/imageramutils.h>
#include <inviwo/core/util/indexmapper.h>
#include <inviwo/core/util/stringconversion.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo ImageToDataFrame::processorInfo_{
    "org.inviwo.ImageToDataFrame",  // Class identifier
    "Image To DataFrame",           // Display name
    "Data Creation",                // Category
    CodeState::Stable,              // Code state
    "CPU, DataFrame, Image",        // Tags
};
const ProcessorInfo ImageToDataFrame::getProcessorInfo() const { return processorInfo_; }

ImageToDataFrame::ImageToDataFrame()
    : Processor()
    , inport_("image")
    , outport_("dataframe")
    , mode_{"mode",
            "Mode",
            {{"analytics", "Analytics", Mode::Analytics},
             {"rows", "Rows", Mode::Rows},
             {"columns", "Columns", Mode::Columns}},
            0}
    , layer_{"layer",
             "Layer",
             {{"color", "Color", LayerType::Color},
              {"depth", "Depth", LayerType::Depth},
              {"picking", "Picking", LayerType::Picking}},
             0}
    , layerIndex_{"colorIndex", "Color Index", 0, 0, 0, 1}
    , range_{"range", "Range", 0, 1, 0, 1, 1, 1} {

    addPort(inport_);
    addPort(outport_);

    addProperty(mode_);
    addProperty(layer_);
    addProperty(layerIndex_);
    addProperty(range_);

    auto updateRange = [this]() {
        if (inport_.hasData()) {
            if (mode_ == Mode::Rows) {
                range_.setRangeMax(inport_.getData()->getDimensions().y);
                range_.setReadOnly(false);
            } else if (mode_ == Mode::Columns) {
                range_.setRangeMax(inport_.getData()->getDimensions().x);
                range_.setReadOnly(false);
            } else {
                range_.setReadOnly(true);
            }
        }
    };

    mode_.onChange(updateRange);

    inport_.onChange([this, updateRange]() {
        if (inport_.hasData()) {
            layerIndex_.setMaxValue(inport_.getData()->getNumberOfColorLayers());
            updateRange();
        }
    });
}

void ImageToDataFrame::process() {
    auto layer = inport_.getData()->getLayer(layer_, layerIndex_)->getRepresentation<LayerRAM>();
    auto dims = layer->getDimensions();

    switch (mode_.get()) {
        case Mode::Analytics: {

            auto size = dims.x * dims.y;
            auto dataFrame = std::make_shared<DataFrame>(static_cast<glm::u32>(size));

            std::vector<std::vector<float>*> channelBuffer_;
            auto numCh = layer->getDataFormat()->getComponents();
            for (size_t c = 0; c < numCh; c++) {
                auto col = dataFrame->addColumn<float>("Channel " + toString(c + 1), size);
                channelBuffer_.push_back(
                    &col->getTypedBuffer()->getEditableRAMRepresentation()->getDataContainer());
            }

            auto& magnitudes = dataFrame->addColumn<float>("Magnitude", size)
                                   ->getTypedBuffer()
                                   ->getEditableRAMRepresentation()
                                   ->getDataContainer();

            std::vector<float>* grayscalePerceived = nullptr;
            std::vector<float>* grayscaleRelative = nullptr;
            std::vector<float>* averageRGB = nullptr;
            if (numCh >= 3) {
                grayscalePerceived =
                    &dataFrame->addColumn<float>("Luminance (perceived) (from RGB)", size)
                         ->getTypedBuffer()
                         ->getEditableRAMRepresentation()
                         ->getDataContainer();
                grayscaleRelative =
                    &dataFrame->addColumn<float>("Luminance (relative) (from RGB)", size)
                         ->getTypedBuffer()
                         ->getEditableRAMRepresentation()
                         ->getDataContainer();
                averageRGB = &dataFrame->addColumn<float>("Luminance (average) (from RGB)", size)
                                  ->getTypedBuffer()
                                  ->getEditableRAMRepresentation()
                                  ->getDataContainer();
            }

            auto& averageAll = dataFrame->addColumn<float>("Average (all channels)", size)
                                   ->getTypedBuffer()
                                   ->getEditableRAMRepresentation()
                                   ->getDataContainer();
            auto& row = dataFrame->addColumn<int>("Row", size)
                            ->getTypedBuffer()
                            ->getEditableRAMRepresentation()
                            ->getDataContainer();
            auto& col = dataFrame->addColumn<int>("Column", size)
                            ->getTypedBuffer()
                            ->getEditableRAMRepresentation()
                            ->getDataContainer();

            // Values copied from imagegrayscale.cpp
            static const vec3 perceivedLum(0.299f, 0.587f, 0.114f);
            static const vec3 relativeLum(0.2126f, 0.7152f, 0.0722f);
            static const vec3 avgLum(1.0f / 3.0f);

            layer->dispatch<void>([&](const auto lr) {
                using ValueType = util::PrecisionValueType<decltype(lr)>;
                const auto im = util::IndexMapper2D(dims);
                const auto data = lr->getDataTyped();

                size2_t pos;
                for (pos.y = 0; pos.y < dims.y; pos.y++) {
                    for (pos.x = 0; pos.x < dims.x; pos.x++) {
                        const auto idx = im(pos);
                        const auto v = util::glm_convert<dvec4>(data[idx]);

                        double m = 0.0;
                        double sum = 0.0;
                        for (size_t c = 0; c < DataFormat<ValueType>::comp; c++) {
                            (*channelBuffer_[c])[idx] = static_cast<float>(v[c]);
                            m += v[c] * v[c];
                            sum += v[c];
                        }
                        if (grayscalePerceived) {
                            (*grayscalePerceived)[idx] = glm::dot(perceivedLum, vec3(v));
                        }
                        if (grayscaleRelative) {
                            (*grayscaleRelative)[idx] = glm::dot(relativeLum, vec3(v));
                        }
                        if (averageRGB) {
                            (*averageRGB)[idx] = glm::dot(avgLum, vec3(v));
                        }

                        magnitudes[idx] = static_cast<float>(std::sqrt(m));
                        averageAll[idx] = static_cast<float>(sum / DataFormat<ValueType>::comp);
                        row[idx] = static_cast<int>(pos.y);
                        col[idx] = static_cast<int>(pos.x);
                    }
                }
            });

            outport_.setData(dataFrame);
            break;
        }
        case Mode::Rows: {
            auto dataFrame = std::make_shared<DataFrame>(static_cast<glm::u32>(dims.x));
            layer->dispatch<void>([&](const auto lr) {
                using ValueType = util::PrecisionValueType<decltype(lr)>;
                const auto im = util::IndexMapper2D(dims);
                const auto data = lr->getDataTyped();
                for (size_t j = range_.getStart(); j < range_.getEnd(); ++j) {
                    auto& row = dataFrame->addColumn<ValueType>(toString(j), dims.x)
                                    ->getTypedBuffer()
                                    ->getEditableRAMRepresentation()
                                    ->getDataContainer();

                    for (size_t i = 0; i < dims.x; ++i) {
                        row[i] = data[im(i, j)];
                    }
                }
            });

            outport_.setData(dataFrame);
            break;
        }
        case Mode::Columns: {
            auto dataFrame = std::make_shared<DataFrame>(static_cast<glm::u32>(dims.y));
            layer->dispatch<void>([&](const auto lr) {
                using ValueType = util::PrecisionValueType<decltype(lr)>;
                const auto im = util::IndexMapper2D(dims);
                const auto data = lr->getDataTyped();
                for (size_t i = range_.getStart(); i < range_.getEnd(); ++i) {
                    auto& col = dataFrame->addColumn<ValueType>(toString(i), dims.y)
                                    ->getTypedBuffer()
                                    ->getEditableRAMRepresentation()
                                    ->getDataContainer();

                    for (size_t j = 0; j < dims.y; ++i) {
                        col[j] = data[im(i, j)];
                    }
                }
            });
            outport_.setData(dataFrame);
            break;
        }
    }
}

}  // namespace inviwo
