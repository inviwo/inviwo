#include "streamlines.h"

#include <bitset>
#include <inviwo/core/datastructures/geometry/basicmesh.h>
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>
#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <modules/opengl/image/layergl.h>
#include <inviwo/core/datastructures/image/layerram.h>

#include <inviwo/core/util/imagesampler.h>

namespace inviwo {

const ProcessorInfo StreamLines::processorInfo_{
    "org.inviwo.StreamLines",      // Class identifier
    "Stream Lines",                // Display name
    "Vector Field Visualization",  // Category
    CodeState::Experimental,       // Code state
    Tags::CPU,                     // Tags
};
const ProcessorInfo StreamLines::getProcessorInfo() const {
    return processorInfo_;
}

StreamLines::StreamLines()
    : Processor()
    , volume_("vectorvolume")
    , seedPoints_("seedpoints")
    , linesStripsMesh_("linesStripsMesh_")
    , numberOfSteps_("steps", "Number of Steps", 100, 1, 1000)
    , normalizeSamples_("normalizeSamples", "Normalize Samples", true)
    , stepSize_("stepSize", "StepSize", 0.001f, 0.0001f, 1.0f)
    , stepDirection_("stepDirection", "Step Direction")
    , integrationScheme_("integrationScheme","Integration Scheme")
    , seedPointsSpace_("seedPointsSpace", "Seed Points Space")
    , tf_("transferFunction", "Transfer Function")
    , velocityScale_("velocityScale_", "Velocity Scale (inverse)", 1, 0, 10)
    , maxVelocity_("minMaxVelocity", "Velocity Range", "0", InvalidationLevel::Valid) {
    addPort(volume_);
    addPort(seedPoints_);
    addPort(linesStripsMesh_);

    stepSize_.setIncrement(0.0001f);
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

    stepSize_.setCurrentStateAsDefault();
    stepDirection_.setCurrentStateAsDefault();

    maxVelocity_.setReadOnly(true);

    addProperty(numberOfSteps_);
    addProperty(stepSize_);
    addProperty(stepDirection_);
    addProperty(integrationScheme_);
    addProperty(normalizeSamples_);
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

StreamLines::~StreamLines() {}

void StreamLines::process() {
    auto mesh = util::make_unique<BasicMesh>();
    mesh->setModelMatrix(volume_.getData()->getModelMatrix());
    mesh->setWorldMatrix(volume_.getData()->getWorldMatrix());

    auto m = volume_.getData()->getCoordinateTransformer().getMatrix(
        seedPointsSpace_.get(), StructuredCoordinateTransformer<3>::Space::Texture);

    ImageSampler tf(tf_.get().getData());

    float maxVelocity = 0;
    StreamLineTracer tracer(volume_.getData().get(),integrationScheme_.get());

    std::vector<BasicMesh::Vertex> vertices;

    for (const auto &seeds : seedPoints_) {
        for (auto &p : (*seeds)) {
            vec4 P = m * vec4(p, 1.0f);
            auto indexBuffer =
                mesh->addIndexBuffer(DrawType::LINES, ConnectivityType::STRIP_ADJACENCY);
            auto line = tracer.traceFrom(P.xyz(), numberOfSteps_.get(), stepSize_.get(),
                                         stepDirection_.get(), normalizeSamples_.get());

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

                vertices.push_back({pos,glm::normalize(v),pos,c});
                
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

}  // namespace

