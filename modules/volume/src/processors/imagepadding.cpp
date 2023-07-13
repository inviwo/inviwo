/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022 Inviwo Foundation
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

#include <inviwo/volume/processors/imagepadding.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/datastructures/image/layerramprecision.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo ImagePadding::processorInfo_{
    "org.inviwo.ImagePadding",  // Class identifier
    "Image Padding",            // Display name
    "Undefined",                // Category
    CodeState::Experimental,    // Code state
    Tags::None,                 // Tags
};
const ProcessorInfo ImagePadding::getProcessorInfo() const { return processorInfo_; }

ImagePadding::ImagePadding()
    : Processor()
    , inport_("inport")
    , outport_("outport")
    , newSize_("newSize", "New Size", size2_t(4),
               std::make_pair<size2_t, ConstraintBehavior>(size2_t(1), ConstraintBehavior::Mutable),
               {size2_t(128), ConstraintBehavior::Ignore})
    , offset_(
          "offset", "Data Offset", size2_t(0),
          std::make_pair<size2_t, ConstraintBehavior>(size2_t(0), ConstraintBehavior::Immutable),
          {size2_t(128), ConstraintBehavior::Mutable}) {
    inport_.setOutportDeterminesSize(true);
    outport_.setHandleResizeEvents(false);
    addPorts(inport_, outport_);
    addProperties(newSize_, offset_);
}

template <typename Format>
struct PropertyCompatible : std::integral_constant<bool, (Format::precision() > 16)> {};

void ImagePadding::process() {
    auto image = inport_.getData();
    size2_t inputSize = image->getDimensions();
    size2_t outputSize = newSize_.get();

    if (inport_.isChanged()) {
        newSize_.setMinValue(inputSize);
        newSize_.setMaxValue(inputSize * 2);
        outputSize = newSize_.get();

        offset_.setMaxValue(outputSize - inputSize);
    }

    auto newImage =
        image->getColorLayer()
            ->getRepresentation<LayerRAM>()
            ->dispatch<std::shared_ptr<Image>, PropertyCompatible>([&](auto precisionLayer) {
                using ValueType = util::PrecisionValueType<decltype(precisionLayer)>;
                auto newLayer = std::make_shared<LayerRAMPrecision<ValueType>>(outputSize);

                auto precisionPaddingProperty =
                    dynamic_cast<OrdinalProperty<ValueType>*>(getPropertyByIdentifier("padding"));
                if (!precisionPaddingProperty) {
                    removeProperty("padding");
                    precisionPaddingProperty =
                        new OrdinalProperty<ValueType>("padding", "Padding Value", ValueType{0});

                    addProperty(precisionPaddingProperty);  // PropertyOwner takes care of deletion.
                }
                const ValueType* data = precisionLayer->getDataTyped();
                ValueType* newData = newLayer->getDataTyped();
                size_t numElements = outputSize.x * outputSize.y;
                std::fill_n(newData, numElements, precisionPaddingProperty->get());

                for (size_t y = 0; y < inputSize.y; ++y)
                    for (size_t x = 0; x < inputSize.x; ++x) {
                        newData[(x + offset_.get().x) + (y + offset_.get().y) * outputSize.x] =
                            data[x + y * inputSize.x];
                    }
                auto layer = std::make_shared<Layer>(newLayer);
                return std::make_shared<Image>(layer);
            });

    outport_.setData(newImage);
}

}  // namespace inviwo
