/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 *FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#include "gaussianproperty.h"

namespace inviwo {

PropertyClassIdentifier(Gaussian1DProperty, "org.inviwo.Gaussian1DProperty");
PropertyClassIdentifier(Gaussian2DProperty, "org.inviwo.Gaussian2DProperty");

Gaussian1DProperty::Gaussian1DProperty(
    const std::string &identifier, const std::string &displayName,
    InvalidationLevel invalidationLevel , PropertySemantics semantics )
    : GaussianProperty<double>(identifier, displayName, invalidationLevel,
                               semantics) {}

Gaussian1DProperty::~Gaussian1DProperty() {}

double Gaussian1DProperty::evaluate(const double &inV) const {
  double v = inV - center_.get();
  double x = v * v;
  double s = 2.0 * M_PI * glm::pow<double>(sigma_.get(), 2.0);
  return (height_.get() / 1.0) * std::exp(-(x) / s);
}

Gaussian2DProperty::Gaussian2DProperty(
    const std::string &identifier, const std::string &displayName,
    InvalidationLevel invalidationLevel, PropertySemantics semantics)
    : GaussianProperty<dvec2>(identifier, displayName, invalidationLevel,
                              semantics) {}

Gaussian2DProperty::~Gaussian2DProperty() {}

double Gaussian2DProperty::evaluate(const dvec2 &inV) const {
  dvec2 v = inV - center_.get();
  double x = v.x * v.x;
  double y = v.y * v.y;
  double s = 2.0 * M_PI * glm::pow<double>(sigma_.get(), 2.0);
  return (height_.get() / (1.0)) * std::exp(-(x + y) / s);
}

} // namespace
