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
#include <inviwo/propertybasedtesting/html/report.h>

#include <inviwo/core/datastructures/image/imageram.h>
#include <inviwo/core/io/imagewriterutil.h>
#include <inviwo/core/network/networklock.h>

#include <inviwo/core/util/datetime.h>

#include <iostream>
#include <filesystem>

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

void Histogram::resetAllProps() {
	for(auto prop : props_) {
		prop->setToDefault();
	}
}

void Histogram::onProcessorNetworkDidAddConnection(const PortConnection& conn) {
	ProcessorNetworkObserver::onProcessorNetworkDidAddConnection(conn);
	updateProcessors();
}
void Histogram::onProcessorNetworkDidRemoveConnection(const PortConnection& conn) {
	ProcessorNetworkObserver::onProcessorNetworkDidRemoveConnection(conn);
	updateProcessors();
}
void Histogram::updateProcessors() {
	std::unordered_set<Processor*> visitedProcessors;
	for(Processor* const processor : util::getPredecessors(this)) {
		if(processor == this) continue;
		std::cerr << "visiting processor " << processor << " "
			<< processor->getIdentifier() << std::endl;
		visitedProcessors.insert(processor);
	}

	// remove no longer connected processors
	std::unordered_set<Processor*> processorsToRemove;
	for(const auto& [processor,_] : processors_) {
		if(!visitedProcessors.count(processor)) { // processor no longer visited
			processorsToRemove.insert(processor);
		}
	}
	for(const auto& processor : processorsToRemove) {
		const auto& [comp, testProps] = processors_.at(processor);
		this->removeProperty(comp);
		processors_.erase(processor);
	}
	
	// add newly connected processors
	for(const auto& processor : visitedProcessors) {
		std::cerr << processor << " " << processor->getIdentifier() << ": " << processors_.count(processor) << std::endl;
		if(!processors_.count(processor)) {
			std::vector<std::pair<BoolCompositeProperty*, std::shared_ptr<TestProperty>>> props;
			for(Property* prop : processor->getProperties()) {
				if(std::optional<std::shared_ptr<TestProperty>> p = testableProperty(prop); p != std::nullopt) {
					props.emplace_back(nullptr, *p);
				}
			}

			// Skip processors with no testable property
			if(props.empty())
				continue;

			std::string ident = processor->getIdentifier();
			std::replace(ident.begin(), ident.end(), ' ', '_');
			CompositeProperty* comp = new CompositeProperty(
					ident,
					processor->getDisplayName());
			comp->setCollapsed(true);
			for(auto&[propComp,p] : props) {
				propComp = new BoolCompositeProperty(
						p->getProperty()->getIdentifier() + "Comp",
						p->getProperty()->getDisplayName(),
						false);
				propComp->setCollapsed(true);
				p->withSubProperties([&propComp](auto opt){ propComp->addProperty(opt); });

				comp->addProperty(propComp);
			}
			processors_.emplace(processor, std::make_pair(comp, props));
			std::cerr << processor->getDisplayName() << ": " << props.size() << std::endl;

			this->addProperty(comp);
		}
	}

	std::cerr << std::endl;
}

Histogram::Histogram(InviwoApplication* app)
	: Processor()
	, testingState(TestingState::NONE)
	, app_(app)
	, tempDir_{std::filesystem::temp_directory_path() / ("inviwo_histogram_" + std::to_string(rand()))}
	, inport_("imageInport")
	, reportDirectory_("reportDirectory", "Report Directory", tempDir_.string())
	, useDepth_("useDepth", "Use Depth", false, InvalidationLevel::Valid)
	, color_("color", "Color", vec4(1.0f), vec4(0.0f), vec4(0.0f))
	, countPixelsButton_("cntPixelsButton", "Count number of pixels with set color")
	, startButton_("startButton", "Start")
	, numTests_("numTests", "Maximum number of tests", 200, 1, 10000) {

	std::cerr << "Histogram(app=" << app << ")" << std::endl;

	countPixelsButton_.onChange([this]() {
			NetworkLock lock(this);
			testingState = TestingState::SINGLE_COUNT;
			this->invalidate(InvalidationLevel::InvalidOutput);
		} );

	useDepth_.onChange([this]() {
			if(useDepth_) {
				countPixelsButton_.setDisplayName("Count number of pixels with depth value 1");
			} else {
				countPixelsButton_.setDisplayName("Count number of pixels with set color");
			}
		} );

	startButton_.onChange([this]() { this->initTesting(); } );

	inport_.setOutportDeterminesSize(true);
	addPort(inport_);

	addProperty(reportDirectory_);

	addProperty(useDepth_);
	color_.setSemantics(PropertySemantics::Color);
	color_.visibilityDependsOn(useDepth_, [](const auto& d) -> bool { return !d; });
	addProperty(color_);

	addProperty(countPixelsButton_);
	addProperty(numTests_);
	addProperty(startButton_);

	if (std::filesystem::create_directory(tempDir_.string())) {
		std::stringstream str;
		str << "Using " << tempDir_ << " to store failed tests.";
		util::log(IVW_CONTEXT, str.str(), LogLevel::Info, LogAudience::User);
	}


	// make this observe the processor network
	app_->getProcessorNetwork()->addObserver(this);
}

void Histogram::serialize(Serializer& s) const {
	Processor::serialize(s);

	std::cerr << "Histogram::serialize()" << std::endl;

	std::vector<Processor*> procs;
	std::vector<CompositeProperty*> comps;
	std::vector<std::vector<BoolCompositeProperty*>> boolComps;
	std::vector<std::vector<Property*>> testProps;

	for(const auto&[processor,procData] : processors_) {
		std::vector<BoolCompositeProperty*> boolComp;
		std::vector<Property*> testProp;

		const auto&[comp,props] = procData;
		for(const auto&[propComp, prop] : props) {
			boolComp.emplace_back(propComp);
			testProp.emplace_back(prop->getProperty());
		}

		procs.emplace_back(processor);
		comps.emplace_back(comp);
		boolComps.emplace_back(boolComp);
		testProps.emplace_back(testProp);
	}

	s.serialize("TestProcessors", procs);
	s.serialize("CompositeProps", comps);
	s.serialize("BoolComps", boolComps);
	s.serialize("TestProps", testProps);
}

void Histogram::deserialize(Deserializer& d) {
	Processor::deserialize(d);

	std::cerr << "Histogram::deserialize()" << std::endl;
	
	std::vector<Processor*> procs;
	std::vector<CompositeProperty*> comps;
	std::vector<std::vector<BoolCompositeProperty*>> boolComps;
	std::vector<std::vector<Property*>> testProps;

	d.deserialize("TestProcessors", procs);
	d.deserialize("CompositeProps", comps);
	d.deserialize("BoolComps", boolComps);
	d.deserialize("TestProps", testProps);

	assert(procs.size() == comps.size() &&
			procs.size() == boolComps.size() &&
			procs.size() == testProps.size());

	for(size_t i = 0; i < procs.size(); i++) {
		assert(boolComps[i].size() == testProps[i].size());
		std::vector<std::pair<BoolCompositeProperty*, std::shared_ptr<TestProperty>>> props;
		for(size_t j = 0; j < boolComps[i].size(); j++)
			props.emplace_back(boolComps[i][j], *testableProperty(testProps[i][j]));

		processors_[procs[i]] = std::make_pair(comps[i], props);
	}
}

void Histogram::initTesting() {
	testResults.clear();
	assert(remainingTests.empty());

	props_.clear();

	for(const auto&[processor,procData] : processors_) {
		const auto&[comp,props] = procData;
		for(const auto&[propComp, prop] : props)
			if(propComp->isChecked())
				props_.emplace_back(prop);
	}

	// store current state in order to reset it after testing
	for(auto prop : props_) {
		prop->storeDefault();
	}

	std::vector<std::vector<std::shared_ptr<PropertyAssignment>>> assignments(props_.size());
	for(size_t pi = 0; pi < props_.size(); pi++) {
		for(const auto& assignment : props_[pi]->generateAssignments())
			assignments[pi].push_back(assignment);
	}

	std::cerr << "assignments: ";
	for(const auto& x : assignments) std::cerr << " [" << x.size() << "]";
	std::cerr << std::endl;

	auto allTests = util::coveringArray(Test(), assignments);
	// TODO: find set of tests with size <= numTests_ and maximum number of testable pairs
	if(numTests_.get() < allTests.size()) {
		allTests.resize(numTests_.get());
	}
	for(const auto& test : allTests) {
		remainingTests.emplace(test);
	}

	std::cerr << "remainingTests.size() = " << remainingTests.size() << std::endl;
	util::log(IVW_CONTEXT, std::string("Testing ") + std::to_string(remainingTests.size()) + " configurations...", LogLevel::Info, LogAudience::User);

	if(remainingTests.size() > 0) {
		app_->dispatchFront([this]() {
				setupTest(remainingTests.front());
			});
	}
}

std::ostream& printError(std::ostream& out, const std::vector<std::shared_ptr<TestProperty>> props, const TestingError& err) {
	const auto&[testResult1, testResult2, effect, res1, res2] = err;
	out << "Tests with values" << std::endl;
	for(auto prop : props)
		prop->ostr(out << " - ", testResult1, testResult2) << std::endl;
	out << "Got " << res1 << " and " << res2 << " pixels, respectively."
		<< " These numbers should be " << effect
		<< std::endl;
	return out;
}

void Histogram::checkTestResults() {
	std::vector< TestingError > errors;

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

	util::log(IVW_CONTEXT,
			"Found " + std::to_string(numComparable) + " comparable pairs of test results",
			LogLevel::Info, LogAudience::User);
	if(!errors.empty()) {
		std::stringstream str;
		str << errors.size() << " tests failed: ";
		for(size_t i = 0; i < std::min(size_t(5), errors.size()); i++) {
			printError(str, props_, errors[i]) << std::endl;
		}
		util::log(IVW_CONTEXT, str.str(), LogLevel::Warn, LogAudience::User);

		// writing errors to file
		const auto errFileDir = std::filesystem::path(reportDirectory_.get());

		const auto errFilePath = errFileDir / (std::string("err_") + currentDateTime() + std::string(".txt"));
		std::ofstream errFile(errFilePath, std::ios::out);
		for(const auto& e : errors) {
			printError(errFile, props_, e) << std::endl;
		}
		errFile.close();
		util::log(IVW_CONTEXT, "Wrote errors to " + errFilePath.string(), LogLevel::Info, LogAudience::User);

		// write report
		const auto reportFilePath = errFileDir / std::string("report.html");
		std::ofstream reportFile(reportFilePath.string(), std::ios::out);
		PropertyBasedTestingReport report(reportFile, errors, props_);
		reportFile.close();
		util::log(IVW_CONTEXT, "Wrote report to " + reportFilePath.string(), LogLevel::Info, LogAudience::User);
	} else {
		util::log(IVW_CONTEXT, "All tests passed.", LogLevel::Info, LogAudience::User);
	}
}

bool Histogram::testIsSetUp(const Test& test) const {
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
			std::this_thread::sleep_for(std::chrono::milliseconds(20)); // necessary because of synchronicity issues, TODO: find better solution
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
		
			static size_t num = 0;
			const auto imagePath = tempDir_ / (std::string("img_") + std::to_string(num++) + std::string(".png"));
			static const auto pngExt = inviwo::FileExtension::createFileExtensionFromString("png");
			inviwo::util::saveLayer(*img->getColorLayer(), imagePath.string(), pngExt);

			testResults.push_back(std::make_shared<TestResult>(props_, test, pixelCount, imagePath));

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
