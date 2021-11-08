/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021 Inviwo Foundation
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

#include <modules/vectorfieldvisualization/processors/flowfieldprocessor.h>
#include <inviwo/core/datastructures/image/layerramprecision.h>
#include <modules/base/algorithm/randomutils.h>
#include <modules/base/algorithm/dataminmax.h>

namespace inviwo {

FlowField2DProcessor::FlowField2DProcessor()
    : sampler_("sampler")
    , ftleOut_("FTLE", DataVec3Float32::get(), false)
    , hallerField_("Hallers", false)
    , dispersionOut_("relativeDispersion", DataFloat32::get(), false)
    , linesOut_("pathLines")
    , properties_("properties", "Properties")
    , fieldSize_("fieldSize", "Field Size", {10, 10}, {1, 1}, {1024, 1024})
    , seedAngle_("seedAngle", "Seed Angle", 0, 0, 360)
    , seedEpsilon_("seedEpsilon", "Seed Offset in \% cell", 0.1, 0.001, 0.5) {

    addPort(sampler_);
    addPort(ftleOut_);
    addPort(hallerField_);
    addPort(dispersionOut_);
    addPort(linesOut_);
    addProperties(properties_, fieldSize_, seedEpsilon_, seedAngle_);

    properties_.normalizeSamples_.set(true);  //! Tracer::IsTimeDependent);
    properties_.normalizeSamples_.setCurrentStateAsDefault();
}

FlowField2DProcessor::~FlowField2DProcessor() {}

void FlowField2DProcessor::process() {
    auto sampler = sampler_.getData();
    auto lines_ =
        std::make_shared<IntegralLineSet>(sampler->getModelMatrix(), sampler->getWorldMatrix());

    Tracer tracer(sampler, properties_);
    size2_t fieldSize = fieldSize_.get();
    size_t numSeeds = fieldSize[0] * fieldSize[1];
    double angleRad = seedAngle_.get() * M_PI / 180;
    vec2 neighborStep = {seedEpsilon_.get() / fieldSize[0], seedEpsilon_.get() / fieldSize[1]};
    vec2 neighborStepAngled = {neighborStep[0] * std::cos(angleRad),
                               neighborStep[1] * std::sin(angleRad)};
    double seedDist = glm::length(neighborStep);

    size_t startID = 0;
    std::vector<vec3> seedList(numSeeds);
    for (size_t y = 0; y < fieldSize[1]; ++y)
        for (size_t x = 0; x < fieldSize[0]; ++x) {
            seedList[x + y * fieldSize[0]] = {(0.5f + x) / fieldSize[0], (0.5f + y) / fieldSize[1],
                                              0};
        }

    // Create image.
    auto dispersionImage = std::make_shared<Image>(fieldSize, DataFormat<float>::get());
    auto dispersionLayer = dispersionImage->getColorLayer();
    auto dispersionLayerRam = dynamic_cast<LayerRAMPrecision<float>*>(
        dispersionLayer->getEditableRepresentation<LayerRAM>());
    auto dispersionField = dispersionLayerRam->getDataTyped();
    if (!dispersionField) return;

    std::mutex mutex;
    util::forEachParallel(seedList, [&](const auto& p, size_t i) {
        IntegralLine baseLine = tracer.traceFrom(p);
        auto& points = baseLine.getPositions();
        auto size = points.size();
        if (!size) {
            ftleField[i] = 0;  //{0, 0, 0};
            // dispersionField[i] = 0;
            return;
        }

        // Neighbor line end distance.
        vec3 neighborSeed = {p.x + neighborStepAngled.x, p.y + neighborStepAngled.y, 0};

        IntegralLine neighborLine = tracer.traceFrom({neighborSeed});
        auto& neighborPoints = neighborLine.getPositions();
        if (neighborPoints.size() <= 1) {
            ftleField[i] = 0;  //{0, 0, 0};
            // dispersionField[i] = 0;
            return;
        }
        size_t numPoints = std::min(neighborPoints.size(), points.size());

        double endDist = glm::distance(points[numPoints - 1], neighborPoints[numPoints - 1]);
        double relativeDispersion = std::log(endDist / seedDist);
        // dispersionField[i] = float(relativeDispersion);
        // ftleField[i] = float(relativeDispersion * relativeDispersion);

        // if (size > 1) {
        //     std::lock_guard<std::mutex> lock(mutex);
        //     lines_->push_back(std::move(baseLine), i);
        // }
    });

    dispersionOut_.setData(ftleImage);

    // Create an image.
    auto ftleImage = std::make_shared<Image>(fieldSize, DataFormat<float>::get());
    auto ftleLayer = ftleImage->getColorLayer();
    auto ftleLayerRam =
        dynamic_cast<LayerRAMPrecision<float>*>(ftleLayer->getEditableRepresentation<LayerRAM>());
    auto ftleField = ftleLayerRam->getDataTyped();
    if (!ftleField) return;

    util::forEachParallel(seedList, [&](const auto& p, size_t i) {
        // Compute FTLE.
        std::array<IntegralLine, 4> ftleLines = {
            tracer.traceFrom({p.x - neighborStep.x, p.y, 0.001}),
            tracer.traceFrom({p.x + neighborStep.x, p.y, 0.001}),
            tracer.traceFrom({p.x, p.y - neighborStep.y, 0.001}),
            tracer.traceFrom({p.x, p.y + neighborStep.y, 0.001})};
        double minTime = ftleLines[0].getPositions().back().z;
        size_t minLength = ftleLines[0].getPositions().size();
        for (auto& line : ftleLines) {
            auto& points = line.getPositions();

            // If any one line failed, abort.
            if (!points.size()) {
                ftleField[i] = {0, 0, 0};
                return;
            }

            minTime = std::min(minTime, points.back().z);
            minLength = std::min(minLength, points.size());
        }

        // // if (minLength > 1) {
        // //     std::lock_guard<std::mutex> lock(mutex);
        // //     for (auto& line : lines) lines_->push_back(std::move(line), i);
        // // }

        if (minLength <= 1) {
            ftleField[i] = vec3(0, 0, 0);
            return;
        }
        // // ftleField[i] = {maxTime, maxTime, 0, 1};

        // Calculate FTLE: form right Cauchy Green tensor, take eigenvalue,
        // logarithm and normalize.
        mat2 J(
            ftleLines[1].getPositions()[minLength - 1] - ftleLines[0].getPositions()[minLength - 1],
            ftleLines[3].getPositions()[minLength - 1] -
                ftleLines[2].getPositions()[minLength - 1]);
        mat2 cauchy = glm::transpose(J) * J;  // Flip?
        double P = 0.5 * (cauchy[0][0] + cauchy[1][1]);
        double Q = (cauchy[1][0] * cauchy[0][1] - cauchy[0][0] * cauchy[1][1]);
        double root = std::sqrt(P * P + Q);
        double l0 = P + root;
        double l1 = P - root;
        double lambdaMax = std::max(l0, l1);
        double ftle = 1.0 / minTime * std::log(lambdaMax);

        ftleField[i] = vec3(ftle, 0, -ftle) / 10;
        // glm::distance(ftleLines[0].getPositions()[minLength - 1],
        //                              ftleLines[1].getPositions()[minLength - 1]) /
        //                minTime;

        for (size_t l = 0; l < 4; ++l) {
            if (ftleLines[l].getPositions().size() > 1) {
                std::lock_guard<std::mutex> lock(mutex);
                lines_->push_back(std::move(ftleLines[l]), i * 4 + l);
            }
        }
    });

    linesOut_.setData(lines_);

    // if (!ftleImage) {
    //     LogWarn("Invalid FTLE image.");
    //     std::cout << "Invalid FTLE image" << std::endl;
    //     return;
    // }

    // std::cout << "! Data format FTLE: " << ftleImage->getDataFormat()->getString() << std::endl;
    // auto minMax = util::layerMinMax(ftleLayer);
    // std::cout << "FTLE range: " << minMax.first << " = " << minMax.second << std::endl;
    ftleOut_.setData(ftleImage);
    // std::random_device rd;
    // std::mt19937 mt(rd());
    // std::uniform_real_distribution<float> r(0, 1);
    // auto randomImage = util::randomImage<float>(fieldSize, mt, r);
    // ftleOut_.setData(randomImage);
}

const ProcessorInfo FlowField2DProcessor::processorInfo_{
    "org.inviwo.FlowField2D",  // Class identifier
    "Flow Field 2D",           // Display name
    "Flow Field",              // Category
    CodeState::Stable,         // Code state
    Tags::CPU                  // Tags
};

const ProcessorInfo FlowField2DProcessor::getProcessorInfo() const {
    return ProcessorTraits<FlowField2DProcessor>::getProcessorInfo();
}

}  // namespace inviwo
