/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022 Inviwo Foundation
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

#include <modules/vectorfieldvisualization/processors/streamlinepathways.h>
#include <modules/base/algorithm/meshutils.h>
#include <inviwo/core/datastructures/geometry/basicmesh.h>
#include <inviwo/core/util/foreach.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo StreamlinePathways::processorInfo_{
    "org.inviwo.StreamlinePathways",  // Class identifier
    "Streamline Pathways",            // Display name
    "Undefined",                      // Category
    CodeState::Experimental,          // Code state
    Tags::None,                       // Tags
};
const ProcessorInfo StreamlinePathways::getProcessorInfo() const { return processorInfo_; }

StreamlinePathways::VolumeRegion::VolumeRegion(const std::string& identifier,
                                               const std::string& displayName,
                                               const vec4& defaultColor,
                                               ConstraintBehavior limitBehavior)
    : CompositeProperty(identifier, displayName)
    , min_("Min", "Min", dvec3(0, 0, 0), {{0, 0, 0}, limitBehavior}, {{1, 1, 1}, limitBehavior})
    , max_("Max", "Max", dvec3(1, 1, 1), {{0, 0, 0}, limitBehavior}, {{1, 1, 1}, limitBehavior})
    , color_("Color", "Start color", defaultColor, {{0, 0, 0, 0}, ConstraintBehavior::Immutable},
             {{1, 1, 1, 1}, ConstraintBehavior::Immutable}, {0.1, 0.1, 0.1, 0.1},
             InvalidationLevel::InvalidOutput, PropertySemantics::Color) {
    addProperties(min_, max_, color_);
}

void StreamlinePathways::VolumeRegion::updateSamplerRegion(const dvec3& min, const dvec3& max) {
    min_.setMinValue(min);
    min_.setMaxValue(max);
    max_.setMinValue(min);
    max_.setMaxValue(max);
}
std::shared_ptr<Mesh> StreamlinePathways::VolumeRegion::toMesh(const mat4& worldToModelMat) const {
    return meshutil::boundingBoxAdjacency(getAsMatrix(worldToModelMat), color_);
}

mat4 StreamlinePathways::VolumeRegion::getAsMatrix(const dmat4& worldToModelMat) const {
    return worldToModelMat * glm::translate(min_.get()) * glm::scale(max_.get() - min_.get());
}

StreamlinePathways::StreamlinePathways()
    : Processor()
    , sampler_("sampler")
    , seeds_("seeds")
    , lineSelection_("lineSelection")
    , startMesh_("startMesh")
    , endMesh_("endMesh")
    , integralLines_("streamlines")

    , startRegion_("startRegion", "Start region", {1, 0, 0, 1})
    , endRegion_("endRegion", "End region", {0, 0, 1, 1}, ConstraintBehavior::Editable)
    , integrationProperties_("integrationProperties", "Integration properties")

    , integrationRequired_(false) {

    addPorts(sampler_, seeds_, lineSelection_, startMesh_, endMesh_, integralLines_);
    addProperties(startRegion_, endRegion_, integrationProperties_);

    integrationProperties_.seedPointsSpace_.set(CoordinateSpace::Model);
    integrationProperties_.seedPointsSpace_.setVisible(false);

    auto requireIntegration = [this]() { integrationRequired_ = true; };
    integrationProperties_.onChange(requireIntegration);
    startRegion_.min_.onChange(requireIntegration);
    startRegion_.max_.onChange(requireIntegration);
}

void StreamlinePathways::process() {
    if (!sampler_.hasData()) return;

    auto sampler = sampler_.getData();

    if (sampler_.isChanged()) {
        auto modelMat = sampler->getModelMatrix();
        auto samplerMin = modelMat * dvec4(0, 0, 0, 1);
        auto samplerMax = modelMat * dvec4(1, 1, 1, 1);
        startRegion_.updateSamplerRegion(samplerMin, samplerMax);
        endRegion_.updateSamplerRegion(samplerMin, samplerMax);

        LogWarn("ModelMat:\n" << modelMat);
        LogWarn("Updated property ranges");
    }

    auto modelToWorld = sampler->getCoordinateTransformer().getMatrix(CoordinateSpace::Model,
                                                                      CoordinateSpace::World);
    auto modelToData = sampler->getCoordinateTransformer().getMatrix(CoordinateSpace::Model,
                                                                     CoordinateSpace::Data);

    startMesh_.setData(startRegion_.toMesh(modelToWorld));
    endMesh_.setData(endRegion_.toMesh(modelToWorld));

    if (!integrationRequired_ && !sampler_.isChanged() && !seeds_.isChanged()) {
        return;
    }

    integrationRequired_ = false;
    lines_ =
        std::make_shared<IntegralLineSet>(sampler->getModelMatrix(), sampler->getWorldMatrix());
    lineSelection_.sendSelectionEvent({});
    // lineSelection_.invalidate(InvalidationLevel::InvalidResources);
    // Integrate some lines.
    mat4 startMatrix = startRegion_.getAsMatrix(mat4(1.0f));

    Tracer tracer(sampler, integrationProperties_);

    std::mutex mutex;
    size_t startID = 0;
    // std::unordered_set<size_t> selectedLines;
    std::unordered_set<size_t> filteredLines;
    const dvec3& endMin = modelToData * dvec4(endRegion_.min_.get(), 1.0);
    const dvec3& endMax = modelToData * dvec4(endRegion_.max_.get(), 1.0);
    for (const auto& seeds : seeds_) {
        util::forEachParallel(*seeds, [&](const auto& p, size_t i) {
            dvec3 seed = startMatrix * dvec4(p, 1);
            IntegralLine line = tracer.traceFrom(seed);

            auto size = line.getPositions().size();
            if (size > 1) {
                auto& depth = line.getMetaData<double>("depth", true);
                // auto& ends = line.getMetaData<double>("reachesEnd", true);
                depth.resize(size);
                // ends.resize(size);

                bool reachedEnd = false;
                for (size_t p = 0; p < size; ++p) {
                    auto& pos = line.getPositions()[p];
                    depth[p] = pos.z;
                    if (pos.x > endMin.x && pos.x < endMax.x && pos.y > endMin.y &&
                        pos.y < endMax.y && pos.z > endMin.z && pos.z < endMax.z) {
                        reachedEnd = true;
                        std::cout << "Line " << i << " reached the end!" << std::endl;
                        break;
                    }
                }

                // std::fill(ends.begin(), ends.end(), reachedEnd ? 1.0 : 0.0);

                std::lock_guard<std::mutex> lock(mutex);
                lines_->push_back(std::move(line), startID + i);
                if (!reachedEnd)  // selectedLines.insert(startID + i);
                    filteredLines.insert(startID + i);
            }
        });
        startID += seeds->size();
    }
    integralLines_.setData(lines_);
    // lineSelection_.sendSelectionEvent(selectedLines);
    lineSelection_.sendFilterEvent(filteredLines);
    // std::cout << "Done with Streamline Pathways" << std::endl;
}

}  // namespace inviwo
