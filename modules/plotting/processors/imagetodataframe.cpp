/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2018 Inviwo Foundation
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

#include <modules/plotting/processors/imagetodataframe.h>

#include <inviwo/core/datastructures/image/image.h>
#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/datastructures/buffer/bufferram.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/util/imageramutils.h>
#include <inviwo/core/util/indexmapper.h>

namespace inviwo {

namespace plot {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo ImageToDataFrame::processorInfo_{
    "org.inviwo.ImageToDataFrame",  // Class identifier
    "Image To DataFrame",           // Display name
    "Data Creation",                // Category
    CodeState::Experimental,        // Code state
    "CPU, DataFrame, Image",        // Tags
};
const ProcessorInfo ImageToDataFrame::getProcessorInfo() const { return processorInfo_; }

ImageToDataFrame::ImageToDataFrame() : Processor(), image_("image"), dataframe_("dataframe") {
    addPort(image_);
    addPort(dataframe_);
}

void ImageToDataFrame::process() {
    auto volume = image_.getData()->getColorLayer()->getRepresentation<LayerRAM>();
    // auto volume = volume_.getData()->getRepresentation<VolumeRAM>();
    auto dims = volume->getDimensions();
    auto size = dims.x * dims.y;

    auto dataFrame = std::make_shared<plot::DataFrame>(static_cast<glm::u32>(size));

    auto index = util::IndexMapper2D(dims);

    std::vector<std::vector<float>*> channelBuffer_;
    auto numCh = volume->getDataFormat()->getComponents();
    for (size_t c = 0; c < numCh; c++) {
        auto col = dataFrame->addColumn<float>("Channel " + toString(c + 1), size);
        channelBuffer_.push_back(
            &col->getTypedBuffer()->getEditableRAMRepresentation()->getDataContainer());
    }
    std::vector<float>* magnitudes = &dataFrame->addColumn<float>("Magnitude", size)
                                          ->getTypedBuffer()
                                          ->getEditableRAMRepresentation()
                                          ->getDataContainer();
    std::vector<float>* greycalePerceived = nullptr;
    std::vector<float>* greycaleRelative = nullptr;
    std::vector<float>* averageRGB = nullptr;
    if (numCh >= 3) {
        greycalePerceived = &dataFrame->addColumn<float>("Luminance (perceived) (from RGB)", size)
                                 ->getTypedBuffer()
                                 ->getEditableRAMRepresentation()
                                 ->getDataContainer();
        greycaleRelative = &dataFrame->addColumn<float>("Luminance (relative) (from RGB)", size)
                                ->getTypedBuffer()
                                ->getEditableRAMRepresentation()
                                ->getDataContainer();
        averageRGB = &dataFrame->addColumn<float>("Luminance (average) (from RGB)", size)
                          ->getTypedBuffer()
                          ->getEditableRAMRepresentation()
                          ->getDataContainer();
    }
    std::vector<float>* averageAll = &dataFrame->addColumn<float>("Average (all channels)", size)
                                          ->getTypedBuffer()
                                          ->getEditableRAMRepresentation()
                                          ->getDataContainer();

    // Calues copied from imagegrayscale.cpp
    static const dvec3 perceivedLum(0.299, 0.587f, 0.114f);
    static const dvec3 relativeLum(0.2126, 0.7152, 0.0722);
    static const dvec3 avgLum(1.0 / 3.0);

    util::forEachPixel(*volume, [&](const size2_t& pos) {
        auto idx = index(pos);
        auto v = volume->getAsDVec4(pos);
        float m = 0;
        float sum = 0;
        for (size_t c = 0; c < numCh; c++) {
            channelBuffer_[c]->at(idx) = v[c];
            m += v[c] * v[c];
            sum += v[c];
        }
        if (greycalePerceived != nullptr) {
            greycalePerceived->at(idx) = glm::dot(perceivedLum, dvec3(v));
        }
        if (greycaleRelative != nullptr) {
            greycaleRelative->at(idx) = glm::dot(relativeLum, dvec3(v));
        }
        if (averageRGB != nullptr) {
            averageRGB->at(idx) = glm::dot(avgLum, dvec3(v));
        }

        magnitudes->at(idx) = std::sqrt(m);
        averageAll->at(idx) = sum / numCh;
    });
    dataframe_.setData(dataFrame);
}

}  // namespace plot

}  // namespace inviwo
