/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020 Inviwo Foundation
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

#include <inviwo/propertybasedtesting/propertybasedtestingmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/directoryproperty.h>
#include <inviwo/core/ports/imageport.h>

#include <filesystem>

namespace inviwo {

/** \docpage{org.inviwo.ImageComparator, Image Comparator}
 * ![](org.inviwo.ImageComparator.png?classIdentifier=org.inviwo.ImageComparator)
 * Explanation of how to use the processor.
 *
 * ### Inports
 *   * __<Inport1>__ <description>.
 *
 * ### Outports
 *   * __<Outport1>__ <description>.
 *
 * ### Properties
 *   * __<Prop1>__ <description>.
 *   * __<Prop2>__ <description>
 */
class IVW_MODULE_PROPERTYBASEDTESTING_API ImageComparator
	: public Processor
	, public ProcessorNetworkObserver {
public:
    ImageComparator();

    virtual void process() override;

    virtual void setNetwork(ProcessorNetwork* network) override;
    virtual void createReport();

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;
private:
    enum class ComparisonType { Diff, Perceptual, Local, Global };
	enum class ReductionType { MEAN, MAX, MIN, SUM };
	std::string reductionTypeName(const ReductionType& r) {
		#define CASE_VAL(p) case(ReductionType::p): return #p;
		switch(r) {
		CASE_VAL(MEAN)
		CASE_VAL(MAX)
		CASE_VAL(MIN)
		CASE_VAL(SUM)
		};
	}

	template<typename T>
	T getUnitForReduction(const ReductionType& r) {
		switch(r) {
		case ReductionType::MEAN:
			return 0;
		case ReductionType::MAX:
			return Defaultvalues<T>::getMin();
		case ReductionType::MIN:
			return Defaultvalues<T>::getMax();
		case ReductionType::SUM:
			return 0;
		};
	}
	template<typename T>
	T combine(const ReductionType& r, const T& a, const T& b) {
		switch(r) {
			case ReductionType::MEAN:
			case ReductionType::SUM:
				return a+b;
			case ReductionType::MIN:
				return std::min(a,b);
			case ReductionType::MAX:
				return std::max(a,b);
		}
	}

    ImageInport inport1_;
    ImageInport inport2_;
    ImageOutport differencePort_;
    ImageOutport maskPort_;

    FloatProperty maxDeviation_;
	FloatProperty maxPixelwiseDeviation_;
    TemplateOptionProperty<ComparisonType> comparisonType_;
	TemplateOptionProperty<ReductionType> reductionType_;
    DirectoryProperty reportDir_;
    int imageCompCount_ = 0;

    struct Comparison {
        time_t timestamp;
        double result;
		ReductionType reduction;
        size_t differentPixels;
        size_t pixelCount;
        std::filesystem::path img1;
        std::filesystem::path img2;
        std::filesystem::path diff;
        std::filesystem::path mask;
    };
    std::vector<Comparison> comparisons_;
};

}  // namespace inviwo
