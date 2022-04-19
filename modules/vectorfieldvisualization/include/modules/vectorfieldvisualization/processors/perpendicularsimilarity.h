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
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/ports/meshport.h>
#include <modules/vectorfieldvisualization/algorithms/integrallineoperations.h>
#include <modules/vectorfieldvisualization/integrallinetracer.h>

namespace inviwo {

/** \docpage{org.inviwo.PerpendicularSimilarity, Scalar Convolution Processor.h}
 * ![](org.inviwo.PerpendicularSimilarity.png?classIdentifier=org.inviwo.PerpendicularSimilarity)
 */
class IVW_MODULE_VECTORFIELDVISUALIZATION_API PerpendicularSimilarity : public Processor {
public:
    using VeloSampler = SpatialSampler<2, 2, double>;

    PerpendicularSimilarity();
    virtual ~PerpendicularSimilarity() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    DataInport<VeloSampler> velocitySampler_;
    ImageOutport image_;
    MeshOutport exampleLines_;

    IntSize2Property outputSize_;
    DoubleProperty threshold_, perpendicularStep_;
    IntSize2Property examplePixel_;
    DoubleProperty exampleScale_;

    struct TmpMeasure {
        dvec2 referenceVector_;
        double threshold_;
    };

    struct SimilarityMeasure {
        SimilarityMeasure(double threshold) : threshold_(threshold) {}
        virtual ~SimilarityMeasure() = default;
        virtual void setReferenceVector(dvec2 refVec) = 0;
        virtual double similarity(dvec2 vec) const = 0;
        virtual std::string getThresholdName() const = 0;
        dvec2 referenceVector_;
        double threshold_;
    };
    struct AngleMeasure : public SimilarityMeasure {
        AngleMeasure(double threshold) : SimilarityMeasure(threshold) {}  // std::cos(threshold)
        virtual void setReferenceVector(dvec2 refVec) override {
            referenceVector_ = glm::normalize(refVec);
        }
        virtual double similarity(dvec2 vec) const override {
            double dot = glm::dot(glm::normalize(vec), referenceVector_);
            if (std::isnan(dot)) return -1;
            return (threshold_ - std::acos(dot)) / threshold_;
        }
        virtual std::string getThresholdName() const { return "Max Angle"; }
    };

    struct DistanceMeasure : public SimilarityMeasure {
        DistanceMeasure(double threshold) : SimilarityMeasure(threshold) {}  // std::cos(threshold)
        virtual void setReferenceVector(dvec2 refVec) override { referenceVector_ = refVec; }
        virtual double similarity(dvec2 vec) const override {
            double length = glm::length(referenceVector_ - vec);
            if (std::isnan(length)) return -1;
            return (threshold_ - length) / threshold_;
        }
        virtual std::string getThresholdName() const { return "Max Distance"; }
    };

    struct RelativeDistanceMeasure : public SimilarityMeasure {
        RelativeDistanceMeasure(double threshold)
            : SimilarityMeasure(threshold) {}  // std::cos(threshold)
        virtual void setReferenceVector(dvec2 refVec) override {
            referenceVector_ = refVec;
            referenceLength_ = glm::length(refVec);
        }
        virtual double similarity(dvec2 vec) const override {
            double length = glm::length(referenceVector_ - vec);
            if (std::isnan(length)) return -1;
            length /= referenceLength_;
            return (threshold_ - length) / threshold_;
        }
        virtual std::string getThresholdName() const { return "Max Distance"; }
        double referenceLength_;
    };
};

}  // namespace inviwo
