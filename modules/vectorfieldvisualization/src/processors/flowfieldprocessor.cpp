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
    , imageOut_("field", false)  //  DataVec3Float32::get(),
    // , hallerField_("Hallers", false)
    // , dispersionOut_("relativeDispersion", DataFloat32::get(), false)
    , linesOut_("pathLines")
    , measureType_("measureType", "Measure",
                   {{"uv", "UV", MeasureType::UV},
                    {"ftle", "FTLE", MeasureType::FTLE},
                    {"dispersion", "Relative Dispersion", MeasureType::RelativeDispersion},
                    {"flowMap", "Flow Map", MeasureType::FlowMap},
                    {"TSE", "TSE", MeasureType::TSE},
                    {"TSE_line", "TSE line", MeasureType::TSE_line},
                    {"pTSE", "TSE / v0", MeasureType::paramTSE},
                    {"pTSE_line", "TSE line / v0", MeasureType::paramTSE_line},
                    {"paramRelativeDispersion", "Relative Dispersion / angle",
                     MeasureType::paramRelativeDispersion}},
                   0, InvalidationLevel::InvalidResources)
    , properties_("properties", "Properties")
    , fieldSize_("fieldSize", "Field Size", {64, 64}, {32, 32}, {1024, 1024})
    , fieldSubSize_("fieldSubSize", "Sample Size", 0.5, 0, 1)
    , seedAngle_("seedAngle", "Seed Angle", 0, 0, 360)
    , seedEpsilon_("seedEpsilon", "Seed Offset in \% cell", 0.1, 0.001, 0.5)
    , v0_("v0", "v0", 1.0, {0.0, ConstraintBehavior::Ignore}, {10.0, ConstraintBehavior::Ignore})
    , dataRange_("dataRange", "Data Range", dvec2(0.1, 0.1),
                 std::pair<dvec2, ConstraintBehavior>{dvec2(0.0), ConstraintBehavior::Ignore},
                 std::pair<dvec2, ConstraintBehavior>{dvec2(1.0), ConstraintBehavior::Ignore},
                 dvec2(0.1), InvalidationLevel::InvalidOutput, PropertySemantics::Text)
    , normalize_("normalize", "Normalize?", false)
    , logScale_("logScale", "Log Scale?", false) {

    addPort(sampler_);
    addPort(imageOut_);
    // addPort(hallerField_);
    // addPort(dispersionOut_);
    addPort(linesOut_);
    addProperties(measureType_, properties_, fieldSize_, fieldSubSize_, seedEpsilon_, seedAngle_,
                  v0_, dataRange_, normalize_, logScale_);

    properties_.normalizeSamples_.set(true);  //! Tracer::IsTimeDependent);
    properties_.normalizeSamples_.setCurrentStateAsDefault();
    // dataRange_.setReadOnly(true);
}

FlowField2DProcessor::~FlowField2DProcessor() {}

void FlowField2DProcessor::process() {
    auto sampler = sampler_.getData();
    auto lines_ =
        std::make_shared<IntegralLineSet>(sampler->getModelMatrix(), sampler->getWorldMatrix());

    if (measureType_.get() == MeasureType::TSE || measureType_.get() == MeasureType::TSE_line) {
        properties_.normalizeSamples_.set(false);
    }
    Tracer tracer(sampler, properties_);

    size2_t fieldSize = fieldSize_.get();
    size_t numSeeds = fieldSize[0] * fieldSize[1];
    double angleRad = seedAngle_.get() * M_PI / 180;
    vec2 neighborStep = {seedEpsilon_.get() / fieldSize[0], seedEpsilon_.get() / fieldSize[1]};
    vec2 neighborStepAngled = {neighborStep[0] * std::cos(angleRad),
                               neighborStep[1] * std::sin(angleRad)};
    double seedDist = glm::length(neighborStep);
    double seedPercent = fieldSubSize_.get();

    size_t startID = 0;
    bool isScanline = static_cast<int>(measureType_.get()) >= int(MeasureType::PARAM_TYPES);
    std::cout << "= Is scanline? " << (isScanline ? "YES" : "NO") << std::endl;
    std::vector<vec3> seedList(isScanline ? fieldSize.x : numSeeds);

    for (size_t y = 0; y < fieldSize[1]; ++y) {
        for (size_t x = 0; x < fieldSize[0]; ++x) {
            seedList[x + y * fieldSize[0]] = {seedPercent * ((0.5f + x) / fieldSize[0] - 0.5) + 0.5,
                                              seedPercent * ((0.5f + y) / fieldSize[1] - 0.5) + 0.5,
                                              0};
        }
        if (isScanline) break;
    }

    // Create image for vec3 data.
    vec3* vecField;
    std::shared_ptr<Image> image;

    image = std::make_shared<Image>(fieldSize, DataFormat<vec3>::get());
    auto vecLayer = image->getColorLayer();
    auto vecLayerRam =
        dynamic_cast<LayerRAMPrecision<vec3>*>(vecLayer->getEditableRepresentation<LayerRAM>());

    vecField = vecLayerRam->getDataTyped();
    if (!vecField) return;

    std::mutex mutex;

    // Fill the image depending on selected measure.
    switch (measureType_.get()) {

        // ====================== UV coordinates ======================
        case MeasureType::UV:
            util::forEachParallel(seedList, [&](const auto& p, size_t i) {
                vecField[i] = {p.x, p.y, 0};
            });
            break;

        // ========================= Flow Map =========================
        case MeasureType::FlowMap:
            util::forEachParallel(seedList, [&](const auto& p, size_t i) {
                IntegralLine baseLine = tracer.traceFrom(p);
                auto& points = baseLine.getPositions();
                auto size = points.size();
                if (!size) {
                    vecField[i] = {0, 0, 0};
                    return;
                }

                vecField[i] = {float(points.back().x), float(points.back().y),
                               float(points.back().z)};

                if (size > 1) {
                    std::lock_guard<std::mutex> lock(mutex);
                    lines_->push_back(std::move(baseLine), i);
                }
            });

            break;

        // ==================== Relative Dispersion ====================
        case MeasureType::RelativeDispersion:
            util::forEachParallel(seedList, [&](const auto& p, size_t i) {
                IntegralLine baseLine = tracer.traceFrom(p);
                auto& points = baseLine.getPositions();
                auto size = points.size();
                if (!size) {
                    vecField[i] = {0, 0, 0};
                    return;
                }

                // Neighbor line end distance.
                vec3 neighborSeed = {p.x + neighborStepAngled.x, p.y + neighborStepAngled.y, 0};

                IntegralLine neighborLine = tracer.traceFrom({neighborSeed});
                auto& neighborPoints = neighborLine.getPositions();
                if (neighborPoints.size() <= 1) {
                    vecField[i] = {0, 0, 0};
                    return;
                }
                size_t numPoints = std::min(neighborPoints.size(), points.size());

                double endDist =
                    glm::distance(vec2(points[numPoints - 1]), vec2(neighborPoints[numPoints - 1]));
                double relativeDispersion = std::log(endDist / seedDist);
                vecField[i] = {float(relativeDispersion), 0, 0};

                if (size > 1) {
                    std::lock_guard<std::mutex> lock(mutex);
                    lines_->push_back(std::move(baseLine), i);
                }
            });

            break;

        // ====================== FTLE ======================
        case MeasureType::FTLE:
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
                        vecField[i] = {0, 0, 0};
                        return;
                    }

                    minTime = std::min(minTime, points.back().z);
                    minLength = std::min(minLength, points.size());
                }

                if (minLength <= 1) {
                    vecField[i] = vec3(0, 0, 0);
                    return;
                }

                // Calculate FTLE: form right Cauchy Green tensor, take eigenvalue,
                // logarithm and normalize.
                mat2 J(ftleLines[1].getPositions()[minLength - 1] -
                           ftleLines[0].getPositions()[minLength - 1],
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

                vecField[i] = vec3(lambdaMax, 0, 0);  // vec3(ftle, 0, -ftle) / 10;

                for (size_t l = 0; l < 4; ++l) {
                    if (ftleLines[l].getPositions().size() > 1) {
                        std::lock_guard<std::mutex> lock(mutex);
                        lines_->push_back(std::move(ftleLines[l]), i * 4 + l);
                    }
                }
            });
            break;

        // ====================== Haller's TSE ======================
        case MeasureType::TSE:
            util::forEachParallel(seedList, [&](const auto& p, size_t i) {
                IntegralLine baseLine = tracer.traceFrom(p);
                auto& points = baseLine.getPositions();
                auto size = points.size();
                if (size < 3) {
                    vecField[i] = {0, 0, 0};
                    return;
                }

                double timeDelta = points.back().z - points[0].z;
                double derivative0 = glm::length2(vec2(points[1] - points[0]));
                double derivativeN = glm::length2(vec2(points[size - 1] - points[size - 2]));
                double v0 = v0_.get();

                double tse = 1.0 / timeDelta *
                             std::log(std::sqrt((derivativeN + v0 * v0) / (derivative0 + v0 * v0)));
                vecField[i] = {tse, 0, 0};

                if (size > 1) {
                    std::lock_guard<std::mutex> lock(mutex);
                    lines_->push_back(std::move(baseLine), i);
                }
            });
            break;

        // ================ Haller's TSE with a line on top ================
        case MeasureType::TSE_line:
            util::forEachParallel(seedList, [&](const auto& p, size_t i) {
                IntegralLine baseLine = tracer.traceFrom(p);
                auto& points = baseLine.getPositions();
                auto size = points.size();
                if (!size) {
                    vecField[i] = {0, 0, 0};
                    return;
                }

                double timeDelta = points.back().z - points[0].z;
                double v0 = v0_.get();
                double tse = 0;
                for (int point = 0; point < size - 1; ++point) {
                    int leftNeigh = std::max(0, point - 1);
                    double derivativeI = glm::length(vec2(points[point + 1] - points[leftNeigh]));

                    int rightNeigh = std::min(int(size - 1), point + 2);
                    double derivativeIplus = glm::length(vec2(points[rightNeigh] - points[point]));

                    tse +=
                        std::abs(std::log(std::sqrt((derivativeIplus * derivativeIplus + v0 * v0) /
                                                    (derivativeI * derivativeI + v0 * v0))));
                }

                vecField[i] = {1.0 / timeDelta * tse, 0, 0};

                if (size > 1) {
                    std::lock_guard<std::mutex> lock(mutex);
                    lines_->push_back(std::move(baseLine), i);
                }
            });
            break;

        // ================== Haller's TSE over v0 ==================
        case MeasureType::paramTSE:
            util::forEachParallel(seedList, [&](const auto& p, size_t i) {
                vec3 seed(p.x, 0.5, p.z);
                IntegralLine baseLine = tracer.traceFrom(seed);
                auto& points = baseLine.getPositions();
                auto size = points.size();
                if (size < 3) {
                    vecField[i] = {0, 0, 0};
                    return;
                }

                double timeDelta = points.back().z - points[0].z;
                double derivative0 = glm::length2(vec2(points[1] - points[0]));
                double derivativeN = glm::length2(vec2(points[size - 1] - points[size - 2]));

                for (size_t v = 0; v < fieldSize.y; ++v) {
                    double v0 = v0_.get() * (v + 1) / fieldSize.y;
                    double tse =
                        1.0 / timeDelta *
                        std::log(std::sqrt((derivativeN + v0 * v0) / (derivative0 + v0 * v0)));
                    vecField[i + fieldSize.x * v] = {tse, 0, 0};
                }

                if (size > 1) {
                    std::lock_guard<std::mutex> lock(mutex);
                    lines_->push_back(std::move(baseLine), i);
                }
            });
            break;

        // ============ Haller's TSE with a line on top over v0 ============
        case MeasureType::paramTSE_line:
            util::forEachParallel(seedList, [&](const auto& p, size_t i) {
                vec3 seed(p.x, 0.5, p.z);
                IntegralLine baseLine = tracer.traceFrom(seed);
                auto& points = baseLine.getPositions();
                auto size = points.size();
                if (size < 3) {
                    vecField[i] = {0, 0, 0};
                    return;
                }

                double timeDelta = points.back().z - points[0].z;

                for (size_t v = 0; v < fieldSize.y; ++v) {
                    double v0 = v0_.get() * (v + 1) / fieldSize.y;
                    double tse = 0;

                    for (int point = 0; point < size - 1; ++point) {
                        int leftNeigh = std::max(0, point - 1);
                        double derivativeI =
                            glm::length(vec2(points[point + 1] - points[leftNeigh]));

                        int rightNeigh = std::min(int(size - 1), point + 2);
                        double derivativeIplus =
                            glm::length(vec2(points[rightNeigh] - points[point]));

                        tse += std::abs(
                            std::log(std::sqrt((derivativeIplus * derivativeIplus + v0 * v0) /
                                               (derivativeI * derivativeI + v0 * v0))));
                    }

                    vecField[i + fieldSize.x * v] = {1.0 / timeDelta * tse, 0, 0};
                }

                if (size > 1) {
                    std::lock_guard<std::mutex> lock(mutex);
                    lines_->push_back(std::move(baseLine), i);
                }
            });
            break;

        // ==================== Relative Dispersion over angle ====================
        case MeasureType::paramRelativeDispersion:
            util::forEachParallel(seedList, [&](const auto& p, size_t i) {
                vec3 seed(p.x, 0.5, p.z);
                IntegralLine baseLine = tracer.traceFrom(seed);
                auto& points = baseLine.getPositions();
                auto size = points.size();
                if (!size) {
                    vecField[i] = {0, 0, 0};
                    return;
                }

                for (size_t a = 0; a < fieldSize.y; ++a) {
                    double seedAngle = M_PI * 2 * a / fieldSize.y;
                    vec2 seedStepAngled = {neighborStep[0] * std::cos(seedAngle),
                                           neighborStep[1] * std::sin(seedAngle)};

                    // Neighbor line end distance.
                    vec3 neighborSeed = {seed.x + seedStepAngled.x, seed.y + seedStepAngled.y, 0};

                    IntegralLine neighborLine = tracer.traceFrom({neighborSeed});
                    auto& neighborPoints = neighborLine.getPositions();
                    if (neighborPoints.size() <= 1) {
                        vecField[i] = {0, 0, 0};
                        return;
                    }
                    size_t numPoints = std::min(neighborPoints.size(), points.size());

                    double endDist = glm::distance(vec2(points[numPoints - 1]),
                                                   vec2(neighborPoints[numPoints - 1]));
                    double relativeDispersion = std::log(endDist / seedDist);
                    vecField[i + fieldSize.x * a] = {float(relativeDispersion), 0, 0};
                }

                if (size > 1) {
                    std::lock_guard<std::mutex> lock(mutex);
                    lines_->push_back(std::move(baseLine), i);
                }
            });

            break;

        default:
            break;
    }

    if (!image) {
        imageOut_.detachData();
        return;
    }

    if (logScale_.get()) {
        for (size_t idx = 0; idx < fieldSize.x * fieldSize.y; ++idx) {
            for (size_t i = 0; i < 3; ++i) {
                float& y = vecField[idx][i];
                y = (std::signbit(y) ? 1 : -1) * std::log10(1 + std::abs(y * std::log(10)));
                // y = std::log10(std::max(y, 0.0f));
            }
        }
    }

    auto minMax = util::layerMinMax(vecLayer);

    vec3 min = minMax.first;
    vec3 extent = vec3(minMax.second) - min;

    for (size_t i = 0; i < 3; ++i) {
        extent[i] = std::max(0.001f, extent[i]);
    }

    dataRange_.setReadOnly(false);
    dataRange_.set(dvec2{minMax.first[0], minMax.second[0]});
    dataRange_.setReadOnly(true);

    if (normalize_.get()) {
        for (size_t idx = 0; idx < fieldSize.x * fieldSize.y; ++idx) {
            vecField[idx] = (vecField[idx] - min) / extent;
        }
    }

    minMax = util::layerMinMax(vecLayer);
    // std::cout << "Min: " << minMax.first << " - max: " << minMax.second
    //           << " - logExtent: " << extent << std::endl;

    linesOut_.setData(lines_);
    imageOut_.detachData();
    imageOut_.setData(image);
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
