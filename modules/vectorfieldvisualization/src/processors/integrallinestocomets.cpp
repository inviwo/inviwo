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

#include <modules/vectorfieldvisualization/processors/integrallinestocomets.h>
#include <modules/vectorfieldvisualization/processors/3d/pathlines.h>
#include <modules/vectorfieldvisualization/processors/3d/streamlines.h>
#include <inviwo/core/datastructures/geometry/basicmesh.h>
#include <inviwo/core/network/networklock.h>
#include <inviwo/core/util/zip.h>
#include <inviwo/core/util/raiiutils.h>

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

const std::string IntegralLinesToComets::ColorByProperty::classIdentifier =
    "org.inviwo.IntegralLinesToComets.ColorByProperty";
std::string IntegralLinesToComets::ColorByProperty::getClassIdentifier() const {
    return classIdentifier;
}

IntegralLinesToComets::ColorByProperty::ColorByProperty(
    std::string identifier, std::string displayName,
    InvalidationLevel invalidationLevel /*= InvalidationLevel::InvalidOutput*/)
    : CompositeProperty(identifier, displayName, invalidationLevel)
    , scaleBy_("scaleBy", "Data Range (for normalization)", 0, 1,
               std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max(), 0.01)
    , loopTF_("loopTF", "Loop Transfer Function", false)
    , minValue_("minValue", "Min " + displayName, 0, std::numeric_limits<double>::lowest(),
                std::numeric_limits<double>::max(), 0.01, InvalidationLevel::InvalidOutput,
                PropertySemantics::Text)
    , maxValue_("maxValue", "Max " + displayName, 0, std::numeric_limits<double>::lowest(),
                std::numeric_limits<double>::max(), 0.01, InvalidationLevel::InvalidOutput,
                PropertySemantics::Text)
    , tf_("transferFunction", "Transfer function")
    , key_(identifier) {
    scaleBy_.setSemantics(PropertySemantics::Text);
    addProperties();
}

IntegralLinesToComets::ColorByProperty::ColorByProperty(const ColorByProperty& rhs)
    : CompositeProperty(rhs)
    , scaleBy_(rhs.scaleBy_)
    , loopTF_(rhs.loopTF_)
    , minValue_(rhs.minValue_)
    , maxValue_(rhs.maxValue_)
    , tf_(rhs.tf_) {
    addProperties();
}

IntegralLinesToComets::ColorByProperty* IntegralLinesToComets::ColorByProperty::clone() const {
    return new ColorByProperty(*this);
}

IntegralLinesToComets::ColorByProperty::~ColorByProperty() {}

void IntegralLinesToComets::ColorByProperty::serialize(Serializer& s) const {
    CompositeProperty::serialize(s);
    s.serialize("key", key_);
}

void IntegralLinesToComets::ColorByProperty::deserialize(Deserializer& d) {
    CompositeProperty::deserialize(d);
    d.deserialize("key", key_);
}

std::string IntegralLinesToComets::ColorByProperty::getKey() const { return key_; }

void IntegralLinesToComets::ColorByProperty::addProperties() {
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

const ProcessorInfo IntegralLinesToComets::getProcessorInfo() const { return processorInfo_; }

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo IntegralLinesToComets::processorInfo_{
    "org.inviwo.IntegralLinesToComets",  // Class identifier
    "Integral Lines To Comets Mesh",     // Display name
    "Vector Field Visualization",        // Category
    CodeState::Stable,                   // Code state
    Tags::CPU,                           // Tags
};

bool IntegralLinesToComets::isFiltered(const IntegralLine& line, size_t idx) const {
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

bool IntegralLinesToComets::isSelected(const IntegralLine& line, size_t idx) const {
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

void IntegralLinesToComets::updateOptions() {
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

IntegralLinesToComets::IntegralLinesToComets()
    : Processor()
    , lines_("lines")
    , referenceFrameLine_("referenceFrameLine")
    , brushingList_("brushingList")
    , colors_("colors")
    , mesh_("mesh")
    , pointMesh_("pointMesh")
    , brushBy_("brushBy_", "Brush Line by",
               {{"never", "Ignore brushing list", BrushBy::Nothing},
                {"lineindex", "Use Line Index (seed point index)", BrushBy::LineIndex},
                {"vectorposition", "Use position in input vector.", BrushBy::VectorPosition}})

    , colorBy_("colorBy", "Color by", {{"constant", "constant color"}})

    , stride_("stride", "Vertex stride", 1, 1, 10)

    , timeBasedFiltering_("timeBasedFiltering", "Time Based Filtering", false)
    , minMaxT_("minMaxT", "Min/Max Timestep", -1, 1, -10, 10)
    , setFromData_("setFromData", "Set from data")

    , output_("output", "Output",
              {{"lines", "Lines", Output::Lines}, {"ribbons", "Ribbons", Output::Ribbons}})

    , ribbonWidth_("ribbonWidth", "Ribbon Width", 0.1f, 0.00001f)
    , selectedColor_("selectedColor", "Selected Color", vec4{1.f, 0.f, 0.f, 1.f}, vec4{0.f},
                     vec4{1.f}, vec4{0.01f}, InvalidationLevel::InvalidOutput,
                     PropertySemantics::Color)
    , time_("time", "Time", 0, {0.0, ConstraintBehavior::Immutable},
            {10.0, ConstraintBehavior::Ignore})
    , arrowScale_("arrowScale", "Arrow length", 0.5, 0, 10)
    , sphereScale_("sphereScale_", "Sphere Radius", 0.1, 0, 1)
    , removeReferenceFrame_("removeReferenceFrame", "Remove RF", false)
    , cometLength_("cometLength", "Comet Length", 10, 2, 1000)
    , scaleWidth_("scaleWidth", "Line width from alpha?")
    , origin_("origin", "Origin", vec3(0, 0, 0), {vec3(-1), ConstraintBehavior::Ignore},
              {vec3(1), ConstraintBehavior::Ignore})
    , endX_("endX", "End of X base arrow", vec3(0, 0, 0), {vec3(-1), ConstraintBehavior::Ignore},
            {vec3(1), ConstraintBehavior::Ignore})
    , endY_("endY", "End of Y base arrow", vec3(0, 0, 0), {vec3(-1), ConstraintBehavior::Ignore},
            {vec3(1), ConstraintBehavior::Ignore}) {
    colors_.setOptional(true);

    addPort(lines_);
    addPort(colors_);
    addPort(brushingList_);
    addPort(referenceFrameLine_);
    addPorts(mesh_, pointMesh_);
    referenceFrameLine_.setOptional(true);

    addProperties(brushBy_, output_, colorBy_, ribbonWidth_, selectedColor_, stride_, cometLength_,
                  scaleWidth_, time_, arrowScale_, sphereScale_, removeReferenceFrame_, origin_,
                  endX_, endY_);

    origin_.setReadOnly(true);
    endX_.setReadOnly(true);
    endY_.setReadOnly(true);

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

            size_t idx = 0;
            const auto data = lines_.getData();
            ;
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

void IntegralLinesToComets::process() {
    if (colorBy_.size() == 0) {
        updateOptions();
    }
    auto mesh = std::make_shared<BasicMesh>();
    if (lines_.getData()->size() == 0) {
        mesh_.setData(mesh);
        pointMesh_.setData(mesh);
        return;
    }

    const auto data = lines_.getData();

    mesh->setModelMatrix(data->getModelMatrix());
    mesh->setWorldMatrix(data->getWorldMatrix());

    std::vector<BasicMesh::Vertex> lineVertices;

    lineVertices.reserve(data->size() * 2000);

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

    auto pointMesh = std::make_shared<SphereMesh>();
    pointMesh->setModelMatrix(data->getModelMatrix());
    pointMesh->setWorldMatrix(data->getWorldMatrix());
    std::vector<SphereMesh::Vertex> pointVertices;
    pointVertices.reserve(data->size());
    auto pointIndex = pointMesh->addIndexBuffer(DrawType::Points, ConnectivityType::None);

    const double cometLength = cometLength_.get();
    const double time = time_.get();
    const double timeFrac = time - (int)time;
    bool removeRF = (removeReferenceFrame_.get() && referenceFrameLine_.hasData() &&
                     referenceFrameLine_.getData()->size() == 1);
    const IntegralLine* lineRF;
    if (removeRF) {
        auto lines = referenceFrameLine_.getData();

        lineRF = &lines->at(0);
        if (lineRF->getPositions().size() - 1 <= time) removeRF = false;
    }

    auto funcRemoveRF = [&](vec3 pos, size_t idx) {
        if (!removeRF) return pos;

        vec3 origin = lineRF->getPositions()[idx];
        vec3 baseX = glm::normalize(lineRF->getMetaData<dvec3>("baseX")[idx]);
        vec3 baseY = glm::normalize(lineRF->getMetaData<dvec3>("baseY")[idx]);

        vec3 posVec = pos - origin;
        vec3 posRF = {glm::dot(baseX, posVec), glm::dot(baseY, posVec), 0};
        return posRF;
    };

    size_t lineIdx = 0;
    for (auto& line : *data) {
        util::OnScopeExit incIdx([&lineIdx]() { lineIdx++; });
        auto size = line.getPositions().size();

        // std::cout << fmt::format("line {}, length {}", line.getIndex(),
        // line.getPositions().size())
        //           << std::endl;

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

        auto coloring = [&, this](auto sampleMD, size_t lineIndex,
                                  size_t lineNumber) -> std::pair<vec4, double> {
            if (constantColor || this->isSelected(line, lineIdx)) {
                return {selectedColor_.get(), 1};
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
                return {colors->at(index), 1.0};
            } else {
                auto& mdValue = sampleMD;
                double md = detail::norm(mdValue);
                minMetaData = std::min(minMetaData, md);
                maxMetaData = std::max(maxMetaData, md);

                md -= mdProp->scaleBy_.get().x;
                md /= mdProp->scaleBy_.get().y - mdProp->scaleBy_.get().x;
                if (mdProp->loopTF_) {
                    md -= std::floor(md);
                }

                return {mdProp->tf_.get().sample(md), md};
            }
        };

        auto lineLoop = [&, this](const IntegralLine& line, auto mdContainter) {
            auto velocity = line.getMetaData<dvec3>("velocity");
            auto& vertices = line.getPositions();

            // Add a point in time if possible.
            if (size_t(time) + 1 < vertices.size() - 1) {
                vec3 pos = (1.0 - timeFrac) * line.getPositions()[(size_t)time] +
                           timeFrac * line.getPositions()[(size_t)time + 1];
                auto color = coloring(mdContainter[time], line.getIndex(), lineIdx);
                vec3 propSample = util::glm_convert<vec3>(mdContainter[(size_t)time]);

                indexBuffer->add(static_cast<std::uint32_t>(lineVertices.size()));
                lineVertices.push_back({funcRemoveRF(pos, size_t(time)), velocity[(size_t)time],
                                        propSample, color.first});

                pointIndex->add(static_cast<std::uint32_t>(pointVertices.size()));
                // color.a = 1.0;
                pointVertices.push_back({funcRemoveRF(pos, size_t(time)),
                                         sphereScale_.get() * color.second, color.first});
            }

            for (int idx = std::min(vertices.size() - 1, size_t(time)); idx >= 0; --idx) {
                double strength = (double(idx) - time + cometLength) / cometLength;
                // if (line.getIndex() == 0) {
                //     std::cout << fmt::format("Pos {}, strength {} (length {})", idx, strength,
                //                              cometLength)
                //               << std::endl;
                // }
                strength = std::min(strength, 1.0);
                strength = std::max(strength, 0.0);
                vec3 pos = line.getPositions()[idx];
                vec3 vel = velocity[idx];
                auto color = coloring(mdContainter[idx], line.getIndex(), lineIdx);
                color.first *= strength;
                // vec4 colorOp = color.first;
                // colorOp.a *= strength;

                indexBuffer->add(static_cast<std::uint32_t>(lineVertices.size()));
                // if (line.getIndex() == 2)
                // if (strength > 0.95)
                //     std::cout << "- Added point " << lineVertices.size() << std::endl;

                vec3 propSample = util::glm_convert<vec3>(mdContainter[idx]);
                lineVertices.push_back({funcRemoveRF(pos, idx), vel, propSample, color.first});
                // if (line.getIndex() == 2)
                //     std::cout << "-   new position" << get<0>(lineVertices.back()) << " - "
                // << strength << std::endl;
                if (strength <= 0) break;
            }
        };

        auto ribbonLoop = [&coloring, &lineVertices, &indexBuffer, &lineIdx, this](
                              const IntegralLine& line, auto mdContainter) {
            for (auto&& sample : util::zip(line.getPositions(), line.getMetaData<dvec3>("velocity"),
                                           mdContainter, line.getMetaData<dvec3>("vorticity"))) {
                vec3 pos = get<0>(sample);
                vec3 vel = get<1>(sample);
                vec3 vor = get<3>(sample);

                auto color = coloring(vor, line.getIndex(), lineIdx);

                auto N = glm::normalize(glm::cross(vor, vel));

                auto off = glm::normalize(vor) * (ribbonWidth_.get() / 2.0f);
                // if (scaleWidth_.get()) off *=
                auto pos1 = pos - off;
                auto pos2 = pos + off;
                vec3 propSample = util::glm_convert<vec3>(get<2>(sample));
                indexBuffer->add(static_cast<std::uint32_t>(lineVertices.size()));
                lineVertices.push_back({pos1, vel, propSample, color.first});
                indexBuffer->add(static_cast<std::uint32_t>(lineVertices.size()));
                lineVertices.push_back({pos2, vel, propSample, color.first});
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

    // Remove reference frame.
    // if (removeReferenceFrame_.get() && referenceFrameLine_.hasData()) {
    //     for (auto& point : lineVertices) {
    //         vec3& pos = get<0>(point);
    //         pos -=
    //     }
    // }

    mesh->addVertices(lineVertices);
    mesh_.setData(mesh);

    pointMesh->addVertices(pointVertices);
    pointMesh_.setData(pointMesh);

    if (mdProp) {
        mdProp->minValue_.set(minMetaData);
        mdProp->maxValue_.set(maxMetaData);
    }

    // Reference frame arrow properties for rendering.
    if (!referenceFrameLine_.hasData()) return;

    auto lines = referenceFrameLine_.getData();
    if (lines->size() != 1) return;

    auto referenceLine = lines->at(0);
    if (referenceLine.getPositions().size() - 1 <= time) return;

    vec3 origin = referenceLine.getPositions()[int(time)] * (1.0 - timeFrac) +
                  referenceLine.getPositions()[int(time) + 1] * timeFrac;

    vec3 baseX = referenceLine.getMetaData<dvec3>("baseX")[int(time)] * (1.0 - timeFrac) +
                 referenceLine.getMetaData<dvec3>("baseX")[int(time) + 1] * timeFrac;

    vec3 baseY = referenceLine.getMetaData<dvec3>("baseY")[int(time)] * (1.0 - timeFrac) +
                 referenceLine.getMetaData<dvec3>("baseY")[int(time) + 1] * timeFrac;
    origin.z = 0;
    baseX.z = 0;
    baseY.z = 0;

    origin_.setReadOnly(false);
    endX_.setReadOnly(false);
    endY_.setReadOnly(false);

    origin_.set(origin);
    endX_.set(origin + glm::normalize(baseX) * arrowScale_.get());
    endY_.set(origin + glm::normalize(baseY) * arrowScale_.get());
    // origin_.set({0, 0, 0});
    // endX_.set(baseX);
    // endY_.set(baseY);

    origin_.setReadOnly(true);
    endX_.setReadOnly(true);
    endY_.setReadOnly(true);
}

}  // namespace inviwo
