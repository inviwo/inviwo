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

#include <modules/vectorfieldvisualization/processors/perpendicularsimilarity.h>
#include <inviwo/core/datastructures/image/layerramprecision.h>
#include <inviwo/core/datastructures/image/imageram.h>
#include <inviwo/core/datastructures/geometry/basicmesh.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo PerpendicularSimilarity::processorInfo_{
    "org.inviwo.PerpendicularSimilarity",  // Class identifier
    "Perpendicular Similarity",            // Display name
    "Undefined",                           // Category
    CodeState::Experimental,               // Code state
    Tags::None,                            // Tags
};
const ProcessorInfo PerpendicularSimilarity::getProcessorInfo() const { return processorInfo_; }

PerpendicularSimilarity::PerpendicularSimilarity()
    : Processor()
    , velocitySampler_("velocitySampler")
    , image_("image")
    , exampleLines_("exampleLines")
    , outputSize_("outputSize", "Output Size", {128, 128},
                  {{32, 32}, ConstraintBehavior::Immutable},
                  {{512, 512}, ConstraintBehavior::Ignore})
    , threshold_("maxAngle", "Max Angle", 0.1, {0.0000001, ConstraintBehavior::Immutable},
                 {3.14, ConstraintBehavior::Immutable})
    , perpendicularStep_("perpendicularStep", "Neighbor Step", 0.0001, 0.00000001, 0.1)
    , examplePixel_("examplePixel", "Example Pixel", {1, 1},
                    {{0, 0}, ConstraintBehavior::Immutable},
                    {{128, 128}, ConstraintBehavior::Mutable})
    , exampleScale_("exampleScale", "Example Scale", 1.0, 0.00001, 10.0) {

    addPorts(velocitySampler_, image_, exampleLines_);
    addProperties(outputSize_, threshold_, perpendicularStep_, examplePixel_, exampleScale_);
    outputSize_.onChange([&]() { examplePixel_.setMaxValue(outputSize_.get()); });
}

void PerpendicularSimilarity::process() {
    size2_t imageSize = outputSize_.get();
    size_t numCells = imageSize[0] * imageSize[1];
    auto sampler = velocitySampler_.getData();

    // float* data = new float[numCells];
    auto image =
        std::make_shared<Image>(size2_t(imageSize.x, imageSize.y), DataFormat<float>::get());

    auto layer = image->getColorLayer();
    if (!layer) return;

    auto layerRam =
        dynamic_cast<LayerRAMPrecision<float>*>(layer->getEditableRepresentation<LayerRAM>());
    if (!layerRam) return;

    auto data = layerRam->getDataTyped();
    if (!data) return;
    std::fill(data, data + numCells, 0);

    // Make the mesh of example vectors.
    auto mesh = std::make_shared<ColoredMesh>();
    mesh->setModelMatrix(sampler->getModelMatrix());
    mesh->setWorldMatrix(sampler->getWorldMatrix());

    auto indexBuffer = mesh->addIndexBuffer(DrawType::Lines, ConnectivityType::None);
    unsigned int maxData = 0;
    SimilarityMeasure* measure = new AngleMeasure(threshold_.get());
    threshold_.setDisplayName(measure->getThresholdName());

    for (size_t y = 0; y < imageSize.y; ++y)
        for (size_t x = 0; x < imageSize.x; ++x) {
            bool isExample = (examplePixel_.get().x == x && examplePixel_.get().y == y);
            if (!image_.isConnected() && !isExample) continue;

            // Sample field.
            dvec2 samplePos = {double(x) / (imageSize.x - 1), double(y) / (imageSize.y - 1)};
            dvec2 sampled = sampler->sample(samplePos, CoordinateSpace::Data);
            if (glm::length2(sampled) == 0) {
                data[x + y * imageSize.x] = -1;
                continue;
            }

            // All computations will be done in reference to the first sample.
            measure->setReferenceVector(sampled);
            const dvec2 perpendicularVec =
                dvec2{measure->referenceVector_.y, -measure->referenceVector_.x} *
                perpendicularStep_.get();
            bool isSimilar = true;
            unsigned int numNeighbors = 0;
            if (isExample) {
                mesh->addVertices({{vec3(samplePos, 0.0), vec4(1)},
                                   {vec3(samplePos + sampled * exampleScale_.get(), 0.0), vec4(1)},
                                   {vec3(samplePos + perpendicularVec, 0.0), vec4(1, 0, 0, 1)},
                                   {vec3(samplePos - perpendicularVec, 0.0), vec4(1, 0, 0, 1)}});
                indexBuffer->add({0, 1, 2, 3});
            }

            while (isSimilar && numNeighbors < 10000) {
                for (int sign = -1; sign <= 1; sign += 2) {
                    dvec2 neighSamplePos =
                        samplePos + (perpendicularVec * double(sign * int(numNeighbors + 1)));
                    dvec2 neighSample = sampler->sample(neighSamplePos, CoordinateSpace::Data);
                    if (glm::length2(neighSample) == 0) {
                        isSimilar = false;
                        break;
                    }

                    double similarity = measure->similarity(neighSample);
                    if (similarity < 0) {
                        isSimilar = false;
                        break;
                    }

                    if (isExample) {
                        vec4 neighColor(similarity, similarity, similarity, 1.0);

                        neighSample.x = std::max(neighSample.x, 0.000001);
                        if (sign == -1) neighColor.x = 1;

                        mesh->addVertices(
                            {{vec3(neighSamplePos, 0.0), neighColor},
                             {vec3(neighSamplePos + neighSample * exampleScale_.get(), 0.0),
                              neighColor}});

                        indexBuffer->add(
                            {4 + numNeighbors * 4 + sign + 1, 4 + numNeighbors * 4 + sign + 2});
                    }
                }

                if (isSimilar) numNeighbors++;
            }

            data[x + y * imageSize.x] = numNeighbors;
            maxData = std::max(maxData, numNeighbors);
        }
    delete measure;

    std::cout << "  max val: " << maxData << std::endl;
    if (image_.isConnected()) {
        for (size_t p = 0; p < numCells; ++p) {
            data[p] /= maxData;
        }
    }

    image_.setData(image);
    exampleLines_.setData(mesh);
}

}  // namespace inviwo
