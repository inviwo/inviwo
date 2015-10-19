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

#include "streamribbons.h"
#include <inviwo/core/datastructures/geometry/basicmesh.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>
#include <inviwo/core/util/imagesampler.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
ProcessorClassIdentifier(StreamRibbons, "org.inviwo.StreamRibbons")
ProcessorDisplayName(StreamRibbons, "Stream Ribbons") 
ProcessorTags(StreamRibbons, Tags::CPU);
ProcessorCategory(StreamRibbons, "Vector Field Visualization");
ProcessorCodeState(StreamRibbons, CODE_STATE_EXPERIMENTAL);

StreamRibbons::StreamRibbons()
    : Processor()
    , vectorVolume_("vectorVolume")
    , vorticityVolume_("vorticityVolume")
    , seedPoints_("seedpoints")
    , mesh_("mesh")
    , numberOfSteps_("steps", "Number of Steps", 100, 1, 1000)
    , normalizeSamples_("normalizeSamples", "Normalize Samples", true)
    , stepSize_("stepSize", "StepSize", 0.001f, 0.0001f, 1.0f)
    , stepDirection_("stepDirection", "Step Direction", INVALID_RESOURCES)
    , integrationScheme_("integrationScheme", "Integration Scheme")
    , ribbonWidth_("ribbonWidth", "Ribbon Width", 0.1f, 0.00001f)
    , seedPointsSpace_("seedPointsSpace", "Seed Points Space")
    , tf_("transferFunction", "Transfer Function")
    , velocityScale_("velocityScale_", "Velocity Scale (inverse)", 1, 0, 10)
    , maxVelocity_("minMaxVelocity", "Max Velocity", "0", VALID)
    , maxVorticity_("maxVorticity", "Max Vorticity", "0", VALID)

{
    addPort(vectorVolume_);
    addPort(vorticityVolume_);
    addPort(seedPoints_);
    addPort(mesh_);

    stepSize_.setIncrement(0.0001f);
    stepDirection_.addOption("fwd", "Forward", StreamLineTracer::Direction::FWD);
    stepDirection_.addOption("bwd", "Backwards", StreamLineTracer::Direction::BWD);
    stepDirection_.addOption("bi", "Bi Directional", StreamLineTracer::Direction::BOTH);

    integrationScheme_.addOption("euler", "Euler", StreamLineTracer::IntegrationScheme::Euler);
    integrationScheme_.addOption("rk4", "Runge-Kutta (RK4)", StreamLineTracer::IntegrationScheme::RK4);
    integrationScheme_.setSelectedValue(StreamLineTracer::IntegrationScheme::RK4);

    seedPointsSpace_.addOption("texture", "Texture",
                               StructuredCoordinateTransformer<3>::Space::Texture);
    seedPointsSpace_.addOption("model", "Model", StructuredCoordinateTransformer<3>::Space::Model);
    seedPointsSpace_.addOption("world", "World", StructuredCoordinateTransformer<3>::Space::World);
    seedPointsSpace_.addOption("data", "Data", StructuredCoordinateTransformer<3>::Space::Data);
    seedPointsSpace_.addOption("index", "Index", StructuredCoordinateTransformer<3>::Space::Index);

    addProperty(numberOfSteps_);
    addProperty(stepSize_);
    addProperty(stepDirection_);
    addProperty(integrationScheme_);
    addProperty(normalizeSamples_);
    addProperty(ribbonWidth_);

    addProperty(seedPointsSpace_);

    addProperty(tf_);
    addProperty(velocityScale_);
    addProperty(maxVelocity_);
    addProperty(maxVorticity_);

    maxVelocity_.setReadOnly(true);
    maxVorticity_.setReadOnly(true);

    setAllPropertiesCurrentStateAsDefault();
}

void StreamRibbons::process() {
    auto mesh = std::make_shared<BasicMesh>();
    mesh->setModelMatrix(vectorVolume_.getData()->getModelMatrix());
    mesh->setWorldMatrix(vectorVolume_.getData()->getWorldMatrix());

    auto m = vectorVolume_.getData()->getCoordinateTransformer().getMatrix(
        seedPointsSpace_.get(), StructuredCoordinateTransformer<3>::Space::Texture);
    double maxVelocity = 0;
    double maxVorticity = 0;
    StreamLineTracer tracer(vectorVolume_.getData().get(),integrationScheme_.get());
    ImageSampler tf(tf_.get().getData());
    tracer.addMetaVolume("vorticity", vorticityVolume_.getData()->getRepresentation<VolumeRAM>());
    mat3 invBasis = glm::inverse(vectorVolume_.getData()->getBasis());
    std::vector<BasicMesh::Vertex> vertices;
    for (const auto &seeds : seedPoints_) {
        for (auto &p : (*seeds)) {
            vec4 P = m * vec4(p, 1.0f);
            auto indexBuffer = mesh->addIndexBuffer(DrawType::TRIANGLES, ConnectivityType::STRIP);
            auto line = tracer.traceFrom(P.xyz(), numberOfSteps_.get(), stepSize_.get(),
                                         stepDirection_.get(), normalizeSamples_.get());

            auto position = line.getPositions().begin();
            auto velocity = line.getMetaData("velocity").begin();
            auto vorticity = line.getMetaData("vorticity").begin();

            auto size = line.getPositions().size();

            for (size_t i = 0; i < size; i++) {
                auto vort = invBasis * glm::normalize(vec3(*vorticity));
                auto velo = invBasis * glm::normalize(vec3(*velocity));
                auto N = glm::normalize(glm::cross(vort, velo));
                vort *= (0.5f * ribbonWidth_.get());
                auto l = glm::length(*velocity);
                float d = glm::clamp(static_cast<float>(l) / velocityScale_.get(), 0.0f, 1.0f);
                auto vortictyMagnitude = glm::length(*vorticity);

                maxVelocity = std::max(maxVelocity, l);
                maxVorticity = std::max(maxVorticity, vortictyMagnitude);

                vec3 p0 = vec3(*position) - vort;
                vec3 p1 = vec3(*position) + vort;

                auto c = vec4(tf.sample(dvec2(d, 0.0)));

                indexBuffer->add(static_cast<std::uint32_t>(vertices.size()));
                indexBuffer->add(static_cast<std::uint32_t>(vertices.size()+1));
                vertices.push_back({ p0, N, p0, c });
                vertices.push_back({ p1, N, p1, c });

                position++;
                velocity++;
                vorticity++;
            }
        }

        maxVelocity_.set(toString(maxVelocity));
        maxVorticity_.set(toString(maxVorticity));
    }
    mesh->addVertices(vertices);
    mesh_.setData(mesh);
}

}  // namespace
