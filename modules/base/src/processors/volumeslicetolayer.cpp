/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2025-2026 Inviwo Foundation
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

#include <modules/base/processors/volumeslicetolayer.h>

#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/util/indexmapper.h>
#include <inviwo/core/util/glm.h>

#include <algorithm>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo VolumeSliceToLayer::processorInfo_{
    "org.inviwo.VolumeSliceToLayer",  // Class identifier
    "Volume Slice To Layer",          // Display name
    "Volume Operation",               // Category
    CodeState::Stable,                // Code state
    Tags::CPU,                        // Tags
    R"(Extracts an axis-aligned 2D layer from the input volume.)"_unindentHelp,
};

const ProcessorInfo& VolumeSliceToLayer::getProcessorInfo() const { return processorInfo_; }

VolumeSliceToLayer::VolumeSliceToLayer()
    : Processor{}
    , inport_{"volume", "The input volume"_help}
    , outport_{"output", "The extracted volume slice"_help}
    , sliceAlongAxis_("sliceAxis", "Slice along axis",
                      "Defines the volume axis for the output slice"_help,
                      {{"x", "X axis", CartesianCoordinateAxis::X},
                       {"y", "Y axis", CartesianCoordinateAxis::Y},
                       {"z", "Z axis", CartesianCoordinateAxis::Z}},
                      0)
    , sliceNumber_("sliceNumber", "Slice", "Position of the slice"_help, 128,
                   {1, ConstraintBehavior::Immutable}, {256, ConstraintBehavior::Mutable}, 1) {

    addPorts(inport_, outport_);
    addProperties(sliceAlongAxis_, sliceNumber_);
}

namespace {

size2_t sliceDimensions(const size3_t volumeDims, CartesianCoordinateAxis axis) {
    switch (axis) {
        default:
        case CartesianCoordinateAxis::X:
            return {volumeDims.z, volumeDims.y};
        case CartesianCoordinateAxis::Y:
            return {volumeDims.x, volumeDims.z};
        case CartesianCoordinateAxis::Z:
            return {volumeDims.x, volumeDims.y};
    }
}

Wrapping2D getWrapping(const VolumeRepresentation* v, CartesianCoordinateAxis axis) {
    const auto wrapping = v->getOwner()->getWrapping();
    switch (axis) {
        default:
        case CartesianCoordinateAxis::X:
            return {{wrapping[2], wrapping[1]}};
        case CartesianCoordinateAxis::Y:
            return {{wrapping[0], wrapping[2]}};
        case CartesianCoordinateAxis::Z:
            return {{wrapping[0], wrapping[1]}};
    }
}

std::array<Axis, 2> getAxes(const VolumeRepresentation* v, CartesianCoordinateAxis axis) {
    const auto axes = v->getOwner()->axes;
    switch (axis) {
        default:
        case CartesianCoordinateAxis::X:
            return {axes[2], axes[1]};
        case CartesianCoordinateAxis::Y:
            return {axes[0], axes[2]};
        case CartesianCoordinateAxis::Z:
            return {axes[0], axes[1]};
    }
}

mat3 getBasis(const VolumeRepresentation* v, CartesianCoordinateAxis axis) {
    const mat3 basis = v->getOwner()->getBasis();
    switch (axis) {
        default:
        case CartesianCoordinateAxis::X:
            return mat3{basis[2], basis[1], glm::normalize(basis[0])};
        case CartesianCoordinateAxis::Y:
            return mat3{basis[0], basis[2], -glm::normalize(basis[1])};
        case CartesianCoordinateAxis::Z:
            return mat3{basis[0], basis[1], glm::normalize(basis[2])};
    }
}

vec3 getOffset(const VolumeRepresentation* v, CartesianCoordinateAxis axis, size_t slice) {
    const size3_t dims = v->getDimensions();
    const vec3 offset = v->getOwner()->getOffset();
    const mat3 basis = v->getOwner()->getBasis();
    const vec3 t = vec3{static_cast<float>(slice)} / vec3{dims - size3_t{1}};

    switch (axis) {
        default:
        case CartesianCoordinateAxis::X:
            return offset + basis[0] * t[0];
        case CartesianCoordinateAxis::Y:
            return offset + basis[1] * t[1];
        case CartesianCoordinateAxis::Z:
            return offset + basis[2] * t[2];
    }
}

template <typename T>
std::shared_ptr<Layer> extractSlice(const VolumeRAMPrecision<T>* vrprecision,
                                    CartesianCoordinateAxis axis, size_t slice) {
    const T* voldata = vrprecision->getDataTyped();
    const auto& voldim = vrprecision->getDimensions();

    const auto layerdim = sliceDimensions(voldim, axis);

    auto layerram = std::make_shared<LayerRAMPrecision<T>>(
        LayerReprConfig{.dimensions = layerdim,
                        .format = vrprecision->getDataFormat(),
                        .swizzleMask = vrprecision->getSwizzleMask(),
                        .interpolation = vrprecision->getInterpolation(),
                        .wrapping = getWrapping(vrprecision, axis)});
    auto layerdata = layerram->getDataTyped();

    auto layer = std::make_shared<Layer>(layerram);
    layer->setBasis(getBasis(vrprecision, axis));
    layer->setOffset(getOffset(vrprecision, axis, slice));
    layer->dataMap = vrprecision->getOwner()->dataMap;
    layer->axes = getAxes(vrprecision, axis);

    switch (axis) {
        case CartesianCoordinateAxis::X: {
            const util::IndexMapper3D vm(voldim);
            const util::IndexMapper2D im(layerdim);
            auto x = glm::clamp(slice, size_t{0}, voldim.x - 1);
            for (size_t z = 0; z < voldim.z; z++) {
                for (size_t y = 0; y < voldim.y; y++) {
                    auto offsetVolume = vm(x, y, z);
                    auto offsetImage = im(z, y);
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
                auto offsetVolume = (j * voldim.x * voldim.y) + initialStartPos;
                auto offsetImage = j * voldim.x;
                std::copy(voldata + offsetVolume, voldata + offsetVolume + dataSize,
                          layerdata + offsetImage);
            }
            break;
        }
        case CartesianCoordinateAxis::Z: {
            auto z = glm::clamp(slice, size_t{0}, voldim.z - 1);
            const size_t dataSize = voldim.x * voldim.y;
            const size_t initialStartPos = z * voldim.x * voldim.y;
            std::copy(voldata + initialStartPos, voldata + initialStartPos + dataSize, layerdata);
            break;
        }
    }
    return layer;
}

}  // namespace

void VolumeSliceToLayer::process() {
    auto vol = inport_.getData();

    const auto dims = vol->getDimensions();
    const auto slicePos = size3_t{dvec3{dims} * static_cast<double>(sliceNumber_.get()) /
                                  static_cast<double>(sliceNumber_.getMaxValue())};

    switch (sliceAlongAxis_.get()) {
        case CartesianCoordinateAxis::X:
            if (dims.x != sliceNumber_.getMaxValue()) {
                sliceNumber_.setMaxValue(dims.x);
                sliceNumber_.set(slicePos.x);
            }
            break;
        case CartesianCoordinateAxis::Y:
            if (dims.y != sliceNumber_.getMaxValue()) {
                sliceNumber_.setMaxValue(dims.y);
                sliceNumber_.set(slicePos.y);
            }
            break;
        case CartesianCoordinateAxis::Z:
            if (dims.z != sliceNumber_.getMaxValue()) {
                sliceNumber_.setMaxValue(dims.z);
                sliceNumber_.set(slicePos.z);
            }
            break;
    }

    auto layer =
        vol->getRepresentation<VolumeRAM>()
            ->dispatch<std::shared_ptr<Layer>, dispatching::filter::All>(
                [&]<typename T>(const VolumeRAMPrecision<T>* vrprecision) {
                    return extractSlice(vrprecision, sliceAlongAxis_, sliceNumber_.get() - 1);
                });
    layer->setWorldMatrix(vol->getWorldMatrix());
    outport_.setData(layer);
}

}  // namespace inviwo
