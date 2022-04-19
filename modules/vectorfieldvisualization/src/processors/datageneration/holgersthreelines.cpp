/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021 Inviwo Foundation
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

#include <modules/vectorfieldvisualization/processors/datageneration/holgersthreelines.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo HolgersThreeLines::processorInfo_{
    "org.inviwo.HolgersThreeLines",  // Class identifier
    "Holgers Three Lines",           // Display name
    "Undefined",                     // Category
    CodeState::Experimental,         // Code state
    Tags::None,                      // Tags
};
const ProcessorInfo HolgersThreeLines::getProcessorInfo() const { return processorInfo_; }

HolgersThreeLines::HolgersThreeLines()
    : Processor()
    , linesOut_("linesOut")
    , referenceFrameLine_("referenceFrameLine_")
    , integrationLength_("integrationLength", "Integration Length", 1, 0.001, 10.0)
    , integrationSteps_("integrationSteps_", "Integration Steps", 1000, 3, 10000)
    // , parameters_("parameters", "(p,q) parameters",
    //               [this]() {
    //                   std::vector<std::unique_ptr<Property>> v;
    //                   v.emplace_back(std::make_unique<DoubleVec2Property>(
    //                       "pq", "(p,q)", dvec2(6.75, -2.25), dvec2(-10), dvec2(10)));
    //                   return v;
    //               }())
    , paramP_("paramP", "Parameter p", 6.75, -10, 10)
    , paramQ_("paramQ", "Parameter q", -2.25, -10, 10)
    , optimalParam_("optimalParam", "Remove optimal reference system?", false) {

    addPorts(linesOut_, referenceFrameLine_);
    addProperties(integrationLength_, integrationSteps_, paramP_, paramQ_, optimalParam_);
}

void HolgersThreeLines::process() {

    auto lines = std::make_shared<IntegralLineSet>(mat4(1.0));

    lines->push_back(IntegralLine(), 0);
    lines->push_back(IntegralLine(), 1);
    lines->push_back(IntegralLine(), 2);
    std::array<std::vector<dvec3>*, 3> posVecs, velVecs;
    std::array<std::vector<double>*, 3> timeVecs, trvVecs;
    for (size_t l = 0; l < 3; ++l) {
        posVecs[l] = &lines->at(l).getPositions();
        posVecs[l]->resize(integrationSteps_);

        velVecs[l] = &lines->at(l).getMetaData<dvec3>("velocity", true);
        velVecs[l]->resize(integrationSteps_);

        timeVecs[l] = &lines->at(l).getMetaData<double>("time", true);
        timeVecs[l]->resize(integrationSteps_);

        trvVecs[l] = &lines->at(l).getMetaData<double>("trv", true);
        trvVecs[l]->resize(integrationSteps_);
    }

    // One line encompassing the reference frame info.
    IntegralLine referenceLine;
    auto& referencePos = referenceLine.getPositions();
    referencePos.resize(integrationSteps_);
    auto& baseX = referenceLine.getMetaData<dvec3>("baseX", true);
    baseX.resize(integrationSteps_);
    auto& baseY = referenceLine.getMetaData<dvec3>("baseY", true);
    baseY.resize(integrationSteps_);
    auto& velo = referenceLine.getMetaData<dvec3>("velocity", true);
    velo.resize(integrationSteps_);

    for (size_t i = 0; i < integrationSteps_.get(); ++i) {
        double t = integrationLength_ * i / integrationSteps_;

        dvec3 o = optimalParam_ ? dvec3{0, 0, t} : dvec3{std::cos(t), std::sin(t), t};
        dvec3 r1 =
            optimalParam_ ? dvec3{1, 0, 0} : dvec3{std::cos(paramP_ * t), std::sin(paramP_ * t), 0};
        dvec3 r2 = optimalParam_ ? dvec3{0, 1, 0}
                                 : dvec3{-std::sin(paramP_ * t), std::cos(paramP_ * t), 0};

        posVecs[0]->at(i) =
            o + r1 * (0.3 * std::cos(paramQ_ * t)) + r2 * (0.2 * std::sin(paramQ_ * t));

        posVecs[1]->at(i) = o + (r1 * (0.3 * std::cos(paramQ_ * t - 2.0 / 3.0 * M_PI)) +
                                 r2 * (0.2 * std::sin(paramQ_ * t - 2.0 / 3.0 * M_PI))) *
                                    0.8;

        posVecs[2]->at(i) = o + (r1 * (0.3 * std::cos(paramQ_ * t + 2.0 / 3.0 * M_PI)) +
                                 r2 * (0.2 * std::sin(paramQ_ * t + 2.0 / 3.0 * M_PI))) *
                                    1.2;

        velVecs[0]->at(i) = {double(i) / integrationSteps_, 0, 0};
        velVecs[1]->at(i) = {double(i) / integrationSteps_, 0, 0};
        velVecs[2]->at(i) = {double(i) / integrationSteps_, 0, 0};

        timeVecs[0]->at(i) = double(i) / integrationSteps_;
        timeVecs[1]->at(i) = double(i) / integrationSteps_;
        timeVecs[2]->at(i) = double(i) / integrationSteps_;

        trvVecs[0]->at(i) = 0;    // 13.0 / 12 * paramQ_;
        trvVecs[1]->at(i) = 0.5;  // 13.0 / 12 * paramQ_;
        trvVecs[2]->at(i) = 1.0;  // 13.0 / 12 * paramQ_;

        referencePos[i] = o;
        baseX[i] = r1;  // vec3{r1.x, r2.x, 0};
        baseY[i] = r2;  // vec3{r1.x, r2.y, 0};
        velo[i] = {double(i) / integrationSteps_.get(), 0, 0};
    }

    linesOut_.setData(lines);

    auto referenceLineSet = std::make_shared<IntegralLineSet>(mat4(1.0));
    referenceLineSet->push_back(std::move(referenceLine), 0);
    referenceFrameLine_.setData(referenceLineSet);
}
}  // namespace inviwo
