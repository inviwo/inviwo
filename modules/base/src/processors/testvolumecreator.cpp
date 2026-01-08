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

#include <modules/base/processors/testvolumecreator.h>

#include <inviwo/core/datastructures/volume/volumeram.h>

#include <cmath>
#include <numbers>

namespace inviwo {

namespace {

std::pair<std::shared_ptr<Volume>, std::shared_ptr<Volume>> createVolumes(
    TestVolumeCreator::Mode mode, size3_t dims, mat4 basis, VolumeReuseCache& scalars,
    VolumeReuseCache& gradients, pool::Progress progress, pool::Stop stop) {

    const VolumeConfig scalarCfg = {.dimensions = dims,
                                    .format = DataFloat32::get(),
                                    .swizzleMask = swizzlemasks::defaultData(1),
                                    .interpolation = InterpolationType::Linear,
                                    .dataRange = dvec2{-1.0, 1.0},
                                    .valueRange = dvec2{-1.0, 1.0},
                                    .model = basis};

    const VolumeConfig gradientCfg = {.dimensions = dims,
                                      .format = DataVec3Float32::get(),
                                      .swizzleMask = swizzlemasks::defaultData(3),
                                      .interpolation = InterpolationType::Linear,
                                      .dataRange = dvec2{-1.0, 1.0},
                                      .valueRange = dvec2{-1.0, 1.0},
                                      .model = basis};

    auto scalar = scalars(scalarCfg);
    auto gradient = gradients(gradientCfg);

    const auto im = util::IndexMapper3D(dims);
    const auto i2w = scalar->getCoordinateTransformer().getIndexToWorldMatrix();

    auto* scalarRep =
        dynamic_cast<VolumeRAMPrecision<float>*>(scalar->getEditableRepresentation<VolumeRAM>());
    IVW_ASSERT(scalarRep, "should exist");

    auto* gradientRep =
        dynamic_cast<VolumeRAMPrecision<vec3>*>(gradient->getEditableRepresentation<VolumeRAM>());
    IVW_ASSERT(gradientRep, "should exist");

    auto scalarView = scalarRep->getView();
    auto gradientView = gradientRep->getView();

    switch (mode) {
        case TestVolumeCreator::Mode::Sin1D:
            for (size_t k = 0; k < dims.z; ++k) {
                if (stop) return {};
                progress(static_cast<double>(k) / static_cast<double>(dims.z));
                for (size_t j = 0; j < dims.y; ++j) {
                    for (size_t i = 0; i < dims.x; ++i) {
                        const auto w = std::numbers::pi_v<float> * vec3(i2w * vec4(i, j, k, 1.0));
                        scalarView[im(i, j, k)] = std::sin(w.x);
                        gradientView[im(i, j, k)] =
                            std::numbers::pi_v<float> * vec3{std::cos(w.x), 0.0f, 0.0f};
                    }
                }
            }
            break;
        case TestVolumeCreator::Mode::Sin2D:
            for (size_t k = 0; k < dims.z; ++k) {
                if (stop) return {};
                progress(static_cast<double>(k) / static_cast<double>(dims.z));
                for (size_t j = 0; j < dims.y; ++j) {
                    for (size_t i = 0; i < dims.x; ++i) {
                        const auto w = std::numbers::pi_v<float> * vec3(i2w * vec4(i, j, k, 1.0));
                        scalarView[im(i, j, k)] = std::sin(w.x) * std::sin(w.y);
                        gradientView[im(i, j, k)] =
                            std::numbers::pi_v<float> *
                            vec3{std::cos(w.x) * std::sin(w.y), std::cos(w.y) * std::sin(w.x), 0};
                    }
                }
            }
            break;
        case TestVolumeCreator::Mode::Sin3D:
            for (size_t k = 0; k < dims.z; ++k) {
                if (stop) return {};
                progress(static_cast<double>(k) / static_cast<double>(dims.z));
                for (size_t j = 0; j < dims.y; ++j) {
                    for (size_t i = 0; i < dims.x; ++i) {
                        const auto w = std::numbers::pi_v<float> * vec3(i2w * vec4(i, j, k, 1.0));
                        scalarView[im(i, j, k)] = std::sin(w.x) * std::sin(w.y) * std::sin(w.z);
                        gradientView[im(i, j, k)] =
                            std::numbers::pi_v<float> *
                            vec3{std::cos(w.x) * std::sin(w.y) * std::sin(w.z),
                                 std::cos(w.y) * std::sin(w.x) * std::sin(w.z),
                                 std::cos(w.z) * std::sin(w.x) * std::sin(w.y)};
                    }
                }
            }
            break;
    }

    return {scalar, gradient};
}

}  // namespace

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo TestVolumeCreator::processorInfo_{
    "org.inviwo.TestVolumeCreator",  // Class identifier
    "Test Volume Creator",           // Display name
    "Data Creation",                 // Category
    CodeState::Stable,               // Code state
    Tags::CPU | Tag{"Volume"},       // Tags
    R"(Create an analytical scalar field and gradients
    There are 3 modes available:
    - Sin 1D: A sine wave along the x-axis
    - Sin 2D: A product of sine waves along the x- and y-axes
    - Sin 3D: A product of sine waves along the x-, y- and z-axes
    )"_unindentHelp,
};

const ProcessorInfo& TestVolumeCreator::getProcessorInfo() const { return processorInfo_; }

TestVolumeCreator::TestVolumeCreator()
    : PoolProcessor{}
    , scalar_{"scalar"}
    , gradient_{"gradient"}

    , scalars_{}
    , gradients_{}
    , mode_{"mode",
            "Mode",
            {{"sin1d", "Sin 1D", Mode::Sin1D},
             {"sin2d", "Sin 2D", Mode::Sin2D},
             {"sin3d", "Sin 3D", Mode::Sin3D}},
            1}
    , basis_{"basis", "Basis", mat4{1.0}}
    , dimensions_("dims", "Dimensions",
                  util::ordinalCount(size3_t(10), size3_t(512)).set("Volume Dimensions"_help))
    , scalarInformation_{"scalarInformation", "Scalar Information"}
    , gradientInformation_{"gradientInformation", "Gradient Information"} {

    addPorts(scalar_, gradient_);

    addProperties(mode_, basis_, dimensions_, scalarInformation_, gradientInformation_);
}

void TestVolumeCreator::process() {
    const auto calc = [this, mode = mode_.get(), dims = dimensions_.get(), basis = basis_.get()](
                          pool::Progress progress, pool::Stop stop) {
        return createVolumes(mode, dims, basis, scalars_, gradients_, progress, stop);
    };

    scalar_.clear();
    gradient_.clear();

    dispatchOne(calc, [this](const auto& result) {
        scalar_.setData(result.first);
        gradient_.setData(result.second);

        scalarInformation_.updateForNewVolume(*result.first, util::OverwriteState::No);
        scalarInformation_.updateVolume(*result.first);
        gradientInformation_.updateForNewVolume(*result.second, util::OverwriteState::No);
        gradientInformation_.updateVolume(*result.second);

        newResults();
    });
}

}  // namespace inviwo
