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

Histogram::Histogram(InviwoApplication* app)
    : Processor()
    , testingState(TestingState::NONE)
	, app_(app)
	, inport_("imageInport")
	, startButton_("startButton", "Start")
	, collectButton_("collectButton", "Collect Properties") {

	std::cerr << "Histogram::Histogram(app = " << app << ")" << std::endl;

	startButton_.onChange([this]() { 
			std::cerr << "startButton_.onChange()" << std::endl;
            initTesting();
			std::cerr << "startButton_.onChange() End" << std::endl;
		} );

	collectButton_.onChange([this]() {
			NetworkLock lock(this);
            // Only necessary because there is no actual serializing yet
            props_.clear();
            for(Property* _property : getProperties()) {
                if(IntMinMaxProperty* property = dynamic_cast<IntMinMaxProperty*>(_property); property != nullptr) {
                    props_.emplace_back(property);
                }
            }

            // remove previously collected properties
            for(auto& property : props_)
				removeProperty(property);
			props_.clear();

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

            std::unordered_set<std::string> usedIdentifiers;
            // TODO: only insert minimal set S of properties such that all properties
            // are either in S or directly or indirectly set by a property in S
			for(Property* prop : properties) {
				if(auto* _p = dynamic_cast<IntMinMaxProperty*>(prop); _p != nullptr) {
					auto* p = _p->clone();
                    if(!usedIdentifiers.insert(p->getIdentifier()).second) {
                        size_t num = 0;
                        do {
                            p->setIdentifier(p->getIdentifier() + "_" + std::to_string(num));
                        } while(!usedIdentifiers.insert(p->getIdentifier()).second);
                    }
					
					addProperty(p);
					//p->setReadOnly(true);
					props_.emplace_back(p);

					app_->getProcessorNetwork()->addLink(p, _p);
				}
			}
		} );

	inport_.setOutportDeterminesSize(true);
	addPort(inport_);
	addProperty(startButton_);
	addProperty(collectButton_);
}

std::vector<IntMinMaxProperty::range_type> genRanges(const IntMinMaxProperty* prop) {
	const static size_t maxStepsPerVal = 4;
	std::vector<IntMinMaxProperty::range_type> res;

	const auto minSeparation = prop->getMinSeparation();
	const size_t maxSteps = (prop->getRangeMax() - prop->getRangeMin()) / minSeparation;
	for(size_t stepsFromMin = 0; stepsFromMin < std::min(maxSteps, maxStepsPerVal); stepsFromMin++) {
		for(size_t stepsFromMax = 0; stepsFromMax + stepsFromMin < std::min(maxSteps, maxStepsPerVal); stepsFromMax++) {
			res.emplace_back(
						prop->getRangeMin() + stepsFromMin * minSeparation,
						prop->getRangeMax() - stepsFromMax * minSeparation);
		}
	}

	return res;
}

std::vector<Histogram::Test> Histogram::generateTests(IntMinMaxProperty* p1, IntMinMaxProperty* p2) {
    std::vector<Histogram::Test> res;
    for(const auto& r1 : genRanges(p1)) {
        for(const auto& r2 : genRanges(p2)) {
            res.emplace_back(std::vector{std::make_pair(p1,r1), std::make_pair(p2,r2)});
        }
    }
    return res;
}

// 2-coverage, randomized discrete SLJ strategy
template<typename T>
std::vector<T> coveringArray(const T& init, const std::vector<std::vector< std::function<void(T&)> >>& vars) {
	const size_t v = std::transform_reduce(vars.begin(),vars.end(), (size_t)0,
			[](auto a, auto b) { return std::max(a,b); },
			[](const auto& x) { return x.size(); });

	std::unordered_set<size_t> uncovered;
	std::map< std::array<size_t,4>, size_t > idx;
	for(size_t i = 1; i < vars.size(); i++) {
		for(size_t j = 0; j < i; j++) {
			for(size_t ii = 0; ii < vars[i].size(); ii++) {
				for(size_t ji = 0; ji < vars[j].size(); ji++) {
					uncovered.insert(idx.size());
					idx[{i,j,ii,ji}] = idx.size();
				}
			}
		}
	}

	std::vector<std::vector<size_t>> coveringArray;
	while(!uncovered.empty()) {
		size_t expectedCoverage = (uncovered.size() + (v*v-1)) / (v*v); // expectedCoverage > 0
		std::cerr << "uncovered.size() = " << uncovered.size() << " "
				  << "expectedCoverage = " << expectedCoverage << std::endl;
		size_t coverage;
		std::vector<size_t> row(vars.size());
		do {
			for(size_t i = 0; i < row.size(); i++)
				row[i] = rand() % vars[i].size();
			coverage = 0; // number of uncovered interactions
			for(size_t i = 1; i < vars.size(); i++) {
				for(size_t j = 0; j < i; j++) {
					size_t id = idx[{i,j,row[i],row[j]}];
					coverage += uncovered.count(id);
				}
			}
		} while(coverage < expectedCoverage);
		for(size_t i = 1; i < vars.size(); i++) {
			for(size_t j = 0; j < i; j++) {
				size_t id = idx[{i,j,row[i],row[j]}];
				uncovered.erase(id);
			}
		}
		coveringArray.emplace_back(row);
	}

	// contruct result
	std::vector<T> res(coveringArray.size(), init);
	for(size_t c = 0; c < coveringArray.size(); c++) {
		for(size_t i = 0; i < vars.size(); i++) {
			vars[i][coveringArray[c][i]](res[c]);
		}
	}
	return res;
}

void Histogram::initTesting() {
	testResults.clear();
	assert(remainingTests.empty());

    // store current state in order to reset it after testing
	defaultValues.clear();
    for(auto* prop : props_) {
		defaultValues[prop] = prop->get();
    }

    std::vector<IntMinMaxProperty*> propsToTest;
    std::copy_if(props_.begin(), props_.end(), std::back_inserter(propsToTest), [this](auto* prop) {
            return !app_->getProcessorNetwork()->getPropertiesLinkedTo(prop).empty();
        });


	std::vector<std::vector<std::function<void(Histogram::Test&)>>> ranges(propsToTest.size());
	for(size_t pi = 0; pi < propsToTest.size(); pi++) {
		for(const auto& range : genRanges(propsToTest[pi]))
			ranges[pi].emplace_back([pi,&propsToTest,range](Histogram::Test& test) {
					test.emplace_back(propsToTest[pi], range);
				});
	}
	std::vector<Histogram::Test> testss = coveringArray(Test(), ranges);

    std::vector<Histogram::Test> tests;
    // iterate over all pairs of properties
    for(size_t i = 0; i < propsToTest.size(); i++) {
        for(size_t j = 0; j < i; j++) {
            auto res = generateTests(propsToTest[i], propsToTest[j]);
            tests.insert(tests.end(), res.begin(), res.end());
        }
    }

	std::cerr << "tests.size() = " << tests.size() << " testss.size() = " << testss.size() << std::endl;

	for(const auto& test : testss) {
		remainingTests.emplace(test);
	}

    assert(remainingTests.size() > 1);

	app_->dispatchFront([this]() {
			setupTest(remainingTests.front());
			testingState = TestingState::GATHERING;
		});
}

void Histogram::checkTestResults() {
    std::cerr << "checking " << testResults.size() << " test results ..." << std::flush;
    std::vector< std::pair<std::shared_ptr<TestResult>, std::shared_ptr<TestResult>> > errors;

	for(auto testResult : testResults) {
		for(auto otherTestResult : testResults) {
			// only consider pairs of test results where testResult has more
			// background pixels than otherTestResult
			if(testResult->backgroundPixels <= otherTestResult->backgroundPixels)
				continue;

			bool validComparison = true; // check if the ranges of testResult lie fully within the ranges of otherTestResult 

			for(const auto& prop : props_) {
				const auto& range = testResult->getValue(prop);
				const auto& otherRange = otherTestResult->getValue(prop);
				validComparison &= range.x <= otherRange.x && range.y >= otherRange.y;
			}

			if(validComparison) {
				errors.emplace_back( testResult, otherTestResult );
			}
		}
	}

	std::cerr << " done with " << errors.size() << " errors" << std::endl;
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
    for(const auto& [prop,val] : test) {
        if(prop->get() != val) {
            return false;
        }
    }
    return true;
}
void Histogram::setupTest(const Test& test) {
	NetworkLock lock(this);
	resetAllProps();

	// then set relevant properties
    for(const auto& [prop,val] : test) {
        prop->set(val);
    }

	// TODO: find better solution
	app_->getProcessorNetwork()->forEachProcessor([](auto proc) {
			proc->invalidate(InvalidationLevel::InvalidOutput);
		});
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

			testResults.emplace_back(std::make_shared<TestResult>(defaultValues, test, pixelCount));

			if(remainingTests.empty()) { // there are no more tests to do
				this->checkTestResults();
				testingState = NONE;
				app_->dispatchFront([this]() { resetAllProps(); });
			} else {
				app_->dispatchFront([this]() { setupTest(remainingTests.front()); });
			}
			break;
	}
}

}  // namespace inviwo
