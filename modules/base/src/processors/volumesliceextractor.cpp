/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2022 Inviwo Foundation
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

#include <modules/base/processors/volumesliceextractor.h>

#include <inviwo/core/datastructures/geometry/geometrytype.h>           // for CartesianCoordina...
#include <inviwo/core/datastructures/image/image.h>                     // for Image
#include <inviwo/core/datastructures/image/imageram.h>                  // IWYU pragma: keep
#include <inviwo/core/datastructures/image/imagetypes.h>                // for ImageChannel, Ima...
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/datastructures/volume/volumeram.h>                // for VolumeRAM
#include <inviwo/core/interaction/events/eventmatcher.h>                // for GestureEventMatcher
#include <inviwo/core/interaction/events/gestureevent.h>                // for GestureEvent
#include <inviwo/core/interaction/events/gesturestate.h>                // for GestureStates
#include <inviwo/core/interaction/events/keyboardkeys.h>                // for IvwKey, KeyState
#include <inviwo/core/interaction/events/wheelevent.h>                  // for WheelEvent
#include <inviwo/core/ports/imageport.h>                                // for ImageOutport
#include <inviwo/core/ports/volumeport.h>                               // for VolumeInport
#include <inviwo/core/processors/processor.h>                           // for Processor
#include <inviwo/core/processors/processorinfo.h>                       // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                      // for CodeState, CodeSt...
#include <inviwo/core/processors/processortags.h>                       // for Tags, Tags::CPU
#include <inviwo/core/properties/boolproperty.h>                        // for BoolProperty
#include <inviwo/core/properties/eventproperty.h>                       // for EventProperty
#include <inviwo/core/properties/invalidationlevel.h>                   // for InvalidationLevel
#include <inviwo/core/properties/optionproperty.h>                      // for OptionPropertyOption
#include <inviwo/core/properties/ordinalproperty.h>                     // for IntSizeTProperty
#include <inviwo/core/util/formatdispatching.h>                         // for All, PrecisionVal...
#include <inviwo/core/util/formats.h>                                   // for DataFormat, DataV...
#include <inviwo/core/util/glmutils.h>                                  // for extent
#include <inviwo/core/util/glmvec.h>                                    // for size2_t, dvec2
#include <inviwo/core/util/indexmapper.h>                               // for IndexMapper, Inde...
#include <inviwo/core/util/staticstring.h>                              // for operator+
#include <inviwo/core/util/document.h>                                  // for Document
#include <modules/base/datastructures/imagereusecache.h>                // for ImageReuseCache

#include <algorithm>      // for copy
#include <cstddef>        // for size_t
#include <memory>         // for shared_ptr, make_...
#include <type_traits>    // for remove_extent_t
#include <unordered_set>  // for unordered_set

#include <flags/flags.h>   // for any
#include <glm/common.hpp>  // for clamp
#include <glm/vec2.hpp>    // for vec<>::(anonymous)
#include <glm/vec3.hpp>    // for vec, vec<>::(anon...

namespace inviwo {
class Event;

const ProcessorInfo VolumeSliceExtractor::processorInfo_{
    "org.inviwo.VolumeSliceExtractor",  // Class identifier
    "Volume Slice Extractor",           // Display name
    "Volume Operation",                 // Category
    CodeState::Stable,                  // Code state
    Tags::CPU,                          // Tags
    R"(
Extracts an axis aligned 2D slice from an input volume. The input data will be renormalized to either 
[0,1] for floating point values or [0, max] of the data format using the data mapper of the volume.
)"_unindentHelp};
const ProcessorInfo VolumeSliceExtractor::getProcessorInfo() const { return processorInfo_; }

VolumeSliceExtractor::VolumeSliceExtractor()
    : Processor()
    , inport_("inputVolume", "The input volume"_help)
    , outport_("outputImage", "The extracted volume slice"_help, DataVec4UInt8::get(),
               HandleResizeEvents::No)
    , trafoGroup_("trafoGroup", "Transformations")
    , tfGroup_("tfGroup", "Transfer Function Mapping", false)
    , sliceAlongAxis_("sliceAxis", "Slice along axis",
                      "Defines the volume axis for the output slice"_help,
                      {{"x", "X axis", CartesianCoordinateAxis::X},
                       {"y", "Y axis", CartesianCoordinateAxis::Y},
                       {"z", "Z axis", CartesianCoordinateAxis::Z}},
                      0)
    , sliceNumber_("sliceNumber", "Slice", "Position of the slice"_help, 128,
                   {1, ConstraintBehavior::Immutable}, {256, ConstraintBehavior::Mutable}, 1)
    , format_("format", "Output Format", "Sets the data format of the resulting image"_help,
              {{"asInput", "As Input", OutputFormat::AsInput},
               {"uint8", "UInt8", OutputFormat::UInt8},
               {"float", "Float32", OutputFormat::Float32}},
              0)
    , flipHorizontal_("flipHorizontal", "Horizontal Flip",
                      "Flips the output image left and right"_help, false)
    , flipVertical_("flipVertical", "Vertical Flip", "Flips the output image up and down"_help,
                    false)
    , transferFunction_(
          "transferFunction", "Transfer Function",
          "Defines the transfer function for mapping voxel values to color and opacity"_help,
          TransferFunction(
              {{0.0, vec4(0.0f, 0.0f, 0.0f, 1.0f)}, {1.0, vec4(1.0f, 1.0f, 1.0f, 1.0f)}}),
          &inport_)
    , tfAlphaOffset_("alphaOffset", "Alpha Offset", "Offset alpha values in transfer function"_help,
                     0.0f, {0.0f, ConstraintBehavior::Editable},
                     {1.0f, ConstraintBehavior::Editable}, 0.01f)

    , handleInteractionEvents_(
          "handleEvents", "Handle interaction events",
          "Toggles whether this processor will handle interaction events like mouse buttons or key presses"_help,
          true, InvalidationLevel::Valid)
    , mouseShiftSlice_(
          "mouseShiftSlice", "Mouse Slice Shift", [this](Event* e) { eventShiftSlice(e); },
          std::make_unique<WheelEventMatcher>())

    , stepSliceUp_(
          "stepSliceUp", "Key Slice Up", [this](Event* e) { eventStepSliceUp(e); }, IvwKey::W,
          KeyState::Press)

    , stepSliceDown_(
          "stepSliceDown", "Key Slice Down", [this](Event* e) { eventStepSliceDown(e); }, IvwKey::S,
          KeyState::Press)

    , gestureShiftSlice_(
          "gestureShiftSlice", "Gesture Slice Shift",
          [this](Event* e) { eventGestureShiftSlice(e); },
          std::make_unique<GestureEventMatcher>(GestureType::Pan, GestureStates(flags::any), 3)) {

    addPort(inport_);
    addPort(outport_);

    trafoGroup_.addProperties(flipHorizontal_, flipVertical_);
    tfGroup_.addProperties(transferFunction_, tfAlphaOffset_);

    addProperties(sliceAlongAxis_, sliceNumber_, format_, trafoGroup_, tfGroup_,
                  handleInteractionEvents_, stepSliceUp_, stepSliceDown_, mouseShiftSlice_,
                  gestureShiftSlice_);

    mouseShiftSlice_.setVisible(false);
    mouseShiftSlice_.setCurrentStateAsDefault();
    gestureShiftSlice_.setVisible(false);
    gestureShiftSlice_.setCurrentStateAsDefault();
}

VolumeSliceExtractor::~VolumeSliceExtractor() = default;

void VolumeSliceExtractor::invokeEvent(Event* event) {
    if (!handleInteractionEvents_) return;
    Processor::invokeEvent(event);
}

void VolumeSliceExtractor::shiftSlice(int shift) {
    auto newSlice = static_cast<size_t>(sliceNumber_.get() + shift);
    if (newSlice >= sliceNumber_.getMinValue() && newSlice <= sliceNumber_.getMaxValue()) {
        sliceNumber_.set(newSlice);
    }
}

namespace detail {

size2_t sliceDimensions(const size3_t volumeDims, CartesianCoordinateAxis axis) {
    switch (axis) {
        default:
            return size2_t(volumeDims.z, volumeDims.y);
        case CartesianCoordinateAxis::X:
            return size2_t(volumeDims.z, volumeDims.y);
        case CartesianCoordinateAxis::Y:
            return size2_t(volumeDims.x, volumeDims.z);
        case CartesianCoordinateAxis::Z:
            return size2_t(volumeDims.x, volumeDims.y);
    }
}

Wrapping2D getWrapping(const VolumeRepresentation* v, CartesianCoordinateAxis axis) {
    const auto wrapping = v->getOwner()->getWrapping();
    switch (axis) {
        default:
            return {{wrapping[2], wrapping[1]}};
        case CartesianCoordinateAxis::X:
            return {{wrapping[2], wrapping[1]}};
        case CartesianCoordinateAxis::Y:
            return {{wrapping[0], wrapping[2]}};
        case CartesianCoordinateAxis::Z:
            return {{wrapping[0], wrapping[1]}};
    }
}

mat2 getBasis(const VolumeRepresentation* v, CartesianCoordinateAxis axis) {
    const mat3 basis = v->getOwner()->getBasis();
    switch (axis) {
        default:
            return mat2(vec2(basis[2][2], basis[2][1]), vec2(basis[1][2], basis[1][1]));
        case CartesianCoordinateAxis::X:
            return mat2(vec2(basis[2][2], basis[2][1]), vec2(basis[1][2], basis[1][1]));
        case CartesianCoordinateAxis::Y:
            return mat2(vec2(basis[0][0], basis[0][2]), vec2(basis[2][0], basis[2][2]));
        case CartesianCoordinateAxis::Z:
            return mat2(vec2(basis[0][0], basis[0][1]), vec2(basis[1][0], basis[1][1]));
    }
}

vec2 getOffset(const VolumeRepresentation* v, CartesianCoordinateAxis axis) {
    const vec3 offset = v->getOwner()->getOffset();
    switch (axis) {
        default:
            return vec2(offset.z, offset.y);
        case CartesianCoordinateAxis::X:
            return vec2(offset.z, offset.y);
        case CartesianCoordinateAxis::Y:
            return vec2(offset.x, offset.z);
        case CartesianCoordinateAxis::Z:
            return vec2(offset.x, offset.y);
    }
}

struct SliceState {
    CartesianCoordinateAxis axis;
    size_t slice;
    ImageReuseCache* cache;
    bool flipHorizontal;
    bool flipVertical;
    TransferFunction* tf = nullptr;
    float alphaOffset = 0.0f;
};

template <typename T, typename D, typename Func>
std::shared_ptr<Image> extractSliceInternal(const VolumeRAMPrecision<T>* vrprecision,
                                            const SliceState& state, Func f) {
    const T* voldata = vrprecision->getDataTyped();
    const auto& voldim = vrprecision->getDimensions();

    const auto imgdim = sliceDimensions(voldim, state.axis);

    auto res = state.cache->getTypedUnused<D>(imgdim);
    auto sliceImage = res.first;
    auto layerrep = res.second;
    auto layerdata = layerrep->getDataTyped();
    layerrep->setSwizzleMask(state.tf ? swizzlemasks::rgba : vrprecision->getSwizzleMask());
    layerrep->setWrapping(getWrapping(vrprecision, state.axis));
    sliceImage->getColorLayer()->setBasis(getBasis(vrprecision, state.axis));
    sliceImage->getColorLayer()->setOffset(getOffset(vrprecision, state.axis));

    switch (state.axis) {
        case CartesianCoordinateAxis::X: {
            util::IndexMapper3D vm(voldim);
            util::IndexMapper2D im(imgdim);
            auto x = glm::clamp(state.slice, size_t{0}, voldim.x - 1);
            for (size_t z = 0; z < voldim.z; z++) {
                for (size_t y = 0; y < voldim.y; y++) {
                    auto offsetVolume = vm(x, y, z);
                    auto offsetImage = im(z, y);
                    layerdata[offsetImage] = f(voldata[offsetVolume]);
                }
            }
            break;
        }
        case CartesianCoordinateAxis::Y: {
            auto y = glm::clamp(state.slice, size_t{0}, voldim.y - 1);
            const size_t dataSize = voldim.x;
            const size_t initialStartPos = y * voldim.x;
            for (size_t j = 0; j < voldim.z; j++) {
                auto offsetVolume = (j * voldim.x * voldim.y) + initialStartPos;
                auto offsetImage = j * voldim.x;
                std::transform(voldata + offsetVolume, voldata + offsetVolume + dataSize,
                               layerdata + offsetImage, f);
            }
            break;
        }
        case CartesianCoordinateAxis::Z: {
            auto z = glm::clamp(state.slice, size_t{0}, voldim.z - 1);
            const size_t dataSize = voldim.x * voldim.y;
            const size_t initialStartPos = z * voldim.x * voldim.y;
            std::transform(voldata + initialStartPos, voldata + initialStartPos + dataSize,
                           layerdata, f);
            break;
        }
    }
    if (state.flipHorizontal) {
        for (size_t y = 0; y < imgdim.y; ++y) {
            std::reverse(layerdata + y * imgdim.x, layerdata + (y + 1) * imgdim.x);
        }
    }
    if (state.flipVertical) {
        for (size_t y = 0; y < imgdim.y / 2; ++y) {
            std::swap_ranges(layerdata + y * imgdim.x, layerdata + (y + 1) * imgdim.x,
                             layerdata + (imgdim.y - 1 - y) * imgdim.x);
        }
    }
    state.cache->add(sliceImage);
    return sliceImage;
}

template <typename T, typename V = util::value_type_t<T>>
std::shared_ptr<Image> extractSlice(const VolumeRAMPrecision<T>* vrprecision,
                                    const SliceState& state, bool useTF) {

    if (useTF) {
        using D = glm::vec<4, V>;
        auto mapData = [&dm = vrprecision->getOwner()->dataMap_, tf = state.tf,
                        offset = state.alphaOffset](T value) {
            auto sample = tf->sample(
                glm::clamp(dm.mapFromDataToNormalized(util::glmcomp(value, 0)), 0.0, 1.0));
            sample.a = glm::clamp(sample.a + offset, 0.0f, 1.0f);
            return util::glm_convert_normalized<D>(sample);
        };
        return extractSliceInternal<T, D>(vrprecision, state, mapData);
    } else {
        using D = util::same_extent_t<T, V>;
        auto mapData = [&dm = vrprecision->getOwner()->dataMap_](T value) {
            return util::glm_convert_normalized<D>(glm::clamp(dm.mapFromDataToNormalized(value),
                                                              util::same_extent_t<T, double>(0.0),
                                                              util::same_extent_t<T, double>(1.0)));
        };
        return extractSliceInternal<T, D>(vrprecision, state, mapData);
    }
}

}  // namespace detail

void VolumeSliceExtractor::process() {
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

    detail::SliceState state{sliceAlongAxis_,     static_cast<size_t>(sliceNumber_.get() - 1),
                             &imageCache_,        flipHorizontal_,
                             flipVertical_,       &transferFunction_.get(),
                             tfAlphaOffset_.get()};

    std::shared_ptr<Image> image;

    switch (format_.get()) {
        case OutputFormat::UInt8:
            image = vol->getRepresentation<VolumeRAM>()
                        ->dispatch<std::shared_ptr<Image>, dispatching::filter::All>(
                            [&](const auto* vrprecision) {
                                using T = util::PrecisionValueType<decltype(vrprecision)>;
                                return detail::extractSlice<T, std::uint8_t>(vrprecision, state,
                                                                             tfGroup_.isChecked());
                            });
            break;
        case OutputFormat::Float32:
            image = vol->getRepresentation<VolumeRAM>()
                        ->dispatch<std::shared_ptr<Image>, dispatching::filter::All>(
                            [&](const auto* vrprecision) {
                                using T = util::PrecisionValueType<decltype(vrprecision)>;
                                return detail::extractSlice<T, float>(vrprecision, state,
                                                                      tfGroup_.isChecked());
                            });
            break;
        case OutputFormat::AsInput:
        default:
            image =
                vol->getRepresentation<VolumeRAM>()
                    ->dispatch<std::shared_ptr<Image>, dispatching::filter::All>(
                        [&](const auto* vrprecision) {
                            return detail::extractSlice(vrprecision, state, tfGroup_.isChecked());
                        });
            break;
    }

    outport_.setData(image);
}

void VolumeSliceExtractor::eventShiftSlice(Event* event) {
    auto wheelEvent = static_cast<WheelEvent*>(event);
    int steps = static_cast<int>(wheelEvent->delta().y);
    shiftSlice(steps);
}

void VolumeSliceExtractor::eventStepSliceUp(Event*) { shiftSlice(1); }

void VolumeSliceExtractor::eventStepSliceDown(Event*) { shiftSlice(-1); }

void VolumeSliceExtractor::eventGestureShiftSlice(Event* event) {
    GestureEvent* gestureEvent = static_cast<GestureEvent*>(event);
    if (gestureEvent->deltaPos().y < 0)
        shiftSlice(1);
    else if (gestureEvent->deltaPos().y > 0)
        shiftSlice(-1);
}

}  // namespace inviwo
