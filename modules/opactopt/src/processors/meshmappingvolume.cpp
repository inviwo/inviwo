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

#include <modules/base/algorithm/dataminmax.h>  // for bufferMinMax
#include <inviwo/core/util/zip.h>               // for enumerate, zipIte...

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo MeshMappingVolume::processorInfo_{
    "org.inviwo.MeshMappingVolume",  // Class identifier
    "Mesh Mapping Volume",           // Display name
    "Mesh Operation",                // Category
    CodeState::Experimental,         // Code state
    "CPU, Mesh, Color Mapping",      // Tags
};
const ProcessorInfo MeshMappingVolume::getProcessorInfo() const { return processorInfo_; }

MeshMappingVolume::MeshMappingVolume()
    : Processor()
    , meshInport_("meshInport")
    , volumeInport_("importanceVolume")
    , outport_("outport")

    , enabled_("enabled", "Enabled", true)
    , tf_("transferfunction", "Transfer Function",
          TransferFunction(
              {{0.0f, vec4(0.0f, 0.1f, 1.0f, 1.0f)}, {1.0f, vec4(1.0f, 0.03f, 0.03f, 1.0f)}}))
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

    auto getVolume = [this]() -> const Volume* {
        if (volumeInport_.hasData()) return volumeInport_.getData().get();
        return nullptr;
    };

    auto updateDataRange = [this, getVolume]() {
        if (const auto volume = getVolume()) {
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

    auto updateComponents = [this, getVolume]() {
        const std::array<OptionPropertyIntOption, 4> options = {{{"component1", "Component 1", 0},
                                                                 {"component2", "Component 2", 1},
                                                                 {"component3", "Component 3", 2},
                                                                 {"component4", "Component 4", 3}}};
        const auto volume = getVolume();
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

void MeshMappingVolume::process() {
    if (enabled_.get() && meshInport_.hasData() &&
        (meshInport_.getData()->getNumberOfBuffers() > 0) && volumeInport_.hasData()) {

        auto inputMesh = meshInport_.getData();
        auto srcBuffer = inputMesh->getBuffer((size_t)BufferType::PositionAttrib)
                             ->getRepresentation<BufferRAM>();
        const auto volume = volumeInport_.getData();
        const auto volumeRAM = volume->getRepresentation<VolumeRAM>();

        // fill color vector);
        std::vector<vec4> colorsOut(srcBuffer->getSize());

        srcBuffer->dispatch<void>([comp = component_.getSelectedIndex(),
                                   range = useCustomDataRange_.get() ? customDataRange_.get()
                                                                     : dataRange_.get(),
                                   dst = &colorsOut, tf = &tf_.get(), volume,
                                   volumeRAM](auto pBuffer) {
            auto& vec = pBuffer->getDataContainer();
            std::transform(vec.begin(), vec.end(), dst->begin(), [&](auto& v) {
                glm::vec4 worldpos = {util::glmcomp(v, 0), util::glmcomp(v, 1), util::glmcomp(v, 2),
                                      1.0};
                auto texpos =
                    size3_t(volume->getCoordinateTransformer().getWorldToDataMatrix() * worldpos);
                auto scalar = glm::all(glm::greaterThanEqual(texpos, glm::zero<size3_t>())) &&
                                      glm::all(glm::lessThan(texpos, volume->getDimensions()))
                                  ? volumeRAM->getAsDouble(texpos)
                                  : 0.0;
                double normalized = (static_cast<double>(scalar) - range.x) / (range.y - range.x);
                return tf->sample(normalized);
            });
        });

        // create a new mesh containing all buffers of the input mesh
        // The first color buffer, if existing, is replaced with the mapped colors.
        // Otherwise, a new color buffer will be added.
        auto colBuffer = std::make_shared<Buffer<vec4>>(
            std::make_shared<BufferRAMPrecision<vec4>>(std::move(colorsOut)));

        auto mesh = inputMesh->clone();
        if (auto [buff, loc] = mesh->findBuffer(BufferType::ColorAttrib); buff) {
            mesh->replaceBuffer(buff, Mesh::BufferInfo{BufferType::ColorAttrib, loc}, colBuffer);
        } else {
            mesh->addBuffer(BufferType::ColorAttrib, colBuffer);
        }

        outport_.setData(mesh);
    } else {
        outport_.setData(meshInport_.getData());
    }
}

}  // namespace inviwo
