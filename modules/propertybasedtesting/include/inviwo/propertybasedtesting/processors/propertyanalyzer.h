/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2021 Inviwo Foundation
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

#include <queue>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/propertybasedtesting/propertybasedtestingmoduledefine.h>
#include <inviwo/propertybasedtesting/testproperty.h>
#include <inviwo/propertybasedtesting/algorithm/coveringarray.h>
#include <inviwo/propertybasedtesting/algorithm/propertyanalyzing.h>
#include <inviwo/propertybasedtesting/algorithm/generatingassignments.h>

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>

#include <inviwo/core/properties/stringproperty.h>
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

/** \docpage{org.inviwo.PropertyAnalyzer, PropertyAnalyzer}
 * ![](org.inviwo.PropertyAnalyzer.png?classIdentifier=org.inviwo.PropertyAnalyzer)
 *
 * Test whether changing properties of the processors generating the
 * input image have the desired effect.
 * Specifically, a test is a set of value assignments to properties
 * such that there is at most one assignment per property.
 * For each test we set each property to its respective value (regarding the
 * assignments) and count the pixels with a given color. We call this number of
 * pixels the score of a test.
 * Two tests are comparable, if and only if the relations implied by the scores
 * are non-contradictory (there are for instance no two properties, such that
 * one implies that the score of test1 should be less than the score of test2 and the
 * other that is should be greater).
 * For all pairs of comparable tests we then verify that the relation of the
 * scores matches the relation implied by the properties. All failed pairs are
 * then exported in an HTML report.
 * The report is also converted into an image that is available at the outport.
 * This enables to verify the behaviour of properties by regression testing.
 * If desired, the testing is followed by a distillation process, where a smallest set of
 * tested properties is found such that some pair of tests still fails.
 *
 * ### Inports
 *   * __inport__ The image that is used to determine the effects of changing
 *   properties, i.e. the number of matching pixels is counted in this image.
 *   Note that only properties from predecessors of this PropertyAnalyzer processor are
 *   available for testing.
 *
 * ### Outports
 *   * __outport__ The report of errors, or a completely white image if there
 *   were no errors.
 *
 * ### Properties
 *   * __Report Directory__ The directory where the report and all generated
 *   images are saved.
 *   * __Use Depth__ Determines, whether the number of background pixels (i.e.
 *   those with depth value 1) or pixels with a specific color should be
 *   counted.
 *   * __Color__ The color of the pixels that should be counted. Is only visible
 *   (and relevant) when __Use Depth__ is unchecked.
 *   * __Count number of pixels__ Counts the number of matching pixels and
 *   prints it to the console.
 *   * __Update Test Results__ Kicks of the testing process.
 *   * __Distill Failed Tests__ Repeats the tests in order to find a smallest
 *   set of properties that lead to errors._
 *   * __Maximum number of tests__ Maximum number of tests that should be
 *   executed
 *   * __Description__ A textual description of the effects of properties.
 *
 *   For each processor that is a predecessor of this one, there is a
 *   *CompositeBoolProperty* containing the settings for all its properties.
 *   The settings for a property consist of *CompositeBoolProperty* and as many
 *   drop-down lists as the property has components. The checkbox of the
 *   *CompositeBoolProperty* dictates
 *   whether the property shall be tested, and the drop-down menus determine the
 *   expected change of the score when the components value is increased.
 *
 *   In the following example, we use a *RandomVolumeGenerator* to create a
 *   volume from a seed and then generate an image by a network containing,
 *   among others, a *Cube Proxy Geometry*.
 *   When the *PropertyAnalyzer*-processor just has been created, it outputs a completely white
 *   image, since no errors have been found.
 *   Note that the *PropertyAnalyzer*-processor contains *CompositeProperty*s for
 *   each preceding processor in the network.
 *   Since we pass the output image of the network to the *PropertyAnalyzer*-processor
 *   before a background is applied, we need use the __Use Depth__ option to count
 *   the number of background pixels.
 *   @image html prop_analyzer_1.png width=85%
 *   We want to test our network with varying seeds for the
 *   *RandomVolumeGenerator* , so the respective box in the *CompositeProperty*
 *   of the *RandomVolumeGenerator* is checked. Since different seeds allow no
 *   meaningful comparison of the number of visible background pixels, we set
 *   its comparator to *NOT_COMPARABLE*.
 *   @image html prop_analyzer_2.png width=85%
 *   We also want to verify the *Cube Proxy Geometry* -processor. For the sake of
 *   simplicity we only consider clipping in the X- and Y-dimensions.
 *   The clipping options for each dimension are *MinMaxProperty* s, so we need
 *   two comparators each, one for the lower and one for the upper setting.
 *   When we move the lower setting up, the score (i.e. the number of background pixels)
 *   should increase or remain equal, so this comparator is set to *GREATER_EQUAL*.
 *   For the upper setting, the opposite is true.
 *   @image html prop_analyzer_3.png width=85%
 *   To demonstrate this processor, we set false comparators for the clipping in
 *   the Y-dimension.
 *   @image html prop_analyzer_4.png width=85%
 *   On hitting "Update Test Results", a bunch of tests is generated and
 *   executed.
 *   @image html prop_analyzer_5.png width=85%
 *   When all tests are finished, some errors are found and the first few are
 *   printed on the console.
 *   @image html prop_analyzer_6.png width=85%
 *   All errors are documented in an HTML-report. For each error, the values
 *   of each tested property, as well as the scores of both tests,
 *   their expected relation and the generated images are shown.
 *   @image html prop_analyzer_7.png width=85%
 *   When errors have been found, we have the option to distill the failed
 *   tests, i.e. to find a subset of properties that - if tested - produces some
 *   errors. This leads to the tests being run again, except that some
 *   properties are not changed.
 *   Afterwards a new report is generated, which now only contains the tests
 *   that have failed with the distilled set of properties.
 */

using namespace pbt;

class IVW_MODULE_PROPERTYBASEDTESTING_API PropertyAnalyzer : public Processor,
                                                             private TestPropertyObserver,
                                                             private ProcessorNetworkObserver {
public:
    PropertyAnalyzer(InviwoApplication*);
    virtual ~PropertyAnalyzer();

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    virtual void serialize(Serializer& d) const override;
    virtual void deserialize(Deserializer& d) override;

    virtual void setNetwork(ProcessorNetwork*) override;

private:
    static std::set<size_t> currAlive_;
    size_t m_id;
    template <typename F>
    void dispatchFrontAndForget(F);

    enum TestingState { NONE, GATHERING, SINGLE_COUNT };
    TestingState testingState;
    bool currently_condensing = false;

    InviwoApplication* const app_;
    std::filesystem::path tempDir_;

    ImageInport inport_;
    std::shared_ptr<Image> outputImage_;
    ImageOutport outport_;

    DirectoryProperty reportDirectory_;  // TODO: create dir after deserialization
    BoolProperty useDepth_;
    FloatVec4Property color_;
    ButtonProperty countPixelsButton_;
    ButtonProperty startButton_;
    ButtonProperty distillButton_;
    IntSizeTProperty numTests_;
    StringProperty description_;
    IntSizeTProperty errorSelector_;

    // updates the textual description
    void onTestPropertyChange() override;

    void onProcessorNetworkWillRemoveProcessor(Processor*) override;
    void onProcessorNetworkDidAddConnection(const PortConnection&) override;
    void onProcessorNetworkDidRemoveConnection(const PortConnection&) override;

    void updateProcessors();
    using ProcessorTestPropertyMap = std::map<std::string, std::unique_ptr<TestPropertyComposite>>;
    ProcessorTestPropertyMap processors_;          // currently connected processors
    ProcessorTestPropertyMap inactiveProcessors_;  // currently disconnected processors

    std::vector<TestProperty*> props_;  // Properties to test
    void resetAllProps();

    // Testing stuff
    void initTesting();
    void forceUpdate();

    bool testIsSetUp(const Test& test) const;
    void setupTest(const Test& test);

    std::vector<bool*> deactivated_;
    int last_deactivated;

    std::vector<Test> allTests;
    std::queue<Test> remainingTests;
    std::vector<std::shared_ptr<TestResult>> testResults;
    std::vector<TestingError> errors;
    void checkTestResults();
};

template <typename F>
void PropertyAnalyzer::dispatchFrontAndForget(F f) {
    const size_t id = m_id;
    app_->dispatchFrontAndForget([id, f]() {
        if (PropertyAnalyzer::currAlive_.count(id)) {
            f();
        }
    });
}

}  // namespace inviwo
