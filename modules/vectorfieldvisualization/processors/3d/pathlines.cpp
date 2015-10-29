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
#include <inviwo/core/util/volumevectorsampler.h>

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
    , linesStripsMesh_("linesStripsMesh_")

    , stepDirection_("stepDirection", "Step Direction")
    , integrationScheme_("integrationScheme", "Integration Scheme")
    , seedPointsSpace_("seedPointsSpace", "Seed Points Space")

    , tf_("transferFunction", "Transfer Function")
    , velocityScale_("velocityScale_", "Velocity Scale (inverse)", 1, 0, 10)
    , maxVelocity_("minMaxVelocity", "Velocity Range", "0", InvalidationLevel::Valid)

{

    addPort(volume_);
    addPort(seedPoints_);
    addPort(linesStripsMesh_);

    stepDirection_.addOption("fwd", "Forward", IntegralLineTracer::Direction::FWD);
    stepDirection_.addOption("bwd", "Backwards", IntegralLineTracer::Direction::BWD);
    stepDirection_.addOption("bi", "Bi Directional", IntegralLineTracer::Direction::BOTH);

    integrationScheme_.addOption("euler", "Euler", IntegralLineTracer::IntegrationScheme::Euler);
    integrationScheme_.addOption("rk4", "Runge-Kutta (RK4)", IntegralLineTracer::IntegrationScheme::RK4);
    integrationScheme_.setSelectedValue(IntegralLineTracer::IntegrationScheme::RK4);

    seedPointsSpace_.addOption("texture", "Texture",
        StructuredCoordinateTransformer<3>::Space::Texture);
    seedPointsSpace_.addOption("model", "Model", StructuredCoordinateTransformer<3>::Space::Model);
    seedPointsSpace_.addOption("world", "World", StructuredCoordinateTransformer<3>::Space::World);
    seedPointsSpace_.addOption("data", "Data", StructuredCoordinateTransformer<3>::Space::Data);
    seedPointsSpace_.addOption("index", "Index", StructuredCoordinateTransformer<3>::Space::Index);

    maxVelocity_.setReadOnly(true);

    addProperty(stepDirection_);
    addProperty(integrationScheme_);
    addProperty(seedPointsSpace_);

    addProperty(tf_);
    addProperty(velocityScale_);
    addProperty(maxVelocity_);

    tf_.get().clearPoints();
    tf_.get().addPoint(vec2(0, 1), vec4(0, 0, 1, 1));
    tf_.get().addPoint(vec2(0.5, 1), vec4(1, 1, 0, 1));
    tf_.get().addPoint(vec2(1, 1), vec4(1, 0, 0, 1));

    setAllPropertiesCurrentStateAsDefault();
}
    
void PathLines::process() {
    auto data = volume_.getData();
    if (data->size() == 0) {
        return;
    }
    
    auto firstVol = data->at(0);

    auto mesh = util::make_unique<BasicMesh>();
    mesh->setModelMatrix(firstVol->getModelMatrix());
    mesh->setWorldMatrix(firstVol->getWorldMatrix());

    auto m = firstVol->getCoordinateTransformer().getMatrix(
        seedPointsSpace_.get(), StructuredCoordinateTransformer<3>::Space::Texture);

    ImageSampler tf(tf_.get().getData());

    float maxVelocity = 0;
    PathLineTracer tracer(data, integrationScheme_.get());

    float startT_ = 0;
    int numberOfSteps_ = 100;
    double dt_ = 0.001;
   

    std::vector<BasicMesh::Vertex> vertices;
    for (const auto &seeds : seedPoints_) {
        for (auto &p : (*seeds)) {
            vec4 P = m * vec4(p, 1.0f);
            auto indexBuffer =
                mesh->addIndexBuffer(DrawType::LINES, ConnectivityType::STRIP_ADJACENCY);
            auto line = tracer.traceFrom(vec4(P.xyz(), startT_), numberOfSteps_, dt_,stepDirection_.get());

            auto position = line.getPositions().begin();
            auto velocity = line.getMetaData("velocity").begin();

            auto size = line.getPositions().size();
            if (size == 0) continue;

            indexBuffer->add(0);


            for (size_t i = 0; i < size; i++) {
                vec3 pos(*position);
                vec3 v(*velocity);

                float l = glm::length(vec3(*velocity));
                float d = glm::clamp(l / velocityScale_.get(), 0.0f, 1.0f);
                maxVelocity = std::max(maxVelocity, l);
                auto c = vec4(tf.sample(dvec2(d, 0.0)));

                indexBuffer->add(static_cast<std::uint32_t>(vertices.size()));

                vertices.push_back({ pos,glm::normalize(v),pos,c });

                position++;
                velocity++;
            }
            indexBuffer->add(vertices.size() - 1);
        }
    }

    mesh->addVertices(vertices);

    linesStripsMesh_.setData(mesh.release());
    maxVelocity_.set(toString(maxVelocity));



}

} // namespace

