/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2018 Inviwo Foundation
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
#include <inviwo/core/util/zip.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo StreamRibbons::processorInfo_{
    "org.inviwo.StreamRibbons",    // Class identifier
    "Stream Ribbons",              // Display name
    "Vector Field Visualization",  // Category
    CodeState::Experimental,       // Code state
    Tags::CPU,                     // Tags
};
const ProcessorInfo StreamRibbons::getProcessorInfo() const { return processorInfo_; }

StreamRibbons::StreamRibbons()
    : Processor()
    , sampler_("sampler")
    , vorticitySampler_("vorticitySampler")
    , seedPoints_("seedpoints")
    , colors_("colors")
    , volume_("vectorVolume")
    , vorticityVolume_("vorticityVolume")
    , streamLineProperties_("streamLineProperties", "Stream Line Properties")
    , ribbonWidth_("ribbonWidth", "Ribbon Width", 0.1f, 0.00001f)
    , tf_("transferFunction", "Transfer Function")
    , coloringMethod_("coloringMethod", "Color by",
                      {{"vel", "Velocity", ColoringMethod::Velocity},
                       {"vorticity", "Vorticity", ColoringMethod::Vorticity},
                       {"port", "Colors in port", ColoringMethod::ColorPort}})
    , velocityScale_("velocityScale_", "Velocity Scale (inverse)", 1, 0, 10)
    , maxVelocity_("minMaxVelocity", "Max Velocity", "0", InvalidationLevel::Valid)
    , maxVorticity_("maxVorticity", "Max Vorticity", "0", InvalidationLevel::Valid)
    , mesh_("mesh") {

    isReady_.setUpdate([this]() {
        if (allInportsAreReady()) {
            return true;
        }

        if (!seedPoints_.isReady()) return false;
        if (colors_.isConnected() && !colors_.isReady()) return false;

        bool velocitiesReady = false;
        bool vorticitiesReady = false;

        velocitiesReady |= sampler_.isConnected() && sampler_.isReady();
        velocitiesReady |= volume_.isConnected() && volume_.isReady();

        vorticitiesReady |= vorticitySampler_.isConnected() && vorticitySampler_.isReady();
        vorticitiesReady |= vorticityVolume_.isConnected() && vorticityVolume_.isReady();

        return velocitiesReady && vorticitiesReady;
    });

    addPort(sampler_);
    addPort(vorticitySampler_);
    addPort(seedPoints_);
    addPort(colors_);
    addPort(volume_);
    addPort(vorticityVolume_);
    addPort(mesh_);

    colors_.setOptional(true);

    maxVelocity_.setReadOnly(true);
    maxVorticity_.setReadOnly(true);

    addProperty(streamLineProperties_);
    addProperty(ribbonWidth_);
    addProperty(tf_);
    addProperty(coloringMethod_);
    addProperty(velocityScale_);
    addProperty(maxVelocity_);
    addProperty(maxVorticity_);
}

void StreamRibbons::process() {
    auto sampler = [&]() -> std::shared_ptr<const SpatialSampler<3, 3, double>> {
        if (sampler_.isConnected()) {
            return sampler_.getData();
        } else {
            return std::make_shared<VolumeDoubleSampler<3>>(volume_.getData());
        }
    }();

    auto vorticitySampler = [&]() -> std::shared_ptr<const SpatialSampler<3, 3, double>> {
        if (vorticitySampler_.isConnected()) {
            return vorticitySampler_.getData();
        } else {
            return std::make_shared<VolumeDoubleSampler<3>>(vorticityVolume_.getData());
        }
    }();

    auto mesh = std::make_shared<BasicMesh>();
    mesh->setModelMatrix(sampler->getModelMatrix());
    mesh->setWorldMatrix(sampler->getWorldMatrix());

    auto m =
        streamLineProperties_.getSeedPointTransformationMatrix(sampler->getCoordinateTransformer());

    StreamLineTracer tracer(sampler, streamLineProperties_);
    ImageSampler tf(tf_.get().getData());
    tracer.addMetaSampler("vorticity", vorticitySampler);

    const std::function<vec4(double, double, size_t)> coloring =
        [&]() -> std::function<vec4(double, double, size_t)> {
        switch (coloringMethod_.get()) {
            case ColoringMethod::Vorticity: {
                return [&](double vortictyMagnitude, double /*velocityMagnitude*/, size_t /*lineId*/) {
                    auto d = glm::clamp(
                        static_cast<float>(vortictyMagnitude) / velocityScale_.get(), 0.0f, 1.0f);
                    return vec4(tf.sample(dvec2(d, 0.0)));
                };
            }
            case ColoringMethod::ColorPort:
                if (colors_.hasData()) {
                    return [&](double /*vortictyMagnitude*/, double /*velocityMagnitude*/, size_t lineId) {
                        if (lineId < colors_.getData()->size()) {
                            return (*colors_.getData())[lineId];
                        } else if (lineId == colors_.getData()->size()) {
                            LogWarn(
                                "The vector of colors is smaller then the vector of seed points");
                            return colors_.getData()->back();
                        } else {
                            return colors_.getData()->back();
                        }

                    };
                } else {
                    LogWarn(
                        "No colors in the color port, using velocity for coloring "
                        "instead ");
                }
            default:
            case ColoringMethod::Velocity: {
                return [&](double /*vortictyMagnitude*/, double velocityMagnitude, size_t /*lineId*/) {
                    auto d = glm::clamp(
                        static_cast<float>(velocityMagnitude) / velocityScale_.get(), 0.0f, 1.0f);
                    return vec4(tf.sample(dvec2(d, 0.0)));
                };
            }
        }
    }();

    std::vector<BasicMesh::Vertex> vertices;
    double maxVelocity = 0;
    double maxVorticity = 0;
    size_t lineId = 0;

    for (const auto& seeds : seedPoints_) {
        for (auto& p : *seeds) {
            vec4 P = m * vec4(p, 1.0f);
            auto line = tracer.traceFrom(vec3(P));
            auto size = line.getPositions().size();
            if (size <= 1) continue;

            auto indexBuffer = mesh->addIndexBuffer(DrawType::Triangles, ConnectivityType::Strip);
            indexBuffer->getDataContainer().reserve(size * 2);

            for (auto&& item : util::zip(line.getPositions(), line.getMetaData("velocity"),
                                         line.getMetaData("vorticity"))) {
                const auto& position = item.first();
                const auto& velocity = item.second();
                const auto& vorticity = item.third();

                auto vort = glm::normalize(vec3(vorticity));
                const auto velo = glm::normalize(vec3(velocity));
                const auto N = glm::normalize(glm::cross(vort, velo));
                vort *= (0.5f * ribbonWidth_.get());
                const auto velocityMagnitude = glm::length(velocity);
                const auto vortictyMagnitude = glm::length(vorticity);

                maxVelocity = std::max(maxVelocity, velocityMagnitude);
                maxVorticity = std::max(maxVorticity, vortictyMagnitude);

                const vec3 p0 = vec3(position) - vort;
                const vec3 p1 = vec3(position) + vort;
                const vec4 c = coloring(vortictyMagnitude, velocityMagnitude, lineId);
                indexBuffer->add(static_cast<std::uint32_t>(vertices.size()));
                indexBuffer->add(static_cast<std::uint32_t>(vertices.size() + 1));
                vertices.push_back({p0, N, p0, c});
                vertices.push_back({p1, N, p1, c});
            }
            lineId++;
        }

        maxVelocity_.set(toString(maxVelocity));
        maxVorticity_.set(toString(maxVorticity));
    }
    mesh->addVertices(vertices);
    mesh_.setData(mesh);
}

}  // namespace inviwo
