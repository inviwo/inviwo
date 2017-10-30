#include "streamlines.h"

#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>
#include <inviwo/core/datastructures/geometry/basicmesh.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <modules/opengl/image/layergl.h>
#include <inviwo/core/util/zip.h>
#include <bitset>

#include <inviwo/core/util/imagesampler.h>
#include <modules/vectorfieldvisualization/algorithms/integrallineoperations.h>

namespace inviwo {

const ProcessorInfo StreamLines::processorInfo_{
    "org.inviwo.StreamLines",      // Class identifier
    "Stream Lines",                // Display name
    "Vector Field Visualization",  // Category
    CodeState::Experimental,       // Code state
    Tags::CPU,                     // Tags
};
const ProcessorInfo StreamLines::getProcessorInfo() const { return processorInfo_; }

StreamLines::StreamLines()
    : Processor()
    , sampler_("sampler")
    , seedPoints_("seedpoints")
    , volume_("vectorvolume")
    , linesStripsMesh_("linesStripsMesh_")
    , lines_("lines")
    , streamLineProperties_("streamLineProperties", "Stream Line Properties")
    , tf_("transferFunction", "Transfer Function",
          TransferFunction{
              {{0.0f, vec4(0, 0, 1, 1)}, {0.5f, vec4(1, 1, 0, 1)}, {1.0, vec4(1, 0, 0, 1)}}})
    , velocityScale_("velocityScale_", "Velocity Scale (inverse)", 1, 0, 10)
    , maxVelocity_("minMaxVelocity", "Velocity Range", "0", InvalidationLevel::Valid)
    , useOpenMP_("useOpenMP", "Use OpenMP", true) {

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

    addPort(sampler_);
    addPort(seedPoints_);
    addPort(volume_);
    addPort(lines_);
    addPort(linesStripsMesh_);

    maxVelocity_.setReadOnly(true);

    addProperty(streamLineProperties_);
    addProperty(useOpenMP_);
    addProperty(tf_);
    addProperty(velocityScale_);
    addProperty(maxVelocity_);
}

StreamLines::~StreamLines() = default;

void StreamLines::process() {

    auto sampler = [&]() -> std::shared_ptr<const SpatialSampler<3, 3, double>> {
        if (sampler_.isConnected()) {
            return sampler_.getData();
        } else {
            return std::make_shared<VolumeDoubleSampler<3>>(volume_.getData());
        }
    }();

    auto mesh = std::make_shared<BasicMesh>();

    mesh->setModelMatrix(sampler->getModelMatrix());
    mesh->setWorldMatrix(sampler->getWorldMatrix());

    auto m =
        streamLineProperties_.getSeedPointTransformationMatrix(sampler->getCoordinateTransformer());

    StreamLineTracer tracer(sampler, streamLineProperties_);
    auto lines = std::make_shared<IntegralLineSet>(sampler->getModelMatrix());

    if (useOpenMP_) {
        size_t startID = 0;
        for (const auto &seeds : seedPoints_) {
#pragma omp parallel for
            for (long long j = 0; j < static_cast<long long>(seeds->size()); j++) {
                auto p = seeds->at(j);
                vec4 P = m * vec4(p, 1.0f);
                auto line = tracer.traceFrom(vec3(P));
                auto size = line.getPositions().size();
                if (size > 1) {
#pragma omp critical
                    lines->push_back(line, startID + j);
                };
            }
            startID += seeds->size();
        }
    } else {
        size_t startID = 0;
        for (const auto &seeds : seedPoints_) {
            for (const auto &p : *seeds.get()) {
                vec4 P = m * vec4(p, 1.0f);
                auto line = tracer.traceFrom(vec3(P));
                auto size = line.getPositions().size();
                if (size > 1) {
                    lines->push_back(line, startID);
                }
                startID++;
            }
        }
    }

    std::vector<BasicMesh::Vertex> vertices;
    float maxVelocity = 0;

    for (auto &line : *lines) {
        if (line.getPositions().size() <= 1) continue;

        auto indexBuffer = mesh->addIndexBuffer(DrawType::Lines, ConnectivityType::StripAdjacency);
        indexBuffer->add(0);
        for (auto &&item : util::zip(line.getPositions(), line.getMetaData("velocity"))) {
            const vec3 pos(item.first());
            const vec3 v(item.second());

            const float l = glm::length(v);
            const float d = glm::clamp(l / velocityScale_.get(), 0.0f, 1.0f);
            const auto c = vec4(tf_.get().sample(d));

            maxVelocity = std::max(maxVelocity, l);
            indexBuffer->add(static_cast<std::uint32_t>(vertices.size()));
            vertices.push_back({pos, glm::normalize(v), pos, c});
        }
        indexBuffer->add(static_cast<std::uint32_t>(vertices.size() - 1));
    }

    mesh->addVertices(vertices);

    maxVelocity_.set(toString(maxVelocity));

    linesStripsMesh_.setData(mesh);

    util::curvature(*lines);
    util::tortuosity(*lines);

    lines_.setData(lines);
}

}  // namespace inviwo
