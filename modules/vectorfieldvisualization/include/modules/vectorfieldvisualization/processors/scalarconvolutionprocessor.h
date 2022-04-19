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
#include <inviwo/core/ports/volumeport.h>
#include <modules/vectorfieldvisualization/algorithms/integrallineoperations.h>
#include <modules/vectorfieldvisualization/integrallinetracer.h>

namespace inviwo {

/** \docpage{org.inviwo.ScalarConvolutionProcessor, Scalar Convolution Processor.h}
 * ![](org.inviwo.ScalarConvolutionProcessor.png?classIdentifier=org.inviwo.ScalarConvolutionProcessor)
 */
class IVW_MODULE_VECTORFIELDVISUALIZATION_API ScalarConvolutionProcessor : public Processor {
public:
    using VeloSampler = SpatialSampler<3, 3, double>;
    using ScalarSampler = SpatialSampler<3, 1, double>;
    using Tracer = IntegralLineTracer<VeloSampler>;

    ScalarConvolutionProcessor();
    virtual ~ScalarConvolutionProcessor() = default;

    void integrateLines();
    void gatherScalars();
    void computeGeometricScalars();
    void updateScalarOptions();
    void generateVolume();
    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    DataInport<VeloSampler> velocitySampler_;
    DataInport<ScalarSampler, 0> scalarSamplers_;
    IntegralLineSetOutport integralLines_;
    VolumeOutport convolutedVolume_;

    IntegralLineProperties integrationProperties_;

    IntSize3Property outputSize_;

    enum ConvolutionType { Sum, Max, Avg, None };
    TemplateOptionProperty<ConvolutionType> convolutionType_;
    OptionPropertyString baseScalar_;
    CompositeProperty metaData_;
    BoolProperty calculateCurvature_;
    BoolProperty calculateTortuosity_;

    std::shared_ptr<IntegralLineSet> savedLines_;
    bool integrationRequired_;

    struct Convoluter {
        virtual double getResult() { return result_; }
        virtual void addElement(double val) = 0;

    protected:
        double result_ = 0;
    };

    struct SumConvoluter : public Convoluter {
        virtual void addElement(double val) override { result_ += val; }
    };

    struct MaxConvoluter : public Convoluter {
        virtual void addElement(double val) override { result_ = std::max(result_, val); }
    };

    struct AvgConvoluter : public Convoluter {
        virtual double getResult() override { return result_ / count_; }
        virtual void addElement(double val) override {
            result_ += val;
            count_++;
        }

    private:
        size_t count_ = 0;
    };
};

}  // namespace inviwo
