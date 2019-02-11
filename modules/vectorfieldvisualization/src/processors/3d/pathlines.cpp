/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2019 Inviwo Foundation
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

#include <modules/vectorfieldvisualization/processors/3d/pathlines.h>
#include <inviwo/core/util/volumesequencesampler.h>
#include <inviwo/core/datastructures/geometry/basicmesh.h>
#include <inviwo/core/util/imagesampler.h>
#include <inviwo/core/io/serialization/versionconverter.h>
#include <modules/vectorfieldvisualization/algorithms/integrallineoperations.h>
#include <inviwo/core/util/zip.h>
#include <modules/vectorfieldvisualization/integrallinetracer.h>

namespace inviwo {

const ProcessorInfo PathLinesDeprecated::processorInfo_{
    "org.inviwo.PathLinesDeprecated",  // Class identifier
    "Path Lines (Deprecated)",         // Display name
    "Vector Field Visualization",      // Category
    CodeState::Deprecated,             // Code state
    Tags::CPU,                         // Tags
};

const ProcessorInfo PathLinesDeprecated::getProcessorInfo() const { return processorInfo_; }

PathLinesDeprecated::PathLinesDeprecated()
    : Processor()
    , sampler_("sampler")
    , seedPoints_("seedpoints")
    , colors_("colors")
    , volume_("vectorvolume")
    , lines_("lines")
    , linesStripsMesh_("linesStripsMesh")
    , pathLineProperties_("pathLineProperties", "Path Line Properties")
    , tf_("transferFunction", "Transfer Function",
          TransferFunction{
              {{0.0f, vec4(0, 0, 1, 1)}, {0.5f, vec4(1, 1, 0, 1)}, {1.0, vec4(1, 0, 0, 1)}}})
    , coloringMethod_("coloringMethod", "Color by",
                      {{"vel", "Velocity", ColoringMethod::Velocity},
                       {"time", "Timestamp", ColoringMethod::Timestamp},
                       {"port", "Colors in port", ColoringMethod::ColorPort}})
    , velocityScale_("velocityScale_", "Velocity Scale (inverse)", 1, 0, 10)
    , maxVelocity_("minMaxVelocity", "Velocity Range", "0", InvalidationLevel::Valid)
    , allowLooping_("allowLooping", "Allow looping", true) {

    isReady_.setUpdate([this]() {
        if (allInportsAreReady()) {
            return true;
        }

        if (seedPoints_.isConnected() && !seedPoints_.isReady()) return false;
        if (colors_.isConnected() && !colors_.isReady()) return false;

        if (sampler_.isConnected()) {
            return sampler_.isReady();
        }
        if (volume_.isConnected()) {
            return volume_.isReady();
        }
        return false;
    });

    colors_.setOptional(true);

    addPort(sampler_);
    addPort(seedPoints_);
    addPort(colors_);
    addPort(volume_);
    addPort(lines_);
    addPort(linesStripsMesh_);

    maxVelocity_.setReadOnly(true);
    allowLooping_.setVisible(false);

    addProperty(pathLineProperties_);
    addProperty(tf_);
    addProperty(coloringMethod_);
    addProperty(velocityScale_);
    addProperty(maxVelocity_);
    addProperty(allowLooping_);

    LogWarn(
        "This Path Lines Processor is Deprecated, use the new Path Lines processor together "
        "with the IntegralLineToMesh processor.");
}

void PathLinesDeprecated::process() {
    auto sampler = [&]() -> std::shared_ptr<const Spatial4DSampler<3, double>> {
        if (sampler_.isConnected()) {
            if (allowLooping_.getVisible()) {
                allowLooping_.setVisible(false);
            }
            return sampler_.getData();
        } else {
            if (!allowLooping_.getVisible()) {
                allowLooping_.setVisible(true);
            }
            auto s = std::make_shared<VolumeSequenceSampler>(volume_.getData());
            s->setAllowedLooping(allowLooping_.get());
            return s;
        }
    }();

    if (!sampler) return;

    auto mesh = std::make_shared<BasicMesh>();
    mesh->setModelMatrix(sampler->getModelMatrix());
    mesh->setWorldMatrix(sampler->getWorldMatrix());

    auto m =
        pathLineProperties_.getSeedPointTransformationMatrix(sampler->getCoordinateTransformer());

    float maxVelocity = 0;
    PathLine3DTracer tracer(sampler, pathLineProperties_);

    bool hasColors = colors_.hasData();

    bool warnOnce = true;
    bool warnOnce2 = true;

    auto lines = std::make_shared<IntegralLineSet>(sampler->getModelMatrix());
    std::vector<BasicMesh::Vertex> vertices;
    size_t startID = 0;
    for (const auto &seeds : seedPoints_) {
#pragma omp parallel for
        for (long long j = 0; j < static_cast<long long>(seeds->size()); j++) {
            const auto &p = (*seeds)[j];
            vec4 P = m * vec4(p, 1.0f);
            IntegralLine line = tracer.traceFrom(vec4(vec3(P), pathLineProperties_.getStartT()));
            auto size = line.getPositions().size();
            if (size > 1) {
#pragma omp critical
                // lines->push_back(line, startID + j);
                lines->push_back(line, lines->size());
            };
        }
        startID += seeds->size();
    }

    for (auto &line : *lines) {
        auto size = line.getPositions().size();
        if (size <= 1) continue;

        auto position = line.getPositions().begin();
        auto velocity = line.getMetaData<dvec3>("velocity").begin();
        auto timestamp = line.getMetaData<double>("timestamp").begin();

        auto indexBuffer = mesh->addIndexBuffer(DrawType::Lines, ConnectivityType::StripAdjacency);
        indexBuffer->add(0);

        vec4 c{0};
        if (hasColors) {
            if (line.getIndex() >= colors_.getData()->size()) {
                if (warnOnce2) {
                    warnOnce2 = false;
                    LogWarn("The vector of colors is smaller then the vector of seed points");
                }
            } else {
                c = colors_.getData()->at(line.getIndex());
            }
        }

        for (size_t ii = 0; ii < size; ii++) {
            vec3 pos(*position);
            vec3 v(*velocity);
            float t = static_cast<float>(*timestamp);

            float l = glm::length(v);
            float d = glm::clamp(l / velocityScale_.get(), 0.0f, 1.0f);
            maxVelocity = std::max(maxVelocity, l);

            switch (coloringMethod_.get()) {
                case ColoringMethod::Timestamp:
                    c = tf_.get().sample(t);
                    break;
                case ColoringMethod::ColorPort:
                    if (hasColors) {
                        break;
                    } else {
                        if (warnOnce) {
                            warnOnce = false;
                            LogWarn(
                                "No colors in the color port, using velocity for coloring "
                                "instead ");
                        }
                        [[fallthrough]];
                    }
                default:
                    [[fallthrough]];
                case ColoringMethod::Velocity:
                    c = tf_.get().sample(d);
                    break;
            }

            indexBuffer->add(static_cast<std::uint32_t>(vertices.size()));

            vertices.push_back({pos, glm::normalize(v), pos, c});

            position++;
            velocity++;
            timestamp++;
        }
        indexBuffer->add(static_cast<std::uint32_t>(vertices.size() - 1));
    }

    mesh->addVertices(vertices);

    linesStripsMesh_.setData(mesh);

    util::curvature(*lines);
    util::tortuosity(*lines);

    lines_.setData(lines);
    maxVelocity_.set(toString(maxVelocity));
}

void PathLinesDeprecated::deserialize(Deserializer &d) {
    DoubleProperty dProperty("stepSize", "Step size", 0.001f, 0.001f, 1.0f, 0.001f);
    util::renameProperty(d, {{&dProperty, "dt"}});
    util::changePropertyType(d, {{&dProperty, PropertyTraits<FloatProperty>::classIdentifier()}});
    Processor::deserialize(d);
}

}  // namespace inviwo
