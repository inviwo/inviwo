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

namespace inviwo {
namespace discretedata {

enum InterpolationType { Ignore, Nearest, SquaredDistance, Linear };

/*
 * Base class to be cast into correct dimensionality.
 */
class IVW_MODULE_DISCRETEDATA_API InterpolantBase {
public:
    const unsigned int dimension_;
    virtual bool supportsInterpolationType(InterpolationType type) const = 0;

protected:
    InterpolantBase(unsigned int dim) : dimension_(dim) {}
};

/**
 * /brief a lightweight class for sampling a cell.
 * Connectivity-specific interpolant.
 * Given cell vertex coordinates and a sample point it will return weights for interpolation.
 */
template <unsigned int Dim>
struct IVW_MODULE_DISCRETEDATA_API Interpolant : public InterpolantBase {
    Interpolant() : InterpolantBase(Dim){};

    virtual bool supportsInterpolationType(InterpolationType type) const override;
    virtual bool getWeights(InterpolationType type,
                            const std::vector<std::array<float, Dim>>& coordinates,
                            std::vector<float> weights,
                            const std::array<float, Dim>& position) const;
    // virtual bool isInside(const std::vector<std::array<float, Dim>>& coordinates,
    //                       const std::array<float, Dim>& position) const = 0;
};

template <unsigned int Dim>
struct IVW_MODULE_DISCRETEDATA_API SkewedBoxInterpolant : public Interpolant<Dim> {
    virtual bool getWeights(InterpolationType type,
                            const std::vector<std::array<float, Dim>>& coordinates,
                            std::vector<float> weights,
                            const std::array<float, Dim>& position) const override;
    // virtual bool isInside(const std::vector<std::array<float, Dim>>& coordinates,
    //                       const std::array<float, Dim>& position) const override;
};

}  // namespace discretedata
}  // namespace inviwo

#include "interpolant.inl"
