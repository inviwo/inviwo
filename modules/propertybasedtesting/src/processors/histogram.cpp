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
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOr
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#include <inviwo/propertybasedtesting/processors/histogram.h>

#include <inviwo/propertybasedtesting/algorithm/coveringarray.h>

#include <inviwo/core/datastructures/image/imageram.h>
#include <inviwo/core/network/networklock.h>

#include <iostream>

#undef NDEBUG
#include <assert.h>

namespace inviwo {

template<typename T>
std::ostream& operator<<(std::ostream& out, const std::vector<T>& v);
template<typename A, typename B>
std::ostream& operator<<(std::ostream& out, const std::pair<A,B>& v) {
    return out << "(" << v.first << ", " << v.second << ")";
}
template<typename T>
std::ostream& operator<<(std::ostream& out, const std::vector<T>& v) {
    out << "{";
    for(size_t i = 0; i < v.size(); i++) {
        if(i > 0) out << ", ";
        out << v[i];
    }
    return out << "}";
}

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo Histogram::processorInfo_{
    "org.inviwo.Histogram",      // Class identifier
    "Histogram",                // Display name
    "Undefined",              // Category
    CodeState::Experimental,  // Code state
    Tags::None,               // Tags
};
const ProcessorInfo Histogram::getProcessorInfo() const { return processorInfo_; }

template<typename... Args>
struct TestableProperty;
template<typename T, typename... Args>
struct TestableProperty<T, Args...> {
	static std::optional<std::shared_ptr<TestProperty>> _testableProperty(Property* prop) {
		if(auto tmp = dynamic_cast<T*>(prop); tmp != nullptr)
			return {std::make_shared<TestPropertyTyped<T>>(tmp->clone())};
		else
			return TestableProperty<Args...>::_testableProperty(prop);
	}
};
template<>
struct TestableProperty<>
{
	static std::optional<std::shared_ptr<TestProperty>> _testableProperty(Property*) {
		return std::nullopt;
	}
};
std::optional<std::shared_ptr<TestProperty>> testableProperty(Property* prop) {
	return TestableProperty<OrdinalProperty<int>, IntMinMaxProperty>::_testableProperty(prop);
}

Histogram::Histogram(InviwoApplication* app)
    : Processor()
    , testingState(TestingState::NONE)
	, app_(app)
	, inport_("imageInport")
	, startButton_("startButton", "Start")
	, collectButton_("collectButton", "Collect Properties") {

	startButton_.onChange([this]() { 
			std::cerr << "startButton_.onChange()" << std::endl;
            initTesting();
			std::cerr << "startButton_.onChange() End" << std::endl;
		} );

	collectButton_.onChange([this]() {
			NetworkLock lock(this);

            // remove previously collected properties
			props_.clear();
            for(auto& property : compositeProperties_)
				removeProperty(property);
			compositeProperties_.clear();

			std::set<Property*> properties;

            std::unordered_set<const Processor*> visited;
            std::queue<const Processor*> q;
            q.emplace(this);
            visited.emplace(this);

            // collect all properties of the processors feeding directly or
            // indirectly into this
            while(!q.empty()) {
                auto processor = q.front();
                q.pop();

                std::cerr << "visiting processor " << processor << " "
                          << processor->getIdentifier() << std::endl;
                
                if(processor != this) {
                    const auto& processorProperties = processor->getProperties();
                    properties.insert(processorProperties.begin(), processorProperties.end());
                }

                for(const Inport* inport : processor->getInports()) {
                    for(const Outport* outport : inport->getConnectedOutports()) {
                        if(const Processor* proc = outport->getProcessor(); visited.insert(proc).second) {
                            q.emplace(proc);
                        }
                    }
                }
            }

			std::unordered_map<Processor*, CompositeProperty*> composites; // processor to its composite property

            std::unordered_set<std::string> usedIdentifiers;
            // TODO: only insert minimal set S of properties such that all properties
            // are either in S or directly or indirectly set by a property in S
			for(Property* prop : properties) {
				if(auto _p = testableProperty(prop); _p != std::nullopt) {
					props_.emplace_back(*_p);
					auto* p = (*_p)->getProperty();

                    if(!usedIdentifiers.insert(p->getIdentifier()).second) {
                        size_t num = 0;
                        do {
                            p->setIdentifier(p->getIdentifier() + "_" + std::to_string(num));
                        } while(!usedIdentifiers.insert(p->getIdentifier()).second);
                    }

					// add to processor
					auto parentProcessor = dynamic_cast<Processor*>(prop->getOwner());
					assert(parentProcessor != nullptr);
					auto& comp = composites[parentProcessor];
					if(comp == nullptr) {
						std::string parentIdentifier = parentProcessor->getIdentifier();
						std::replace(parentIdentifier.begin(), parentIdentifier.end(), ' ', '_');
						comp = new CompositeProperty(
								parentIdentifier,
								parentProcessor->getDisplayName());
						addProperty(comp);
						compositeProperties_.emplace_back(comp);
					}
					comp->addProperty(p);
					// add link to original property
					app_->getProcessorNetwork()->addLink(p, prop);
				}
			}
		} );

	inport_.setOutportDeterminesSize(true);
	addPort(inport_);
	addProperty(startButton_);
	addProperty(collectButton_);

	app_->dispatchFrontAndForget([this]() {
            // Only necessary because there is no actual serializing yet
            for(Property* _compProp : getProperties()) {
                if(auto compProp = dynamic_cast<CompositeProperty*>(_compProp); compProp != nullptr) {
                    compositeProperties_.emplace_back(compProp);
                }
            }
		});
}


void Histogram::initTesting() {
	testResults.clear();
	assert(remainingTests.empty());

    // store current state in order to reset it after testing
    for(auto prop : props_) {
		prop->storeDefault();
    }

    std::vector<std::shared_ptr<TestProperty>> propsToTest;
    std::copy_if(props_.begin(), props_.end(), std::back_inserter(propsToTest), [this](auto prop) {
            return !app_->getProcessorNetwork()->getPropertiesLinkedTo(prop->getProperty()).empty();
        });

	std::cerr << "propsToTest.size() = " << propsToTest.size() << std::endl;

	std::vector<std::vector<std::shared_ptr<PropertyAssignment>>> ranges(propsToTest.size());
	for(size_t pi = 0; pi < propsToTest.size(); pi++) {
		for(const auto& assignment : propsToTest[pi]->generateAssignments())
			ranges[pi].emplace_back(assignment);
	}
	
	std::cerr << "ranges: ";
	for(const auto& x : ranges) std::cerr << " [" << x.size() << "]";
	std::cerr << std::endl;

	for(const auto& test : util::coveringArray(Test(),
				ranges)) {
		remainingTests.emplace(test);
	}

	std::cerr << "remainingTests.size() = " << remainingTests.size() << std::endl;
	util::log(IVW_CONTEXT, std::string("Testing ") + std::to_string(remainingTests.size()) + " configurations...", LogLevel::Info, LogAudience::User);

    assert(remainingTests.size() > 1);

	app_->dispatchFront([this]() {
			setupTest(remainingTests.front());
		});
}

void Histogram::checkTestResults() {
    std::cerr << "checking " << testResults.size() << " test results ..." << std::flush;
    std::vector< std::pair<std::shared_ptr<TestResult>, std::shared_ptr<TestResult>> > errors;

	for(auto testResult : testResults) {
		std::cerr << testResult->getNumberOfBackgroundPixels() << std::endl;
	}

	for(auto testResult : testResults) {
		for(auto otherTestResult : testResults) {
			// only consider pairs of test results where testResult has more
			// background pixels than otherTestResult
			if(testResult->getNumberOfBackgroundPixels() <= otherTestResult->getNumberOfBackgroundPixels())
				continue;

			// TODO
			// bool validComparison = true; // check if the ranges of testResult lie fully within the ranges of otherTestResult 

			// for(auto prop : props_) {
			// 	const auto& range = testResult->getValue(prop);
			// 	const auto& otherRange = otherTestResult->getValue(prop);
			// 	validComparison &= range.x <= otherRange.x && range.y >= otherRange.y;
			// }

			// if(validComparison) {
			// 	errors.emplace_back( testResult, otherTestResult );
			// }
		}
	}

	std::cerr << " done with " << errors.size() << " tests failed" << std::endl;
	if(!errors.empty()) {
        std::stringstream str;
        str << errors.size() << " tests failed: ";
        errors.resize(std::min(size_t(5), errors.size())); // print at most 5 examples
        str << errors << std::endl;
        util::log(IVW_CONTEXT, str.str(), LogLevel::Warn, LogAudience::User);
    } else {
		util::log(IVW_CONTEXT, "All tests passed.", LogLevel::Info, LogAudience::User);
	}
}

bool Histogram::testIsSetUp(const Test& test) {
    for(const auto& assignment : test)
		if(!assignment->isApplied())
			return false;
    return true;
}
void Histogram::setupTest(const Test& test) {
	NetworkLock lock(this);
	
	resetAllProps();

	// then set relevant properties
    for(const auto& assignment : test) {
		assignment->apply();
    }

	// TODO: find better solution
	app_->getProcessorNetwork()->forEachProcessor([](auto proc) {
			proc->invalidate(InvalidationLevel::InvalidOutput);
		});
	
	testingState = TestingState::GATHERING;
}

void Histogram::process() {
	auto img = inport_.getData();
	const auto dim = img->getDimensions();

    switch(testingState) {
        case TestingState::NONE:

            break;
		case TestingState::GATHERING:
			assert(!remainingTests.empty());

			auto test = remainingTests.front();
			remainingTests.pop();

			assert(testIsSetUp(test));

			size_t pixelCount = 0;
			auto imgRam = img->getRepresentation<ImageRAM>();
			auto depthLayerRAM = imgRam->getDepthLayerRAM();
			for(size_t x = 0; x < dim.x; x++) {
				for(size_t y = 0; y < dim.y; y++) {
					const auto col = depthLayerRAM->getAsDVec4(size2_t(x,y));
					if(col.x == 1)
						pixelCount++;
				}
			}
			if(0)
			std::cerr
				<< "dim = " << dim << " "
				<< "pixelCount = " << pixelCount << " "
				<< "remaining tests : " << remainingTests.size() << " "
				<< std::endl;

			testResults.emplace_back(std::make_shared<TestResult>(props_, test, pixelCount));

			testingState = NONE;
			if(remainingTests.empty()) { // there are no more tests to do
				this->checkTestResults();
				app_->dispatchFrontAndForget([this]() { resetAllProps(); });
			} else {
				app_->dispatchPool([this]() {
						std::this_thread::sleep_for(std::chrono::milliseconds(2)); // just so we can see what's going on, TODO: remove
						app_->dispatchFrontAndForget([this]() { setupTest(remainingTests.front()); });
					});
			}
			break;
	}
}

}  // namespace inviwo
