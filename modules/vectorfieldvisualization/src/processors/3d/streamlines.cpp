/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2026 Inviwo Foundation
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

#include <modules/vectorfieldvisualization/processors/3d/streamlines.h>

#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/datastructures/geometry/geometrytype.h>
#include <inviwo/core/datastructures/geometry/typedmesh.h>
#include <inviwo/core/datastructures/representationconverter.h>
#include <inviwo/core/datastructures/representationconverterfactory.h>
#include <inviwo/core/datastructures/transferfunction.h>
#include <inviwo/core/ports/datainport.h>
#include <inviwo/core/ports/inportiterable.h>
#include <inviwo/core/ports/meshport.h>
#include <inviwo/core/ports/outportiterable.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/processorinfo.h>
#include <inviwo/core/processors/processorstate.h>
#include <inviwo/core/processors/processortags.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/invalidationlevel.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/properties/transferfunctionproperty.h>
#include <inviwo/core/util/foreach.h>
#include <inviwo/core/util/glmutils.h>
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/logcentral.h>
#include <inviwo/core/util/spatialsampler.h>
#include <inviwo/core/util/statecoordinator.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/util/volumesampler.h>
#include <modules/vectorfieldvisualization/algorithms/integrallineoperations.h>
#include <modules/vectorfieldvisualization/datastructures/integralline.h>
#include <modules/vectorfieldvisualization/datastructures/integrallineset.h>
#include <modules/vectorfieldvisualization/integrallinetracer.h>
#include <modules/vectorfieldvisualization/ports/seedpointsport.h>
#include <modules/vectorfieldvisualization/properties/streamlineproperties.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iterator>
#include <memory>
#include <mutex>
#include <string_view>
#include <type_traits>
#include <unordered_set>

#include <fmt/core.h>
#include <glm/common.hpp>
#include <glm/geometric.hpp>

namespace inviwo {

const ProcessorInfo StreamLinesDeprecated::processorInfo_{
    "org.inviwo.StreamLinesDeprecated",  // Class identifier
    "Stream Lines (Deprecated)",         // Display name
    "Vector Field Visualization",        // Category
    CodeState::Deprecated,               // Code state
    Tags::CPU,                           // Tags
};
const ProcessorInfo& StreamLinesDeprecated::getProcessorInfo() const { return processorInfo_; }

StreamLinesDeprecated::StreamLinesDeprecated()
    : Processor()
    , sampler_("sampler")
    , seedPoints_("seedpoints")
    , volume_("vectorvolume")
    , linesStripsMesh_("linesStripsMesh_")
    , lines_("lines")
    , streamLineProperties_("streamLineProperties", "Stream Line Properties")
    , tf_("transferFunction", "Transfer Function")
    , velocityScale_("velocityScale_", "Velocity Scale (inverse)", 1, 0, 10)
    , maxVelocity_("minMaxVelocity", "Velocity Range", "0", InvalidationLevel::Valid)
    , useMutliThreading_("usetultiThreading", "Use Multi Threading", true) {

    addPort(sampler_);
    addPort(seedPoints_);
    addPort(volume_);

    addPort(lines_);
    addPort(linesStripsMesh_);

    isReady_.setUpdate([this]() {
        if (allInportsAreReady()) {
            return true;
        }
        if (!seedPoints_.isReady()) return false;

        if (sampler_.isConnected()) {
            return sampler_.isReady();
        }
        if (volume_.isConnected()) {
            return volume_.isReady();
        }
        return false;
    });

    maxVelocity_.setReadOnly(true);

    addProperty(streamLineProperties_);

    addProperty(useMutliThreading_);
    addProperty(tf_);
    addProperty(velocityScale_);
    addProperty(maxVelocity_);

    tf_.get().clear();
    tf_.get().add(0.0, vec4(0, 0, 1, 1));
    tf_.get().add(0.5, vec4(1, 1, 0, 1));
    tf_.get().add(1.0, vec4(1, 0, 0, 1));

    setAllPropertiesCurrentStateAsDefault();

    log::warn(
        "This Stream Lines Processor is Deprecated, use the new Stream Lines processor together "
        "with the IntegralLineToMesh processor.");
}

StreamLinesDeprecated::~StreamLinesDeprecated() {}

void StreamLinesDeprecated::process() {

    auto sampler = [&]() -> std::shared_ptr<const SpatialSampler<dvec3>> {
        if (sampler_.isConnected())
            return sampler_.getData();
        else
            return std::make_shared<VolumeDoubleSampler<3>>(volume_.getData());
    }();

    auto mesh = std::make_shared<BasicMesh>();

    mesh->setModelMatrix(sampler->getModelMatrix());
    mesh->setWorldMatrix(sampler->getWorldMatrix());

    auto m =
        streamLineProperties_.getSeedPointTransformationMatrix(sampler->getCoordinateTransformer());

    float maxVelocity = 0;

    StreamLine3DTracer tracer(sampler, streamLineProperties_);
    auto lines = std::make_shared<IntegralLineSet>(sampler->getModelMatrix());

    std::vector<BasicMesh::Vertex> vertices;

    std::mutex mutex;

    if (useMutliThreading_) {
        size_t startID = 0;

        for (const auto& seeds : seedPoints_) {
            util::forEachParallel(*seeds, [&](const auto& p, size_t i) {
                vec4 P = m * vec4(p, 1.0f);
                IntegralLine line = tracer.traceFrom(vec3(P));
                auto size = line.getPositions().size();
                if (size > 1) {
                    std::lock_guard<std::mutex> lock(mutex);
                    lines->push_back(line, startID + i);
                };
            });
            startID += seeds->size();
        }
    } else {
        size_t startID = 0;
        for (const auto& seeds : seedPoints_) {
            for (const auto& p : *seeds.get()) {
                vec4 P = m * vec4(p, 1.0f);
                IntegralLine line = tracer.traceFrom(vec3(P));
                auto size = line.getPositions().size();
                if (size > 1) {
                    lines->push_back(line, startID);
                }
                startID++;
            }
        }
    }

    for (auto& line : *lines) {
        auto position = line.getPositions().begin();
        auto velocity = line.getMetaData<dvec3>("velocity").begin();

        auto size = line.getPositions().size();
        if (size <= 1) continue;

        auto indexBuffer = mesh->addIndexBuffer(DrawType::Lines, ConnectivityType::StripAdjacency);

        indexBuffer->add(0);

        for (size_t i = 0; i < size; i++) {
            vec3 pos(*position);
            vec3 v(*velocity);

            float l = glm::length(vec3(*velocity));
            float d = glm::clamp(l / velocityScale_.get(), 0.0f, 1.0f);
            maxVelocity = std::max(maxVelocity, l);
            auto c = vec4(tf_.get().sample(d));

            indexBuffer->add(static_cast<std::uint32_t>(vertices.size()));

            vertices.push_back({pos, glm::normalize(v), pos, c});

            position++;
            velocity++;
        }
        indexBuffer->add(static_cast<std::uint32_t>(vertices.size() - 1));
    }
    //}

    mesh->addVertices(vertices);

    maxVelocity_.set(fmt::to_string(maxVelocity));

    linesStripsMesh_.setData(mesh);

    util::curvature(*lines);
    util::tortuosity(*lines);

    lines_.setData(lines);
}

}  // namespace inviwo
