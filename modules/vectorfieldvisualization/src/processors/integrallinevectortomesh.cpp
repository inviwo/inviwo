/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2024 Inviwo Foundation
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

#include <modules/vectorfieldvisualization/processors/integrallinevectortomesh.h>

#include <inviwo/core/datastructures/buffer/bufferram.h>                      // for BufferRAM
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>             // for IndexBufferRAM
#include <inviwo/core/datastructures/geometry/geometrytype.h>                 // for Connectivit...
#include <inviwo/core/datastructures/geometry/typedmesh.h>                    // for TypedMesh<>...
#include <inviwo/core/datastructures/representationconverter.h>               // for Representat...
#include <inviwo/core/datastructures/representationconverterfactory.h>        // for Representat...
#include <inviwo/core/datastructures/transferfunction.h>                      // for TransferFun...
#include <inviwo/core/io/serialization/deserializer.h>                        // for Deserializer
#include <inviwo/core/io/serialization/serializer.h>                          // for Serializer
#include <inviwo/core/network/networklock.h>                                  // for NetworkLock
#include <inviwo/core/ports/datainport.h>                                     // for DataInport
#include <inviwo/core/ports/meshport.h>                                       // for MeshOutport
#include <inviwo/core/ports/outportiterable.h>                                // for OutportIter...
#include <inviwo/core/processors/processor.h>                                 // for Processor
#include <inviwo/core/processors/processorinfo.h>                             // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                            // for CodeState
#include <inviwo/core/processors/processortags.h>                             // for Tags, Tags:...
#include <inviwo/core/properties/boolcompositeproperty.h>                     // for BoolComposi...
#include <inviwo/core/properties/boolproperty.h>                              // for BoolProperty
#include <inviwo/core/properties/buttonproperty.h>                            // for ButtonProperty
#include <inviwo/core/properties/compositeproperty.h>                         // for CompositePr...
#include <inviwo/core/properties/invalidationlevel.h>                         // for Invalidatio...
#include <inviwo/core/properties/minmaxproperty.h>                            // for DoubleMinMa...
#include <inviwo/core/properties/optionproperty.h>                            // for OptionPrope...
#include <inviwo/core/properties/ordinalproperty.h>                           // for DoubleProperty
#include <inviwo/core/properties/property.h>                                  // for Property
#include <inviwo/core/properties/propertysemantics.h>                         // for PropertySem...
#include <inviwo/core/properties/transferfunctionproperty.h>                  // for TransferFun...
#include <inviwo/core/properties/valuewrapper.h>                              // for PropertySer...
#include <inviwo/core/util/exception.h>                                       // for Exception
#include <inviwo/core/util/glmconvert.h>                                      // for glm_convert
#include <inviwo/core/util/glmvec.h>                                          // for vec4, dvec3
#include <inviwo/core/util/logcentral.h>                                      // for LogCentral
#include <inviwo/core/util/raiiutils.h>                                       // for OnScopeExit
#include <inviwo/core/util/sourcecontext.h>                                   // for IVW_CONTEXT
#include <inviwo/core/util/staticstring.h>                                    // for operator+
#include <inviwo/core/util/zip.h>                                             // for zipper, get
#include <modules/brushingandlinking/ports/brushingandlinkingports.h>         // for BrushingAnd...
#include <modules/vectorfieldvisualization/datastructures/integralline.h>     // for IntegralLine
#include <modules/vectorfieldvisualization/datastructures/integrallineset.h>  // for IntegralLin...

#include <algorithm>      // for max, min
#include <cmath>          // for floor
#include <cstddef>        // for size_t
#include <cstdint>        // for uint32_t
#include <limits>         // for numeric_limits
#include <memory>         // for shared_ptr
#include <ostream>        // for operator<<
#include <type_traits>    // for remove_exte...
#include <unordered_set>  // for unordered_set

#include <fmt/core.h>                // for format, for...
#include <glm/detail/qualifier.hpp>  // for tvec2, qual...
#include <glm/detail/setup.hpp>      // for length_t
#include <glm/geometric.hpp>         // for length, nor...
#include <glm/vec2.hpp>              // for operator*
#include <glm/vec3.hpp>              // for operator*
#include <glm/vec4.hpp>              // for operator*

namespace inviwo {

namespace detail {

template <typename T>
static double norm(const T& t) {
    return static_cast<double>(t);
}

template <
    glm::length_t L, typename T, glm::qualifier Q,
    typename F = typename std::conditional<std::is_same<T, float>::value, float, double>::type>
static double norm(const glm::vec<L, T, Q>& glm) {
    return glm::length(util::glm_convert<glm::vec<L, F, Q>>(glm));
}
}  // namespace detail

const std::string IntegralLineVectorToMesh::ColorByProperty::classIdentifier =
    "org.inviwo.IntegralLineVectorToMesh.ColorByProperty";
std::string_view IntegralLineVectorToMesh::ColorByProperty::getClassIdentifier() const {
    return classIdentifier;
}

IntegralLineVectorToMesh::ColorByProperty::ColorByProperty(
    std::string_view identifier, std::string_view displayName,
    InvalidationLevel invalidationLevel /*= InvalidationLevel::InvalidOutput*/)
    : CompositeProperty(identifier, displayName, invalidationLevel)
    , scaleBy_("scaleBy", "Data Range (for normalization)", 0, 1,
               std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max(), 0.01)
    , loopTF_("loopTF", "Loop Transfer Function", false)
    , minValue_("minValue", fmt::format("Min {}", displayName), 0,
                std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max(), 0.01,
                InvalidationLevel::InvalidOutput, PropertySemantics::Text)
    , maxValue_("maxValue", fmt::format("Max {}", displayName), 0,
                std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max(), 0.01,
                InvalidationLevel::InvalidOutput, PropertySemantics::Text)
    , tf_("transferFunction", "Transfer function")
    , key_(identifier) {
    scaleBy_.setSemantics(PropertySemantics::Text);
    addProperties();
}

IntegralLineVectorToMesh::ColorByProperty::ColorByProperty(const ColorByProperty& rhs)
    : CompositeProperty(rhs)
    , scaleBy_(rhs.scaleBy_)
    , loopTF_(rhs.loopTF_)
    , minValue_(rhs.minValue_)
    , maxValue_(rhs.maxValue_)
    , tf_(rhs.tf_) {
    addProperties();
}

IntegralLineVectorToMesh::ColorByProperty* IntegralLineVectorToMesh::ColorByProperty::clone()
    const {
    return new ColorByProperty(*this);
}

IntegralLineVectorToMesh::ColorByProperty::~ColorByProperty() {}

void IntegralLineVectorToMesh::ColorByProperty::serialize(Serializer& s) const {
    CompositeProperty::serialize(s);
    s.serialize("key", key_);
}

void IntegralLineVectorToMesh::ColorByProperty::deserialize(Deserializer& d) {
    CompositeProperty::deserialize(d);
    d.deserialize("key", key_);
}

std::string IntegralLineVectorToMesh::ColorByProperty::getKey() const { return key_; }

void IntegralLineVectorToMesh::ColorByProperty::addProperties() {
    addProperty(scaleBy_);
    addProperty(loopTF_);
    addProperty(minValue_);
    addProperty(maxValue_);
    addProperty(tf_);

    tf_.get().clear();
    tf_.get().add(0.0, vec4(0, 0, 1, 1));
    tf_.get().add(0.5, vec4(1, 1, 0, 1));
    tf_.get().add(1.0, vec4(1, 0, 0, 1));
    tf_.setCurrentStateAsDefault();
}

const ProcessorInfo& IntegralLineVectorToMesh::getProcessorInfo() const { return processorInfo_; }

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo IntegralLineVectorToMesh::processorInfo_{
    "org.inviwo.IntegralLineVectorToMesh",  // Class identifier
    "Integral Line Vector To Mesh",         // Display name
    "Vector Field Visualization",           // Category
    CodeState::Stable,                      // Code state
    Tags::CPU,                              // Tags
};

bool IntegralLineVectorToMesh::isFiltered(const IntegralLine& line, uint32_t idx) const {
    switch (brushBy_.get()) {
        case BrushBy::LineIndex:
            return brushingList_.isFiltered(line.getIndex());
        case BrushBy::VectorPosition:
            return brushingList_.isFiltered(idx);
        case BrushBy::Nothing:
        default:
            return false;
    }
}

bool IntegralLineVectorToMesh::isSelected(const IntegralLine& line, uint32_t idx) const {
    switch (brushBy_.get()) {
        case BrushBy::LineIndex:
            return brushingList_.isSelected(line.getIndex());
        case BrushBy::VectorPosition:
            return brushingList_.isSelected(idx);
        case BrushBy::Nothing:
        default:
            return false;
    }
}

void IntegralLineVectorToMesh::updateOptions() {
    auto lines = lines_.getData();
    if (lines->size() == 0) return;

    std::vector<OptionPropertyStringOption> options = {{"constant", "constant color"}};

    for (const auto& key : lines->front().getMetaDataKeys()) {
        options.emplace_back(key, key);

        if (!getPropertyByIdentifier(key)) {
            auto prop = std::make_unique<ColorByProperty>(key, "Color by " + key);
            prop->setVisible(false);
            prop->setSerializationMode(PropertySerializationMode::All);
            addProperty(prop.release());
        }
    }
    if (colors_.isConnected()) {
        options.emplace_back("portIndex", "Colors in port (line index)");
        options.emplace_back("portNumber", "Colors in port (line vector position)");
    }

    colorBy_.replaceOptions(options);
}

IntegralLineVectorToMesh::IntegralLineVectorToMesh()
    : Processor()
    , lines_("lines")
    , brushingList_("brushingList")
    , colors_("colors")
    , mesh_("mesh")
    , brushBy_("brushBy_", "Brush Line by",
               {{"never", "Ignore brushing list", BrushBy::Nothing},
                {"lineindex", "Use Line Index (seed point index)", BrushBy::LineIndex},
                {"vectorposition", "Use position in input vector.", BrushBy::VectorPosition}})

    , colorBy_("colorBy", "Color by", {{"constant", "constant color"}}, 0)

    , stride_("stride", "Vertex stride", 1, 1, 10)

    , timeBasedFiltering_("timeBasedFiltering", "Time Based Filtering", false)
    , minMaxT_("minMaxT", "Min/Max Timestep", -1, 1, -10, 10)
    , setFromData_("setFromData", "Set from data")

    , output_("output", "Output",
              {{"lines", "Lines", Output::Lines}, {"ribbons", "Ribbons", Output::Ribbons}})

    , ribbonWidth_("ribbonWidth", "Ribbon Width", 0.1f, 0.00001f)
    , selectedColor_("selectedColor", "Selected Color", vec4{1.f, 0.f, 0.f, 1.f}, vec4{0.f},
                     vec4{1.f}, vec4{0.01f}, InvalidationLevel::InvalidOutput,
                     PropertySemantics::Color) {
    colors_.setOptional(true);

    addPort(lines_);
    addPort(colors_);
    addPort(brushingList_);
    addPort(mesh_);

    addProperty(brushBy_);
    addProperty(output_);
    addProperty(colorBy_);
    addProperty(ribbonWidth_);
    addProperty(selectedColor_);
    addProperty(stride_);

    addProperty(timeBasedFiltering_);
    timeBasedFiltering_.addProperty(minMaxT_);
    timeBasedFiltering_.addProperty(setFromData_);

    colorBy_.setSerializationMode(PropertySerializationMode::All);

    setAllPropertiesCurrentStateAsDefault();

    auto updateVisibility = [&]() {
        for (auto& prop : getPropertiesByType<ColorByProperty>()) {
            prop->setVisible(prop->getKey() == colorBy_.getSelectedIdentifier());
        }
    };
    colorBy_.onChange(updateVisibility);
    updateVisibility();

    lines_.onChange([&]() { updateOptions(); });

    colors_.onChange([&]() { updateOptions(); });

    setFromData_.onChange([&]() {
        if (lines_.hasData()) {
            double minT = std::numeric_limits<double>::max();
            double maxT = std::numeric_limits<double>::lowest();

            uint32_t idx = 0;
            const auto data = lines_.getData();
            for (auto& line : *data) {
                util::OnScopeExit incIdx([&idx]() { idx++; });
                auto size = line.getPositions().size();
                if (size == 0) continue;

                if (this->isFiltered(line, idx)) {
                    continue;
                }

                if (!line.hasMetaData("timestamp")) {
                    minT = std::min(minT, 0.);
                    maxT = std::max(maxT, 1.);
                } else {
                    const auto& timestamp = line.getMetaData<double>("timestamp");
                    for (const auto& t : timestamp) {
                        minT = std::min(minT, t);
                        maxT = std::max(t, maxT);
                    }
                }
            }
            NetworkLock lock(getNetwork());
            minMaxT_.setRangeMin(minT);
            minMaxT_.setRangeMax(maxT);
            minMaxT_.set(vec2(minT, maxT));
        }
    });

    brushingList_.onConnect([this]() { updatePropertyVisibility(); });
    brushingList_.onDisconnect([this]() { updatePropertyVisibility(); });

    output_.onChange([this]() { updatePropertyVisibility(); });

    updatePropertyVisibility();
}

void IntegralLineVectorToMesh::process() {
    if (colorBy_.size() == 0) {
        updateOptions();
    }
    auto mesh = std::make_shared<BasicMesh>();
    if (lines_.getData()->size() == 0) {
        mesh_.setData(mesh);
        return;
    }

    mesh->setModelMatrix(lines_.getData()->getModelMatrix());
    mesh->setWorldMatrix(lines_.getData()->getWorldMatrix());

    std::vector<BasicMesh::Vertex> vertices;

    vertices.reserve(lines_.getData()->size() * 2000);

    auto metaDataKey = colorBy_.get();

    const bool constantColor = (metaDataKey == "constant");
    const bool colorByPortIndex = (metaDataKey == "portIndex");
    const bool colorByPortNumber = (metaDataKey == "portNumber");

    const bool colorByPort = colorByPortIndex || colorByPortNumber;

    ColorByProperty* mdProp = nullptr;
    if (!colorByPort && !constantColor) {
        mdProp = dynamic_cast<ColorByProperty*>(getPropertyByIdentifier(metaDataKey));

        if (!mdProp) {  // Fallback
            auto props = getPropertiesByType<ColorByProperty>();
            if (props.size() == 0) {
                throw Exception("Couldn't get ColorByProperty for meta data " + metaDataKey,
                                IVW_CONTEXT);
            }
            mdProp = props.front();
            LogWarn("Couldn't get ColorByProperty for meta data "
                    << metaDataKey << ", using " << mdProp->getDisplayName() << " instead");
        }
    }

    double minMetaData = std::numeric_limits<double>::max();
    double maxMetaData = std::numeric_limits<double>::lowest();

    bool colorWarningOnce = true;

    Output output = output_.get();

    uint32_t lineIdx = 0;
    const auto data = lines_.getData();
    for (auto& line : *data) {
        util::OnScopeExit incIdx([&lineIdx]() { lineIdx++; });
        auto size = line.getPositions().size();

        if (size == 0 || isFiltered(line, lineIdx)) continue;

        auto indexBuffer = [&]() -> std::shared_ptr<IndexBufferRAM> {
            if (output == Output::Lines) {
                auto ib = mesh->addIndexBuffer(DrawType::Lines, ConnectivityType::StripAdjacency);
                ib->getDataContainer().reserve(size + 2);
                return ib;
            } else if (output == Output::Ribbons) {
                auto ib = mesh->addIndexBuffer(DrawType::Triangles, ConnectivityType::Strip);
                ib->getDataContainer().reserve(size * 2);
                return ib;
            }
            throw Exception("Unsupported output type", IVW_CONTEXT);
        }();

        auto coloring = [&, this](auto sample, uint32_t lineIndex, uint32_t lineNumber) -> vec4 {
            if (constantColor || this->isSelected(line, lineIdx)) {
                return selectedColor_.get();
            }

            if (colorByPort) {
                auto colors = colors_.getData();
                size_t index = 0;
                if (colorByPortNumber) {
                    index = lineNumber;
                } else if (colorByPortIndex) {
                    index = lineIndex;
                }

                if (index >= colors->size()) {
                    if (colorWarningOnce) {
                        colorWarningOnce = false;
                        LogWarn("Line index for color is out of range");
                    }
                    index %= colors->size();
                }
                return colors->at(index);
            } else {
                auto& mdValue = get<2>(sample);
                double md = detail::norm(mdValue);
                minMetaData = std::min(minMetaData, md);
                maxMetaData = std::max(maxMetaData, md);

                md -= mdProp->scaleBy_.get().x;
                md /= mdProp->scaleBy_.get().y - mdProp->scaleBy_.get().x;
                if (mdProp->loopTF_) {
                    md -= std::floor(md);
                }

                return mdProp->tf_.get().sample(md);
            }
        };

        auto lineLoop = [&coloring, &vertices, &indexBuffer, &lineIdx, this](
                            const IntegralLine& line, auto mdContainter) {
            size_t pointIdx = 0;
            for (auto&& sample : util::zip(line.getPositions(), line.getMetaData<dvec3>("velocity"),
                                           mdContainter)) {
                util::OnScopeExit incPointIdx([&pointIdx]() { pointIdx++; });
                bool first = pointIdx <= 1;
                bool last = pointIdx >= line.getPositions().size() - 2;
                // need to keep the two first and two last when using adjendency information
                if (!first && !last && pointIdx % stride_.get() != 0) {
                    continue;
                }

                vec3 pos = get<0>(sample);
                vec3 vel = get<1>(sample);

                vec4 color = coloring(sample, line.getIndex(), lineIdx);

                indexBuffer->add(static_cast<std::uint32_t>(vertices.size()));
                vertices.push_back({pos, glm::normalize(vel), pos, color});
            }
        };

        auto ribbonLoop = [&coloring, &vertices, &indexBuffer, &lineIdx, this](
                              const IntegralLine& line, auto mdContainter) {
            for (auto&& sample : util::zip(line.getPositions(), line.getMetaData<dvec3>("velocity"),
                                           mdContainter, line.getMetaData<dvec3>("vorticity"))) {
                vec3 pos = get<0>(sample);
                vec3 vel = get<1>(sample);
                vec3 vor = get<3>(sample);

                vec4 color = coloring(sample, line.getIndex(), lineIdx);

                auto N = glm::normalize(glm::cross(vor, vel));

                auto off = glm::normalize(vor) * (ribbonWidth_.get() / 2.0f);
                auto pos1 = pos - off;
                auto pos2 = pos + off;
                indexBuffer->add(static_cast<std::uint32_t>(vertices.size()));
                vertices.push_back({pos1, N, pos1, color});
                indexBuffer->add(static_cast<std::uint32_t>(vertices.size()));
                vertices.push_back({pos2, N, pos2, color});
            }
        };

        if (mdProp) {
            line.getMetaDataBuffer(metaDataKey)
                ->getRepresentation<BufferRAM>()
                ->dispatch<void>([&](auto mdBuf) {
                    if (output == Output::Lines)
                        lineLoop(line, mdBuf->getDataContainer());
                    else {
                        ribbonLoop(line, mdBuf->getDataContainer());
                    }
                });
        } else {
            if (output == Output::Lines)
                lineLoop(line, std::vector<int>(line.getPositions().size()));
            else {
                ribbonLoop(line, std::vector<int>(line.getPositions().size()));
            }
        }
    }

    mesh->addVertices(vertices);

    mesh_.setData(mesh);
    if (mdProp) {
        mdProp->minValue_.set(minMetaData);
        mdProp->maxValue_.set(maxMetaData);
    }
}

}  // namespace inviwo
