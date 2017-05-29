/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2017 Inviwo Foundation
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

namespace inviwo {
namespace {
struct MetaDataSampler {
    MetaDataSampler(const IntegralLine &line, std::string name)
        : hasMetaData_(line.hasMetaData(name)), v(0), dv(0) {
        if (hasMetaData_) {
            it = line.getMetaData(name).begin();
            v = *it;
        }
    }

    operator dvec3 &() { return v; }
    operator const dvec3 &() const { return v; }

    dvec3 operator++() {
        if (hasMetaData_) {
            it++;
            v = *it;
        } else {
            v += dv;
        }
        return v;
    }
    dvec3 operator++(int) {
        auto prev = v;
        operator++();
        return prev;
    }

protected:
    std::vector<dvec3>::const_iterator it;
    bool hasMetaData_;
    dvec3 v;
    dvec3 dv;
};

struct Timestep : public MetaDataSampler {
    Timestep(const IntegralLine &line) : MetaDataSampler(line, "timestamp") {
        hasMetaData_ = line.hasMetaData("timestamp");
        if (!hasMetaData_) {
            dv = dvec3(1.0 / (line.getPositions().size() - 1));
        }
    }

    operator double &() { return v.x; }
    operator const double &() const { return v.x; }
};
}  // namespace

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo IntegralLineVectorToMesh::processorInfo_{
    "org.inviwo.IntegralLineVectorToMesh",  // Class identifier
    "Integral Line Vector To Mesh",         // Display name
    "Vector Field Visualization",           // Category
    CodeState::Experimental,                // Code state
    Tags::None,                             // Tags
};
const ProcessorInfo IntegralLineVectorToMesh::getProcessorInfo() const { return processorInfo_; }

IntegralLineVectorToMesh::IntegralLineVectorToMesh()
    : Processor()
    , lines_("lines")
    , brushingList_("brushingList")
    , colors_("colors")
    , mesh_("mesh")
    , ignoreBrushingList_("ignoreBrushingList", "Ignore Brushing List", false)

    , stride_("stride", "Vertex stride", 1, 1, 10)

    , timeBasedFiltering_("timeBasedFiltering", "Time Based Filtering", false)
    , minMaxT_("minMaxT", "Min/Max Timestep", -1, 1, -10, 10)
    , setFromData_("setFromData", "Set from data")

    , tf_("transferFunction", "Transfer Function")
    , coloringMethod_("coloringMethod", "Color by")
    , velocityScale_("velocityScale_", "Velocity Scale (inverse)", 1, 0, 10)
    , maxVelocity_("minMaxVelocity", "Velocity Range", "0", InvalidationLevel::Valid)
    , curvatureScale_("curvatureScale", "Curvature Scale (inverse)", 1, 0, 10)
    , maxCurvature_("maxCurvature", "Curvature Range", "0", InvalidationLevel::Valid)

{
    colors_.setOptional(true);

    addPort(lines_);
    addPort(colors_);
    addPort(brushingList_);
    addPort(mesh_);

    addProperty(ignoreBrushingList_);

    addProperty(tf_);
    addProperty(coloringMethod_);
    addProperty(velocityScale_);
    addProperty(maxVelocity_);

    addProperty(curvatureScale_);
    addProperty(maxCurvature_);

    addProperty(stride_);

    addProperty(timeBasedFiltering_);
    timeBasedFiltering_.addProperty(minMaxT_);
    timeBasedFiltering_.addProperty(setFromData_);

    coloringMethod_.addOption("vel", "Velocity", ColoringMethod::Velocity);
    coloringMethod_.addOption("time", "Timestamp", ColoringMethod::Timestamp);
    coloringMethod_.addOption("port", "Colors in port", ColoringMethod::ColorPort);
    coloringMethod_.addOption("curvature", "Curvature", ColoringMethod::Curvature);

    tf_.autoLinkToProperty<PathLines>("transferFunction");
    tf_.autoLinkToProperty<StreamLines>("transferFunction");

    tf_.get().clearPoints();
    tf_.get().addPoint(vec2(0, 1), vec4(0, 0, 1, 1));
    tf_.get().addPoint(vec2(0.5, 1), vec4(1, 1, 0, 1));
    tf_.get().addPoint(vec2(1, 1), vec4(1, 0, 0, 1));

    setAllPropertiesCurrentStateAsDefault();

    setFromData_.onChange([&]() {
        if (lines_.hasData()) {
            float minT = std::numeric_limits<float>::max();
            float maxT = std::numeric_limits<float>::lowest();

            for (auto &line : (*lines_.getData())) {
                auto size = line.getPositions().size();
                if (size == 0) continue;

                if (!ignoreBrushingList_.get() && brushingList_.isFiltered(line.getIndex())) {
                    continue;
                }

                Timestep t(line);
                for (size_t ii = 0; ii < size; ii++) {
                    float tt = (t++).x;
                    minT = std::min(minT, tt);
                    maxT = std::max(tt, maxT);
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
    auto mesh = std::make_shared<BasicMesh>();

    mesh->setModelMatrix(lines_.getData()->getModelMatrix());

    std::vector<BasicMesh::Vertex> vertices;
    float maxVelocity = 0;
    float maxCurvature = 0;

    vertices.reserve(lines_.getData()->size() * 2000);

    bool hasColors = colors_.hasData();

    auto coloringMethod = coloringMethod_.get();
    if (coloringMethod == ColoringMethod::ColorPort) {
        if (!hasColors) {
            LogWarn("No colors in the color port, using velocity for coloring instead ");
            coloringMethod = ColoringMethod::Velocity;
        }
    } else if (coloringMethod == ColoringMethod::Velocity) {
    }

    bool warnOnce = true;

    size_t idx = 0;
    for (auto &line : (*lines_.getData())) {
        auto size = line.getPositions().size();
        if (size == 0) continue;

        if (!ignoreBrushingList_.get() && brushingList_.isFiltered(line.getIndex())) {
            idx++;
            continue;
        }

        auto position = line.getPositions().begin();
        auto velocity = line.getMetaData("velocity").begin();
        Timestep t(line);
        MetaDataSampler k(line, "curvature");

        auto indexBuffer = mesh->addIndexBuffer(DrawType::Lines, ConnectivityType::StripAdjacency);

        indexBuffer->getDataContainer().reserve(size + 2);

        vec4 c(1, 1, 1, 1);
        if (hasColors) {
            if (idx >= colors_.getData()->size()) {
                if (warnOnce) {
                    warnOnce = false;
                    LogWarn("The vector of colors is smaller then the vector of seed points");
                }
            } else {
                c = colors_.getData()->at(idx);
                // c = colors_.getData()->at(line.getIndex());
            }
        }
        idx++;
        indexBuffer->add(static_cast<std::uint32_t>(vertices.size()));

        bool first = true;
        for (size_t ii = 0; ii < size - 1; ii++) {
            vec3 pos(*position);
            vec3 v(*velocity);

            position++;
            velocity++;
            float tt = (t++).x;
            float kk = (k++).x;

            if (timeBasedFiltering_.isChecked() &&
                (tt < minMaxT_.get().x || tt > minMaxT_.get().y)) {
                continue;
            }

            if (!first && (!(ii == size - 1 || ii % stride_.get() == 0))) {
                continue;
            }
            first = false;

            float l = glm::length(v);
            maxVelocity = std::max(maxVelocity, l);
            maxCurvature = std::max(maxCurvature, kk);

            switch (coloringMethod) {
                case ColoringMethod::Timestamp:
                    c = tf_.get().sample(t);
                    break;
                case ColoringMethod::Curvature:
                    c = tf_.get().sample(kk / curvatureScale_.get());
                    break;
                case ColoringMethod::ColorPort:
                    break;  // color is set once outside the loop
                case ColoringMethod::Velocity:
                    c = tf_.get().sample(l / velocityScale_.get());
                default:
                    break;
            }

            indexBuffer->add(static_cast<std::uint32_t>(vertices.size()));

            vertices.push_back({pos, glm::normalize(v), pos, c});
        }
        indexBuffer->add(static_cast<std::uint32_t>(vertices.size() - 1));
    }
    mesh->addVertices(vertices);

    mesh_.setData(mesh);
    maxVelocity_.set(toString(maxVelocity));
    maxCurvature_.set(toString(maxCurvature));
}

}  // namespace inviwo
