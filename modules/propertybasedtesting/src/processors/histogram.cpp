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

#include <inviwo/core/datastructures/image/layerramprecision.h>

#include <iostream>
#include <filesystem>

namespace inviwo {

std::mutex Histogram::mutex_;

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
	const std::lock_guard<std::mutex> lock(mutex_);

	for(auto prop : props_) {
		prop->setToDefault();
	}
}

void Histogram::onProcessorNetworkDidAddConnection(const PortConnection& conn) {
	const std::lock_guard<std::mutex> lock(mutex_);

	ProcessorNetworkObserver::onProcessorNetworkDidAddConnection(conn);
	updateProcessors();
}
void Histogram::onProcessorNetworkDidRemoveConnection(const PortConnection& conn) {
	const std::lock_guard<std::mutex> lock(mutex_);

	ProcessorNetworkObserver::onProcessorNetworkDidRemoveConnection(conn);
	updateProcessors();
}
void Histogram::updateProcessors() {
	std::cerr << "UPDATE_PROCESSOR" << std::endl;
	processors_.insert(inactiveProcessors.begin(), inactiveProcessors.end());
	inactiveProcessors.clear();

	std::unordered_set<Processor*> visitedProcessors;
	for(Processor* const processor : util::getPredecessors(this)) {
		if(processor == this) continue;
		//std::cerr << "visiting processor " << processor << " "
		//	<< processor->getIdentifier() << " " <<
		//	(processors_.count(processor) > 0 ? "known" : "new") << std::endl;
		visitedProcessors.insert(processor);
	}

	// remove no longer connected processors
	std::unordered_set<Processor*> processorsToRemove;
	for(const auto& [processor,testProp] : processors_) {
		if(!visitedProcessors.count(processor)) { // processor no longer visited
			processorsToRemove.insert(processor);
		} else { // make visible again
			testProp->getBoolComp()->setVisible(true);
		}
	}
	for(const auto& processor : processorsToRemove) {
		const auto& testProp = processors_.at(processor);
		//std::cerr << " removing " << testProp->getBoolComp() << std::endl;

		std::cerr << "testProp = " << testProp << std::endl;
		std::cerr << "testProp->getBoolComp() = " << testProp->getBoolComp() << std::endl;
		testProp->getBoolComp()->setVisible(false);
		//auto tmp = removeProperty(testProp->getBoolComp());
		////std::cerr << "tmp = " << tmp << ", tmp->getOwner() = " << tmp->getOwner() << std::endl;
		//assert(tmp != nullptr);
		//assert(tmp == testProp->getBoolComp());
		//assert(tmp->getOwner() == nullptr);

		inactiveProcessors.emplace(processor, testProp);
		processors_.erase(processor);
	}
	
	// add newly connected processors
	for(const auto& processor : visitedProcessors) {
		//std::cerr << processor << " "
		//	<< processor->getIdentifier() << ": "
		//	<< processors_.count(processor) << std::endl;
		if(processors_.count(processor) == 0) {
			std::vector<std::shared_ptr<TestProperty>> props;
			for(Property* prop : processor->getProperties()) {
				if(std::optional<std::shared_ptr<TestProperty>> p = testableProperty(prop);
						p != std::nullopt) {
					props.emplace_back(*p);
				}
			}

			// Skip processors with no testable property
			if(props.empty())
				continue;

			auto comp = TestPropertyComposite::make<Processor>(processor);
			processors_.emplace(processor, comp);
			//std::cerr << processor->getDisplayName() << ": " << props.size() << std::endl;

			addProperty(comp->getBoolComp());
		}
	}

	//std::cerr << std::endl;
}

Histogram::Histogram(InviwoApplication* app)
	: Processor()
	, testingState(TestingState::NONE)
	, app_(app)
	, tempDir_{std::filesystem::temp_directory_path() / ("inviwo_histogram_" + std::to_string(rand()))}
	, inport_("imageInport")
	, outport_("imageOutport")
	, reportDirectory_("reportDirectory", "Report Directory", tempDir_.string())
	, useDepth_("useDepth", "Use Depth", false, InvalidationLevel::Valid)
	, color_("color", "Color", vec4(1.0f), vec4(0.0f), vec4(0.0f))
	, countPixelsButton_("cntPixelsButton", "Count number of pixels with set color")
	, startButton_("startButton", "Update Test Results")
	, numTests_("numTests", "Maximum number of tests", 200, 1, 10000) {

	//std::cerr << "Histogram(app=" << app << ")" << std::endl;

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

	outport_.setHandleResizeEvents(false); // No resize-event handling
	// TODO: maybe generate fitting image on resize event?
	addPort(outport_);

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
	const std::lock_guard<std::mutex> lock(mutex_);

	Processor::serialize(s);

	std::cerr << "Histogram::serialize()" << std::endl;

	std::vector<Processor*> processors;
	std::vector<TestProperty*> testProperties;
	for(auto[proc, test] : processors_) {
		processors.emplace_back(proc);
		testProperties.emplace_back(static_cast<TestProperty*>(test.get()));
	}
	for(auto[proc, test] : inactiveProcessors) {
		processors.emplace_back(proc);
		testProperties.emplace_back(static_cast<TestProperty*>(test.get()));
	}
	s.serialize("Processors", processors);
	std::cerr << "\tserialized " << processors.size() << " processors" << std::endl;
	s.serialize("TestProperties", testProperties);
	std::cerr << "\tserialized " << testProperties.size() << " testProperties" << std::endl;
}

void Histogram::deserialize(Deserializer& d) {
	const std::lock_guard<std::mutex> lock(mutex_);

	Processor::deserialize(d);

	std::cerr << this << "->Histogram::deserialize()" << std::endl;

	std::vector<Processor*> processors;
	d.deserialize("Processors", processors);
	std::cerr << "\tdeserialized " << processors.size() << " processors: " << processors << std::endl;
	std::vector<TestProperty*> testProperties;
	d.deserialize("TestProperties", testProperties);
	std::cerr << "\tdeserialized " << testProperties.size() << " testProperties: " << testProperties << std::endl;
	assert(processors.size() == testProperties.size());

	inactiveProcessors.insert(processors_.begin(), processors_.end());
	processors_.clear();
	const auto old = inactiveProcessors;
	std::cerr << "old.size() = " << old.size() << std::endl;

	ProcessorTestPropertyMap keep;
	for(size_t i = 0; i < processors.size(); i++) {
		TestPropertyComposite* tmp = dynamic_cast<TestPropertyComposite*>(testProperties[i]);
		assert(tmp != nullptr);
		assert(keep.count(processors[i]) == 0);
		if(old.count(processors[i]) == 0) {
			keep.emplace(processors[i], std::shared_ptr<TestPropertyComposite>(tmp));
		} else {
			keep.emplace(processors[i], old.at(processors[i]));
			assert(keep.at(processors[i]).get() == tmp);
		}
	}
	std::cerr << "keep.size() = " << keep.size() << std::endl;

	inactiveProcessors = keep;
}

void Histogram::initTesting() {
	assert(remainingTests.empty());
	testResults.clear();

	props_.clear();

	for(const auto&[processor,comp] : processors_) {
		if(comp->getBoolComp()->isChecked()) {
			props_.emplace_back(comp);
			std::cerr << "Checked: " << comp->getIdentifier() << " @ " << processor->getIdentifier() << std::endl;
		}
	}

	if(props_.empty()) {
		std::cerr << "not testing because there are no selected properties" << std::endl;
		checkTestResults(); // create Image with (empty) results
		return;
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

	if(assignments.empty()) {
		std::cerr << "not testing because there are no assignments generated" << std::endl;
		return;
	}

	auto allTests = util::coveringArray(Test(), assignments);
	// TODO: find set of tests with size <= numTests_ and maximum number of testable pairs
	if(numTests_.get() < allTests.size()) {
		allTests.resize(numTests_.get());
	}
	for(const auto& test : allTests) {
		remainingTests.emplace(test);
	}

	std::cerr << "remainingTests.size() = " << remainingTests.size() << std::endl;
	util::log(IVW_CONTEXT, std::string("Testing ") + std::to_string(remainingTests.size()) + " configurations...",
			LogLevel::Info, LogAudience::User);

	if(remainingTests.size() > 0) {
		app_->dispatchFront([this]() {
				setupTest(remainingTests.front());
			});
	}
}

std::ostream& printError(std::ostream& out,
		const std::vector<std::shared_ptr<TestProperty>>& props, const TestingError& err) {
	const auto&[testResult1, testResult2, effect, res1, res2] = err;
	out << "Tests with values" << std::endl;
	for(auto prop : props)
		prop->ostr(out << " - ", testResult1, testResult2) << std::endl;
	out << "Got " << res1 << " and " << res2 << " pixels, respectively."
		<< " These numbers should be " << effect
		<< std::endl;
	return out;
}

template<typename F, typename T = typename F::type>
auto generateImageFromData(const std::vector<unsigned char>& data) {
	auto swizzleMask = [](size_t numComponents) {
		switch (numComponents) {
			case 1:
				return swizzlemasks::luminance;
			case 2:
				return swizzlemasks::luminanceAlpha;
			case 3:
				return swizzlemasks::rgb;
			case 4:
			default:
				return swizzlemasks::rgba;
		}
	};

	size2_t dimensions;
	// determine dimensions: find smallest power of two that is at least as large as
	// data.size(), and split it as evenly as possible, i.e. 2^a*2^b >= data.size(),
	// a>=b, a<=b+1. let x=2^a,y<=2^b, x*y>=data.size()y
	{
		const size_t numElements = std::max(size_t(1), (data.size()+3)/4);
		size_t e = 0;
		while((1ull<<e) < numElements) e++;
		size_t b = e/2, a = e - b;
		dimensions.x = 1ull<<b;
		dimensions.y = 1ull<<a;
		while(dimensions.x * (dimensions.y-1) >= numElements)
			dimensions.y--;
	}

	T* raw = new T[dimensions.x * dimensions.y];
	std::memcpy(raw, data.data(), data.size());
	for(size_t i = data.size(); i < dimensions.x * dimensions.y; i++) // padding
		raw[i] = T(255,0,0,255);

	auto errLayerRAM = std::make_shared<LayerRAMPrecision<T>>(
				raw, dimensions, LayerType::Color, swizzleMask(F::comp));
	auto errLayer = std::make_shared<Layer>(errLayerRAM);
	return std::make_shared<Image>(errLayer);
}

void Histogram::checkTestResults() {
	std::vector< TestingError > errors;

	size_t numComparable = 0;

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

			const bool ok = util::propertyEffectComparator(*propEff, num, otherNum);
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

		const auto errFilePath = errFileDir /
			(std::string("err_") + currentDateTime() + std::string(".txt"));
		std::ofstream errFile(errFilePath, std::ios::out);
		for(const auto& e : errors) {
			printError(errFile, props_, e) << std::endl;
		}
		errFile.close();
		util::log(IVW_CONTEXT, "Wrote errors to " + errFilePath.string(),
				LogLevel::Info, LogAudience::User);

		// write report
		const auto reportFilePath = errFileDir / std::string("report.html");
		std::ofstream reportFile(reportFilePath.string(), std::ios::out);
		PropertyBasedTestingReport report(reportFile, errors, props_);
		reportFile.close();
		util::log(IVW_CONTEXT, "Wrote report to " + reportFilePath.string(),
				LogLevel::Info, LogAudience::User);
	} else {
		util::log(IVW_CONTEXT, "All tests passed.", LogLevel::Info, LogAudience::User);
	}

	// generate image from errors
	{
		using F = DataFormat<glm::u8vec4>;
		std::vector<unsigned char> tmpData;

		for(const auto& error : errors)
			testingErrorToBinary(tmpData, props_, error);

		outputImage = { generateImageFromData<F>(tmpData) };
		this->invalidate(InvalidationLevel::InvalidOutput);
	}
}

bool Histogram::testIsSetUp(const Test& test) const {
	for(const auto& assignment : test)
		if(!assignment->isApplied())
			return false;
	return true;
}
void Histogram::setupTest(const Test& test) {
	std::cerr << "setup Test" << std::endl;
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
			// necessary because of synchronicity issues, TODO: find better solution
			std::this_thread::sleep_for(std::chrono::milliseconds(20));
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
	const std::lock_guard<std::mutex> lock(mutex_);

	auto img = inport_.getData();

	switch(testingState) {
		case TestingState::NONE:
			if(!outputImage && remainingTests.empty()) {
				// output image does not exist and we are currently not testing
				//	   generate output image
				app_->dispatchFrontAndForget([this]() { initTesting(); });
			}
			break;
		case TestingState::SINGLE_COUNT:
			{
				const size_t pixelCount = countPixels(img, color_.get(), useDepth_);
				util::log(IVW_CONTEXT, std::to_string(pixelCount) + " pixels",
						LogLevel::Info, LogAudience::User);
			}
			break;
		case TestingState::GATHERING:
			assert(!remainingTests.empty());

			auto test = remainingTests.front();
			remainingTests.pop();

			assert(testIsSetUp(test));

			const size_t pixelCount = countPixels(img, color_.get(), useDepth_);
		
			static size_t num = 0;
			const auto imagePath = tempDir_ /
				(std::string("img_") + std::to_string(num++) + std::string(".png"));
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

	if(outputImage) {
		outport_.setData(*outputImage);
		std::cerr << "setting output image" << std::endl;
	}
}

}  // namespace inviwo
