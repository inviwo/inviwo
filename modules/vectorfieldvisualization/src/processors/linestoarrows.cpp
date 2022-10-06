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

#include <modules/vectorfieldvisualization/processors/linestoarrows.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo LinesToArrows::processorInfo_{
    "org.inviwo.LinesToArrows",  // Class identifier
    "Lines To Arrows",           // Display name
    "Undefined",                 // Category
    CodeState::Experimental,     // Code state
    Tags::None,                  // Tags
};
const ProcessorInfo LinesToArrows::getProcessorInfo() const { return processorInfo_; }

LinesToArrows::LinesToArrows()
    : Processor()
    , linesIn_("lines")
    , arrowsOut_("arrowMesh")
    , arrowScale_("arrowScale", "ArrowScale", 1.0, 0, 10.0)
    , colorScale_("colorScale", "Color Map Max", 1.0,
                  std::make_pair<double, ConstraintBehavior>(0, ConstraintBehavior::Ignore),
                  std::make_pair<double, ConstraintBehavior>(2, ConstraintBehavior::Ignore))
    , transferFunction_("transferFunction", "Transfer Function")
    , normalizeLength_("normalizeLength", "Normalize length?", false)
    , colorSource_("colorSource", "Color Source",
                   {{"input", "Input", ColorSource::Input},
                    {"length", "Length", ColorSource::Length},
                    {"angle", "Angle", ColorSource::Angle}}) {

    transferFunction_.visibilityDependsOn(
        colorSource_, [](const auto& prop) { return prop == ColorSource::Length; });

    addPorts(linesIn_, arrowsOut_);
    addProperties(arrowScale_, colorScale_, transferFunction_, normalizeLength_, colorSource_);
}

void LinesToArrows::process() {
    if (!linesIn_.hasData()) {
        arrowsOut_.clear();
        return;
    }

    auto lineMesh = linesIn_.getData();
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

    // Get data from incoming mesh.
    const BufferBase *coords, *color;
    coords = lineMesh->getBuffer(BufferType::PositionAttrib);
    const auto* indices = lineMesh->getIndices(0);

    bool inputColor = (colorSource_.get() == ColorSource::Input);
    if (inputColor) {
        color = lineMesh->getBuffer(BufferType::ColorAttrib);
    }

    if (!indices || !coords || (inputColor && !color)) {
        arrowsOut_.clear();
        LogWarn("Mesh does not have the required buffers.");
        return;
    }

    // Setup arrow mesh.
    using NormalMesh = TypedMesh<buffertraits::PositionsBuffer, buffertraits::NormalBuffer,
                                 buffertraits::ColorsBuffer>;
    auto arrowMesh = std::make_shared<NormalMesh>(DrawType::Triangles, ConnectivityType::None);
    std::vector<NormalMesh::Vertex> vertices;
    vertices.reserve(coords->getSize() * basicArrowVertices.size() / 2);

    // Dispatch for position dimensions.
    auto coordBuffer = coords->getRepresentation<BufferRAM>();
    coordBuffer->dispatch<void, dispatching::filter::Floats>([&](auto coordData) {
        auto indexData = indices->getRAMRepresentation();
        const BufferRAMPrecision<vec4, BufferTarget::Data>* colorData;
        if (inputColor) {
            colorData = color->getRepresentation<BufferRAMPrecision<vec4, BufferTarget::Data>>();
        }
        if (!indexData || !coordData || (inputColor && !colorData)) {
            arrowsOut_.clear();
            LogWarn("Buffers do not have the expected type (assuming 3D coordinates).");
            return;
        }

        mat4 linesToWorldMat = lineMesh->getCoordinateTransformer().getMatrix(
            CoordinateSpace::Data, CoordinateSpace::World);

        for (size_t idx = 1; idx < indices->getSize(); idx += 2) {
            uint32_t idx0 = indexData->get(idx);
            uint32_t idx1 = indexData->get(idx - 1);

            vec3 origin = util::glm_convert<vec3>(coordData->get(idx0));
            origin = vec3(linesToWorldMat * vec4(origin, 1.0));
            vec3 vector = util::glm_convert<vec3>(coordData->get(idx1));
            vector = vec3(linesToWorldMat * vec4(vector, 1.0)) - origin;

            double vectorLength = glm::length(vector);
            double originalLength = vectorLength;
            if (normalizeLength_.get()) vectorLength = 1.0;
            mat4 transform = glm::lookAtLH(vec3(0), vector, vec3(0, 0, 1));
            mat4 normalTransform = glm::inverseTranspose(transform);
            transform *= glm::scale(vec3(arrowScale_.get() * vectorLength));

            // Set a start and end color based on the selected source.
            vec4 startColor, endColor;
            switch (colorSource_.get()) {
                case ColorSource::Angle:;
                    startColor = endColor =
                        vec4(glm::normalize(vec2(vector.x, vector.y)) * 0.5 + vec2(0.5), 0, 1);
                    break;
                case ColorSource::Length:
                    startColor = endColor =
                        transferFunction_.get().sample(originalLength / colorScale_.get());
                    break;
                case ColorSource::Input:
                    [[fallthrough]];
                default:
                    startColor = colorData->get(idx0);
                    endColor = colorData->get(idx1);
                    break;
            }

            // Populate mesh.
            for (size_t i = 0; i < basicArrowNormals.size(); ++i) {
                vec3 normal = -vec4(basicArrowNormals[i], 1) * normalTransform;
                for (size_t v = 0; v < 3; ++v) {
                    vertices.push_back(
                        {origin + vec3(vec4(basicArrowVertices[basicArrowIndices[i * 3 + v]], 1) *
                                       transform),
                         normal,
                         glm::lerp(startColor, endColor,
                                   basicArrowVertices[basicArrowIndices[i * 3 + v]].z)});
                }
            }
        }
    });

    arrowMesh->addVertices(vertices);
    arrowsOut_.setData(arrowMesh);
}

}  // namespace inviwo
