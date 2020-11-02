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
#include <inviwo/propertybasedtesting/algorithm/reservoirsampling.h>

#include <inviwo/core/datastructures/image/imageram.h>
#include <inviwo/core/io/imagewriterutil.h>
#include <inviwo/core/network/networklock.h>

#include <inviwo/core/util/datetime.h>

#include <inviwo/core/datastructures/image/layerramprecision.h>

#include <iostream>
#include <filesystem>

namespace inviwo {

std::mutex Histogram::mutex_;

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
	processors_.insert(inactiveProcessors.begin(), inactiveProcessors.end());
	inactiveProcessors.clear();

	std::unordered_set<Processor*> visitedProcessors;
	for(Processor* const processor : util::getPredecessors(this)) {
		if(processor == this) continue;
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

		inactiveProcessors.emplace(processor, testProp);
		processors_.erase(processor);
	}
	
	// add newly connected processors
	for(const auto& processor : visitedProcessors) {
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
			comp->addObserver(this);
			processors_.emplace(processor, comp);

			addProperty(comp->getBoolComp());
		}
	}
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
	, distillButton_("distillButton", "Distill Failed Tests")
	, numTests_("numTests", "Maximum number of tests", 200, 1, 10000)
	, description_("description", "Description", "", InvalidationLevel::InvalidOutput, PropertySemantics::Multiline) {

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

	distillButton_.setVisible(false); // make visible when errors have been found
	distillButton_.onChange([this]() {
			this->currently_condensing = true;
			this->checkTestResults();
		});

	inport_.setOutportDeterminesSize(true);
	addPort(inport_);

	outport_.setHandleResizeEvents(false); // No resize-event handling
	addPort(outport_);

	addProperty(reportDirectory_);

	addProperty(useDepth_);
	color_.setSemantics(PropertySemantics::Color);
	color_.visibilityDependsOn(useDepth_, [](const auto& d) -> bool { return !d; });
	addProperty(color_);

	description_.setReadOnly(true);

	addProperty(countPixelsButton_);
	addProperty(numTests_);
	addProperty(startButton_);
	addProperty(distillButton_);
	addProperty(description_);

	if (std::filesystem::create_directory(tempDir_.string())) {
		std::stringstream str;
		str << "Using " << tempDir_ << " to store failed tests.";
		util::log(IVW_CONTEXT, str.str(), LogLevel::Info, LogAudience::User);
	}


	// make this observe the processor network
	app_->getProcessorNetwork()->addObserver(this);
}

void Histogram::onTestPropertyChange() {
	std::string desc;
	for(auto[proc,prop] : processors_) {
		if(prop->getBoolComp()->isChecked())
			desc += prop->textualDescription() + '\n';
	}
	std::cerr << "textual Description:\n" << desc << std::endl;
	description_.set(desc);
}

void Histogram::serialize(Serializer& s) const {
	const std::lock_guard<std::mutex> lock(mutex_);

	Processor::serialize(s);

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
	s.serialize("TestProperties", testProperties);
}

void Histogram::deserialize(Deserializer& d) {
	const std::lock_guard<std::mutex> lock(mutex_);

	Processor::deserialize(d);

	std::vector<Processor*> processors;
	d.deserialize("Processors", processors);
	std::vector<TestProperty*> testProperties;
	d.deserialize("TestProperties", testProperties);
	assert(processors.size() == testProperties.size());

	inactiveProcessors.insert(processors_.begin(), processors_.end());
	processors_.clear();
	const auto old = inactiveProcessors;

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

	inactiveProcessors = keep;
}

void Histogram::initTesting() {
	assert(remainingTests.empty());
	deactivated.clear();
	testResults.clear();

	distillButton_.setVisible(false);

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

	const auto [assignments, assignmentsComp] = [&]() {
			std::vector<
				std::pair<
					util::AssignmentComparator,
					std::vector< std::shared_ptr<PropertyAssignment> >
				>
			> resComp;
			std::vector<std::vector< std::shared_ptr<PropertyAssignment> >> res;

			for(const auto prop : props_) {
				auto why = prop->generateAssignmentsCmp();
				for(auto& [deac, cmp, prop_assignments] : why) {
					resComp.emplace_back(cmp, prop_assignments);
					res.emplace_back(prop_assignments);
					deactivated.emplace_back(std::move(deac));
				}
			}
			return std::make_pair(res, resComp);
		}();

	std::cerr << "assignments: ";
	for(const auto& x : assignments) std::cerr << " [" << x.size() << "]";
	std::cerr << std::endl;

	if(assignments.empty()) {
		std::cerr << "not testing because there are no assignments generated" << std::endl;
		return;
	}


	allTests = util::optCoveringArray(Test{}, assignmentsComp); 
	//auto allTests = util::coveringArray(Test{}, assignments);
	
	{
		const auto sample = reservoirSampling(allTests.size(), numTests_.get());
		const auto tmp = allTests;
		allTests.clear();
		for(const size_t i : sample) {
			allTests.emplace_back(std::move(tmp[i]));
		}
	}
	for(const auto& test : allTests) {
		remainingTests.emplace(test);
	}

	for(const auto& d : deactivated)
		(*d) = false;
	last_deactivated = -1;

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
		const size_t numElements = std::max(size_t(1), (data.size()+sizeof(T)-1)/sizeof(T));
		size_t e = 0;
		while((1ull<<e) < numElements) e++;
		const size_t b = e/2, a = e - b;
		dimensions.x = 1ull<<b;
		dimensions.y = 1ull<<a;
		while(dimensions.x * (dimensions.y-1) >= numElements)
			dimensions.y--;
	}

	T* raw = new T[dimensions.x * dimensions.y];
	memcpy(raw, data.data(), data.size());
	// padding
	memset(reinterpret_cast<unsigned char*>(raw) + data.size(), 0xFF, dimensions.x*dimensions.y*sizeof(T) - data.size());
	//std::fill_n(raw + data.size(), dimensions.x*dimensions.y - data.size(), T(0,255,0,255));

	auto errLayerRAM = std::make_shared<LayerRAMPrecision<T>>(
				raw, dimensions, LayerType::Color, swizzleMask(F::comp));
	auto errLayer = std::make_shared<Layer>(errLayerRAM);
	return std::make_shared<Image>(errLayer);
}

void Histogram::checkTestResults() {
	std::cerr << "checking test results" << std::endl;
	assert(remainingTests.empty());
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

		distillButton_.setVisible(true);
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

	
	if(currently_condensing) {
		if(errors.empty()) {
			assert(last_deactivated != -1);
			(*deactivated[last_deactivated]) = false;
			last_deactivated++;
		} else {
			last_deactivated++;
		}

		if(last_deactivated >= deactivated.size()) {
			// we have tried to deactivate all properties
			// terminate condensing
			currently_condensing = false;
		} else {
			(*deactivated[last_deactivated]) = true;

			// copy tests to 'remainingTests'
			for(const auto& test : allTests) {
				remainingTests.emplace(test);
			}

			// kick off testing
			if(remainingTests.size() > 0)
				app_->dispatchFront([this]() {
						setupTest(remainingTests.front());
					});
		}
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
	testingState = TestingState::NONE;

	if(outputImage) {
		outport_.setData(*outputImage);
	}
}

}  // namespace inviwo
