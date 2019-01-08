/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2018 Inviwo Foundation
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

#include <modules/base/processors/pixeltobufferprocessor.h>
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/interaction/events/mouseevent.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo PixelToBufferProcessor::processorInfo_{
    "org.inviwo.PixelToBufferProcessor",  // Class identifier
    "Pixel to buffer",                    // Display name
    "Image Operation",                    // Category
    CodeState::Experimental,              // Code state
    Tags::CPU,                            // Tags
};
const ProcessorInfo PixelToBufferProcessor::getProcessorInfo() const { return processorInfo_; }

PixelToBufferProcessor::PixelToBufferProcessor()
    : Processor()
    , inport_("input", true)
    , pixelValues_("pixelValues")
    , fromPixel_("fromPixel", "From pixel", ivec2(0), ivec2(0), ivec2(1))
    , channel_("channel", "Channel", 0, 0, 3)
    , clearValues_("clearValues", "Clear collected values", InvalidationLevel::Valid)
    , handleInteractionEvents_("handleEvents", "Enable picking", false)
    , values_(std::make_shared<PosBuffer>()) {

    addPort(inport_);
    addPort(pixelValues_);

    inport_.onChange([this]() { inportChanged(); });

    addProperty(fromPixel_);
    addProperty(channel_);
    addProperty(clearValues_);
    clearValues_.onChange([this]() { clearOutput(); });
    addProperty(handleInteractionEvents_);
    pixelValues_.setData(values_);
}

void PixelToBufferProcessor::inportChanged() {
    if (!inport_.hasData()) return;

    auto data = inport_.getData();
    fromPixel_.setMaxValue(ivec2(data->getDimensions()) - 1);
    channel_.setMaxValue(static_cast<int>(data->getDataFormat()->getComponents()) - 1);
}

void PixelToBufferProcessor::process() {
    double value =
        inport_.getData()->getColorLayer()->getRepresentation<LayerRAM>()->getAsNormalizedDVec4(
            fromPixel_.get())[channel_.get()];

    auto values = values_->getEditableRepresentation<BufferRAMPrecision<double>>();
    values->add(value);
}

void PixelToBufferProcessor::setPixelToCollectFrom(const ivec2& xy) { fromPixel_.set(xy); }

void PixelToBufferProcessor::clearOutput() {
    values_->getEditableRepresentation<BufferRAMPrecision<double>>()->setSize(0);
    invalidate(InvalidationLevel::InvalidOutput);
}

void PixelToBufferProcessor::invokeEvent(Event* event) {
    if (handleInteractionEvents_) {
        if (event->hash() == MouseEvent::chash()) {
            auto mouseEvent = static_cast<MouseEvent*>(event);
            if (mouseEvent->button() == MouseButton::Left &&
                mouseEvent->state() == MouseState::Press) {
                fromPixel_.set(static_cast<ivec2>(mouseEvent->pos()));
                clearOutput();
                mouseEvent->markAsUsed();
            }
        }
    }
    if (event->hasBeenUsed()) return;

    Processor::invokeEvent(event);
}

}  // namespace inviwo
