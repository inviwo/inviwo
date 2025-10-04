/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024 Inviwo Foundation
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
    , meshInport_("meshInport")
    , outport_("outport")
    , volumeInport_("importanceVolume")
    , enabled_("enabled", "Enabled", true)
    , tf_("transferfunction", "Transfer Function",
          TransferFunction({{.pos = 0.0f, .color = vec4{0.0f, 0.1f, 1.0f, 1.0f}},
                            {.pos = 1.0f, .color = vec4{1.0f, 0.03f, 0.03f, 1.0f}}}))
    , component_("component", "Component", {{"component1", "Component 1", 0}})

    , useCustomDataRange_("useCustomRange", "Use Custom Range", false)
    , customDataRange_("customDataRange", "Custom Range", 0.0, 1.0,
                       std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max(),
                       0.01, 0.0, InvalidationLevel::InvalidOutput, PropertySemantics::Text)
    , dataRange_("dataRange", "Data Range", 0.0, 1.0, std::numeric_limits<double>::lowest(),
                 std::numeric_limits<double>::max(), 0.01, 0.0, InvalidationLevel::Valid,
                 PropertySemantics::Text) {

    addPort(meshInport_);
    addPort(volumeInport_);
    addPort(outport_);

    component_.setSerializationMode(PropertySerializationMode::All)
        .setCurrentStateAsDefault()
        .setReadOnly(true);
    dataRange_.setSerializationMode(PropertySerializationMode::None).setReadOnly(true);

    customDataRange_.readonlyDependsOn(useCustomDataRange_, [](const auto& p) { return !p.get(); });

    addProperties(enabled_, tf_, component_, useCustomDataRange_, customDataRange_, dataRange_);

    const auto getVolume = [this]() -> const Volume* {
        if (volumeInport_.hasData()) return volumeInport_.getData().get();
        return nullptr;
    };

    const auto updateDataRange = [this, getVolume]() {
        if (const Volume* const volume = getVolume()) {
            // obtain range of selected component of chosen buffer
            auto range = util::volumeMinMax(volume);
            const int comp = component_.get();
            dataRange_.set(dvec2(range.first[comp], range.second[comp]));
        } else {
            // reset data range, no data
            dataRange_.set(dvec2(0.0, 1.0));
        }
    };
    component_.onChange(updateDataRange);

    const auto updateComponents = [this, getVolume]() {
        const std::array<OptionPropertyIntOption, 4> options = {{{"component1", "Component 1", 0},
                                                                 {"component2", "Component 2", 1},
                                                                 {"component3", "Component 3", 2},
                                                                 {"component4", "Component 4", 3}}};
        const auto* const volume = getVolume();
        const size_t components = volume ? volume->getDataFormat()->getComponents() : 1;
        std::vector<OptionPropertyIntOption> componentOptions{options.begin(),
                                                              options.begin() + components};
        component_.replaceOptions(std::move(componentOptions));
        component_.setCurrentStateAsDefault();
    };

    volumeInport_.onChange([updateComponents, updateDataRange]() {
        updateComponents();
        updateDataRange();
    });

    meshInport_.onChange([this]() { component_.setReadOnly(!meshInport_.hasData()); });
}

namespace {

// TODO replace with VolumeSampler

double triInterp(const std::array<double, 8>& c, vec3 voxelpos) {
    // interpolate along x direction
    std::array<double, 4> cx;
    cx[0] = (1 - voxelpos.x) * c[0] + voxelpos.x * c[4];
    cx[1] = (1 - voxelpos.x) * c[1] + voxelpos.x * c[5];
    cx[2] = (1 - voxelpos.x) * c[2] + voxelpos.x * c[6];
    cx[3] = (1 - voxelpos.x) * c[3] + voxelpos.x * c[7];

    // interpolate along y direction
    std::array<double, 2> cy;
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

    // fill color vector
    std::vector<vec4> colorsOut(srcBuffer->getSize());
    bool accessOutsideBounds = false;
    srcBuffer->dispatch<void, dispatching::filter::Vec3s>(
        [comp = component_.getSelectedIndex(),
         range = useCustomDataRange_.get() ? customDataRange_.get() : dataRange_.get(),
         dst = &colorsOut, tf = &tf_.get(), volume, volumeRAM, worldToIndex, dataToWorld,
         &accessOutsideBounds](auto pBuffer) {
            auto& vecs = pBuffer->getDataContainer();

            std::transform(vecs.begin(), vecs.end(), dst->begin(), [&](auto& vec) {
                const auto texPos = vec3(worldToIndex * dataToWorld * vec4{vec, 1.0f});

                if (glm::any(glm::lessThan(glm::floor(texPos), glm::zero<glm::vec3>())) ||
                    glm::any(glm::greaterThanEqual(glm::ceil(texPos),
                                                   glm::vec3(volume->getDimensions())))) {
                    accessOutsideBounds = true;
                }

                double res{0.0};
                if (volume->getInterpolation() == InterpolationType::Linear) {
                    // trilinear interpolation
                    std::array<double, 8> c;
                    for (int i = 0; i < 8; i++) {
                        const size3_t samplepos =
                            size3_t(texPos) + size3_t((i >> 2) & 1, (i >> 1) & 1, i & 1);

                        c[i] = volumeRAM->getAsNormalizedDouble(glm::clamp(
                            samplepos, glm::zero<size3_t>(), volume->getDimensions() - size3_t(1)));
                    }
                    vec3 integerPart{};
                    res = triInterp(c, glm::modf(texPos, integerPart));
                } else {
                    const size3_t samplepos = size3_t(glm::round(texPos));
                    res = volumeRAM->getAsDouble(glm::clamp(samplepos, glm::zero<size3_t>(),
                                                            volume->getDimensions() - size3_t(1)));
                }

                const double normalized =
                    (static_cast<double>(res) - range.x) / (range.y - range.x);
                return tf->sample(normalized);
            });
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
