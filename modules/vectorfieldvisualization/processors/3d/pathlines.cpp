/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2017 Inviwo Foundation
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

#include "pathlines.h"
#include <inviwo/core/util/volumesequencesampler.h>
#include <inviwo/core/datastructures/geometry/basicmesh.h>
#include <inviwo/core/util/imagesampler.h>
#include <inviwo/core/io/serialization/versionconverter.h>

namespace inviwo {

    const ProcessorInfo PathLines::processorInfo_{
        "org.inviwo.PathLines",  // Class identifier
        "Path Lines",            // Display name
        "Vector Field Visualization",                // Category
        CodeState::Experimental,  // Code state
        Tags::CPU,               // Tags
    };

    const ProcessorInfo PathLines::getProcessorInfo() const {
        return processorInfo_;
    }


PathLines::PathLines()
    : Processor()
    , sampler_("sampler")
    , seedPoints_("seedpoints")
    , colors_("colors")
    , volume_("vectorvolume")

    , linesStripsMesh_("linesStripsMesh_")
    , lines_("lines")

    , pathLineProperties_("pathLineProperties", "Path Line Properties")



    , coloringMethod_("coloringMethod","Color by")
    , tf_("transferFunction", "Transfer Function")
    , velocityScale_("velocityScale_", "Velocity Scale (inverse)", 1, 0, 10)
    , maxVelocity_("minMaxVelocity", "Velocity Range", "0", InvalidationLevel::Valid)

    , allowLooping_("allowLooping","Allow looping",true)

{

    colors_.setOptional(true);

    addPort(sampler_);
    addPort(seedPoints_);
    addPort(colors_);
    addPort(volume_);
    addPort(lines_);
    addPort(linesStripsMesh_);

    addProperty(pathLineProperties_);

    maxVelocity_.setReadOnly(true);

    addProperty(tf_);
    addProperty(coloringMethod_);
    addProperty(velocityScale_);
    addProperty(maxVelocity_);

    addProperty(allowLooping_);
    allowLooping_.setVisible(false);

    tf_.get().clearPoints();
    tf_.get().addPoint(vec2(0, 1), vec4(0, 0, 1, 1));
    tf_.get().addPoint(vec2(0.5, 1), vec4(1, 1, 0, 1));
    tf_.get().addPoint(vec2(1, 1), vec4(1, 0, 0, 1));

    coloringMethod_.addOption("vel", "Velocity", ColoringMethod::Velocity);
    coloringMethod_.addOption("time", "Timestamp", ColoringMethod::Timestamp);
    coloringMethod_.addOption("port", "Colors in port", ColoringMethod::ColorPort);

    setAllPropertiesCurrentStateAsDefault();
}
    
void PathLines::process() {
    auto sampler = [&]() -> std::shared_ptr<const Spatial4DSampler<3, double> > {
        if (sampler_.isConnected()) {
            if (allowLooping_.getVisible()) {
                allowLooping_.setVisible(false);
            }
            return sampler_.getData();
        }
        else {
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

    auto m = pathLineProperties_.getSeedPointTransformationMatrix(sampler->getCoordinateTransformer());



    float maxVelocity = 0;
    PathLineTracer tracer(sampler, pathLineProperties_);
   
    bool hasColors = colors_.hasData();

    bool warnOnce = true;
    bool warnOnce2 = true;

    auto lines = std::make_shared<IntegralLineSet>(sampler->getModelMatrix());
    std::vector<BasicMesh::Vertex> vertices;
    size_t startID = 0;
    for (const auto &seeds : seedPoints_) {
#pragma omp parallel for
        for (long long j = 0; j < static_cast<long long>(seeds->size());j++){
            const auto &p = (*seeds)[j];
            vec4 P = m * vec4(p, 1.0f);
            auto line = tracer.traceFrom(vec4(P.xyz(), pathLineProperties_.getStartT()));
            auto size = line.getPositions().size();
            if (size>1) {  
                #pragma omp critical
                //lines->push_back(line, startID + j);
                lines->push_back(line, lines->size());
            };
        }
        startID += seeds->size();
    }

    for (auto &line : *lines) {
        auto size = line.getPositions().size();
        if (size <= 1) continue;

        auto position = line.getPositions().begin();
        auto velocity = line.getMetaData("velocity").begin();
        auto timestamp = line.getMetaData("timestamp").begin();

        auto indexBuffer =
            mesh->addIndexBuffer(DrawType::Lines, ConnectivityType::StripAdjacency);
        indexBuffer->add(0);

        vec4 c;
        if (hasColors) {
            if (line.getIndex() >= colors_.getData()->size()) {
                if (warnOnce2) {
                    warnOnce2 = false;
                    LogWarn("The vector of colors is smaller then the vector of seed points");
                }
            }
            else {
                c = colors_.getData()->at(line.getIndex());
            }
        }

        for (size_t ii = 0; ii < size; ii++) {
            vec3 pos(*position);
            vec3 v(*velocity);
            float t = static_cast<float>((*timestamp).x);

            float l = glm::length(v);
            float d = glm::clamp(l / velocityScale_.get(), 0.0f, 1.0f);
            maxVelocity = std::max(maxVelocity, l);

            switch (coloringMethod_.get())
            {
            case ColoringMethod::Timestamp:
                c = tf_.get().sample(t);
                break;
            case ColoringMethod::ColorPort:
                if (hasColors) {
                    break;
                }
                else {
                    if (warnOnce) {
                        warnOnce = false;
                        LogWarn("No colors in the color port, using velocity for coloring instead ");
                    }
                }
            case ColoringMethod::Velocity:
                c = tf_.get().sample(d);
            default:
                break;
            }

            indexBuffer->add(static_cast<std::uint32_t>(vertices.size()));

            vertices.push_back({ pos,glm::normalize(v),pos,c });

            position++;
            velocity++;
            timestamp++;
        }
        indexBuffer->add(static_cast<std::uint32_t>(vertices.size() - 1));
    }

    mesh->addVertices(vertices);

    linesStripsMesh_.setData(mesh);
    lines_.setData(lines);
    maxVelocity_.set(toString(maxVelocity));

}

void PathLines::deserialize(Deserializer& d) {
    DoubleProperty dProperty("stepSize", "Step size", 0.001f, 0.001f, 1.0f, 0.001f);
    util::renameProperty(d, { { &dProperty, "dt" } });
    util::changePropertyType(d, { {&dProperty,  FloatProperty::CLASS_IDENTIFIER } });
    Processor::deserialize(d);
}

} // namespace

