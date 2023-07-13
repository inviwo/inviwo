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

#include <modules/vectorfieldvisualization/processors/currents/highvelocitylineregions.h>
#include <modules/base/algorithm/randomutils.h>
#include <modules/base/algorithm/dataminmax.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>

namespace inviwo {

HighVelocityLineRegions::HighVelocityLineRegions()
    : velocitySampler_("velocity")
    , scalarSampler_("scalar")
    , volumeOut_("field")
    , linesOut_("pathLines")
    , measureType_(
          "measureType", "Measure",
          {
              {"MaxScalar", "MaxScalar", MeasureType::MaxScalar},
              {"ScalarMinLengthThreshold", "Scalar Min Length Threshold",
               MeasureType::ScalarMinLengthThreshold},
              {"MaxScalarForLength", "Max Scalar For Length", MeasureType::MaxScalarForLength},
          },
          0, InvalidationLevel::InvalidResources)
    , integrationProperties_("properties", "integration Properties")
    , fieldSize_("fieldSize", "Field Size", {64, 64, 64},
                 {{32, 32, 32}, ConstraintBehavior::Immutable},
                 {{1024, 1024, 1024}, ConstraintBehavior::Ignore})
    , fieldSubSize_("fieldSubSize", "Sample Size", 0.5, 0, 1)
    , dataRange_("dataRange", "Data Range", dvec2(0.1, 0.1),
                 std::pair<dvec2, ConstraintBehavior>{dvec2(0.0), ConstraintBehavior::Ignore},
                 std::pair<dvec2, ConstraintBehavior>{dvec2(1.0), ConstraintBehavior::Ignore},
                 dvec2(0.1), InvalidationLevel::InvalidOutput, PropertySemantics::Text)
    , scalarThreshold_("scalarThreshold", "Scalar Threshold", 0.5, 0.0, 1.0)
    , filterLines_("filterLines", "Filter Lines?", true)
    , topLinesOnly_("topLinesOnly", "Line in Top Slice only?", false) {

    addPorts(velocitySampler_, scalarSampler_, volumeOut_, linesOut_);
    addProperties(measureType_, integrationProperties_, fieldSize_, fieldSubSize_, dataRange_,
                  scalarThreshold_, filterLines_, topLinesOnly_);
}

HighVelocityLineRegions::~HighVelocityLineRegions() {}

void HighVelocityLineRegions::process() {
    auto veloSampler = velocitySampler_.getData();
    auto lines_ = std::make_shared<IntegralLineSet>(veloSampler->getModelMatrix(),
                                                    veloSampler->getWorldMatrix());
    bool forceBidirectional = (measureType_.get() == MeasureType::ScalarMinLengthThreshold ||
                               measureType_.get() == MeasureType::ScalarMinLengthThreshold);
    if (forceBidirectional) {
        integrationProperties_.stepDirection_.set(IntegralLineProperties::Direction::BOTH);
    }
    integrationProperties_.stepDirection_.setVisible(!forceBidirectional);
    Tracer tracer(veloSampler, integrationProperties_);
    tracer.addMetaDataSampler("scalar", scalarSampler_.getData());
    size3_t fieldSize = fieldSize_.get();
    size_t numSeeds = fieldSize.x * fieldSize.y * fieldSize.z;

    // double seedPercent = fieldSubSize_.get();

    size_t startID = 0;
    std::vector<vec3> seedList(numSeeds);

    for (size_t z = 0; z < fieldSize.z; ++z) {
        for (size_t y = 0; y < fieldSize.y; ++y) {
            for (size_t x = 0; x < fieldSize.x; ++x) {
                seedList[x + y * fieldSize.x + z * fieldSize.x * fieldSize.y] = {
                    (0.5f + x) / fieldSize.x,
                    (0.5f + y) / fieldSize.y,
                    (0.5f + z) / fieldSize.z,
                };
            }
        }
    }

    // Create volume for vec3 data.
    auto ramData = std::make_shared<VolumeRAMPrecision<float>>(fieldSize);
    float* vecField = ramData->getDataTyped();
    if (!vecField) return;

    size_t requiredLineLength = integrationProperties_.numberOfSteps_ / 2;

    std::mutex mutex;
    // Fill the image depending on selected measure.
    switch (measureType_.get()) {

        // ========================= Max Scalar =========================
        // Maximum scalar value along line.
        case MeasureType::MaxScalar:

            util::forEachParallel(seedList, [&](const auto& p, size_t i) {
                IntegralLine baseLine = tracer.traceFrom(p);
                // auto& points = baseLine.getPositions();
                // auto size = points.size();
                // if (!size) {
                //     vecField[i] = 0.0f;
                //     return;
                // }
                const auto& scalars = baseLine.getMetaData<dvec3>("scalar");
                auto maxValueIter =
                    std::max_element(scalars.begin(), scalars.end(),
                                     [](const dvec3& a, const dvec3& b) { return a[0] < b[0]; });
                bool validLine = (maxValueIter != scalars.end());
                vecField[i] = validLine ? (*maxValueIter)[0] : 0.0;

                if (validLine && (!filterLines_.get() || vecField[i] > scalarThreshold_.get()) &&
                    (!topLinesOnly_.get() || i < fieldSize.x * fieldSize.y)) {
                    std::lock_guard<std::mutex> lock(mutex);
                    lines_->push_back(std::move(baseLine), i);
                }
            });

            break;

            // ========================= Max Scalar =========================
            // Binary. Does the line have a piece with length integrationProperties_.numberOfSteps
            // of values above scalarThreshold_?
        case MeasureType::ScalarMinLengthThreshold:

            util::forEachParallel(seedList, [&](const auto& p, size_t i) {
                IntegralLine baseLine = tracer.traceFrom(p);
                // auto& points = baseLine.getPositions();

                const auto& scalars = baseLine.getMetaData<dvec3>("scalar");
                auto size = scalars.size();
                if (!size) {
                    vecField[i] = 0.0f;
                    return;
                }

                size_t maxLength = 0;
                for (const dvec3& scalar : scalars) {
                    if (scalar.x < scalarThreshold_.get()) {
                        maxLength = 0;
                        continue;
                    }
                    maxLength++;
                    if (maxLength >= requiredLineLength) break;
                }
                bool validLine = (maxLength >= requiredLineLength);
                vecField[i] = validLine ? 1.0f : 0.0f;

                if ((!filterLines_.get() || validLine) &&
                    (!topLinesOnly_.get() || i < fieldSize.x * fieldSize.y)) {
                    std::lock_guard<std::mutex> lock(mutex);
                    lines_->push_back(std::move(baseLine), i);
                }
            });

            break;

            // ========================= None of the above =========================
        default:
            break;
    }

    std::shared_ptr<Volume> volume = std::make_shared<Volume>(ramData);
    if (!volume) {
        volumeOut_.detachData();
        return;
    }
    volume->setModelMatrix(veloSampler->getModelMatrix());
    volume->setWorldMatrix(veloSampler->getWorldMatrix());
    volume->setBasis(veloSampler->getBasis());

    // if (logScale_.get()) {
    //     for (size_t idx = 0; idx < fieldSize.x * fieldSize.y; ++idx) {
    //         for (size_t i = 0; i < 3; ++i) {
    //             float& y = vecField[idx][i];
    //             y = (std::signbit(y) ? 1 : -1) * std::log10(1 + std::abs(y * std::log(10)));
    //             // y = std::log10(std::max(y, 0.0f));
    //         }
    //     }
    // }
    auto minMax = util::volumeMinMax(ramData.get());
    dvec2 dataRange = dvec2{minMax.first[0], minMax.second[0]};
    dataRange_.setReadOnly(false);
    dataRange_.set(dataRange);
    dataRange_.setReadOnly(true);

    volume->dataMap_.dataRange = dataRange;
    volume->dataMap_.valueRange = dataRange;
    // float min = minMax.first[0];
    // float extent = minMax.second[0] - min;
    // extent = std::max(0.001f, extent);
    // if (normalize_.get()) {
    //     for (size_t idx = 0; idx < fieldSize.x * fieldSize.y; ++idx) {
    //         vecField[idx] = (vecField[idx] - min) / extent;
    //     }
    // }

    linesOut_.setData(lines_);
    volumeOut_.detachData();
    volumeOut_.setData(volume);
}

const ProcessorInfo HighVelocityLineRegions::processorInfo_{
    "org.inviwo.HighVelocityLineRegion",  // Class identifier
    "High Velocity Line Region",          // Display name
    "Flow Field",                         // Category
    CodeState::Experimental,              // Code state
    Tags::CPU                             // Tags
};

const ProcessorInfo HighVelocityLineRegions::getProcessorInfo() const {
    return ProcessorTraits<HighVelocityLineRegions>::getProcessorInfo();
}

}  // namespace inviwo
