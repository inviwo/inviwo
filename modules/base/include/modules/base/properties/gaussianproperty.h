/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2022 Inviwo Foundation
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

#include <inviwo/core/properties/compositeproperty.h>  // for CompositeProperty
#include <inviwo/core/properties/invalidationlevel.h>  // for InvalidationLevel, InvalidationLev...
#include <inviwo/core/properties/ordinalproperty.h>    // for DoubleProperty, OrdinalProperty
#include <inviwo/core/properties/propertysemantics.h>  // for PropertySemantics, PropertySemanti...
#include <inviwo/core/util/glmvec.h>                   // for dvec2, dvec3

#include <cmath>        // for exp, M_PI
#include <string>       // for string
#include <string_view>  // for string_view

#include <glm/gtx/norm.hpp>  // for distance2

namespace inviwo {
template <typename T>
struct PropertyTraits;

/**
 * \ingroup properties
 * A property representing a Gaussian function
 */
template <typename T>
class GaussianProperty : public CompositeProperty {
public:
    GaussianProperty(std::string_view identifier, std::string_view displayName,
                     InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                     PropertySemantics semantics = PropertySemantics::Default)
        : CompositeProperty(identifier, displayName, invalidationLevel, semantics)
        , height_("height_", "Height", 1.0)
        , sigma_("sigma", "Sigma", 1.0)
        , center_("center", "Center", T{0}, T{-1}, T{1}) {
        addProperty(height_);
        addProperty(sigma_);
        addProperty(center_);
    }
    GaussianProperty(const GaussianProperty& rhs)
        : CompositeProperty(rhs), height_{rhs.height_}, sigma_{rhs.sigma_}, center_{rhs.center_} {

        addProperty(height_);
        addProperty(sigma_);
        addProperty(center_);
    }

    virtual GaussianProperty<T>* clone() const override { return new GaussianProperty<T>(*this); }

    virtual ~GaussianProperty() = default;

    double operator()(const T& r) const { return eveluate(r); }

    double evaluate(const T& r) const {
        // TODO This definition seems strange, should be verified
        const double s =
            0.5 * glm::distance2(r, center_.get()) / (sigma_.get() * sigma_.get()) / M_PI;
        return height_.get() * std::exp(-s);
    }

    DoubleProperty height_;
    DoubleProperty sigma_;
    OrdinalProperty<T> center_;
};

using Gaussian1DProperty = GaussianProperty<double>;
using Gaussian2DProperty = GaussianProperty<dvec2>;
using Gaussian3DProperty = GaussianProperty<dvec3>;

template <>
struct PropertyTraits<Gaussian1DProperty> {
    static std::string classIdentifier() { return "org.inviwo.Gaussian1DProperty"; }
};
template <>
struct PropertyTraits<Gaussian2DProperty> {
    static std::string classIdentifier() { return "org.inviwo.Gaussian2DProperty"; }
};
template <>
struct PropertyTraits<Gaussian3DProperty> {
    static std::string classIdentifier() { return "org.inviwo.Gaussian3DProperty"; }
};

}  // namespace inviwo
