/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#ifndef IVW_INTEGRALLINETRACERPROCESSOR_H
#define IVW_INTEGRALLINETRACERPROCESSOR_H

#include <modules/vectorfieldvisualization/vectorfieldvisualizationmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/processortraits.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/ports/datainport.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/util/utilities.h>
#include <inviwo/core/util/foreach.h>
#include <modules/vectorfieldvisualization/algorithms/integrallineoperations.h>
#include <modules/vectorfieldvisualization/integrallinetracer.h>
#include <modules/vectorfieldvisualization/ports/seedpointsport.h>

namespace inviwo {

template <typename Tracer>
class IntegralLineTracerProcessor : public Processor {
public:
    IntegralLineTracerProcessor();
    virtual ~IntegralLineTracerProcessor();

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;

private:
    DataInport<typename Tracer::Sampler> sampler_;
    SeedPointsInport<Tracer::Sampler::SpatialDimensions> seeds_;
    DataInport<typename Tracer::Sampler, 0> annotationSamplers_;

    IntegralLineSetOutport lines_;

    IntegralLineProperties properties_;

    CompositeProperty metaData_;
    BoolProperty calculateCurvature_;
    BoolProperty calculateTortuosity_;
};

template <typename Tracer>
IntegralLineTracerProcessor<Tracer>::IntegralLineTracerProcessor()
    : sampler_("sampler")
    , seeds_("seeds")
    , annotationSamplers_("annotationSamplers")
    , lines_("lines")
    , properties_("properties", "Properties")

    , metaData_("metaData", "Meta Data")
    , calculateCurvature_("calculateCurvature", "Calculate Curvature", false)
    , calculateTortuosity_("calculateTortuosity", "Calculate Tortuosity", false) {
    addPort(sampler_);
    addPort(seeds_);
    addPort(annotationSamplers_);
    addPort(lines_);

    addProperty(properties_);
    addProperty(metaData_);
    metaData_.addProperty(calculateCurvature_);
    metaData_.addProperty(calculateTortuosity_);

    properties_.normalizeSamples_.set(!Tracer::IsTimeDependent);
    properties_.normalizeSamples_.setCurrentStateAsDefault();

    annotationSamplers_.setOptional(true);
}

template <typename Tracer>
IntegralLineTracerProcessor<Tracer>::~IntegralLineTracerProcessor() {}

template <typename Tracer>
void IntegralLineTracerProcessor<Tracer>::process() {
    auto sampler = sampler_.getData();
    auto lines =
        std::make_shared<IntegralLineSet>(sampler->getModelMatrix(), sampler->getWorldMatrix());

    Tracer tracer(sampler, properties_);

    for (auto meta : annotationSamplers_.getSourceVectorData()) {
        auto key = meta.first->getProcessor()->getIdentifier();
        key = util::stripIdentifier(key);
        tracer.addMetaDataSampler(key, meta.second);
    }

    std::mutex mutex;
    size_t startID = 0;
    for (const auto &seeds : seeds_) {
        util::forEachParallel(*seeds, [&](const auto &p, size_t i) {
            IntegralLine line = tracer.traceFrom(p);
            auto size = line.getPositions().size();
            if (size > 1) {
                std::lock_guard<std::mutex> lock(mutex);
                lines->push_back(std::move(line), startID + i);
            }
        });
        startID += seeds->size();
    }

    if (calculateCurvature_) {
        util::curvature(*lines);
    }
    if (calculateTortuosity_) {
        util::tortuosity(*lines);
    }

    lines_.setData(lines);
}

using StreamLines2D = IntegralLineTracerProcessor<StreamLine2DTracer>;
using StreamLines3D = IntegralLineTracerProcessor<StreamLine3DTracer>;
using PathLines3D = IntegralLineTracerProcessor<PathLine3DTracer>;

template <>
struct ProcessorTraits<StreamLines2D> {
    static ProcessorInfo getProcessorInfo() {
        return {
            "org.inviwo.StreamLines2D",  // Class identifier
            "Stream Lines 2D",           // Display name
            "Integral Line Tracer",      // Category
            CodeState::Stable,           // Code state
            Tags::CPU                    // Tags
        };
    }
};

template <>
struct ProcessorTraits<StreamLines3D> {
    static ProcessorInfo getProcessorInfo() {
        return {
            "org.inviwo.StreamLines3D",  // Class identifier
            "Stream Lines 3D",           // Display name
            "Integral Line Tracer",      // Category
            CodeState::Stable,           // Code state
            Tags::CPU                    // Tags
        };
    }
};

template <>
struct ProcessorTraits<PathLines3D> {
    static ProcessorInfo getProcessorInfo() {
        return {
            "org.inviwo.PathLines3D",  // Class identifier
            "Path Lines 3D",           // Display name
            "Integral Line Tracer",    // Category
            CodeState::Stable,         // Code state
            Tags::CPU                  // Tags
        };
    }
};

template <typename Tracer>
const ProcessorInfo IntegralLineTracerProcessor<Tracer>::getProcessorInfo() const {
    return ProcessorTraits<IntegralLineTracerProcessor<Tracer>>::getProcessorInfo();
}

}  // namespace inviwo

#endif  // IVW_INTEGRALLINETRACERPROCESSOR_H
