/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2018 Inviwo Foundation
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
#include <modules/vectorfieldvisualization/processors/3d/pathlines.h>
#include <modules/vectorfieldvisualization/processors/3d/streamlines.h>
#include <inviwo/core/datastructures/geometry/basicmesh.h>
#include <inviwo/core/network/networklock.h>
#include <inviwo/core/util/zip.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo IntegralLineVectorToMesh::processorInfo_{
    "org.inviwo.IntegralLineVectorToMesh",  // Class identifier
    "Integral Line Vector To Mesh",         // Display name
    "Vector Field Visualization",           // Category
    CodeState::Experimental,                // Code state
    Tags::None,                             // Tags
};
const ProcessorInfo IntegralLineVectorToMesh::getProcessorInfo() const { return processorInfo_; }

PropertyClassIdentifier(IntegralLineVectorToMesh::ColorByPropertiy,
                        "org.inviwo.IntegralLineVectorToMesh.ColorByProperty");

IntegralLineVectorToMesh::IntegralLineVectorToMesh()
    : Processor()
    , lines_("lines")
    , brushingList_("brushingList")
    , colorBy_("colorBy", "Color by")
    , colors_("colors")
    , mesh_("mesh")
    , brushBy_("brushBy_", "Brush Line by",
               {{"never", "Ignore brushing list", BrushBy::Never},
                {"lineindex", "Use Line Index (seed point index)", BrushBy::LineIndex},
                {"vectorposition", "Use position in input vector.", BrushBy::VectorPosition}})

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

    colorBy_.onChange([&]() {
        for (auto &prop : getPropertiesByType<ColorByPropertiy>()) {
            prop->setVisible(prop->getKey() == colorBy_.get());
        }
    });

    lines_.onChange([&]() { updateOptions(); });

    colors_.onChange([&]() { updateOptions(); });

    setFromData_.onChange([&]() {
        if (lines_.hasData()) {
            double minT = std::numeric_limits<double>::max();
            double maxT = std::numeric_limits<double>::lowest();

            size_t idx = 0;
            for (auto &line : (*lines_.getData())) {
                util::OnScopeExit incIdx = [&idx]() { idx++; };
                auto size = line.getPositions().size();
                if (size == 0) continue;

                if (isNotFiltered(line, idx)) {
                    continue;
                }

                if (!line.hasMetaData("timestamp")) {
                    minT = std::min(minT, 0.);
                    maxT = std::max(maxT, 1.);
                } else {
                    for (const auto &t : line.getMetaData<double>("timestamp")) {
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

    bool colorByPortIndex = false;
    bool colorByPortNumber = false;

    auto metaDataKey = colorBy_.get();
    if (metaDataKey == "portIndex") {
        colorByPortIndex = true;
    }
    if (metaDataKey == "portNumber") {
        colorByPortNumber = true;
    }
    bool colorByPort = colorByPortIndex || colorByPortNumber;

    ColorByPropertiy *mdProp = nullptr;
    if (!colorByPort) {
        mdProp = dynamic_cast<ColorByPropertiy *>(getPropertyByIdentifier(metaDataKey));

        if (!mdProp) {  // Fallback
            auto props = getPropertiesByType<ColorByPropertiy>();
            if (props.size() == 0) {
                throw Exception("Couldn't get ColorByPropertiy for meta data " + metaDataKey,
                                IvwContext);
            }
            mdProp = props.front();
            LogWarn("Couldn't get ColorByPropertiy for meta data "
                    << metaDataKey << ", using " << mdProp->getDisplayName() << " instead");
        }
    }

    double minMetaData = std::numeric_limits<double>::max();
    double maxMetaData = std::numeric_limits<double>::lowest();

    bool colorWarningOnce = true;

    Output output = output_.get();

    size_t idx = 0;
    for (auto &line : (*lines_.getData())) {
        util::OnScopeExit incIdx = [&idx]() { idx++; };
        auto size = line.getPositions().size();

        if (size == 0 || isNotFiltered(line, idx)) continue;

        auto indexBuffer = [&, this]() -> std::shared_ptr<IndexBufferRAM> {
            if (output == Output::Lines) {
                auto ib = mesh->addIndexBuffer(DrawType::Lines, ConnectivityType::StripAdjacency);
                ib->getDataContainer().reserve(size + 2);
                return ib;
            } else if (output == Output::Ribbons) {
                auto ib = mesh->addIndexBuffer(DrawType::Triangles, ConnectivityType::Strip);
                ib->getDataContainer().reserve(size * 2);
                return ib;
            }
            throw Exception("Unsupported output type", IvwContext);
        }();

        auto coloring = [&, this](auto sample, size_t lineIndex, size_t lineNumber) -> vec4 {
            if (isSelected(line, idx)) {
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
                auto &mdValue = get<2>(sample);
                double md = metadataToDouble(mdValue);
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

        auto lineLoop = [&coloring, &vertices, &indexBuffer, &idx, this](const IntegralLine &line,
                                                                         auto mdContainter) {
            for (auto &&sample : util::zip(line.getPositions(), line.getMetaData<dvec3>("velocity"),
                                           mdContainter)) {
                vec3 pos = get<0>(sample);
                vec3 vel = get<1>(sample);

                vec4 color = coloring(sample, line.getIndex(), idx);

                indexBuffer->add(static_cast<std::uint32_t>(vertices.size()));
                vertices.push_back({pos, glm::normalize(vel), pos, color});
            }
        };

        auto ribbonLoop = [&coloring, &vertices, &indexBuffer, &idx, this](const IntegralLine &line,
                                                                           auto mdContainter) {
            for (auto &&sample : util::zip(line.getPositions(), line.getMetaData<dvec3>("velocity"),
                                           mdContainter, line.getMetaData<dvec3>("vorticity"))) {
                vec3 pos = get<0>(sample);
                vec3 vel = get<1>(sample);
                vec3 vor = get<3>(sample);

                vec4 color = coloring(sample, line.getIndex(), idx);

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
    if (!colorByPort) {
        mdProp->minValue_.set(minMetaData);
        mdProp->maxValue_.set(maxMetaData);
    }
}

}  // namespace inviwo
