/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2025 Inviwo Foundation
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

#include <modules/animation/animationmoduledefine.h>  // for IVW_MODULE_ANIMATION_API

#include <memory>  // for unique_ptr, make_unique
#include <string>  // for string
#include <string_view>

namespace inviwo {
namespace animation {
class Interpolation;

class IVW_MODULE_ANIMATION_API InterpolationFactoryObject {
public:
    explicit InterpolationFactoryObject(std::string_view classIdentifier);
    virtual ~InterpolationFactoryObject() = default;

    virtual std::unique_ptr<Interpolation> create() const = 0;
    std::string_view getClassIdentifier() const;

protected:
    const std::string classIdentifier_;
};

/*
 * Base for Keyframe-based interpolations.
 * Required for InterpolationFactory::getSupportedInterpolations to find interpolations for a
 * specific Keyframe type.
 */
template <typename Keyframe>
class InterpolationFactoryObjectKeyframe : public InterpolationFactoryObject {
public:
    explicit InterpolationFactoryObjectKeyframe(std::string_view classIdentifier)
        : InterpolationFactoryObject(classIdentifier) {}
    virtual ~InterpolationFactoryObjectKeyframe() = default;
};

/*
 * Factory object for InterpolationTyped interpolations
 */
template <typename InterpTyped>
class InterpolationFactoryObjectTemplate
    : public InterpolationFactoryObjectKeyframe<typename InterpTyped::key_type> {
public:
    using key_type = typename InterpTyped::key_type;
    using result_type = typename InterpTyped::result_type;
    // Requires a static classIdentifier() method on InterpolationType
    InterpolationFactoryObjectTemplate()
        : InterpolationFactoryObjectKeyframe<key_type>(InterpTyped::classIdentifier()) {}

    explicit InterpolationFactoryObjectTemplate(std::string_view classIdentifier)
        : InterpolationFactoryObjectKeyframe<key_type>(classIdentifier){};

    virtual ~InterpolationFactoryObjectTemplate() = default;

    virtual std::unique_ptr<Interpolation> create() const override {
        return std::make_unique<InterpTyped>();
    }
};

}  // namespace animation

}  // namespace inviwo
