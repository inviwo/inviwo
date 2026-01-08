/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024-2026 Inviwo Foundation
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

#include <modules/opactopt/processors/meshmappingvolume.h>

#include <modules/base/algorithm/dataminmax.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/datastructures/buffer/bufferram.h>
#include <inviwo/core/datastructures/data.h>
#include <inviwo/core/datastructures/geometry/geometrytype.h>
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/datastructures/image/imagetypes.h>
#include <inviwo/core/datastructures/transferfunction.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/processorinfo.h>
#include <inviwo/core/processors/processorstate.h>
#include <inviwo/core/properties/invalidationlevel.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/property.h>
#include <inviwo/core/properties/propertysemantics.h>
#include <inviwo/core/properties/valuewrapper.h>
#include <inviwo/core/util/logcentral.h>
#include <inviwo/core/util/glmvec.h>

#include <array>
#include <limits>
#include <memory>
#include <type_traits>
#include <vector>
#include <array>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo MeshMappingVolume::processorInfo_{
    "org.inviwo.MeshMappingVolume",  // Class identifier
    "Mesh Mapping Volume",           // Display name
    "Mesh Operation",                // Category
    CodeState::Experimental,         // Code state
    "CPU, Mesh, Color Mapping",      // Tags
};
const ProcessorInfo& MeshMappingVolume::getProcessorInfo() const { return processorInfo_; }

MeshMappingVolume::MeshMappingVolume()
    : Processor()
    , meshInport_{"meshInport"}
    , volumeInport_{"importanceVolume"}
    , outport_{"outport"}
    , enabled_("enabled", "Enabled", true)
    , tf_("transferfunction", "Transfer Function",
          TransferFunction({{.pos = 0.0f, .color = vec4{0.0f, 0.1f, 1.0f, 1.0f}},
                            {.pos = 1.0f, .color = vec4{1.0f, 0.03f, 0.03f, 1.0f}}}))
    , channel_("channel", "Channel", util::enumeratedOptions("Channel", 4)) {

    addPorts(meshInport_, volumeInport_, outport_);

    addProperties(enabled_, tf_, channel_);
}

namespace {

// TODO(Peter): Replace with VolumeSampler after refactor

double triInterp(const std::array<double, 8>& c, vec3 voxelpos) {
    // interpolate along x direction
    std::array<double, 4> cx{};
    cx[0] = (1 - voxelpos.x) * c[0] + voxelpos.x * c[4];
    cx[1] = (1 - voxelpos.x) * c[1] + voxelpos.x * c[5];
    cx[2] = (1 - voxelpos.x) * c[2] + voxelpos.x * c[6];
    cx[3] = (1 - voxelpos.x) * c[3] + voxelpos.x * c[7];

    // interpolate along y direction
    std::array<double, 2> cy{};
    cy[0] = (1 - voxelpos.y) * cx[0] + voxelpos.y * cx[2];
    cy[1] = (1 - voxelpos.y) * cx[1] + voxelpos.y * cx[3];

    // interpolate along z direction
    return (1 - voxelpos.z) * cy[0] + voxelpos.z * cy[1];
}
}  // namespace

void MeshMappingVolume::process() {
    if (!enabled_.get() || !meshInport_.hasData() ||
        (meshInport_.getData()->getNumberOfBuffers() == 0) || !volumeInport_.hasData()) {
        outport_.setData(meshInport_.getData());
        return;
    }

    const auto inputMesh = meshInport_.getData();
    const auto* srcBuffer =
        inputMesh->findBuffer(BufferType::PositionAttrib).first->getRepresentation<BufferRAM>();

    if (!srcBuffer) {
        throw Exception(SourceContext{}, "No position buffer found in mesh");
    }

    const auto volume = volumeInport_.getData();
    const auto* volumeRAM = volume->getRepresentation<VolumeRAM>();
    const auto worldToIndex = volume->getCoordinateTransformer().getWorldToIndexMatrix();
    const auto dataToWorld = inputMesh->getCoordinateTransformer().getDataToWorldMatrix();

    if (channel_.getSelectedIndex() >= volume->getDataFormat()->getComponents()) {
        throw Exception{SourceContext{},
                        "Selected channel {} does not exist in volume with {} channels",
                        channel_.getSelectedIndex(), volume->getDataFormat()->getComponents()};
    }

    // fill color vector
    std::vector<vec4> colorsOut(srcBuffer->getSize());
    bool accessOutsideBounds = false;
    srcBuffer->dispatch<void, dispatching::filter::Vec3s>([comp = channel_.getSelectedIndex(),
                                                           &dst = colorsOut, &tf = tf_.get(),
                                                           dm = volume->dataMap, volumeRAM,
                                                           transform = worldToIndex * dataToWorld,
                                                           &accessOutsideBounds](auto pBuffer) {
        const auto& vecs = pBuffer->getDataContainer();
        const auto dims = volumeRAM->getDimensions();
        const auto dimsM1 = dims - size3_t(1);

        if (volumeRAM->getInterpolation() == InterpolationType::Linear) {
            std::transform(vecs.begin(), vecs.end(), dst.begin(), [&](auto& vec) {
                const auto texPos = vec3(transform * vec4{vec, 1.0f});

                if (glm::any(glm::lessThan(glm::floor(texPos), vec3{0.0f})) ||
                    glm::any(glm::greaterThanEqual(glm::ceil(texPos), glm::vec3(dims)))) {
                    accessOutsideBounds = true;
                }

                // trilinear interpolation
                std::array<double, 8> c{};
                for (int i = 0; i < 8; i++) {
                    const size3_t samplePos =
                        size3_t(texPos) + size3_t((i >> 2) & 1, (i >> 1) & 1, i & 1);
                    c[i] = volumeRAM->getAsDVec4(glm::clamp(samplePos, size3_t{0}, dimsM1))[comp];
                }
                vec3 integerPart{};
                const auto res = triInterp(c, glm::modf(texPos, integerPart));
                const auto normalized = dm.mapFromDataToNormalized(res);
                return tf.sample(normalized);
            });
        } else {
            std::transform(vecs.begin(), vecs.end(), dst.begin(), [&](auto& vec) {
                const auto texPos = vec3(transform * vec4{vec, 1.0f});

                if (glm::any(glm::lessThan(glm::floor(texPos), vec3{0.0f})) ||
                    glm::any(glm::greaterThanEqual(glm::ceil(texPos), glm::vec3(dims)))) {
                    accessOutsideBounds = true;
                }

                const auto samplePos = size3_t(glm::round(texPos));
                const auto res = volumeRAM->getAsDVec4(glm::clamp(samplePos, size3_t{0}, dimsM1))[comp];
                const auto normalized = dm.mapFromDataToNormalized(res);
                return tf.sample(normalized);
            });
        }
    });
    if (accessOutsideBounds) {
        LogWarn(
            "The volume is being sampled out of bounds one or more times. The mesh and volume "
            "may not be aligned.");
    }

    // create a new mesh containing all buffers of the input mesh
    // The first color buffer, if existing, is replaced with the mapped colors.
    // Otherwise, a new color buffer will be added.
    auto colBuffer = std::make_shared<Buffer<vec4>>(
        std::make_shared<BufferRAMPrecision<vec4>>(std::move(colorsOut)));

    auto mesh = std::shared_ptr<Mesh>{inputMesh->clone()};
    if (auto [buff, loc] = mesh->findBuffer(BufferType::ColorAttrib); buff) {
        mesh->replaceBuffer(buff, Mesh::BufferInfo{BufferType::ColorAttrib, loc}, colBuffer);
    } else {
        mesh->addBuffer(BufferType::ColorAttrib, colBuffer);
    }

    outport_.setData(mesh);
}

}  // namespace inviwo
