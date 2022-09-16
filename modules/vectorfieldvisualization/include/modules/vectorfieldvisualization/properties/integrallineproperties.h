/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2022 Inviwo Foundation
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

#include <modules/vectorfieldvisualization/vectorfieldvisualizationmoduledefine.h>  // for IVW_M...

#include <inviwo/core/datastructures/coordinatetransformer.h>                       // for Coord...
#include <inviwo/core/properties/boolproperty.h>                                    // for BoolP...
#include <inviwo/core/properties/compositeproperty.h>                               // for Compo...
#include <inviwo/core/properties/optionproperty.h>                                  // for Optio...
#include <inviwo/core/properties/ordinalproperty.h>                                 // for Float...
#include <inviwo/core/util/glmutils.h>                                              // for Matrix
#include <inviwo/core/util/staticstring.h>                                          // for opera...

#include <functional>                                                               // for __base
#include <string>                                                                   // for opera...
#include <string_view>                                                              // for opera...
#include <vector>                                                                   // for opera...

namespace inviwo {

class IVW_MODULE_VECTORFIELDVISUALIZATION_API IntegralLineProperties : public CompositeProperty {
public:
    enum class IntegrationScheme { Euler, RK4 };

    enum class Direction { FWD = 1, BWD = 2, BOTH = 3 };

    IntegralLineProperties(std::string_view identifier, std::string_view displayName);
    IntegralLineProperties(const IntegralLineProperties& rhs);
    virtual IntegralLineProperties* clone() const override;
    virtual ~IntegralLineProperties();

    template <unsigned int N>
    Matrix<N + 1, float> getSeedPointTransformationMatrix(
        const SpatialCoordinateTransformer<N>& T) const;

    int getNumberOfSteps() const;
    float getStepSize() const;

    IntegralLineProperties::Direction getStepDirection() const;
    IntegralLineProperties::IntegrationScheme getIntegrationScheme() const;
    CoordinateSpace getSeedPointsSpace() const;
    bool getNormalizeSamples() const;

private:
    void setUpProperties();

public:
    IntProperty numberOfSteps_;
    FloatProperty stepSize_;
    BoolProperty normalizeSamples_;

    OptionProperty<IntegralLineProperties::Direction> stepDirection_;
    OptionProperty<IntegralLineProperties::IntegrationScheme> integrationScheme_;
    OptionProperty<CoordinateSpace> seedPointsSpace_;
};

template <unsigned int N>
Matrix<N + 1, float> IntegralLineProperties::getSeedPointTransformationMatrix(
    const SpatialCoordinateTransformer<N>& T) const {
    return T.getMatrix(seedPointsSpace_.get(), CoordinateSpace::Data);
}

}  // namespace inviwo
