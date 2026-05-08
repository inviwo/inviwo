/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <modules/animation/animationmoduledefine.h>
#include <inviwo/core/algorithm/easing.h>
#include <inviwo/core/datastructures/transferfunction.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <modules/animation/datastructures/animationtime.h>
#include <modules/animation/interpolation/interpolation.h>
#include <modules/animation/datastructures/valuekeyframe.h>

#include <memory>
#include <string>
#include <vector>

namespace inviwo {

class Deserializer;
class Serializer;

namespace animation {

class IVW_MODULE_ANIMATION_API TransferFunctionInterpolation
    : public InterpolationTyped<ValueKeyframe<TransferFunction>, TransferFunction> {
public:
    TransferFunctionInterpolation(InviwoApplication* app = nullptr);
    TransferFunctionInterpolation(const TransferFunctionInterpolation&);
    virtual ~TransferFunctionInterpolation() = default;
    virtual TransferFunctionInterpolation* clone() const override;

    virtual std::string_view getDisplayName() const override;
    virtual std::string_view getIdentifier() const override {
        return "TransferFunctionInterpolation";
    }

    static std::string_view classIdentifier();
    virtual std::string_view getClassIdentifier() const override;

    virtual bool equal(const Interpolation& other) const override;

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

    virtual void operator()(
        const std::vector<std::unique_ptr<ValueKeyframe<TransferFunction>>>& keys, Seconds from,
        Seconds to, TransferFunction& out) const override;

    OrdinalProperty<size_t> segments;
};

}  // namespace animation
}  // namespace inviwo
