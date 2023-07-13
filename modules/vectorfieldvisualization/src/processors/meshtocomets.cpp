/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2021 Inviwo Foundation
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

#include <modules/vectorfieldvisualization/processors/meshtocomets.h>
#include <modules/vectorfieldvisualization/processors/3d/pathlines.h>
#include <modules/vectorfieldvisualization/processors/3d/streamlines.h>
#include <inviwo/core/datastructures/geometry/basicmesh.h>
#include <inviwo/core/network/networklock.h>
#include <inviwo/core/util/zip.h>
#include <inviwo/core/util/raiiutils.h>

namespace inviwo {

// namespace detail {

// template <typename T>
// static double norm(const T& t) {
//     return static_cast<double>(t);
// }

// template <
//     glm::length_t L, typename T, glm::qualifier Q,
//     typename F = typename std::conditional<std::is_same<T, float>::value, float, double>::type>
// static double norm(const glm::vec<L, T, Q>& glm) {
//     return glm::length(util::glm_convert<glm::vec<L, F, Q>>(glm));
// }
// }  // namespace detail

MeshToComets::ColorByBuffer::ColorByBuffer()
    : CompositeProperty("colorByBuffer", "Color by Buffer")
    , colorBy_("colorBy", "Color by",
               {
                   {"none", "None", ColorBy::None},
                   {"color", "Color as-is", ColorBy::OriginalColor},
                   {"colorR", "Color R", ColorBy::Color},
                   {"textureU", "Texture U", ColorBy::Texture},
                   {"normalX", "Normal X", ColorBy::Normal},
               })
    , scaleBy_("scaleBy", "Data Range (for normalization)", 0, 1,
               std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max(), 0.01)
    , loopTF_("loopTF", "Loop Transfer Function", false)
    , minValue_("minValue", "Min", 0, std::numeric_limits<double>::lowest(),
                std::numeric_limits<double>::max(), 0.01, InvalidationLevel::InvalidOutput,
                PropertySemantics::Text)
    , maxValue_("maxValue", "Max", 0, std::numeric_limits<double>::lowest(),
                std::numeric_limits<double>::max(), 0.01, InvalidationLevel::InvalidOutput,
                PropertySemantics::Text)
    , tf_("transferFunction", "Transfer function") {
    scaleBy_.setSemantics(PropertySemantics::Text);
    addProperties(colorBy_, scaleBy_, minValue_, maxValue_, tf_);

    auto visFunc = [](const auto& p) {
        ColorBy color = dynamic_cast<const TemplateOptionProperty<ColorBy>&>(p).get();
        return color != ColorBy::None && color != ColorBy::OriginalColor;
    };
    scaleBy_.visibilityDependsOn(colorBy_, visFunc);
    loopTF_.visibilityDependsOn(colorBy_, visFunc);
    minValue_.visibilityDependsOn(colorBy_, visFunc);
    maxValue_.visibilityDependsOn(colorBy_, visFunc);
    tf_.visibilityDependsOn(colorBy_, visFunc);
}

const ProcessorInfo MeshToComets::getProcessorInfo() const { return processorInfo_; }

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo MeshToComets::processorInfo_{
    "org.inviwo.MeshToComets",     // Class identifier
    "Mesh To Comets Mesh",         // Display name
    "Vector Field Visualization",  // Category
    CodeState::Stable,             // Code state
    Tags::CPU,                     // Tags
};

bool MeshToComets::isFiltered(size_t idx) const {
    if (filter_.get()) return brushingList_.isFiltered(idx);
    return false;
}

bool MeshToComets::isSelected(size_t idx) const {
    if (filter_.get()) return brushingList_.isSelected(idx);
    return false;
}

MeshToComets::MeshToComets()
    : Processor()
    , lines_("lines")
    , brushingList_("brushingList")
    , mesh_("mesh")
    , pointMesh_("pointMesh")
    , filter_("filter", "Filter/select?")
    , colorByBuffer_()
    , timeBy_(
          "timeBy", "Time by",
          {{"z", "Z position", TimeBy::PositionZ}, {"idx", "Vertex index", TimeBy::VertexIndex}})
    , selectedColor_("selectedColor", "Selected Color", vec4{1.f, 0.f, 0.f, 1.f}, vec4{0.f},
                     vec4{1.f}, vec4{0.01f}, InvalidationLevel::InvalidOutput,
                     PropertySemantics::Color)
    , time_("time", "Time", 0, {0.0, ConstraintBehavior::Immutable},
            {10.0, ConstraintBehavior::Ignore})
    , sphereScale_("sphereScale", "Sphere Radius", 0.1, 0, 1)
    , cometLength_("cometLength", "Comet Length", 10, 2, 1000)
    , scaleWidth_("scaleWidth", "Line width from alpha?") {

    addPorts(lines_, brushingList_, mesh_, pointMesh_);

    addProperties(colorByBuffer_, timeBy_,                         // Mapping
                  cometLength_, scaleWidth_, time_, sphereScale_,  // Rendering
                  filter_, selectedColor_                          // Filtering & selection
    );

    setAllPropertiesCurrentStateAsDefault();
}

void MeshToComets::process() {
    const auto linesIn = lines_.getData();
    if (linesIn->getNumberOfBuffers() == 0) {
        mesh_.setData(linesIn);
        pointMesh_.setData(linesIn);
        LogError("No data");
        return;
    }

    auto mesh = std::make_shared<ColoredMesh>();

    mesh->setModelMatrix(linesIn->getModelMatrix());
    mesh->setWorldMatrix(linesIn->getWorldMatrix());

    std::vector<ColoredMesh::Vertex> lineVertices;

    lineVertices.reserve(linesIn->getNumberOfIndicies() * 2000);

    auto positionBuffer = linesIn->getBuffer(BufferType::PositionAttrib);
    auto textureBuffer = linesIn->getBuffer(BufferType::TexcoordAttrib);
    // LogWarn(fmt::format("Texture buffer type: {}", textureBuffer->getDataFormat()->getString()));
    auto positionIn =
        dynamic_cast<const Vec3BufferRAM*>(positionBuffer->getRepresentation<BufferRAM>());
    auto textureIn =
        dynamic_cast<const Vec3BufferRAM*>(textureBuffer->getRepresentation<BufferRAM>());
    if (!textureIn) {
        LogError("Texture buffer not Vec3Float32 as expected");
        return;
    }
    /*
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


        bool colorWarningOnce = true;

        Output output = output_.get();
    */
    double minMetaData = std::numeric_limits<double>::max();
    double maxMetaData = std::numeric_limits<double>::lowest();

    auto pointMesh = std::make_shared<SphereMesh>();
    pointMesh->setModelMatrix(linesIn->getModelMatrix());
    pointMesh->setWorldMatrix(linesIn->getWorldMatrix());
    std::vector<SphereMesh::Vertex> pointVertices;
    pointVertices.reserve(linesIn->getNumberOfIndicies());
    auto pointIndex = pointMesh->addIndexBuffer(DrawType::Points, ConnectivityType::None);

    const double cometLength = cometLength_.get();
    const double time = time_.get();
    const double timeFrac = time - (int)time;

    auto getScalar = [&](size_t idx) { return float(textureIn->get(idx).x); };

    // TODO: Where to get time from?
    // auto getTime = [&](size_t idx) {
    //     switch (timeBy_.get()) {
    //         case TimeBy::PositionZ:
    //             LogError("Not implemented yet. Sorry!");
    //             return size_t(0);
    //         case TimeBy::VertexIndex:
    //             return idx;
    //     }
    // };

    auto coloring = [&, this](auto sampleMD, size_t lineIndex) -> std::pair<vec4, double> {
        if (this->isSelected(lineIndex)) {
            return {selectedColor_.get(), 1};
        }

        // auto& mdValue = sampleMD;
        double md = sampleMD;
        minMetaData = std::min(minMetaData, md);
        maxMetaData = std::max(maxMetaData, md);

        md -= colorByBuffer_.scaleBy_.get().x;
        md /= colorByBuffer_.scaleBy_.get().y - colorByBuffer_.scaleBy_.get().x;
        if (colorByBuffer_.loopTF_) {
            md -= std::floor(md);
        }

        return {colorByBuffer_.tf_.get().sample(md), md};
    };

    struct LineStat {
        size_t startIdxInIndexBuffer = 0;
        size_t numberOfPoints = 0;
    };

    std::vector<LineStat> lineStats;
    lineStats.emplace_back(LineStat{});

    auto indexBufferIn = linesIn->getIndices(0)->getRAMRepresentation();
    size_t lastPointIdx = 0;

    // TODO: This has a strong opinion about how indices should lay in memory...
    for (size_t idx = 0; idx < indexBufferIn->getSize(); idx += 2) {
        size_t pointIdx0 = indexBufferIn->get(idx);
        size_t pointIdx1 = indexBufferIn->get(idx + 1);
        if (lastPointIdx == pointIdx0) {  // The line continues.
            lineStats.back().numberOfPoints++;
        } else {
            lineStats.emplace_back(LineStat{idx, 0});
        }
        lastPointIdx = pointIdx1;
    }

    double radiusMin = sphereScale_.get().x;
    double radiusExtent = sphereScale_.get().y - radiusMin;
    // LogInfo(fmt::format("Radius. Min: {}\tExtent:{}", radiusMin, radiusExtent));

    auto lineLoop = [&, this](size_t lineIndex) {
        LineStat lineStat = lineStats[lineIndex];
        if (this->isFiltered(lineIndex)) return;
        // auto velocity = line.getMetaData<dvec3>("velocity");
        // auto& vertices = line.getPositions();
        auto indexBufferOut = [&]() -> std::shared_ptr<IndexBufferRAM> {
            auto ib = mesh->addIndexBuffer(DrawType::Lines, ConnectivityType::StripAdjacency);
            ib->getDataContainer().reserve(cometLength * 2 + 2);
            return ib;
        }();

        // Add a point in time if possible.
        if (size_t(time) + 1 < lineStat.numberOfPoints - 1) {
            vec3 pos = (1.0 - timeFrac) * positionIn->get(indexBufferIn->get(
                                              lineStat.startIdxInIndexBuffer + 2 * (size_t)time)) +
                       timeFrac * positionIn->get(indexBufferIn->get(
                                      lineStat.startIdxInIndexBuffer + 2 * ((size_t)time + 1)));

            float scalar =
                (1.0 - timeFrac) * getScalar(indexBufferIn->get(lineStat.startIdxInIndexBuffer +
                                                                2 * (size_t)time)) +
                timeFrac * getScalar(indexBufferIn->get(lineStat.startIdxInIndexBuffer +
                                                        2 * ((size_t)time + 1)));
            auto color = coloring(scalar, lineIndex);
            // vec3 propSample = util::glm_convert<vec3>(mdContainter[(size_t)time]);

            indexBufferOut->add(static_cast<std::uint32_t>(lineVertices.size()));
            lineVertices.push_back({vec4(pos, size_t(time)), color.first});
            indexBufferOut->add(static_cast<std::uint32_t>(lineVertices.size()));
            lineVertices.push_back({vec4(pos, size_t(time)), color.first});

            pointIndex->add(static_cast<std::uint32_t>(pointVertices.size()));
            pointVertices.push_back(
                {vec4(pos, size_t(time)), radiusMin + radiusExtent * color.second, color.first});
            // LogWarn(fmt::format("Wrote point {}", size_t(time)));
        }

        for (int idx = std::min(lineStat.numberOfPoints - 1, size_t(time)); idx >= 0; --idx) {
            double strength = (double(idx) - time + cometLength) / cometLength;
            strength = std::min(strength, 1.0);
            strength = std::max(strength, 0.0);
            // LogWarn(fmt::format("Wrote point {},\tstrength {}", size_t(idx), strength));

            vec3 pos =
                positionIn->get(indexBufferIn->get(lineStat.startIdxInIndexBuffer + 2 * idx));
            float scalar =
                getScalar(indexBufferIn->get(lineStat.startIdxInIndexBuffer + 2 * (size_t)idx));

            auto color = coloring(scalar, lineIndex);
            auto colorAlphaed = color.first * strength;
            // auto colorAlphaed = color.first;
            // colorAlphaed.a *= color.second;

            indexBufferOut->add(static_cast<std::uint32_t>(lineVertices.size()));
            lineVertices.push_back({vec4(pos, idx), colorAlphaed});
            if (strength <= 0) break;
        }
    };

    size_t maxLineLength = 0;
    for (size_t lineIndex = 0; lineIndex < lineStats.size(); ++lineIndex) {
        lineLoop(lineIndex);
        maxLineLength = std::max(maxLineLength, lineStats[lineIndex].numberOfPoints);
        // LogInfo(fmt::format("Line {}, length {}, start index {}", lineIndex,
        //                     lineStats[lineIndex].numberOfPoints,
        //                     lineStats[lineIndex].startIdxInIndexBuffer));
        // if (lineIndex > 1) break;  // TMP
    }
    LogWarn("Wrote al lthe stuff... I'm stalling apparently?!");
    time_.setMaxValue(maxLineLength - 4);

    mesh->addVertices(lineVertices);
    mesh_.setData(mesh);
    // LogInfo(fmt::format("Made a point mesh! Actually! {}/{} points", pointVertices.size(),
    //                     pointIndex->getSize()));
    pointMesh->addVertices(pointVertices);
    pointMesh_.setData(pointMesh);

    colorByBuffer_.minValue_.set(minMetaData);
    colorByBuffer_.maxValue_.set(maxMetaData);
}

}  // namespace inviwo
