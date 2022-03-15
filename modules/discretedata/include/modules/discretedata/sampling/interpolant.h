/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2020 Inviwo Foundation
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

#include <modules/discretedata/discretedatatypes.h>
#include <inviwo/core/properties/optionproperty.h>

namespace inviwo {
namespace discretedata {

enum InterpolationType { Ignore, Nearest, SquaredDistance, Linear, NUM_TYPES };

/*
 * Base class to be cast into correct dimensionality.
 */
class IVW_MODULE_DISCRETEDATA_API InterpolantBase {
public:
    virtual ~InterpolantBase() = default;
    // virtual InterpolantBase* copy() const = 0;
    virtual unsigned int getDimension() const = 0;
    virtual bool supportsInterpolationType(InterpolationType type) const = 0;

    using InterpolationOption = OptionPropertyOption<InterpolationType>;
    static const std::vector<InterpolationOption> InterpolationTypeOptions;
};

/**
 * /brief a lightweight class for sampling a cell.
 * Connectivity-specific interpolant.
 * Given cell vertex coordinates and a sample point it will return weights for interpolation.
 */
template <unsigned int Dim>
struct IVW_MODULE_DISCRETEDATA_API Interpolant : public InterpolantBase {
protected:
    Interpolant() = default;

public:
    Interpolant(const Interpolant<Dim>&) = default;
    Interpolant<Dim>& operator=(const Interpolant<Dim>&) = default;
    virtual Interpolant<Dim>* copy() const = 0;

    virtual unsigned int getDimension() const override { return Dim; }

    virtual bool supportsInterpolationType(InterpolationType type) const override;
    virtual bool getWeights(InterpolationType type,
                            const std::vector<std::array<float, Dim>>& coordinates,
                            std::vector<double>& weights,
                            const std::array<float, Dim>& position) const;
};

template <unsigned int Dim>
struct IVW_MODULE_DISCRETEDATA_API SkewedBoxInterpolant : public Interpolant<Dim> {
    virtual bool supportsInterpolationType(InterpolationType type) const override;
    virtual bool getWeights(InterpolationType type,
                            const std::vector<std::array<float, Dim>>& coordinates,
                            std::vector<double>& weights,
                            const std::array<float, Dim>& position) const override;
    Interpolant<Dim>* copy() const override;
};

template <unsigned int Dim>
struct IVW_MODULE_DISCRETEDATA_API ExtendedInterpolant : public Interpolant<Dim> {

    ExtendedInterpolant(const Interpolant<Dim - 1>& baseInterpolant);
    ExtendedInterpolant(const ExtendedInterpolant<Dim>& other);
    ExtendedInterpolant(ExtendedInterpolant<Dim>&& other);
    ExtendedInterpolant<Dim>& operator=(const ExtendedInterpolant<Dim>& other);
    ExtendedInterpolant<Dim>& operator=(ExtendedInterpolant<Dim>&& other);
    virtual ~ExtendedInterpolant();
    virtual bool supportsInterpolationType(InterpolationType type) const override;
    virtual bool getWeights(InterpolationType type,
                            const std::vector<std::array<float, Dim>>& coordinates,
                            std::vector<double>& weights,
                            const std::array<float, Dim>& position) const override;
    Interpolant<Dim>* copy() const override;

    Interpolant<Dim - 1>* baseInterpolant_;
};

}  // namespace discretedata
}  // namespace inviwo

#include "interpolant.inl"
