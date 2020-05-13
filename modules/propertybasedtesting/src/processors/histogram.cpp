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

template<class TupType, size_t... I>
std::ostream& printTuple(std::ostream& out, const TupType& tup, std::index_sequence<I...>) {
	out << "(";
	(..., (out << (I==0 ? "" : ", ") << std::get<I>(tup)));
	return out << ")";
}
template<class... T>
std::ostream& operator<<(std::ostream& out, const std::tuple<T...>& tup) {
	return printTuple(out, tup, std::make_index_sequence<sizeof...(T)>());
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

using PropertyTypes = std::tuple<OrdinalProperty<int>, IntMinMaxProperty>;

struct TestablePropertyHelper {
	template<typename T>
	auto operator()(std::optional<std::shared_ptr<TestProperty>>& res, Property* prop) {
		 if(auto tmp = dynamic_cast<T*>(prop); tmp != nullptr)
			 res = {std::make_shared<TestPropertyTyped<T>>(tmp->clone())};
	}
};
std::optional<std::shared_ptr<TestProperty>> testableProperty(Property* prop) {
	std::optional<std::shared_ptr<TestProperty>> res = std::nullopt;
	util::for_each_type<PropertyTypes>{}(TestablePropertyHelper{}, res, prop);
	return res;
}

Histogram::Histogram(InviwoApplication* app)
    : Processor()
    , testingState(TestingState::NONE)
	, app_(app)
	, inport_("imageInport")
	, useDepth_("useDepth", "Use Depth", false, InvalidationLevel::Valid)
    , color_("color", "Color", vec4(1.0f), vec4(0.0f), vec4(0.0f))
	, countPixelsButton_("cntPixelsButton", "Count number of pixels with set color")
	, startButton_("startButton", "Start")
	, collectButton_("collectButton", "Collect Properties") {

	countPixelsButton_.onChange([this]() {
			app_->dispatchFrontAndForget([this]{
					NetworkLock lock(this);
					testingState = TestingState::SINGLE_COUNT;
					this->invalidate(InvalidationLevel::InvalidOutput);
			});
		} );

	useDepth_.onChange([this]() {
			color_.setVisible(!useDepth_);
			if(useDepth_) {
				countPixelsButton_.setDisplayName("Count number of pixels with depth value 1");
			} else {
				countPixelsButton_.setDisplayName("Count number of pixels with set color");
			}
		} );

	startButton_.onChange([this]() { 
            initTesting();
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

					(*_p)->withOptionProperties([&comp](auto opt){
								comp->addProperty(opt);
							});
				}
			}
		} );

	inport_.setOutportDeterminesSize(true);
	addPort(inport_);
    
	addProperty(useDepth_);
	color_.setSemantics(PropertySemantics::Color);
    addProperty(color_);

	addProperty(countPixelsButton_);
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

	std::vector<std::vector<std::shared_ptr<PropertyAssignment>>> assignments(propsToTest.size());
	for(size_t pi = 0; pi < propsToTest.size(); pi++) {
		for(const auto& assignment : propsToTest[pi]->generateAssignments())
			assignments[pi].push_back(assignment);
	}
	
	std::cerr << "assignments: ";
	for(const auto& x : assignments) std::cerr << " [" << x.size() << "]";
	std::cerr << std::endl;

	for(const auto& test : util::coveringArray(Test(), assignments)) {
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
    std::vector< std::tuple<std::shared_ptr<TestResult>, std::shared_ptr<TestResult>, util::PropertyEffect, size_t, size_t> > errors;
	
	size_t numComparable = 0;
	std::array<size_t,7> cnt;
	std::fill_n(cnt.begin(), 7, 0);

	for(size_t tRi = 0; tRi < testResults.size(); tRi++) {
		const auto& testResult = testResults[tRi];
		for(size_t tRj = 0; tRj < tRi; tRj++) {
			const auto& otherTestResult = testResults[tRj];

			std::optional<util::PropertyEffect> propEff = { util::PropertyEffect::ANY };
			for(auto prop : props_) {
				auto tmp = prop->getPropertyEffect(testResult, otherTestResult);
				if(!tmp)
					propEff = std::nullopt;
				if(!propEff)
					break;
				propEff = util::combine(*propEff, *tmp);
			}

			if(!propEff) // not comparable
				continue;

			numComparable++;

			const auto num = testResult->getNumberOfPixels();
			const auto otherNum = otherTestResult->getNumberOfPixels();
			bool ok;

			switch(*propEff) {
				case util::PropertyEffect::ANY:
					ok = true;
					cnt[0]++;
					break;
				case util::PropertyEffect::NOT_EQUAL:
					ok = (num != otherNum);
					cnt[1]++;
					break;
				case util::PropertyEffect::EQUAL:
					ok = (num == otherNum);
					cnt[2]++;
					break;
				case util::PropertyEffect::LESS:
					ok = (num < otherNum);
					cnt[3]++;
					break;
				case util::PropertyEffect::LESS_EQUAL:
					ok = (num <= otherNum);
					cnt[4]++;
					break;
				case util::PropertyEffect::GREATER:
					ok = (num > otherNum);
					cnt[5]++;
					break;
				case util::PropertyEffect::GREATER_EQUAL:
					ok = (num >= otherNum);
					cnt[6]++;
					break;
				default:
					assert(false);
					break;
			}

			if(!ok) {
				errors.emplace_back( testResult, otherTestResult, *propEff, num, otherNum );
			}
		}
	}

	for(auto x : cnt) std::cerr << " " << x;
	std::cerr << std::endl;
	std::cerr << " " << numComparable << " comparable test result pairs" << std::endl;
	std::cerr << " done with " << errors.size() << " tests failed" << std::endl;
	if(!errors.empty()) {
        std::stringstream str;
        str << errors.size() << " tests failed: ";
        errors.resize(std::min(size_t(5), errors.size())); // print at most 5 examples
		for(auto& e : errors) {
			str << std::endl << e << ": " << std::endl;
			for(auto prop : props_)
				prop->ostr(str << " - ", std::get<0>(e), std::get<1>(e)) << std::endl;
		}
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
	
	app_->dispatchPool([this]() {
			std::this_thread::sleep_for(std::chrono::milliseconds(20)); // just so we can see what's going on, TODO: remove
			app_->dispatchFrontAndForget([this]() {
					testingState = TestingState::GATHERING;
					this->invalidate(InvalidationLevel::InvalidOutput);
				});
		});
}

size_t countPixels(std::shared_ptr<const Image> img, const dvec4& col, const bool useDepth) {
	auto imgRam = img->getRepresentation<ImageRAM>();
	auto layer = useDepth
		? imgRam->getDepthLayerRAM()
		: imgRam->getColorLayerRAM();
	const auto dim = layer->getDimensions();
	size_t res = 0;
	for(size_t x = 0; x < dim.x; x++) {
		for(size_t y = 0; y < dim.y; y++) {
			const auto pixelCol = layer->getAsNormalizedDVec4(size2_t(x,y));
			if(useDepth) {
				if(pixelCol.x == 1)
					res++;
			} else {
				std::cerr << glm::length(pixelCol - col) << std::endl;
				if(glm::length(pixelCol - col) < 1e-3)
					res++;
			}
		}
	}
	return res;
}

void Histogram::process() {
	auto img = inport_.getData();

    switch(testingState) {
        case TestingState::NONE:

            break;
		case TestingState::SINGLE_COUNT:
		{
			const size_t pixelCount = countPixels(img, color_.get(), useDepth_);
			util::log(IVW_CONTEXT, std::to_string(pixelCount) + " pixels", LogLevel::Info, LogAudience::User);
		} break;
		case TestingState::GATHERING:
			assert(!remainingTests.empty());

			auto test = remainingTests.front();
			remainingTests.pop();

			assert(testIsSetUp(test));

			const size_t pixelCount = countPixels(img, color_.get(), useDepth_);

			testResults.push_back(std::make_shared<TestResult>(props_, test, pixelCount));
			
			if(remainingTests.empty()) { // there are no more tests to do
				this->checkTestResults();
				app_->dispatchFrontAndForget([this]() { resetAllProps(); });
			} else {
				app_->dispatchFrontAndForget([this]() { setupTest(remainingTests.front()); });
			}
			break;
	}
	testingState = NONE;
}

}  // namespace inviwo
