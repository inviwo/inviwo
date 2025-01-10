/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2025 Inviwo Foundation
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

#include <modules/vectorfieldvisualization/processors/3d/streamribbons.h>

#include <inviwo/core/datastructures/buffer/buffer.h>                          // for Buffer
#include <inviwo/core/datastructures/geometry/geometrytype.h>                  // for Connectivi...
#include <inviwo/core/datastructures/geometry/typedmesh.h>                     // for TypedMesh<...
#include <inviwo/core/datastructures/representationconverter.h>                // for Representa...
#include <inviwo/core/datastructures/representationconverterfactory.h>         // for Representa...
#include <inviwo/core/datastructures/transferfunction.h>                       // for TransferFu...
#include <inviwo/core/ports/datainport.h>                                      // for DataInport
#include <inviwo/core/ports/inportiterable.h>                                  // for InportIter...
#include <inviwo/core/ports/meshport.h>                                        // for MeshOutport
#include <inviwo/core/ports/outportiterable.h>                                 // for OutportIte...
#include <inviwo/core/ports/volumeport.h>                                      // for VolumeInport
#include <inviwo/core/processors/processor.h>                                  // for Processor
#include <inviwo/core/processors/processorinfo.h>                              // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                             // for CodeState
#include <inviwo/core/processors/processortags.h>                              // for Tags, Tags...
#include <inviwo/core/properties/invalidationlevel.h>                          // for Invalidati...
#include <inviwo/core/properties/optionproperty.h>                             // for OptionProp...
#include <inviwo/core/properties/ordinalproperty.h>                            // for FloatProperty
#include <inviwo/core/properties/stringproperty.h>                             // for StringProp...
#include <inviwo/core/properties/transferfunctionproperty.h>                   // for TransferFu...
#include <inviwo/core/util/glmmat.h>                                           // for mat3
#include <inviwo/core/util/glmutils.h>                                         // for Vector
#include <inviwo/core/util/glmvec.h>                                           // for dvec3, vec4
#include <inviwo/core/util/imagesampler.h>                                     // for ImageSampler
#include <inviwo/core/util/logcentral.h>                                       // for LogCentral
#include <inviwo/core/util/spatialsampler.h>                                   // for SpatialSam...
#include <inviwo/core/util/statecoordinator.h>                                 // for StateCoord...
#include <inviwo/core/util/staticstring.h>                                     // for operator+
#include <inviwo/core/util/stringconversion.h>                                 // for toString
#include <inviwo/core/util/volumesampler.h>                                    // for VolumeDoub...
#include <modules/vectorfieldvisualization/datastructures/integralline.h>      // for IntegralLine
#include <modules/vectorfieldvisualization/integrallinetracer.h>               // for StreamLine...
#include <modules/vectorfieldvisualization/ports/seedpointsport.h>             // for SeedPoints...
#include <modules/vectorfieldvisualization/properties/streamlineproperties.h>  // for StreamLine...

#include <algorithm>      // for max
#include <cstddef>        // for size_t
#include <cstdint>        // for uint32_t
#include <memory>         // for shared_ptr
#include <type_traits>    // for remove_ext...
#include <unordered_set>  // for unordered_set

#include <fmt/core.h>         // for format
#include <glm/common.hpp>     // for mix, clamp
#include <glm/geometric.hpp>  // for normalize
#include <glm/mat3x3.hpp>     // for operator*

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo StreamRibbonsDeprecated::processorInfo_{
    "org.inviwo.StreamRibbonsDeprecated",  // Class identifier
    "Stream Ribbons (Deprecated)",         // Display name
    "Vector Field Visualization",          // Category
    CodeState::Deprecated,                 // Code state
    Tags::CPU,                             // Tags
};
const ProcessorInfo& StreamRibbonsDeprecated::getProcessorInfo() const { return processorInfo_; }

StreamRibbonsDeprecated::StreamRibbonsDeprecated()
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

    log::warn(
        "This Stream Ribbons Processor is Deprecated, use the new Stream Lines processor together "
        "with the IntegralLineToMesh processor.");
}

void StreamRibbonsDeprecated::process() {
    auto sampler = [&]() -> std::shared_ptr<const SpatialSampler<dvec3>> {
        if (sampler_.isConnected())
            return sampler_.getData();
        else
            return std::make_shared<VolumeSampler<dvec3>>(volume_.getData());
    }();

    auto vorticitySampler = [&]() -> std::shared_ptr<const StreamLine3DTracer::Sampler> {
        if (vorticitySampler_.isConnected())
            return vorticitySampler_.getData();
        else
            return std::make_shared<VolumeDoubleSampler<3>>(vorticityVolume_.getData());
    }();

    auto mesh = std::make_shared<BasicMesh>();
    mesh->setModelMatrix(sampler->getModelMatrix());
    mesh->setWorldMatrix(sampler->getWorldMatrix());

    auto m =
        streamLineProperties_.getSeedPointTransformationMatrix(sampler->getCoordinateTransformer());
    double maxVelocity = 0;
    double maxVorticity = 0;
    StreamLine3DTracer tracer(sampler, streamLineProperties_);
    const TransferFunction& tf = tf_.get();
    tracer.addMetaDataSampler("vorticity", vorticitySampler);
    //  mat3 invBasis = glm::inverse(vectorVolume_.getData()->getBasis());
    mat3 invBasis{1};
    std::vector<BasicMesh::Vertex> vertices;

    bool hasColors = colors_.hasData();
    size_t lineId = 0;

    for (const auto& seeds : seedPoints_) {
        for (auto& p : (*seeds)) {
            vec4 P = m * vec4(p, 1.0f);
            IntegralLine line = tracer.traceFrom(vec3(P));

            auto position = line.getPositions().begin();
            auto velocity = line.getMetaData<dvec3>("velocity").begin();
            auto vorticity = line.getMetaData<dvec3>("vorticity").begin();

            auto size = line.getPositions().size();
            if (size <= 1) continue;
            auto indexBuffer = mesh->addIndexBuffer(DrawType::Triangles, ConnectivityType::Strip);
            indexBuffer->getDataContainer().reserve(size);

            vec4 c{0};
            if (hasColors) {
                if (lineId >= colors_.getData()->size()) {
                    log::warn("The vector of colors is smaller then the vector of seed points");
                } else {
                    c = colors_.getData()->at(lineId);
                }
            }
            lineId++;

            for (size_t i = 0; i < size; i++) {
                auto vort = invBasis * glm::normalize(vec3(*vorticity));
                auto velo = invBasis * glm::normalize(vec3(*velocity));
                auto N = glm::normalize(glm::cross(vort, velo));
                vort *= (0.5f * ribbonWidth_.get());
                auto velocityMagnitude = glm::length(*velocity);
                auto vortictyMagnitude = glm::length(*vorticity);

                maxVelocity = std::max(maxVelocity, velocityMagnitude);
                maxVorticity = std::max(maxVorticity, vortictyMagnitude);

                vec3 p0 = vec3(*position) - vort;
                vec3 p1 = vec3(*position) + vort;

                float d;
                switch (coloringMethod_.get()) {
                    case ColoringMethod::Vorticity:
                        d = glm::clamp(static_cast<float>(vortictyMagnitude) / velocityScale_.get(),
                                       0.0f, 1.0f);
                        c = tf.sample(d);
                        break;
                    case ColoringMethod::ColorPort:
                        if (hasColors) {
                            break;
                        } else {
                            log::warn(
                                "No colors in the color port, using velocity for coloring "
                                "instead ");
                            [[fallthrough]];
                        }
                    default:
                        [[fallthrough]];
                    case ColoringMethod::Velocity:
                        d = glm::clamp(static_cast<float>(velocityMagnitude) / velocityScale_.get(),
                                       0.0f, 1.0f);
                        c = tf.sample(d);
                        break;
                }

                indexBuffer->add(static_cast<std::uint32_t>(vertices.size()));
                indexBuffer->add(static_cast<std::uint32_t>(vertices.size() + 1));
                vertices.push_back({p0, N, p0, c});
                vertices.push_back({p1, N, p1, c});

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

}  // namespace inviwo
