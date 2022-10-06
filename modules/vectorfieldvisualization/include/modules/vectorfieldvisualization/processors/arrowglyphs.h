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

#pragma once

#include <modules/vectorfieldvisualization/vectorfieldvisualizationmoduledefine.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/transferfunctionproperty.h>
#include <inviwo/core/ports/meshport.h>
#include <inviwo/core/util/spatialsampler.h>
#include <inviwo/core/datastructures/geometry/typedmesh.h>

namespace inviwo {

/** \docpage{org.inviwo.ArrowGlyphs, Arrow Glyphs}
 * ![](org.inviwo.ArrowGlyphs.png?classIdentifier=org.inviwo.ArrowGlyphs)
 * Arrow glyphs seeded on a regular grid.
 */
template <unsigned int SpatialDims>
class IVW_MODULE_VECTORFIELDVISUALIZATION_API ArrowGlyphs : public Processor {
private:
    using IVec = glm::vec<SpatialDims, size_t>;
    using FVec = glm::vec<SpatialDims, float>;
    DataInport<SpatialSampler<SpatialDims, SpatialDims, double>> samplerIn_;
    MeshOutport meshOut_;

    OrdinalProperty<IVec> seedGridSize_;
    OrdinalProperty<FVec> seedGridOffset_, seedGridExtent_;
    TemplateOptionProperty<CoordinateSpace> sampleSpace_;
    DoubleProperty arrowScale_, colorScale_;
    TransferFunctionProperty transferFunction_;
    BoolProperty normalizeLength_;

public:
    ArrowGlyphs();
    virtual ~ArrowGlyphs() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;
};

template <unsigned int SpatialDims>
ArrowGlyphs<SpatialDims>::ArrowGlyphs()
    : Processor()
    , samplerIn_("spatialSampler")
    , meshOut_("arrowMeshes")
    , seedGridSize_("seedGridSize", "Seed Grid Size", IVec(10),
                    std::make_pair<IVec, ConstraintBehavior>(IVec(1), ConstraintBehavior::Ignore),
                    std::make_pair<IVec, ConstraintBehavior>(IVec(256), ConstraintBehavior::Ignore))
    , seedGridOffset_(
          "seedGridOffset", "Seed Grid Offset", FVec(0),
          std::make_pair<FVec, ConstraintBehavior>(FVec(-512), ConstraintBehavior::Ignore),
          std::make_pair<FVec, ConstraintBehavior>(FVec(512), ConstraintBehavior::Ignore))
    , seedGridExtent_(
          "seedGridExtent", "Seed Grid Extent", FVec(1),
          std::make_pair<FVec, ConstraintBehavior>(FVec(0.0001), ConstraintBehavior::Ignore),
          std::make_pair<FVec, ConstraintBehavior>(FVec(1024), ConstraintBehavior::Ignore))
    , sampleSpace_("sampleSpace", "Sample Space")
    , arrowScale_("arrowScale", "ArrowScale", 1.0, 0, 10.0)
    , colorScale_("colorScale", "Color Map Max", 1.0,
                  std::make_pair<double, ConstraintBehavior>(0, ConstraintBehavior::Ignore),
                  std::make_pair<double, ConstraintBehavior>(2, ConstraintBehavior::Ignore))
    , transferFunction_("transferFunction", "Transfer Function")
    , normalizeLength_("normalizeLength", "Normalize length?", false) {

    sampleSpace_.addOption("data", "Data", CoordinateSpace::Data);
    sampleSpace_.addOption("model", "Model", CoordinateSpace::Model);
    sampleSpace_.addOption("world", "World", CoordinateSpace::World);
    sampleSpace_.setCurrentStateAsDefault();

    addPorts(samplerIn_, meshOut_);
    addProperties(seedGridSize_, seedGridOffset_, seedGridExtent_, sampleSpace_, arrowScale_,
                  colorScale_, transferFunction_, normalizeLength_);
}

template <unsigned int SpatialDims>
void ArrowGlyphs<SpatialDims>::process() {
    if (!samplerIn_.hasData()) {
        meshOut_.clear();
        return;
    }

    auto sampler = samplerIn_.getData();
    static const std::array<vec3, 10> basicArrowVertices = {vec3(0, 0, 0),

                                                            vec3(0.072f, -0.072f, 0.72f),
                                                            vec3(0.072f, 0.072f, 0.72f),
                                                            vec3(-0.072f, 0.072f, 0.72f),
                                                            vec3(-0.072f, -0.072f, 0.72f),
                                                            vec3(0.185f, -0.185f, 0.72f),
                                                            vec3(0.185f, 0.185f, 0.72f),
                                                            vec3(-0.185f, 0.185f, 0.72f),
                                                            vec3(-0.185f, -0.185f, 0.72f),

                                                            vec3(0, 0, 1)};
    static const std::array<std::uint32_t, 3 * 10> basicArrowIndices = {
        0, 1, 2, 0, 2, 3, 0, 3, 4, 0, 4, 1, 5, 6, 7, 7, 8, 5, 9, 6, 5, 9, 7, 6, 9, 8, 7, 9, 5, 8};

    static const std::array<vec3, 10> basicArrowNormals = {
        vec3(-0.99, 0.00, 0.10),   vec3(0.00, -0.99, 0.10),  vec3(0.99, 0.00, 0.10),
        vec3(-0.00, 0.99, 0.10),   vec3(0.00, 0.00, 1.00),   vec3(0.00, 0.00, 1.00),
        vec3(-0.83, -0.00, -0.55), vec3(0.00, -0.83, -0.55), vec3(0.83, 0.00, -0.55),
        vec3(0.00, 0.83, -0.55)};

    // Seed grid sizes.
    size3_t seedDims{seedGridSize_.get().x, seedGridSize_.get().y, 1};
    if constexpr (SpatialDims >= 3) seedDims.z = seedGridSize_.get().z;
    size_t numSeeds = seedDims.x * seedDims.y * seedDims.z;

    using NormalMesh = TypedMesh<buffertraits::PositionsBuffer, buffertraits::NormalBuffer,
                                 buffertraits::ColorsBuffer>;
    auto arrowMesh = std::make_shared<NormalMesh>(DrawType::Triangles, ConnectivityType::None);
    std::vector<NormalMesh::Vertex> vertices;
    vertices.reserve(numSeeds * basicArrowIndices.size());

    const FVec& offset = seedGridOffset_.get();
    const FVec& extent = seedGridExtent_.get();
    float scale = arrowScale_.get();  // / minDim;
    mat4 sampleToWorldMat =
        sampler->getCoordinateTransformer().getMatrix(sampleSpace_.get(), CoordinateSpace::World);

    for (size_t z = 0; z < seedDims.z; ++z) {
        for (size_t y = 0; y < seedDims.y; ++y) {
            for (size_t x = 0; x < seedDims.x; ++x) {
                size3_t idx3D{x, y, z};
                FVec originND;
                for (size_t o = 0; o < SpatialDims; ++o)
                    originND[o] = offset[o] + (extent[o] * idx3D[o]) / (seedDims[o] - 1);
                vec3 origin(originND.x, originND.y, 0);
                if constexpr (SpatialDims >= 3) origin.z = originND.z;

                if (!sampler->withinBounds(originND, sampleSpace_.get())) continue;
                auto velocity = sampler->sample(originND, sampleSpace_.get());

                // originND = FVec(sampleToWorldMat * glm::vec<4, float>(origin, 1.0));
                // origin = {originND.x, originND.y, 0};
                // if constexpr (SpatialDims >= 3) origin.z = originND.z;
                origin = vec3(sampleToWorldMat * vec4(origin, 1.0));

                double velocityLength = glm::length(velocity);
                vec4 color = transferFunction_.get().sample(velocityLength / colorScale_.get());
                if (normalizeLength_.get()) velocityLength = 1.0;
                mat4 transform =
                    glm::lookAtLH(vec3(0), util::glm_convert<vec3>(velocity), vec3(0, 0, 1));
                mat4 normalTransform = glm::inverseTranspose(transform);
                transform *= glm::scale(vec3(scale * velocityLength));

                for (size_t i = 0; i < basicArrowNormals.size(); ++i) {
                    vec3 normal = -vec4(basicArrowNormals[i], 1) * normalTransform;
                    for (size_t v = 0; v < 3; ++v) {
                        vertices.push_back(
                            {origin +
                                 vec3(vec4(basicArrowVertices[basicArrowIndices[i * 3 + v]], 1) *
                                      transform),
                             normal, color});
                    }
                }
            }
        }
    }

    arrowMesh->addVertices(vertices);
    meshOut_.setData(arrowMesh);
}

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
template <unsigned int SpatialDims>
const ProcessorInfo ArrowGlyphs<SpatialDims>::processorInfo_{
    fmt::format("org.inviwo.ArrowGlyph{}D", SpatialDims),  // Class identifier
    fmt::format("Arrow Glyphs {}D", SpatialDims),          // Display name
    "Undefined",                                           // Category
    CodeState::Stable,                                     // Code state
    Tags::None,                                            // Tags
};

template <unsigned int SpatialDims>
const ProcessorInfo ArrowGlyphs<SpatialDims>::getProcessorInfo() const {
    return processorInfo_;
}

using ArrowGlyphs2D = ArrowGlyphs<2>;
using ArrowGlyphs3D = ArrowGlyphs<3>;

}  // namespace inviwo
