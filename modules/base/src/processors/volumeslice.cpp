/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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

#include <modules/base/processors/volumeslice.h>
#include <inviwo/core/interaction/events/keyboardevent.h>
#include <inviwo/core/interaction/events/wheelevent.h>
#include <inviwo/core/interaction/events/gestureevent.h>

#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <inviwo/core/datastructures/image/imageram.h>
#include <inviwo/core/datastructures/image/layerramprecision.h>

#include <inviwo/core/util/indexmapper.h>

namespace inviwo {

const ProcessorInfo VolumeSlice::processorInfo_{
    "org.inviwo.VolumeSlice",  // Class identifier
    "Volume Slice",            // Display name
    "Volume Operation",        // Category
    CodeState::Stable,         // Code state
    Tags::CPU,                 // Tags
};
const ProcessorInfo VolumeSlice::getProcessorInfo() const { return processorInfo_; }

VolumeSlice::VolumeSlice()
    : Processor()
    , inport_("inputVolume")
    , outport_("outputImage", DataVec4UInt8::get(), false)
    , sliceAlongAxis_("sliceAxis", "Slice along axis",
                      {{"x", "X axis", CartesianCoordinateAxis::X},
                       {"y", "Y axis", CartesianCoordinateAxis::Y},
                       {"z", "Z axis", CartesianCoordinateAxis::Z}},
                      0)
    , sliceNumber_("sliceNumber", "Slice Number", 4, 1, 8)
    , handleInteractionEvents_("handleEvents", "Handle interaction events", true,
                               InvalidationLevel::Valid)
    , mouseShiftSlice_("mouseShiftSlice", "Mouse Slice Shift",
                       [this](Event* e) { eventShiftSlice(e); },
                       std::make_unique<WheelEventMatcher>())

    , stepSliceUp_("stepSliceUp", "Key Slice Up", [this](Event* e) { eventStepSliceUp(e); },
                   IvwKey::W, KeyState::Press)

    , stepSliceDown_("stepSliceDown", "Key Slice Down", [this](Event* e) { eventStepSliceDown(e); },
                     IvwKey::S, KeyState::Press)

    , gestureShiftSlice_(
          "gestureShiftSlice", "Gesture Slice Shift",
          [this](Event* e) { eventGestureShiftSlice(e); },
          std::make_unique<GestureEventMatcher>(GestureType::Pan, GestureStates(flags::any), 3)) {

    addPort(inport_);
    addPort(outport_);
    addProperty(sliceAlongAxis_);
    addProperty(sliceNumber_);
    addProperty(handleInteractionEvents_);

    addProperty(stepSliceUp_);
    addProperty(stepSliceDown_);

    mouseShiftSlice_.setVisible(false);
    mouseShiftSlice_.setCurrentStateAsDefault();
    addProperty(mouseShiftSlice_);

    gestureShiftSlice_.setVisible(false);
    gestureShiftSlice_.setCurrentStateAsDefault();
    addProperty(gestureShiftSlice_);
}

VolumeSlice::~VolumeSlice() = default;

void VolumeSlice::invokeEvent(Event* event) {
    if (!handleInteractionEvents_) return;
    Processor::invokeEvent(event);
}

void VolumeSlice::shiftSlice(int shift) {
    auto newSlice = static_cast<size_t>(sliceNumber_.get() + shift);
    if (newSlice >= sliceNumber_.getMinValue() && newSlice <= sliceNumber_.getMaxValue()) {
        sliceNumber_.set(newSlice);
    }
}

void VolumeSlice::process() {
    auto vol = inport_.getData();

    const auto dims(vol->getDimensions());
    double pos{sliceNumber_.get() / static_cast<double>(sliceNumber_.getMaxValue())};
    switch (sliceAlongAxis_.get()) {
        case CartesianCoordinateAxis::X:
            if (dims.x != sliceNumber_.getMaxValue()) {
                sliceNumber_.setMaxValue(dims.x);
                sliceNumber_.set(static_cast<size_t>(pos * dims.x));
            }
            break;
        case CartesianCoordinateAxis::Y:
            if (dims.y != sliceNumber_.getMaxValue()) {
                sliceNumber_.setMaxValue(dims.y);
                sliceNumber_.set(static_cast<size_t>(pos * dims.y));
            }
            break;
        case CartesianCoordinateAxis::Z:
            if (dims.z != sliceNumber_.getMaxValue()) {
                sliceNumber_.setMaxValue(dims.z);
                sliceNumber_.set(static_cast<size_t>(pos * dims.z));
            }
            break;
    }

    auto image =
        vol->getRepresentation<VolumeRAM>()
            ->dispatch<std::shared_ptr<Image>, dispatching::filter::All>(
                [axis = static_cast<CartesianCoordinateAxis>(sliceAlongAxis_.get()),
                 slice = static_cast<size_t>(sliceNumber_.get() - 1),
                 &cache = imageCache_](const auto vrprecision) {
                    using T = util::PrecisionValueType<decltype(vrprecision)>;

                    const T* voldata = vrprecision->getDataTyped();
                    const auto voldim = vrprecision->getDimensions();

                    const auto imgdim = [&]() {
                        switch (axis) {
                            default:
                                return size2_t(voldim.z, voldim.y);
                            case CartesianCoordinateAxis::X:
                                return size2_t(voldim.z, voldim.y);
                            case CartesianCoordinateAxis::Y:
                                return size2_t(voldim.x, voldim.z);
                            case CartesianCoordinateAxis::Z:
                                return size2_t(voldim.x, voldim.y);
                        }
                    }();

                    auto res = cache.getTypedUnused<T>(imgdim);
                    auto sliceImage = res.first;
                    auto layerrep = res.second;
                    auto layerdata = layerrep->getDataTyped();

                    switch (util::extent<T, 0>::value) {
                        case 0:  // util::extent<T, 0>::value returns zero for non-glm types
                        case 1:
                            layerrep->setSwizzleMask({{ImageChannel::Red, ImageChannel::Red,
                                                       ImageChannel::Red, ImageChannel::One}});
                            break;
                        case 2:
                            layerrep->setSwizzleMask({{ImageChannel::Red, ImageChannel::Green,
                                                       ImageChannel::Zero, ImageChannel::One}});
                            break;
                        case 3:
                            layerrep->setSwizzleMask({{ImageChannel::Red, ImageChannel::Green,
                                                       ImageChannel::Blue, ImageChannel::One}});
                            break;
                        default:
                        case 4:
                            layerrep->setSwizzleMask({{ImageChannel::Red, ImageChannel::Green,
                                                       ImageChannel::Blue, ImageChannel::Alpha}});
                    }

                    size_t offsetVolume;
                    size_t offsetImage;
                    switch (axis) {
                        case CartesianCoordinateAxis::X: {
                            util::IndexMapper3D vm(voldim);
                            util::IndexMapper2D im(imgdim);
                            auto x = glm::clamp(slice, size_t{0}, voldim.x - 1);
                            for (size_t z = 0; z < voldim.z; z++) {
                                for (size_t y = 0; y < voldim.y; y++) {
                                    offsetVolume = vm(x, y, z);
                                    offsetImage = im(z, y);
                                    layerdata[offsetImage] = voldata[offsetVolume];
                                }
                            }
                            break;
                        }
                        case CartesianCoordinateAxis::Y: {
                            auto y = glm::clamp(slice, size_t{0}, voldim.y - 1);
                            const size_t dataSize = voldim.x;
                            const size_t initialStartPos = y * voldim.x;
                            for (size_t j = 0; j < voldim.z; j++) {
                                offsetVolume = (j * voldim.x * voldim.y) + initialStartPos;
                                offsetImage = j * voldim.x;
                                std::copy(voldata + offsetVolume, voldata + offsetVolume + dataSize,
                                          layerdata + offsetImage);
                            }
                            break;
                        }
                        case CartesianCoordinateAxis::Z: {
                            auto z = glm::clamp(slice, size_t{0}, voldim.z - 1);
                            const size_t dataSize = voldim.x * voldim.y;
                            const size_t initialStartPos = z * voldim.x * voldim.y;

                            std::copy(voldata + initialStartPos,
                                      voldata + initialStartPos + dataSize, layerdata);
                            break;
                        }
                    }
                    cache.add(sliceImage);
                    return sliceImage;
                });

    outport_.setData(image);
}

void VolumeSlice::eventShiftSlice(Event* event) {
    auto wheelEvent = static_cast<WheelEvent*>(event);
    int steps = static_cast<int>(wheelEvent->delta().y);
    shiftSlice(steps);
}

void VolumeSlice::eventStepSliceUp(Event*) { shiftSlice(1); }

void VolumeSlice::eventStepSliceDown(Event*) { shiftSlice(-1); }

void VolumeSlice::eventGestureShiftSlice(Event* event) {
    GestureEvent* gestureEvent = static_cast<GestureEvent*>(event);
    if (gestureEvent->deltaPos().y < 0)
        shiftSlice(1);
    else if (gestureEvent->deltaPos().y > 0)
        shiftSlice(-1);
}

}  // namespace inviwo
