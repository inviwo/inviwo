/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015 Inviwo Foundation
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
        "Undefined",                // Category
        CodeState::Experimental,  // Code state
        Tags::CPU,               // Tags
    };

    const ProcessorInfo PathLines::getProcessorInfo() const {
        return processorInfo_;
    }


PathLines::PathLines()
    : Processor()
    , volume_("vectorvolume")
    , seedPoints_("seedpoints")
    , colors_("colors")
    , linesStripsMesh_("linesStripsMesh_")

    , pathLineProperties_("pathLineProperties", "Path Line Properties")



    , coloringMethod_("coloringMethod","Color by")
    , tf_("transferFunction", "Transfer Function")
    , velocityScale_("velocityScale_", "Velocity Scale (inverse)", 1, 0, 10)
    , maxVelocity_("minMaxVelocity", "Velocity Range", "0", InvalidationLevel::Valid)

{

    colors_.setOptional(true);

    addPort(volume_);
    addPort(seedPoints_);
    addPort(colors_);
    addPort(linesStripsMesh_);

    addProperty(pathLineProperties_);

    maxVelocity_.setReadOnly(true);

    addProperty(tf_);
    addProperty(coloringMethod_);
    addProperty(velocityScale_);
    addProperty(maxVelocity_);

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
    auto data = volume_.getData();
    if (data->size() == 0) {
        return;
    }
    
    auto firstVol = data->at(0);

    auto mesh = std::make_shared<BasicMesh>();
    mesh->setModelMatrix(firstVol->getModelMatrix());
    mesh->setWorldMatrix(firstVol->getWorldMatrix());

    auto m = pathLineProperties_.getSeedPointTransformationMatrix(firstVol->getCoordinateTransformer());

    ImageSampler tf(tf_.get().getData());

    float maxVelocity = 0;
    PathLineTracer tracer(data, pathLineProperties_);
   
    size_t i = 0;
    bool hasColors = colors_.hasData();

    bool warnOnce = true;
    bool warnOnce2 = true;

    std::vector<BasicMesh::Vertex> vertices;
    for (const auto &seeds : seedPoints_) {
        for (auto &p : (*seeds)) {
            vec4 P = m * vec4(p, 1.0f);
            auto line = tracer.traceFrom(vec4(P.xyz(), pathLineProperties_.getStartT()));

            auto position = line.getPositions().begin();
            auto velocity = line.getMetaData("velocity").begin();
            auto timestamp = line.getMetaData("timestamp").begin();

            auto size = line.getPositions().size();
            if (size == 0) continue;


            auto indexBuffer =
                mesh->addIndexBuffer(DrawType::Lines, ConnectivityType::StripAdjacency);
            indexBuffer->add(0);

            vec4 c;
            if (hasColors) {
                if (i >= colors_.getData()->size()) {
                    if (warnOnce2) {
                        warnOnce2 = false;
                        LogWarn("The vector of colors is smaller then the vector of seed points");
                    }
                }
                else {
                    c = colors_.getData()->at(i);
                }
            }
            i++;

            for (size_t ii = 0; ii < size; ii++) {
                vec3 pos(*position);
                vec3 v(*velocity);
                float t =  static_cast<float>((*timestamp).x);

                float l = glm::length(v);
                float d = glm::clamp(l / velocityScale_.get(), 0.0f, 1.0f);
                maxVelocity = std::max(maxVelocity, l);

                switch (coloringMethod_.get())
                {
                case ColoringMethod::Timestamp:
                    c = vec4(tf.sample(dvec2(t, 0.0)));
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
                    c = vec4(tf.sample(dvec2(d, 0.0)));
                default:
                    break;
                }

                indexBuffer->add(static_cast<std::uint32_t>(vertices.size()));

                vertices.push_back({ pos,glm::normalize(v),pos,c });

                position++;
                velocity++;
                timestamp++;
            }
            indexBuffer->add(static_cast<std::uint32_t>(vertices.size()-1));
        }
    }

    mesh->addVertices(vertices);

    linesStripsMesh_.setData(mesh);
    maxVelocity_.set(toString(maxVelocity));

}

void PathLines::deserialize(Deserializer& d) {
    DoubleProperty dProperty("stepSize", "Step size", 0.001f, 0.001f, 1.0f, 0.001f);
    util::renameProperty(d, { { &dProperty, "dt" } });
    util::changePropertyType(d, { {&dProperty,  FloatProperty::CLASS_IDENTIFIER } });
    Processor::deserialize(d);
}

} // namespace

