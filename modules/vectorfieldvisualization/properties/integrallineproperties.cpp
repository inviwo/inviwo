/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015 Inviwo Foundation
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

#include "integrallineproperties.h"

namespace inviwo {

IntegralLineProperties::IntegralLineProperties(std::string identifier, std::string displayName)
    : CompositeProperty(identifier, displayName)
    , numberOfSteps_("steps", "Number of Steps", 100, 1, 1000)
    , stepSize_("stepSize", "Step size", 0.001f, 0.001f, 1.0f, 0.001f)
    , stepDirection_("stepDirection", "Step Direction")
    , integrationScheme_("integrationScheme", "Integration Scheme")
    , seedPointsSpace_("seedPointsSpace", "Seed Points Space") {
    setUpProperties();
}

IntegralLineProperties::IntegralLineProperties(const IntegralLineProperties& rhs)
    : CompositeProperty(rhs)
    , numberOfSteps_(numberOfSteps_)
    , stepSize_(stepSize_)
    , stepDirection_(stepDirection_)
    , integrationScheme_(integrationScheme_)
    , seedPointsSpace_(seedPointsSpace_) {
    setUpProperties();
}

IntegralLineProperties& IntegralLineProperties::operator=(const IntegralLineProperties& that) {
    if (this != &that) {
        CompositeProperty::operator=(that);
        numberOfSteps_ = that.numberOfSteps_;
        stepSize_ = that.stepSize_;
        stepDirection_ = that.stepDirection_;
        integrationScheme_ = that.integrationScheme_;
        seedPointsSpace_ = that.seedPointsSpace_;
    }
    return *this;
}

IntegralLineProperties* IntegralLineProperties::clone() const {
    return new IntegralLineProperties(*this);
}

IntegralLineProperties::~IntegralLineProperties() {}

inviwo::mat4 IntegralLineProperties::getSeedPointTransformationMatrix(
    const StructuredCoordinateTransformer<3>& T) const {
    return T.getMatrix(seedPointsSpace_.get(), StructuredCoordinateTransformer<3>::Space::Texture);
}

int IntegralLineProperties::getNumberOfSteps() const { return numberOfSteps_.get(); }

float IntegralLineProperties::getStepSize() const { return stepSize_.get(); }

IntegralLineProperties::Direction IntegralLineProperties::getStepDirection() const {
    return stepDirection_.get();
}

IntegralLineProperties::IntegrationScheme IntegralLineProperties::getIntegrationScheme() const {
    return integrationScheme_.get();
}

StructuredCoordinateTransformer<3>::Space IntegralLineProperties::getSeedPointsSpace() const {
    return seedPointsSpace_.get();
}

void IntegralLineProperties::setUpProperties() {
    stepDirection_.addOption("fwd", "Forward", IntegralLineProperties::Direction::FWD);
    stepDirection_.addOption("bwd", "Backwards", IntegralLineProperties::Direction::BWD);
    stepDirection_.addOption("bi", "Bi Directional", IntegralLineProperties::Direction::BOTH);

    integrationScheme_.addOption("euler", "Euler",
                                 IntegralLineProperties::IntegrationScheme::Euler);
    integrationScheme_.addOption("rk4", "Runge-Kutta (RK4)",
                                 IntegralLineProperties::IntegrationScheme::RK4);
    integrationScheme_.setSelectedValue(IntegralLineProperties::IntegrationScheme::RK4);

    seedPointsSpace_.addOption("texture", "Texture",
                               StructuredCoordinateTransformer<3>::Space::Texture);
    seedPointsSpace_.addOption("model", "Model", StructuredCoordinateTransformer<3>::Space::Model);
    seedPointsSpace_.addOption("world", "World", StructuredCoordinateTransformer<3>::Space::World);
    seedPointsSpace_.addOption("data", "Data", StructuredCoordinateTransformer<3>::Space::Data);
    seedPointsSpace_.addOption("index", "Index", StructuredCoordinateTransformer<3>::Space::Index);

    addProperty(numberOfSteps_);
    addProperty(stepSize_);
    addProperty(stepDirection_);
    addProperty(integrationScheme_);
    addProperty(seedPointsSpace_);

    setAllPropertiesCurrentStateAsDefault();
}

}  // namespace
