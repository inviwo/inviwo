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

#ifndef IVW_INTEGRALLINEPROPERTIES_H
#define IVW_INTEGRALLINEPROPERTIES_H

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/coordinatetransformer.h>
#include <modules/vectorfieldvisualization/vectorfieldvisualizationmoduledefine.h>

#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/optionproperty.h>

namespace inviwo {

/**
 * \class IntegralLineProperties
 * \brief VERY_BRIEFLY_DESCRIBE_THE_CLASS
 * DESCRIBE_THE_CLASS
 */
class IVW_MODULE_VECTORFIELDVISUALIZATION_API IntegralLineProperties : public CompositeProperty {
public:
    enum class IntegrationScheme { Euler, RK4 };

    enum class Direction { FWD = 1, BWD = 2, BOTH = 3 };

    IntegralLineProperties(std::string identifier, std::string displayName);
    IntegralLineProperties(const IntegralLineProperties& rhs);
    IntegralLineProperties& operator=(const IntegralLineProperties& that);
    virtual IntegralLineProperties* clone() const override;
    virtual ~IntegralLineProperties();

    mat4 getSeedPointTransformationMatrix(const SpatialCoordinateTransformer<3>& T) const;

    int getNumberOfSteps() const;
    float getStepSize() const;

    IntegralLineProperties::Direction getStepDirection() const;
    IntegralLineProperties::IntegrationScheme getIntegrationScheme() const;
    SpatialCoordinateTransformer<3>::Space getSeedPointsSpace() const;

private:
    void setUpProperties();

protected:
    IntProperty numberOfSteps_;
    FloatProperty stepSize_;

    TemplateOptionProperty<IntegralLineProperties::Direction> stepDirection_;
    TemplateOptionProperty<IntegralLineProperties::IntegrationScheme> integrationScheme_;
    TemplateOptionProperty<SpatialCoordinateTransformer<3>::Space> seedPointsSpace_;
};

}  // namespace

#endif  // IVW_INTEGRALLINEPROPERTIES_H
