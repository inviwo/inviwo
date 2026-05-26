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
#include <inviwo/core/datastructures/isovaluecollection.h>
#include <inviwo/core/properties/isovalueproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <modules/animation/datastructures/animationtime.h>
#include <modules/animation/datastructures/valuekeyframe.h>
#include <modules/animation/datastructures/valuekeyframesequence.h>
#include <modules/animation/interpolation/interpolation.h>

#include <memory>
#include <optional>
#include <string_view>
#include <vector>

namespace inviwo {

class InviwoApplication;

namespace animation {
namespace detail {

IVW_MODULE_ANIMATION_API double interpolationTime(Seconds t1, Seconds t2,
                                                  std::optional<EasingType> easeIn,
                                                  std::optional<EasingType> easeOut,
                                                  Seconds to);

IVW_MODULE_ANIMATION_API void interpolateIsoValuesFade(const IsoValueCollection& source,
                                                        const IsoValueCollection& destination,
                                                        double t, IsoValueCollection& out);

IVW_MODULE_ANIMATION_API void interpolateIsoValuesBlend(
    const IsoValueCollection& source, const IsoValueCollection& destination, double t,
    IsoValueCollection& out);

}  // namespace detail

/**
 * @brief Interpolates between two IsoValueProperty keyframes by matching iso-values at equal
 * positions and fading unmatched ones in or out.
 *
 * Iso-values that exist at the same position in both keyframes are interpolated in both position
 * and color. Iso-values that only exist in the source keyframe are faded out (alpha goes to zero)
 * while iso-values that only exist in the destination keyframe are faded in (alpha comes from
 * zero). This preserves the structural identity of iso-values across the transition.
 */
class IVW_MODULE_ANIMATION_API IsoValuePropertyInterpolationFade
    : public InterpolationTyped<ValueKeyframe<IsoValueProperty::value_type>,
                                IsoValueProperty::value_type> {
public:
    explicit IsoValuePropertyInterpolationFade(InviwoApplication* app = nullptr);
    IsoValuePropertyInterpolationFade(const IsoValuePropertyInterpolationFade&);
    virtual ~IsoValuePropertyInterpolationFade() = default;
    virtual IsoValuePropertyInterpolationFade* clone() const override;

    virtual std::string_view getDisplayName() const override;
    virtual std::string_view getIdentifier() const override {
        return "IsoValuePropertyInterpolationFade";
    }

    static std::string_view classIdentifier();
    virtual std::string_view getClassIdentifier() const override;

    virtual bool equal(const Interpolation& other) const override;

    virtual void operator()(
        const std::vector<std::unique_ptr<ValueKeyframe<IsoValueProperty::value_type>>>& keys,
        Seconds from, Seconds to, IsoValueProperty::value_type& out) const override;
};

/**
 * @brief Interpolates between two IsoValueProperty keyframes by pairing iso-values by index order.
 *
 * Iso-values are matched by their order in the collection: the first iso-value of the source is
 * paired with the first of the destination, and so on. Paired iso-values are interpolated in both
 * position and color. Unmatched iso-values (when the two keyframes have different counts) are
 * faded in or out. This gives a smooth blending effect when the two keyframes have similarly
 * structured iso-value collections.
 */
class IVW_MODULE_ANIMATION_API IsoValuePropertyInterpolationBlend
    : public InterpolationTyped<ValueKeyframe<IsoValueProperty::value_type>,
                                IsoValueProperty::value_type> {
public:
    explicit IsoValuePropertyInterpolationBlend(InviwoApplication* app = nullptr);
    IsoValuePropertyInterpolationBlend(
        const IsoValuePropertyInterpolationBlend& rhs);
    virtual ~IsoValuePropertyInterpolationBlend() = default;
    virtual IsoValuePropertyInterpolationBlend* clone() const override;

    virtual std::string_view getDisplayName() const override;
    virtual std::string_view getIdentifier() const override {
        return "IsoValuePropertyInterpolationBlend";
    }

    static std::string_view classIdentifier();
    virtual std::string_view getClassIdentifier() const override;

    virtual bool equal(const Interpolation& other) const override;

    virtual void operator()(
        const std::vector<std::unique_ptr<ValueKeyframe<IsoValueProperty::value_type>>>& keys,
        Seconds from, Seconds to, IsoValueProperty::value_type& out) const override;

    OrdinalProperty<size_t> segments;
    OrdinalProperty<double> simplify;
};

namespace detail {

template <>
struct DefaultInterpolationCreator<ValueKeyframe<IsoValueProperty::value_type>> {
    static std::unique_ptr<IsoValuePropertyInterpolationBlend> create() {
        return std::make_unique<IsoValuePropertyInterpolationBlend>();
    }
};

}  // namespace detail

}  // namespace animation

}  // namespace inviwo
