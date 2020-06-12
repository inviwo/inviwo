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
#include <inviwo/propertybasedtesting/testproperty.h>
#include <inviwo/propertybasedtesting/algorithm/coveringarray.h>
#include <inviwo/propertybasedtesting/algorithm/histogramtesting.h>
#include <inviwo/propertybasedtesting/algorithm/generatingassignments.h>

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>

#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/boolcompositeproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/directoryproperty.h>

#include <inviwo/core/ports/imageport.h>

#include <filesystem>

namespace inviwo {

/** \docpage{org.inviwo.Histogram, Histogram}
 * ![](org.inviwo.Histogram.png?classIdentifier=org.inviwo.Histogram)
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
class IVW_MODULE_PROPERTYBASEDTESTING_API Histogram
		: public Processor
		, protected ProcessorNetworkObserver {
public:
    Histogram(InviwoApplication*);
    virtual ~Histogram() {
		std::filesystem::remove_all(tempDir_);
	}

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;
    
    virtual void serialize(Serializer& d) const override;
    virtual void deserialize(Deserializer& d) override;
private:
	enum TestingState {
		NONE,
		GATHERING,
		SINGLE_COUNT
	};
	TestingState testingState;

	InviwoApplication* const app_;
    std::filesystem::path tempDir_;

	ImageInport inport_;
	
	DirectoryProperty reportDirectory_;
	BoolProperty useDepth_;
    FloatVec4Property color_;
	ButtonProperty countPixelsButton_;
	ButtonProperty startButton_;
	IntSizeTProperty numTests_;

	void collectProperties();

	void onProcessorNetworkDidAddConnection(const PortConnection&) override;
	void onProcessorNetworkDidRemoveConnection(const PortConnection&) override;

	void updateProcessors();
	std::unordered_map<Processor*,
			std::pair<CompositeProperty*,
				std::vector<
					std::pair<BoolCompositeProperty*, std::shared_ptr<TestProperty>>
				>
			>
		> processors_;

	std::vector<std::shared_ptr<TestProperty>> props_; // Properties to test
	void resetAllProps();

	// Testing stuff
	void initTesting();

	bool testIsSetUp(const Test& test) const;
	void setupTest(const Test& test);

	std::queue<Test> remainingTests;
	std::vector<std::shared_ptr<TestResult>> testResults;
	void checkTestResults();
};

}  // namespace inviwo
