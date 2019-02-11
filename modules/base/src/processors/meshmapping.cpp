/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
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

#include <modules/base/processors/meshmapping.h>

#include <inviwo/core/datastructures/transferfunction.h>
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>
#include <inviwo/core/datastructures/buffer/bufferram.h>
#include <modules/base/algorithm/dataminmax.h>

#include <algorithm>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo MeshMapping::processorInfo_{
    "org.inviwo.MeshMapping",    // Class identifier
    "Mesh Mapping",              // Display name
    "Mesh Operation",            // Category
    CodeState::Stable,           // Code state
    "CPU, Mesh, Color Mapping",  // Tags
};
const ProcessorInfo MeshMapping::getProcessorInfo() const { return processorInfo_; }

MeshMapping::MeshMapping()
    : Processor()
    , meshInport_("meshInport")
    , outport_("outport")

    , enabled_("enabled", "Enabled", true)
    , tf_("transferfunction", "Transfer Function",
          TransferFunction(
              {{0.0f, vec4(0.0f, 0.1f, 1.0f, 1.0f)}, {1.0f, vec4(1.0f, 0.03f, 0.03f, 1.0f)}}))

    , buffer_("buffer", "Buffer")
    , component_("component", "Component", {{"component1", "Component 1", 0}})

    , useCustomDataRange_("useCustomRange", "Use Custom Range", false)
    , customDataRange_("customDataRange", "Custom Data Range", 0.0, 1.0,
                       std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max(),
                       0.01, 0.0, InvalidationLevel::InvalidOutput, PropertySemantics::Text)
    , dataRange_("dataRange", "Output Range", 0.0, 1.0, std::numeric_limits<double>::lowest(),
                 std::numeric_limits<double>::max(), 0.01, 0.0, InvalidationLevel::Valid,
                 PropertySemantics::Text) {

    addPort(meshInport_);
    addPort(outport_);

    addProperty(enabled_);
    addProperty(tf_);

    buffer_.setSerializationMode(PropertySerializationMode::All);
    buffer_.setCurrentStateAsDefault();
    component_.setSerializationMode(PropertySerializationMode::All);
    component_.setCurrentStateAsDefault();

    addProperty(buffer_);
    addProperty(component_);

    dataRange_.setSerializationMode(PropertySerializationMode::None);
    dataRange_.setReadOnly(true);

    component_.setReadOnly(true);

    addProperty(useCustomDataRange_);
    addProperty(customDataRange_);
    addProperty(dataRange_);

    useCustomDataRange_.onChange(
        [&]() { customDataRange_.setReadOnly(!useCustomDataRange_.get()); });
    customDataRange_.setReadOnly(!useCustomDataRange_.get());

    auto updateDataRange = [this]() {
        if (meshInport_.hasData() &&
            (meshInport_.getData()->getNumberOfBuffers() > buffer_.getSelectedIndex())) {

            auto buffer = meshInport_.getData()
                              ->getBuffer(buffer_.getSelectedIndex())
                              ->getRepresentation<BufferRAM>();

            // obtain range of selected component of chosen buffer
            auto range = util::bufferMinMax(buffer);
            const int comp = component_.get();
            dataRange_.set(dvec2(range.first[comp], range.second[comp]));
        } else {
            // reset data range, no data
            dataRange_.set(dvec2(0.0, 1.0));
        }
    };
    component_.onChange(updateDataRange);

    auto updateComponents = [this]() {
        if (meshInport_.hasData()) {
            auto mesh = meshInport_.getData();
            auto buffer = mesh->getBuffer(buffer_.getSelectedIndex());
            if (!buffer) {
                return;
            }
            std::vector<OptionPropertyIntOption> componentOptions;
            for (size_t i = 0; i < buffer->getDataFormat()->getComponents(); ++i) {
                componentOptions.emplace_back("component" + toString(i + 1),
                                              "Component " + toString(i + 1), static_cast<int>(i));
            }
            component_.replaceOptions(componentOptions);
        } else {
            component_.replaceOptions({{"component1", "Component 1", 0}});
        }
        component_.setCurrentStateAsDefault();
    };

    buffer_.onChange(updateComponents);

    meshInport_.onChange([this, updateComponents]() {
        component_.setReadOnly(!meshInport_.hasData());
        if (meshInport_.hasData()) {
            auto mesh = meshInport_.getData();

            std::vector<OptionPropertyIntOption> bufferOptions;

            mesh->getNumberOfBuffers();
            size_t count = 0;
            for (auto& b : mesh->getBuffers()) {
                // extract buffer type
                std::stringstream ss;
                ss << b.first.type;
                std::string bufferType = ss.str();

                bufferOptions.emplace_back("buffer" + toString(count + 1),
                                           bufferType + " (buffer " + toString(count + 1) + ")",
                                           static_cast<int>(count));
                ++count;
            }
            buffer_.replaceOptions(bufferOptions);
        }
        updateComponents();
    });
}

void MeshMapping::process() {
    if (enabled_.get() && meshInport_.hasData() &&
        (meshInport_.getData()->getNumberOfBuffers() > 0)) {

        auto inputMesh = meshInport_.getData();
        auto srcBuffer =
            inputMesh->getBuffer(buffer_.getSelectedIndex())->getRepresentation<BufferRAM>();

        // fill color vector
        std::vector<vec4> colorsOut(srcBuffer->getSize());

        srcBuffer->dispatch<void>(
            [comp = component_.getSelectedIndex(),
             range = useCustomDataRange_.get() ? customDataRange_.get() : dataRange_.get(),
             dst = &colorsOut, tf = &tf_.get()](auto pBuffer) {
                auto& vec = pBuffer->getDataContainer();
                std::transform(vec.begin(), vec.end(), dst->begin(), [&](auto& v) {
                    auto value = util::glmcomp(v, comp);
                    double pos = (static_cast<double>(value) - range.x) / (range.y - range.x);
                    return tf->sample(pos);
                });
            });

        // create a new mesh containing all buffers of the input mesh
        // The first color buffer, if existing, is replaced with the mapped colors.
        // Otherwise, a new color buffer will be added.

        auto colBuffer =
            std::make_shared<Buffer<vec4>>(std::make_shared<BufferRAMPrecision<vec4>>(colorsOut));

        auto mesh = inputMesh->clone();
        // look for suitable color buffer
        std::size_t colIndex = 0;
        for (auto buffer : mesh->getBuffers()) {
            if (buffer.first.type == BufferType::ColorAttrib) {
                // found a suitable color buffer
                mesh->replaceBuffer(colIndex, buffer.first, colBuffer);
                break;
            }
            ++colIndex;
        }
        if (colIndex >= mesh->getNumberOfBuffers()) {
            // color buffer not found, create a new one
            mesh->addBuffer(BufferType::ColorAttrib, colBuffer);
        }

        outport_.setData(mesh);
    } else {
        outport_.setData(meshInport_.getData());
    }
}

}  // namespace inviwo
