/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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
#include <modules/base/processors/meshsplatprocessor.h>
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/logcentral.h>
#include <fmt/base.h>

namespace inviwo {

const ProcessorInfo MeshSplatProcessor::processorInfo_{
    "org.inviwo.MeshGaussianSplatProcessor",  // Class identifier
    "Mesh Gaussian Splat",                    // Display name
    "Volume Creation",                        // Category
    CodeState::Experimental,                  // Code state
    Tags::CPU,                                // Tags
    R"(Processes a mesh and creates a volume using Gaussian splatting.
    
    Supports multiple kernel types (Gaussian, Epanechnikov, Triangular, Uniform) for
    splatting mesh points into a 3D volume. Allows per-point sizes and weights, with
    configurable volume dimensions and basis transformation.
    )"_unindentHelp};

const ProcessorInfo& MeshSplatProcessor::getProcessorInfo() const { return processorInfo_; }

MeshSplatProcessor::MeshSplatProcessor()
    : PoolProcessor()
    , meshInport_{"meshInport"}
    , volumeOutport_{"volumeOutport"}

    , kernelType_{"kernelType",
                  "Kernel Type",
                  {{"Gaussian", "Gaussian", util::SplatKernel::Gaussian},
                   {"Epanechnikov", "Epanechnikov", util::SplatKernel::Epanechnikov},
                   {"Triangular", "Triangular", util::SplatKernel::Triangular},
                   {"Uniform", "Uniform", util::SplatKernel::Uniform}},
                  0}
    , error_{"kernelCutoff", "Kernel Cutoff (error)",
             util::ordinalScale(0.01f).setMin(0.0f).setMax(1.0f).setInc(0.00001f).set(
                 "Used to calculate the support size in case of a Gaussian kernel"_help)}

    , perPointSize_{"perPointSize", "Use Per-Point Sizes", false}
    , size_{"size", "Size",
            util::ordinalScale(1.0f).setInc(0.000001f).set("World space 'size' of the point"_help)}

    , perPointWeight_{"perPointWeight", "Use Per-Point Weights", false}
    , weight_{"weight", "Weight", util::ordinalScale(1.0f).setInc(0.000001f)}
    , volume_{"volume", "Volume Settings"}
    , volumeDims_{"volumeDims", "Volume Dimensions",
                  util::ordinalCount(size3_t{64}, size3_t(512)).setMin(size3_t(1))}
    , basis_{"basis", "Basis"}
    , range_{"range", "Data range"}
    , valueName{"valueName", "Value name", ""}
    , valueUnit{"valueUnit", "Value unit", ""} {

    addPorts(meshInport_, volumeOutport_);
    addProperties(kernelType_, perPointSize_, size_, error_, perPointWeight_, weight_, volume_);

    volume_.addProperties(volumeDims_, basis_, range_, valueName, valueUnit);
}

void MeshSplatProcessor::process() {
    if (!meshInport_.hasData()) return;

    const util::SplatSettings settings{
        .dimensions = volumeDims_.get(),
        .modelMatrix = basis_.getMatrix(),

        .axes = meshInport_.getData()->axes,
        .valueAxis =
            Axis{.name = valueName.get(), .unit = units::unit_from_string(valueUnit.get())},
        .kernel = kernelType_.get(),
        .size = size_.get(),
        .weight = weight_.get(),
        .error = error_.get()};

    std::vector<util::SplatInput> inputs;
    std::vector<std::shared_ptr<const Mesh>> meshes;

    for (auto mesh : meshInport_) {

        meshes.push_back(mesh);
        auto& input = inputs.emplace_back(util::SplatInput{
            .pointTransform = mesh->getCoordinateTransformer().getDataToWorldMatrix()});

        const auto* positionBuffer = mesh->getBuffer(BufferType::PositionAttrib);
        positionBuffer->getRepresentation<BufferRAM>()->dispatch<void>([&](auto posRep) {
            using ValueType = util::PrecisionValueType<decltype(posRep)>;
            if constexpr (std::is_same_v<ValueType, vec3>) {
                input.positions = posRep->getDataContainer();
            }
        });

        if (perPointSize_.get()) {
            if (const auto* radiiBuffer = mesh->getBuffer(BufferType::RadiiAttrib)) {
                radiiBuffer->getRepresentation<BufferRAM>()->dispatch<void>([&](auto radiiRep) {
                    using ValueType = util::PrecisionValueType<decltype(radiiRep)>;
                    if constexpr (std::is_same_v<ValueType, float>) {
                        input.sizes = radiiRep->getDataContainer();
                    }
                });
            }
            if (input.sizes.empty()) {
                log::warn(
                    "Per-point size enabled but no valid Radii buffer found. Falling back to "
                    "global "
                    "size.");
            }
        }

        if (perPointWeight_.get()) {
            if (const auto* scalarBuffer = mesh->getBuffer(BufferType::ScalarMetaAttrib)) {
                scalarBuffer->getRepresentation<BufferRAM>()->dispatch<void>([&](auto scalarRep) {
                    using ValueType = util::PrecisionValueType<decltype(scalarRep)>;
                    if constexpr (std::is_same_v<ValueType, float>) {
                        input.weights = scalarRep->getDataContainer();
                    }
                });
            }
            if (input.weights.empty()) {
                log::warn(
                    "Per-point weight enabled but no valid ScalarMeta buffer found. Falling back "
                    "to "
                    "global weight.");
            }
        }
    }

    auto [collect, jobs] = util::splatJobs(inputs, settings);
    dispatchMany(jobs, [this, collect, meshes](std::vector<vec2> results) {
        auto volume = collect(std::move(results));
        range_.updateFromVolume(volume);
        volume->dataMap.dataRange = range_.getDataRange();
        volume->dataMap.valueRange = range_.getValueRange();

        volumeOutport_.setData(volume);
        newResults();
    });
}

}  // namespace inviwo
