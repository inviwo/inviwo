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

#include <modules/vectorfieldvisualization/processors/scalarconvolutionprocessor.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <inviwo/core/util/utilities.h>
#include <inviwo/core/util/foreach.h>
#include <inviwo/core/util/zip.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo ScalarConvolutionProcessor::processorInfo_{
    "org.inviwo.ScalarConvolutionProcessor",  // Class identifier
    "Scalar Convolution Processor.h",         // Display name
    "Undefined",                              // Category
    CodeState::Experimental,                  // Code state
    Tags::None,                               // Tags
};
const ProcessorInfo ScalarConvolutionProcessor::getProcessorInfo() const { return processorInfo_; }

ScalarConvolutionProcessor::ScalarConvolutionProcessor()
    : Processor()
    , velocitySampler_("velocitySampler")
    , scalarSamplers_("scalarSampler")
    , integralLines_("integralLines")
    , convolutedVolume_("convolutedVolume")

    , integrationProperties_("integrationProperties", "Integration Properties")
    // , separateOutputSize_("separateOutputSize", "Separate Output Size")
    , outputSize_("outputSize", "Output Size", {32, 32, 32},
                  {{32, 32, 32}, ConstraintBehavior::Immutable},
                  {{512, 512, 512}, ConstraintBehavior::Ignore})
    , convolutionType_("convolutionType", "Convolution Type")
    , baseScalar_("baseScalar", "Base Scalar")
    , metaData_("metaData", "Meta Data")
    , calculateCurvature_("calculateCurvature", "Calculate Curvature", false)
    , calculateTortuosity_("calculateTortuosity", "Calculate Tortuosity", false)
    , integrationRequired_(true) {

    addPorts(velocitySampler_, scalarSamplers_, integralLines_, convolutedVolume_);
    addProperties(integrationProperties_, outputSize_, convolutionType_, baseScalar_, metaData_);
    scalarSamplers_.setOptional(true);
    metaData_.addProperties(calculateCurvature_, calculateTortuosity_);

    integrationProperties_.seedPointsSpace_.set(CoordinateSpace::Data);
    integrationProperties_.seedPointsSpace_.setVisible(false);

    convolutionType_.addOption("Sum", "Sum", ConvolutionType::Sum);
    convolutionType_.addOption("Avg", "Avg", ConvolutionType::Avg);
    convolutionType_.addOption("Max", "Max", ConvolutionType::Max);
    convolutionType_.addOption("None", "None", ConvolutionType::None);
    convolutionType_.setCurrentStateAsDefault();

    velocitySampler_.onChange([this]() { integrationRequired_ = true; });
    outputSize_.onChange([this]() { integrationRequired_ = true; });
    integrationProperties_.onChange([this]() { integrationRequired_ = true; });
}

void ScalarConvolutionProcessor::integrateLines() {
    std::cout << "== Integrating, maybe" << std::endl;
    if (!velocitySampler_.hasData()) return;
    auto veloSampler = velocitySampler_.getData();
    if (!veloSampler) return;
    std::cout << "== Integrating, actually" << std::endl;

    savedLines_ = std::make_shared<IntegralLineSet>(veloSampler->getModelMatrix(),
                                                    veloSampler->getWorldMatrix());

    Tracer tracer(veloSampler, integrationProperties_);
    size3_t fieldSize = outputSize_.get();
    size_t numSeeds = fieldSize[0] * fieldSize[1];
    std::vector<vec3> seedList(numSeeds);

    for (size_t z = 0; z < fieldSize[2]; ++z)
        for (size_t y = 0; y < fieldSize[1]; ++y) {
            // for (size_t x = 0; x < fieldSize[0]; ++x) {
            seedList[y + z * fieldSize[1]] = {0, (0.5f + y) / fieldSize[1],
                                              (0.5f + z) / fieldSize[2]};
        }

    std::mutex mutex;
    util::forEachParallel(seedList, [&](const auto& p, size_t i) {
        dvec3 seed = p;
        for (size_t x = 0; x < fieldSize[0]; ++x) {
            // std::vector<IntegralLine> lineList;
            // { size_t x = 0;
            seed.x = (0.5f + x) / fieldSize[0];

            IntegralLine line = tracer.traceFrom(seed);

            auto size = line.getPositions().size();
            if (size > 1) {
                auto& veloMag = line.getMetaData<double>("velocityMagnitude", true);
                veloMag.resize(size);
                for (auto&& point : util::zip(line.getMetaData<dvec3>("velocity"), veloMag)) {
                    get<1>(point) = glm::length(get<0>(point));
                }

                std::lock_guard<std::mutex> lock(mutex);
                savedLines_->push_back(std::move(line), i * fieldSize[0] + x);
            }
        }
    });
    std::cout << "== Integrating no more" << std::endl;
}

void ScalarConvolutionProcessor::computeGeometricScalars() {
    std::cout << "== Comuting geometry, maybe" << std::endl;
    if (!savedLines_ || savedLines_->size() == 0 || (*savedLines_)[0].getPositions().size() == 0)
        return;
    auto& refLine = (*savedLines_)[0];
    std::cout << "== Computing geometry, actually" << std::endl;

    if (calculateCurvature_ && !refLine.hasMetaData("curvature")) {
        util::curvature(*savedLines_);
    }
    if (calculateTortuosity_ && !refLine.hasMetaData("tortuosity")) {
        util::tortuosity(*savedLines_);
    }
}

void ScalarConvolutionProcessor::gatherScalars() {
    std::cout << "== Gathering scalars, maybe" << std::endl;
    if (!scalarSamplers_.hasData() || !scalarSamplers_.isConnected() || !savedLines_ ||
        savedLines_->size() == 0 || (*savedLines_)[0].getPositions().size() == 0)
        return;
    // auto& refLine = (*savedLines_)[0];

    std::cout << "== Gathering scalars, actually" << std::endl;
    for (auto meta : scalarSamplers_.getSourceVectorData()) {
        auto key = meta.first->getProcessor()->getIdentifier();
        key = util::stripIdentifier(key);
        // if (refLine.hasMetaData(key)) continue;

        for (auto& line : *savedLines_) {
            auto& metaData = line.getMetaData<double>(key, true);
            metaData.resize(line.getPositions().size());

            for (size_t p = 0; p < metaData.size(); ++p) {
                dvec3& pos = line.getPositions()[p];
                metaData[p] = meta.second->sample(pos);
            }
        }
    }
}

void ScalarConvolutionProcessor::updateScalarOptions() {
    if (!savedLines_ || savedLines_->size() == 0 || (*savedLines_)[0].getPositions().size() == 0)
        return;
    auto& refLine = (*savedLines_)[0];
    baseScalar_.replaceOptions(refLine.getMetaDataKeys());
    baseScalar_.removeOption("velocity");
}

void ScalarConvolutionProcessor::generateVolume() {
    std::cout << "== Generate volume, maybe" << std::endl;
    if (convolutionType_.get() == ConvolutionType::None || !savedLines_ ||
        savedLines_->size() == 0 || (*savedLines_)[0].getPositions().size() == 0)
        return;

    size3_t fieldSize = outputSize_.get();
    size_t numCells = fieldSize[0] * fieldSize[1] * fieldSize[2];

    float* data = new float[numCells];
    std::fill(data, data + numCells, 0);

    std::cout << "== Generate volume, actually" << std::endl;
    float min = 0;
    float max = 0;
    // std::mutex mutex;
    util::forEachParallel(*savedLines_, [&](const IntegralLine& line, size_t i) {
        size_t idx = line.getIndex();
        const auto& scalar = line.getMetaData<double>(baseScalar_.get());

        Convoluter* functor;
        switch (convolutionType_.get()) {
            case ConvolutionType::Sum:
                functor = new SumConvoluter();
                break;
            case ConvolutionType::Max:
                functor = new MaxConvoluter();
                break;
            default:
                functor = new AvgConvoluter();
                break;
        }
        for (double s : scalar) functor->addElement(s);
        data[idx] = float(functor->getResult());

        // Not very thread-safe, but we're fine with a rough range here.
        min = std::min(min, data[idx]);
        max = std::max(max, data[idx]);
        // if (data[i] < min) {
        //     std::lock_guard<std::mutex> lock(mutex);
        //     min = data[idx];
        // }
        // if (data[i] > max) {
        //     std::lock_guard<std::mutex> lock(mutex);
        //     max = data[idx];
        // }
    });
    // std::cout << "min: " << min << ", max: " << max << std::endl;

    auto ramData = std::make_shared<VolumeRAMPrecision<float>>(data, fieldSize);
    auto volume = std::make_shared<inviwo::Volume>(ramData);

    volume->setModelMatrix(savedLines_->getModelMatrix());
    volume->setWorldMatrix(savedLines_->getWorldMatrix());
    volume->dataMap_.dataRange = dvec2(min, max);
    volume->dataMap_.valueRange = dvec2(min, max);

    convolutedVolume_.setData(volume);
}

void ScalarConvolutionProcessor::process() {
    if (!velocitySampler_.getData()) {
        savedLines_ = nullptr;
        return;
    }
    std::cout << "== Processing" << std::endl;
    if (integrationRequired_) integrateLines();
    if (integrationRequired_ || metaData_.isModified()) computeGeometricScalars();
    if (integrationRequired_ || scalarSamplers_.isChanged()) gatherScalars();
    if (integrationRequired_ || metaData_.isModified() || scalarSamplers_.isChanged())
        updateScalarOptions();

    integrationRequired_ = false;

    generateVolume();

    std::cout << "== Setting lines" << std::endl;

    integralLines_.setData(savedLines_);
}

}  // namespace inviwo
