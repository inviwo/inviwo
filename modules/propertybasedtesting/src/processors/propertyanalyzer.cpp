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

#include <inviwo/propertybasedtesting/processors/propertyanalyzer.h>
#include <inviwo/propertybasedtesting/html/report.h>
#include <inviwo/propertybasedtesting/algorithm/reservoirsampling.h>

#include <inviwo/core/datastructures/image/imageram.h>
#include <inviwo/core/io/imagewriterutil.h>
#include <inviwo/core/network/networklock.h>

#include <inviwo/core/util/datetime.h>
#include <inviwo/core/util/glm.h>

#include <inviwo/core/datastructures/image/layerramprecision.h>

#include <iostream>
#include <filesystem>

namespace inviwo {

std::set<size_t> PropertyAnalyzer::currAlive_;

template <typename F, typename T = typename F::type>
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
    // data.size(), and split it as evenly as possible, i.e. find a,b s.t.
    // 2^a*2^b >= data.size(), a>=b, a<=b+1. Find x=2^a,y<=2^b, x*y>=data.size()
    {
        // take the ceil of data.size()/sizeof(T)
        const size_t numElements =
            std::max(static_cast<size_t>(1), (data.size() + sizeof(T) - 1) / sizeof(T));
        size_t e;
        for (e = 0; (1ull << e) < numElements; e++) {
        }
        const size_t b = e / 2, a = e - b;  // split e into a, b as above
        dimensions.x = 1ull << b;
        dimensions.y = 1ull << a;
        while (dimensions.x * (dimensions.y - 1) >= numElements) {
            dimensions.y--;
        }
    }

    T* const imageData = new T[dimensions.x * dimensions.y];
    unsigned char* const rawImageData = reinterpret_cast<unsigned char*>(imageData);
    std::copy(data.begin(), data.end(), rawImageData);
    // pad the remaining image with 0xFF
    memset(rawImageData + data.size(), 0xFF, dimensions.x * dimensions.y * sizeof(T) - data.size());

    auto errLayerRAM = std::make_shared<LayerRAMPrecision<T>>(
        imageData, dimensions, LayerType::Color, swizzleMask(F::comp));
    auto errLayer = std::make_shared<Layer>(errLayerRAM);
    return std::make_shared<Image>(errLayer);
}

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo PropertyAnalyzer::processorInfo_{
    "org.inviwo.PropertyAnalyzer",  // Class identifier
    "PropertyAnalyzer",             // Display name
    "Undefined",                    // Category
    CodeState::Experimental,        // Code state
    Tags::None,                     // Tags
};
const ProcessorInfo PropertyAnalyzer::getProcessorInfo() const { return processorInfo_; }

void PropertyAnalyzer::resetAllProps() {
    for (auto prop : props_) {
        prop->setToDefault();
    }
}

void PropertyAnalyzer::onProcessorNetworkWillRemoveProcessor(Processor* proc) {
    processors_.erase(proc->getIdentifier());
    inactiveProcessors_.erase(proc->getIdentifier());
}
void PropertyAnalyzer::onProcessorNetworkDidAddConnection(const PortConnection& conn) {
    ProcessorNetworkObserver::onProcessorNetworkDidAddConnection(conn);
    updateProcessors();
}
void PropertyAnalyzer::onProcessorNetworkDidRemoveConnection(const PortConnection& conn) {
    ProcessorNetworkObserver::onProcessorNetworkDidRemoveConnection(conn);
    updateProcessors();
}
void PropertyAnalyzer::updateProcessors() {
    processors_.insert(std::make_move_iterator(inactiveProcessors_.begin()),
                       std::make_move_iterator(inactiveProcessors_.end()));
    inactiveProcessors_.clear();

    std::unordered_set<std::string> visitedProcessors;
    for (Processor* const processor : util::getPredecessors(this)) {
        if (processor == this) continue;
        visitedProcessors.insert(processor->getIdentifier());
    }

    std::unordered_set<std::string> usedIdentifiers;
    // remove no longer connected processors
    std::unordered_set<std::string> processorsToRemove;
    for (const auto& [processor, testProp] : processors_) {
        if (!visitedProcessors.count(processor)) {  // processor no longer visited
            processorsToRemove.insert(processor);
        } else {  // make visible again
            testProp->getBoolComp()->setVisible(true);
        }
        usedIdentifiers.emplace(testProp->getBoolComp()->getIdentifier());
    }
    for (const auto& processor : processorsToRemove) {
        auto& testProp = processors_[processor];

        testProp->getBoolComp()->setVisible(false);

        IVW_ASSERT(inactiveProcessors_.count(processor) == 0,
                   "PropertyAnalyzer: disconnected inactive Processor???");
        inactiveProcessors_.emplace(processor, std::move(testProp));
        processors_.erase(processor);
    }

    // add newly connected processors
    for (const auto& procId : visitedProcessors) {
        if (processors_.count(procId) == 0) {
            Processor* const processor = getNetwork()->getProcessorByIdentifier(procId);
            IVW_ASSERT(
                processor != nullptr,
                "PropertyAnalyzer: Predecessor Processor with given Identifier does not exist");

            size_t numTestableProperties = 0;
            for (Property* prop : processor->getProperties()) {
                if (std::unique_ptr<TestProperty> p = createTestableProperty(prop)) {
                    numTestableProperties++;
                }
            }

            // Skip processors with no testable property
            if (numTestableProperties == 0) continue;

            auto comp_ = std::make_unique<TestPropertyComposite>(processor);
            TestPropertyComposite* const comp = comp_.get();
            processors_.emplace(procId, std::move(comp_));

            comp->setNetwork(getNetwork());
            static_cast<TestPropertyObservable*>(comp)->addObserver(this);

            while (usedIdentifiers.count(comp->getBoolComp()->getIdentifier())) {
                comp->getBoolComp()->setIdentifier(comp->getBoolComp()->getIdentifier() + "_");
            }
            usedIdentifiers.emplace(comp->getBoolComp()->getIdentifier());
            addProperty(comp->getBoolComp());
        }
    }
}

size_t curr_id = 0;

PropertyAnalyzer::PropertyAnalyzer(InviwoApplication* app)
    : Processor()
    , testingState(TestingState::NONE)
    , app_(app)
    , tempDir_{std::filesystem::temp_directory_path() /
               ("inviwo_propertyanalyzer_" + std::to_string(rand()))}
    , inport_("imageInport")
    , outport_("imageOutport")
    , reportDirectory_("reportDirectory", "Report Directory", tempDir_.string())
    , useDepth_("useDepth", "Use Depth", false, InvalidationLevel::Valid)
    , color_("color", "Color", vec4(1.0f), vec4(0.0f), vec4(1.0f))
    , countPixelsButton_("cntPixelsButton", "Count number of pixels with set color")
    , startButton_("startButton", "Update Test Results")
    , distillButton_("distillButton", "Distill Failed Tests")
    , numTests_("numTests", "Maximum number of tests", 200, 1, 10000)
    , description_("description", "Description", "", InvalidationLevel::InvalidOutput,
                   PropertySemantics::Multiline) {

    m_id = curr_id++;
    currAlive_.emplace(m_id);

    countPixelsButton_.onChange([this]() {
        NetworkLock lock(this);
        testingState = TestingState::SINGLE_COUNT;
        this->invalidate(InvalidationLevel::InvalidOutput);
    });

    useDepth_.onChange([this]() {
        if (useDepth_) {
            countPixelsButton_.setDisplayName("Count number of pixels with depth value 1");
        } else {
            countPixelsButton_.setDisplayName("Count number of pixels with set color");
        }
    });

    startButton_.onChange([this]() { this->initTesting(); });

    distillButton_.setVisible(false);  // make visible when errors have been found
    distillButton_.onChange([this]() {
        this->currently_condensing = true;
        this->checkTestResults();
    });

    inport_.setOutportDeterminesSize(true);
    addPort(inport_);

    outport_.setHandleResizeEvents(false);  // No resize-event handling
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

PropertyAnalyzer::~PropertyAnalyzer() { currAlive_.erase(m_id); }

void PropertyAnalyzer::onTestPropertyChange() {
    std::string desc;
    for (const auto& [proc, prop] : processors_) {
        if (prop->getBoolComp()->isChecked()) desc += prop->textualDescription() + '\n';
    }
    description_.set(desc);
}

void PropertyAnalyzer::serialize(Serializer& s) const {
    Processor::serialize(s);

    s.serialize("NumProcs", processors_.size() + inactiveProcessors_.size());
    size_t i = 0;
    for (const auto& [proc, tp] : processors_) {
        s.serialize("Proc" + std::to_string(i), proc);
        s.serialize("Tp" + std::to_string(i), tp);
        i++;
    }
    for (const auto& [proc, tp] : inactiveProcessors_) {
        s.serialize("Proc" + std::to_string(i), proc);
        s.serialize("Tp" + std::to_string(i), tp);
        i++;
    }
}

void PropertyAnalyzer::deserialize(Deserializer& d) {
    Processor::deserialize(d);

    size_t num;
    d.deserialize("NumProcs", num);

    ProcessorTestPropertyMap tmp;
    for (size_t i = 0; i < num; i++) {
        std::string proc;
        d.deserialize("Proc" + std::to_string(i), proc);
        std::unique_ptr<TestPropertyComposite> tp;
        d.deserialize("Tp" + std::to_string(i), tp);
        tmp.emplace(proc, std::move(tp));
    }
    inactiveProcessors_ = std::move(tmp);
    processors_.clear();

    if (getNetwork() != nullptr) {
        setNetwork(getNetwork());
        updateProcessors();
    }
}

void PropertyAnalyzer::setNetwork(ProcessorNetwork* pn) {
    Processor::setNetwork(pn);
    for (const auto& [proc, tp] : processors_) tp->setNetwork(pn);
    for (const auto& [proc, tp] : inactiveProcessors_) tp->setNetwork(pn);
    if (pn) {
        for (const auto& [proc, tp] : processors_)
            static_cast<TestPropertyObservable*>(tp.get())->addObserver(this);
        for (const auto& [proc, tp] : inactiveProcessors_)
            static_cast<TestPropertyObservable*>(tp.get())->addObserver(this);
    }
}

void PropertyAnalyzer::initTesting() {
    IVW_ASSERT(remainingTests.empty(),
               "PropertyAnalyzer: initTesting() in spite of remaining tests");
    deactivated_.clear();
    testResults.clear();

    distillButton_.setVisible(false);

    props_.clear();

    for (const auto& [processor, comp] : processors_) {
        if (comp->getBoolComp()->isChecked()) {
            props_.emplace_back(comp.get());
        }
    }

    // create Image with (empty) results
    outputImage_ = generateImageFromData<DataFormat<glm::u8vec4>>({});
    if (props_.empty()) {
        return;
    }

    // store current state in order to reset it after testing
    for (auto prop : props_) {
        prop->storeDefault();
    }

    std::default_random_engine rng(42);  // make rng deterministic for regression testing

    const auto [assignments, assignmentsComp] = [&]() {
        std::vector<
            std::pair<pbt::AssignmentComparator, std::vector<std::shared_ptr<PropertyAssignment>>>>
            resComp;
        std::vector<std::vector<std::shared_ptr<PropertyAssignment>>> res;

        for (const auto prop : props_) {
            auto why = prop->generateAssignmentsCmp(rng);
            for (auto& [cmp, prop_assignments] : why) {
                resComp.emplace_back(cmp, prop_assignments);
                res.emplace_back(prop_assignments);
            }
            for (size_t i = 0; i < prop->totalNumCheckedProperties(); i++) {
                deactivated_.emplace_back(prop->deactivated(i));
            }
        }
        return std::make_pair(res, resComp);
    }();

    if (assignments.empty()) {
        util::log(IVW_CONTEXT, "Did not generate any assignments.", LogLevel::Warn,
                  LogAudience::User);
        return;
    }

    allTests = optCoveringArray(numTests_.get(), assignmentsComp);
    // auto allTests = coveringArray(assignments);

    {
        const auto sample = reservoirSampling(rng, allTests.size(), numTests_.get());
        const auto tmp = allTests;
        allTests.clear();
        for (const size_t i : sample) {
            allTests.emplace_back(std::move(tmp[i]));
        }
    }
    for (const auto& test : allTests) {
        remainingTests.emplace(test);
    }

    for (const auto& d : deactivated_) (*d) = false;
    last_deactivated = -1;

    util::log(
        IVW_CONTEXT,
        std::string("Testing ") + std::to_string(remainingTests.size()) + " configurations...",
        LogLevel::Info, LogAudience::User);

    if (!remainingTests.empty()) {
        // enqueue next test
        dispatchFrontAndForget([this]() { setupTest(remainingTests.front()); });
    }
}

std::ostream& printError(std::ostream& out, const std::vector<TestProperty*>& props,
                         const TestingError& err) {
    const auto& [testResult1, testResult2, effect, res1, res2] = err;
    out << "Tests with values" << std::endl;
    for (auto prop : props) prop->ostr(out << " - ", testResult1, testResult2) << std::endl;
    out << "Got " << res1 << " and " << res2 << " pixels, respectively."
        << " These numbers should be " << effect << std::endl;
    return out;
}

void PropertyAnalyzer::checkTestResults() {
    IVW_ASSERT(remainingTests.empty(),
               "PropertyAnalyzer: checking test results() in spite of remaining tests");
    std::vector<TestingError> errors;

    size_t numComparable = 0;

    for (size_t tRi = 0; tRi < testResults.size(); tRi++) {
        const auto& testResult = testResults[tRi];
        for (size_t tRj = 0; tRj < tRi; tRj++) {
            const auto& otherTestResult = testResults[tRj];

            pbt::PropertyEffect propEff = pbt::PropertyEffect::ANY;
            for (auto prop : props_) {
                auto tmp = prop->getPropertyEffect(testResult, otherTestResult);
                propEff = combine(propEff, tmp);
            }

            if (propEff == pbt::PropertyEffect::NOT_COMPARABLE) continue;

            numComparable++;

            const auto num = testResult->getNumberOfPixels();
            const auto otherNum = otherTestResult->getNumberOfPixels();

            const bool ok = propertyEffectComparator(propEff, num, otherNum);
            if (!ok) {
                errors.emplace_back(testResult, otherTestResult, propEff, num, otherNum);
            }
        }
    }

    util::log(IVW_CONTEXT,
              "Found " + std::to_string(numComparable) + " comparable pairs of test results",
              LogLevel::Info, LogAudience::User);
    if (!errors.empty()) {
        std::stringstream str;
        str << errors.size() << " tests failed: ";
        for (size_t i = 0; i < std::min(size_t(5), errors.size()); i++) {
            printError(str, props_, errors[i]) << std::endl;
        }
        util::log(IVW_CONTEXT, str.str(), LogLevel::Warn, LogAudience::User);

        // writing errors to file
        const auto errFileDir = std::filesystem::path(reportDirectory_.get());

        const auto errFilePath =
            errFileDir / (std::string("err_") + currentDateTime() + std::string(".txt"));
        std::ofstream errFile(errFilePath, std::ios::out);
        for (const auto& e : errors) {
            printError(errFile, props_, e) << std::endl;
        }
        errFile.close();
        util::log(IVW_CONTEXT, "Wrote errors to " + errFilePath.string(), LogLevel::Info,
                  LogAudience::User);

        // write report
        const auto reportFilePath = errFileDir / std::string("report.html");
        std::ofstream reportFile(reportFilePath.string(), std::ios::out);
        pbt::propertyBasedTestingReport(reportFile, errors, [this]() {
            std::vector<const TestProperty*> res;
            std::copy(props_.begin(), props_.end(), std::back_inserter(res));
            return res;
        }());
        reportFile.close();
        util::log(IVW_CONTEXT, "Wrote report to " + reportFilePath.string(), LogLevel::Info,
                  LogAudience::User);

        distillButton_.setVisible(true);
    } else {
        util::log(IVW_CONTEXT, "All tests passed.", LogLevel::Info, LogAudience::User);
    }

    // generate image from errors
    {
        using F = DataFormat<glm::u8vec4>;
        std::vector<unsigned char> tmpData;

        for (const auto& error : errors) testingErrorToBinary(tmpData, props_, error);

        outputImage_ = generateImageFromData<F>(tmpData);
        this->invalidate(InvalidationLevel::InvalidOutput);
    }

    if (currently_condensing) {
        if (errors.empty()) {
            IVW_ASSERT(last_deactivated != -1,
                       "PropertyAnalyzer: Condensing, but there are neither errors nor a "
                       "previously deactivated Property");

            (*deactivated_[last_deactivated]) = false;
        }
        last_deactivated++;

        if (static_cast<size_t>(last_deactivated) >= deactivated_.size()) {
            // we have tried to deactivate all properties
            // terminate condensing
            currently_condensing = false;
            util::log(IVW_CONTEXT, "Condensed tests, see the generated report.", LogLevel::Info,
                      LogAudience::User);
        } else {
            (*deactivated_[last_deactivated]) = true;

            // copy tests to 'remainingTests'
            for (const auto& test : allTests) {
                remainingTests.emplace(test);
            }

            testResults.clear();
            // kick off testing
            if (!remainingTests.empty()) {
                // enqueue first test
                dispatchFrontAndForget([this]() { setupTest(remainingTests.front()); });
            }
        }
    }
    std::cout << "done checking Test results" << std::endl;
}

bool PropertyAnalyzer::testIsSetUp(const Test& test) const {
    for (const auto& assignment : test)
        if (!assignment->isApplied()) return false;
    return true;
}
void PropertyAnalyzer::setupTest(const Test& test) {
    NetworkLock lock(this);
    IVW_ASSERT(testingState == TestingState::NONE,
               "PropertyAnalyzer: setting up test while testingState is wrong");

    resetAllProps();

    // then set relevant properties
    for (const auto& assignment : test) {
        assignment->apply();
    }

    // Invalidate this in order to force the update even if there
    // are no changes
    this->invalidate(InvalidationLevel::InvalidOutput);
    testingState = TestingState::GATHERING;
}

size_t countPixels(std::shared_ptr<const Image> img, const dvec4& col, const bool useDepth) {
    auto imgRam = img->getRepresentation<ImageRAM>();
    auto layerRAM = useDepth ? imgRam->getDepthLayerRAM() : imgRam->getColorLayerRAM();
    return layerRAM->dispatch<size_t, dispatching::filter::All>([col, useDepth](auto lr) {
        using ValueType = util::PrecisionValueType<decltype(lr)>;
        using Primitive = typename DataFormat<ValueType>::primitive;
        const auto dim = lr->getDimensions();
        const ValueType* data = lr->getDataTyped();
        return static_cast<size_t>(
            std::count_if(data, data + dim.x * dim.y, [useDepth, col](const auto x) {
                if (useDepth) {
                    const Primitive alpha = pbt::GetComponent<ValueType>::get(x, 0);
                    if constexpr (std::is_integral_v<ValueType>) {
                        return alpha == std::numeric_limits<ValueType>::max();
                    } else {
                        return glm::abs(alpha - 1) <= util::epsilon<Primitive>();
                    }
                } else {
                    const auto d = util::glm_convert_normalized<dvec4>(x) - col;
                    return glm::dot(d, d) <= 1e-7;
                }
            }));
    });
}

void PropertyAnalyzer::process() {
    auto img = inport_.getData();

    switch (testingState) {
        case TestingState::NONE:
            if (!outputImage_ && remainingTests.empty()) {
                // output image does not exist and we are currently not testing
                // => generate output image for regression testing
                initTesting();
                return;
            }
            break;
        case TestingState::SINGLE_COUNT: {
            const size_t pixelCount = countPixels(img, color_.get(), useDepth_);
            util::log(IVW_CONTEXT, std::to_string(pixelCount) + " pixels", LogLevel::Info,
                      LogAudience::User);
        } break;
        case TestingState::GATHERING:
            IVW_ASSERT(!remainingTests.empty(),
                       "PropertyAnalyzer: no remaining tests but state is set to GATHERING");

            auto test = remainingTests.front();
            remainingTests.pop();

            IVW_ASSERT(testIsSetUp(test), "PropertyAnalyzer: current test is not set up");

            const size_t pixelCount = countPixels(img, color_.get(), useDepth_);

            static size_t num = 0;
            const auto imagePath =
                tempDir_ / (std::string("img_") + std::to_string(num++) + std::string(".png"));
            static const auto pngExt = inviwo::FileExtension::createFileExtensionFromString("png");
            inviwo::util::saveLayer(*img->getColorLayer(), imagePath.string(), pngExt);

            testResults.push_back(
                std::make_shared<TestResult>(props_, test, pixelCount, imagePath));

            if (remainingTests.empty()) {  // there are no more tests to do
                dispatchFrontAndForget([this]() {
                    resetAllProps();
                    this->checkTestResults();
                });
            } else {
                dispatchFrontAndForget([this]() { setupTest(remainingTests.front()); });
            }
            break;
    }
    testingState = TestingState::NONE;

    outport_.setData(outputImage_);
}

}  // namespace inviwo
