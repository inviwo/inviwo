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

#include <modules/vectorfieldvisualization/processors/3d/pathlines.h>

#include <inviwo/core/datastructures/buffer/buffer.h>                            // for Buffer
#include <inviwo/core/datastructures/geometry/geometrytype.h>                    // for Connecti...
#include <inviwo/core/datastructures/geometry/typedmesh.h>                       // for TypedMes...
#include <inviwo/core/datastructures/representationconverter.h>                  // for Represen...
#include <inviwo/core/datastructures/representationconverterfactory.h>           // for Represen...
#include <inviwo/core/datastructures/tfprimitive.h>                              // for TFPrimit...
#include <inviwo/core/datastructures/transferfunction.h>                         // for Transfer...
#include <inviwo/core/io/serialization/versionconverter.h>                       // for changePr...
#include <inviwo/core/ports/datainport.h>                                        // for DataInport
#include <inviwo/core/ports/inportiterable.h>                                    // for InportIt...
#include <inviwo/core/ports/meshport.h>                                          // for MeshOutport
#include <inviwo/core/ports/outportiterable.h>                                   // for OutportI...
#include <inviwo/core/ports/volumeport.h>                                        // for VolumeSe...
#include <inviwo/core/processors/processor.h>                                    // for Processor
#include <inviwo/core/processors/processorinfo.h>                                // for Processo...
#include <inviwo/core/processors/processorstate.h>                               // for CodeState
#include <inviwo/core/processors/processortags.h>                                // for Tags
#include <inviwo/core/properties/boolproperty.h>                                 // for BoolProp...
#include <inviwo/core/properties/invalidationlevel.h>                            // for Invalida...
#include <inviwo/core/properties/optionproperty.h>                               // for OptionPr...
#include <inviwo/core/properties/ordinalproperty.h>                              // for FloatPro...
#include <inviwo/core/properties/stringproperty.h>                               // for StringPr...
#include <inviwo/core/properties/transferfunctionproperty.h>                     // for Transfer...
#include <inviwo/core/util/glmvec.h>                                             // for dvec3, vec4
#include <inviwo/core/util/logcentral.h>                                         // for LogCentral
#include <inviwo/core/util/spatial4dsampler.h>                                   // for Spatial4...
#include <inviwo/core/util/statecoordinator.h>                                   // for StateCoo...
#include <inviwo/core/util/staticstring.h>                                       // for operator+
#include <inviwo/core/util/stringconversion.h>                                   // for toString
#include <inviwo/core/util/volumesequencesampler.h>                              // for VolumeSe...
#include <modules/vectorfieldvisualization/algorithms/integrallineoperations.h>  // for curvature
#include <modules/vectorfieldvisualization/datastructures/integralline.h>        // for Integral...
#include <modules/vectorfieldvisualization/datastructures/integrallineset.h>     // for Integral...
#include <modules/vectorfieldvisualization/integrallinetracer.h>                 // for Integral...
#include <modules/vectorfieldvisualization/ports/seedpointsport.h>               // for SeedPoin...
#include <modules/vectorfieldvisualization/properties/pathlineproperties.h>      // for PathLine...

#include <algorithm>      // for max
#include <cstddef>        // for size_t
#include <cstdint>        // for uint32_t
#include <memory>         // for shared_ptr
#include <type_traits>    // for remove_e...
#include <unordered_set>  // for unordere...
#include <utility>        // for pair

#include <fmt/core.h>         // for format
#include <glm/common.hpp>     // for clamp
#include <glm/geometric.hpp>  // for length
#include <glm/mat4x4.hpp>     // for operator*
#include <glm/vec3.hpp>       // for operator*
#include <glm/vec4.hpp>       // for operator*

#ifdef IVW_USE_OPENMP
#include <omp.h>
#endif

namespace inviwo {
class Deserializer;

const ProcessorInfo PathLinesDeprecated::processorInfo_{
    "org.inviwo.PathLinesDeprecated",  // Class identifier
    "Path Lines (Deprecated)",         // Display name
    "Vector Field Visualization",      // Category
    CodeState::Deprecated,             // Code state
    Tags::CPU,                         // Tags
};

const ProcessorInfo& PathLinesDeprecated::getProcessorInfo() const { return processorInfo_; }

PathLinesDeprecated::PathLinesDeprecated()
    : Processor()
    , sampler_("sampler")
    , seedPoints_("seedpoints")
    , colors_("colors")
    , volume_("vectorvolume")
    , lines_("lines")
    , linesStripsMesh_("linesStripsMesh")
    , pathLineProperties_("pathLineProperties", "Path Line Properties")
    , tf_("transferFunction", "Transfer Function",
          TransferFunction{
              {{0.0f, vec4(0, 0, 1, 1)}, {0.5f, vec4(1, 1, 0, 1)}, {1.0, vec4(1, 0, 0, 1)}}})
    , coloringMethod_("coloringMethod", "Color by",
                      {{"vel", "Velocity", ColoringMethod::Velocity},
                       {"time", "Timestamp", ColoringMethod::Timestamp},
                       {"port", "Colors in port", ColoringMethod::ColorPort}})
    , velocityScale_("velocityScale_", "Velocity Scale (inverse)", 1, 0, 10)
    , maxVelocity_("minMaxVelocity", "Velocity Range", "0", InvalidationLevel::Valid)
    , allowLooping_("allowLooping", "Allow looping", true) {

    isReady_.setUpdate([this]() {
        if (allInportsAreReady()) {
            return true;
        }

        if (seedPoints_.isConnected() && !seedPoints_.isReady()) return false;
        if (colors_.isConnected() && !colors_.isReady()) return false;

        if (sampler_.isConnected()) {
            return sampler_.isReady();
        }
        if (volume_.isConnected()) {
            return volume_.isReady();
        }
        return false;
    });

    colors_.setOptional(true);

    addPort(sampler_);
    addPort(seedPoints_);
    addPort(colors_);
    addPort(volume_);
    addPort(lines_);
    addPort(linesStripsMesh_);

    maxVelocity_.setReadOnly(true);
    allowLooping_.setVisible(false);

    addProperty(pathLineProperties_);
    addProperty(tf_);
    addProperty(coloringMethod_);
    addProperty(velocityScale_);
    addProperty(maxVelocity_);
    addProperty(allowLooping_);

    log::warn(
        "This Path Lines Processor is Deprecated, use the new Path Lines processor together "
        "with the IntegralLineToMesh processor.");
}

void PathLinesDeprecated::process() {
    auto sampler = [&]() -> std::shared_ptr<const Spatial4DSampler<dvec3>> {
        if (sampler_.isConnected()) {
            if (allowLooping_.getVisible()) {
                allowLooping_.setVisible(false);
            }
            return sampler_.getData();
        } else {
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

    auto m =
        pathLineProperties_.getSeedPointTransformationMatrix(sampler->getCoordinateTransformer());

    float maxVelocity = 0;
    PathLine3DTracer tracer(sampler, pathLineProperties_);

    bool hasColors = colors_.hasData();

    bool warnOnce = true;
    bool warnOnce2 = true;

    auto lines = std::make_shared<IntegralLineSet>(sampler->getModelMatrix());
    std::vector<BasicMesh::Vertex> vertices;
    for (const auto& seeds : seedPoints_) {

#ifdef IVW_USE_OPENMP
#pragma omp parallel for
#endif
        for (long long j = 0; j < static_cast<long long>(seeds->size()); j++) {
            const auto& p = (*seeds)[j];
            vec4 P = m * vec4(p, 1.0f);
            IntegralLine line = tracer.traceFrom(vec4(vec3(P), pathLineProperties_.getStartT()));
            auto size = line.getPositions().size();
            if (size > 1) {
#ifdef IVW_USE_OPENMP
#pragma omp critical
#endif
                lines->push_back(line, lines->size());
            };
        }
    }

    for (auto& line : *lines) {
        auto size = line.getPositions().size();
        if (size <= 1) continue;

        auto position = line.getPositions().begin();
        auto velocity = line.getMetaData<dvec3>("velocity").begin();
        auto timestamp = line.getMetaData<double>("timestamp").begin();

        auto indexBuffer = mesh->addIndexBuffer(DrawType::Lines, ConnectivityType::StripAdjacency);
        indexBuffer->add(0);

        vec4 c{0};
        if (hasColors) {
            if (line.getIndex() >= colors_.getData()->size()) {
                if (warnOnce2) {
                    warnOnce2 = false;
                    log::warn("The vector of colors is smaller then the vector of seed points");
                }
            } else {
                c = colors_.getData()->at(line.getIndex());
            }
        }

        for (size_t ii = 0; ii < size; ii++) {
            vec3 pos(*position);
            vec3 v(*velocity);
            float t = static_cast<float>(*timestamp);

            float l = glm::length(v);
            float d = glm::clamp(l / velocityScale_.get(), 0.0f, 1.0f);
            maxVelocity = std::max(maxVelocity, l);

            switch (coloringMethod_.get()) {
                case ColoringMethod::Timestamp:
                    c = tf_.get().sample(t);
                    break;
                case ColoringMethod::ColorPort:
                    if (hasColors) {
                        break;
                    } else {
                        if (warnOnce) {
                            warnOnce = false;
                            log::warn(
                                "No colors in the color port, using velocity for coloring "
                                "instead ");
                        }
                        [[fallthrough]];
                    }
                default:
                    [[fallthrough]];
                case ColoringMethod::Velocity:
                    c = tf_.get().sample(d);
                    break;
            }

            indexBuffer->add(static_cast<std::uint32_t>(vertices.size()));

            vertices.push_back({pos, glm::normalize(v), pos, c});

            position++;
            velocity++;
            timestamp++;
        }
        indexBuffer->add(static_cast<std::uint32_t>(vertices.size() - 1));
    }

    mesh->addVertices(vertices);

    linesStripsMesh_.setData(mesh);

    util::curvature(*lines);
    util::tortuosity(*lines);

    lines_.setData(lines);
    maxVelocity_.set(toString(maxVelocity));
}

void PathLinesDeprecated::deserialize(Deserializer& d) {
    DoubleProperty dProperty("stepSize", "Step size", 0.001f, 0.001f, 1.0f, 0.001f);
    util::renameProperty(d, {{&dProperty, "dt"}});
    util::changePropertyType(
        d, {{&dProperty, std::string{PropertyTraits<FloatProperty>::classIdentifier()}}});
    Processor::deserialize(d);
}

}  // namespace inviwo
