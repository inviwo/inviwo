/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2025 Inviwo Foundation
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

#include <inviwo/core/datastructures/buffer/buffer.h>                   // for Buffer, BufferBase
#include <inviwo/core/datastructures/buffer/bufferram.h>                // for BufferRAMPrecision
#include <inviwo/core/datastructures/geometry/geometrytype.h>           // for BufferType, Buffe...
#include <inviwo/core/datastructures/geometry/mesh.h>                   // for Mesh, Mesh::Buffe...
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/datastructures/tfprimitive.h>                     // for TFPrimitiveData
#include <inviwo/core/datastructures/transferfunction.h>                // for TransferFunction
#include <inviwo/core/ports/meshport.h>                                 // for MeshInport, MeshO...
#include <inviwo/core/processors/processor.h>                           // for Processor
#include <inviwo/core/processors/processorinfo.h>                       // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                      // for CodeState, CodeSt...
#include <inviwo/core/processors/processortags.h>                       // for Tags
#include <inviwo/core/properties/boolproperty.h>                        // for BoolProperty
#include <inviwo/core/properties/invalidationlevel.h>                   // for InvalidationLevel
#include <inviwo/core/properties/minmaxproperty.h>                      // for DoubleMinMaxProperty
#include <inviwo/core/properties/optionproperty.h>                      // for OptionPropertyOption
#include <inviwo/core/properties/property.h>                            // for Property
#include <inviwo/core/properties/propertysemantics.h>                   // for PropertySemantics
#include <inviwo/core/properties/transferfunctionproperty.h>            // for TransferFunctionP...
#include <inviwo/core/properties/valuewrapper.h>                        // for PropertySerializa...
#include <inviwo/core/util/formats.h>                                   // for DataFormatBase
#include <inviwo/core/util/glmcomp.h>                                   // for glmcomp
#include <inviwo/core/util/glmvec.h>                                    // for vec4, dvec2
#include <inviwo/core/util/zip.h>                                       // for enumerate, zipIte...
#include <modules/base/algorithm/dataminmax.h>                          // for bufferMinMax

#include <algorithm>      // for transform
#include <array>          // for array
#include <cstddef>        // for size_t
#include <functional>     // for __base
#include <limits>         // for numeric_limits
#include <memory>         // for shared_ptr, make_...
#include <string>         // for string
#include <string_view>    // for string_view
#include <type_traits>    // for remove_extent_t
#include <unordered_map>  // for unordered_map
#include <unordered_set>  // for unordered_set
#include <utility>        // for move, pair
#include <vector>         // for vector

#include <fmt/core.h>    // for format, basic_str...
#include <glm/vec2.hpp>  // for vec, vec<>::(anon...
#include <glm/vec4.hpp>  // for vec

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo MeshMapping::processorInfo_{
    "org.inviwo.MeshMapping",    // Class identifier
    "Mesh Mapping",              // Display name
    "Mesh Operation",            // Category
    CodeState::Stable,           // Code state
    "CPU, Mesh, Color Mapping",  // Tags
};
const ProcessorInfo& MeshMapping::getProcessorInfo() const { return processorInfo_; }

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
    , customDataRange_("customDataRange", "Custom Range", 0.0, 1.0,
                       std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max(),
                       0.01, 0.0, InvalidationLevel::InvalidOutput, PropertySemantics::Text)
    , dataRange_("dataRange", "Data Range", 0.0, 1.0, std::numeric_limits<double>::lowest(),
                 std::numeric_limits<double>::max(), 0.01, 0.0, InvalidationLevel::Valid,
                 PropertySemantics::Text) {

    addPort(meshInport_);
    addPort(outport_);

    buffer_.setSerializationMode(PropertySerializationMode::All).setCurrentStateAsDefault();
    component_.setSerializationMode(PropertySerializationMode::All)
        .setCurrentStateAsDefault()
        .setReadOnly(true);
    dataRange_.setSerializationMode(PropertySerializationMode::None).setReadOnly(true);

    customDataRange_.readonlyDependsOn(useCustomDataRange_, [](const auto& p) { return !p.get(); });

    addProperties(enabled_, tf_, buffer_, component_, useCustomDataRange_, customDataRange_,
                  dataRange_);

    auto getBuffer = [this]() -> const BufferBase* {
        if (auto mesh = meshInport_.getData()) {
            if (buffer_.getSelectedIndex() < mesh->getNumberOfBuffers()) {
                if (auto buffer = mesh->getBuffer(buffer_.getSelectedIndex())) {
                    return buffer;
                }
            };
        }
        return nullptr;
    };

    auto updateDataRange = [this, getBuffer]() {
        if (const auto buffer = getBuffer()) {
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

    auto updateComponents = [this, getBuffer]() {
        const std::array<OptionPropertyIntOption, 4> options = {{{"component1", "Component 1", 0},
                                                                 {"component2", "Component 2", 1},
                                                                 {"component3", "Component 3", 2},
                                                                 {"component4", "Component 4", 3}}};
        const auto buffer = getBuffer();
        const size_t components = buffer ? buffer->getDataFormat()->getComponents() : 1;
        std::vector<OptionPropertyIntOption> componentOptions{options.begin(),
                                                              options.begin() + components};
        component_.replaceOptions(std::move(componentOptions));
        component_.setCurrentStateAsDefault();
    };

    buffer_.onChange([updateComponents, updateDataRange]() {
        updateComponents();
        updateDataRange();
    });

    meshInport_.onChange([this, updateComponents, updateDataRange]() {
        component_.setReadOnly(!meshInport_.hasData());
        if (meshInport_.hasData()) {
            auto mesh = meshInport_.getData();

            std::vector<OptionPropertyIntOption> bufferOptions;
            for (auto&& [index, buffer] : util::enumerate(mesh->getBuffers())) {
                bufferOptions.emplace_back(
                    fmt::format("buffer{}", index + 1),
                    fmt::format("{} (Buffer {})", buffer.first.type, index + 1),
                    static_cast<int>(index));
            }
            buffer_.replaceOptions(bufferOptions);
        }
        updateComponents();
        updateDataRange();
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

        srcBuffer->dispatch<void>([comp = component_.getSelectedIndex(),
                                   range = useCustomDataRange_.get() ? customDataRange_.get()
                                                                     : dataRange_.get(),
                                   dst = &colorsOut, tf = &tf_.get()](auto pBuffer) {
            auto& vec = pBuffer->getDataContainer();
            std::transform(vec.begin(), vec.end(), dst->begin(), [&](auto& v) {
                auto value = util::glmcomp(v, comp);
                double normalized = (static_cast<double>(value) - range.x) / (range.y - range.x);
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
