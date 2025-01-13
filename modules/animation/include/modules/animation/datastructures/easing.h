/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2024 Inviwo Foundation
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

#include <inviwo/core/util/assertion.h>  // for ivwAssert

#include <cmath>  // for pow, sin, exp, sqrt
#include <numbers>
#include <iosfwd>  // for ostream

namespace inviwo {

namespace animation {

namespace easing {

enum class EasingType {
    None,
    Linear,

    // Polynomial Types
    InQuadratic,
    InCubic,
    InQuartic,
    InQuintic,
    OutQuadratic,
    OutCubic,
    OutQuartic,
    OutQuintic,
    InOutQuadratic,
    InOutCubic,
    InOutQuartic,
    InOutQuintic,

    // Trigonometric Types
    InSine,
    OutSine,
    InOutSine,

    // Exponential Types
    InExp,
    OutExp,
    InOutExp,

    // Circular Types
    InCircular,
    OutCircular,
    InOutCircular,

    // Overshooting Back Types
    InBack,
    OutBack,
    InOutBack,

    // Overshooting Elastic Types
    InElastic,
    OutElastic,
    InOutElastic,

    // Bounce Types
    InBounce,
    OutBounce,
    InOutBounce
};

constexpr const EasingType FirstEasingType = EasingType::None;
constexpr const EasingType LastEasingType = EasingType::InOutBounce;

IVW_MODULE_ANIMATION_API EasingType& operator++(EasingType& e);

IVW_MODULE_ANIMATION_API EasingType operator++(EasingType& e, int);

IVW_MODULE_ANIMATION_API std::ostream& operator<<(std::ostream& os, EasingType type);

/// Polynomial In-Easing
inline double inPolynomial(const double t, const double Exponent) { return pow(t, Exponent); }

/// Polynomial Out-Easing
inline double outPolynomial(const double t, const double Exponent, const double Range = 1.0) {
    return Range - pow(Range - t, Exponent);
}

/// Polynomial In-Out-Easing
inline double inOutPolynomial(const double t, const double Exponent) {
    const double tstretched = 2.0 * t;
    if (tstretched < 1.0) {
        return inPolynomial(tstretched, Exponent) / 2.0;
    }
    return outPolynomial(tstretched, Exponent, 2.0) / 2.0;
}

/// Trigonometric In-Easing
inline double inSine(const double t) { return 1 - sin((1 - t) * 2.0 * std::numbers::pi); }

/// Trigonometric Out-Easing
inline double outSine(const double t) { return sin(t * 2.0 * std::numbers::pi); }

/// Trigonometric In-Out-Easing
inline double inOutSine(const double t) {
    // This could be done in several ways, like the other sine easing as well.
    // With sin: 0.5 + sin((1.5-t) * std::numbers::pi)/2
    // We could also use a system similar to the polynomial easing,
    //  where we first call the InSine function and then the OutSine function.
    // However, this is most concise:
    return (1 - cos(t * M_PI)) / 2;
}

/// Exponential In-Easing
inline double inExp(const double t) {
    if (t == 0) return 0;
    if (t == 1) return 1;

    return exp(8 * (t - 1));
}

/// Exponential Out-Easing
inline double outExp(const double t, const double range = 1.0) {
    if (t == 0) return 0;
    if (t == 1) return 1;

    return range - exp(-8 * t);
}

/// Exponential In-Out-Easing
inline double inOutExp(const double t) {
    if (t == 0) return 0;
    if (t == 0.5) return 0.5;
    if (t == 1) return 1;

    const double tstretched = 2.0 * t;
    if (tstretched < 1.0) {
        return inExp(tstretched) / 2;
    }
    return outExp(tstretched - 1, 2.0) / 2;
}

/// Circular In-Easing
inline double inCircular(const double t) { return 1 - sqrt(1 - pow(t, 2)); }

/// Circular Out-Easing
inline double outCircular(const double t, const double range = 1.0) {
    return sqrt(1 - pow(range - t, 2));
}

/// Circular In-Out-Easing
inline double inOutCircular(const double t) {
    const double tstretched = 2.0 * t;
    if (tstretched < 1.0) {
        return inCircular(tstretched) / 2;
    }
    return (1 + outCircular(tstretched, 2.0)) / 2;
}

/** Overshooting In-Back-Easing
 *
 *   \verbatim
 *   How to determine the strength of overshooting: (solved with Mathematica)
 *   This is the formula: x^2 (-a + (1 + a) x)
 *   It's lowest (overshooting) value is where this function has a zero slope, i.e., its
 *   derivative is zero.
 *   Derivative: (1 + a) x^2 + 2 x (-a + (1 + a) x)
 *   We get two zeros: {x -> 0}, {x -> (2 a)/(3 (1 + a))}}
 *   The second is the one we are looking for.
 *   We insert this into the original formula: f = ((a + 1) (2 a/(3 (1 + a))) - a) (2 a/(3 (1 +
 *  a))) (2 a/(3 (1 + a)))
 *   We ask where f attains a 10% overshoot, i.e., it becomes -0.1
 *   f + 0.1 == 0
 *   Solve[0.1 - (4 a^3)/(27 (1 + a)^2) == 0, {a}]
 *   {{a -> -0.51327 - 0.365039 I}, {a -> -0.51327 + 0.365039 I}, {a -> 1.70154}}
 *   \endverbatim
 *
 *   Hence, a = 1.70154 gives a 10% overshoot. Rinse and repeat for other overshoot percentages.
 *   The internet says 1.70158, but who believes the internet.
 */
inline double inBack(const double t, const double overshootStrength = 1.70154) {
    return ((overshootStrength + 1) * t - overshootStrength) * pow(t, 2);
}

/// Overshooting Out-Back-Easing
inline double outBack(const double t, const double overshootStrength = 1.70154,
                      const double range = 1.0) {
    return range - ((overshootStrength + 1) * (range - t) - overshootStrength) * pow(range - t, 2);
}

/** Overshooting In-Out-Back-Easing
 *
 *   \verbatim
 *   10% overshooting for the InOut Version:
 *   (2x)^2 (-a + (1 + a) 2x)/2
 *   2 x^2 (-a + 2 (1 + a) x)
 *
 *   x-derivative:
 *   4 (1 + a) x^2 + 4 x (-a + 2 (1 + a) x)
 *
 *   zeros:
 *   {{x -> 0}, {x -> a/(3 (1 + a))}}
 *
 *   Solve[ -((2 a^3)/(27 (1 + a)^2)) == -0.1, {a}]
 *   {{a -> -0.621194 - 0.36725 I}, {a -> -0.621194 + 0.36725 I}, {a -> 2.59239}}
 *   \endverbatim
 */
inline double inOutBack(const double t, const double overshootStrength = 2.59239) {
    const double tstretched = 2.0 * t;
    if (tstretched < 1.0) {
        return inBack(tstretched, overshootStrength) / 2;
    }
    return outBack(tstretched, overshootStrength, 2.0) / 2;
}

/// Overshooting Elastic In-Easing
inline double inElastic(const double t) { return inExp(t) * sin(t * 3.25 * 2 * std::numbers::pi); }

/// Overshooting Elastic Out-Easing
inline double outElastic(const double t) {
    return 1 - exp(-8 * t) * sin((1 - t) * 3.25 * 2 * std::numbers::pi);
}

/// Overshooting Elastic In-Out-Easing
inline double inOutElastic(const double t) {
    if (t == 0.5) return 0.5;
    if (t == 1) return 1;

    const double tstretched = 2.0 * t;
    if (tstretched < 1.0) {
        return inElastic(tstretched) / 2;
    }
    return (2 - exp(-8 * (tstretched - 1)) * sin((2 - tstretched) * 3.25 * 2 * std::numbers::pi)) /
           2;
}

inline double outBounce(const double t) {
    if (t < 1 / 2.75) {
        return 7.5625 * t * t;
    } else if (t < 2 / 2.75) {
        const double tnew = t - (1.5 / 2.75);
        return 7.5625 * tnew * tnew + 0.75;
    } else if (t < 2.5 / 2.75) {
        const double tnew = t - (2.25 / 2.75);
        return 7.5625 * tnew * tnew + 0.9375;
    } else {
        const double tnew = t - (2.625 / 2.75);
        return 7.5625 * tnew * tnew + 0.984375;
    }
}

inline double inBounce(const double t) { return 1 - outBounce(1 - t); }

inline double inOutBounce(const double t) {
    const double tstretched = 2.0 * t;
    if (t < 0.5) {
        return inBounce(tstretched) / 2;
    }
    return 0.5 + outBounce(tstretched - 1) / 2;
}

/// A time input t in [0,1] is assumed.
/// Most easing functions return a value in [0,1] as well.
/// Some functions overshoot, which means you get also values outside of [0,1].
/// Most interpolations should be fine with that. Linear Interpolation is fine with that.
inline double ease(const double t, easing::EasingType howToEase) {
    IVW_ASSERT(t >= 0 && t <= 1, "Normalized time required as easing input.");

    switch (howToEase) {
        default:
        case easing::EasingType::None:
        case easing::EasingType::Linear: {
            return t;
        }

        case easing::EasingType::InQuadratic:
            return inPolynomial(t, 2);
        case easing::EasingType::InCubic:
            return inPolynomial(t, 3);
        case easing::EasingType::InQuartic:
            return inPolynomial(t, 4);
        case easing::EasingType::InQuintic:
            return inPolynomial(t, 5);

        case easing::EasingType::OutQuadratic:
            return outPolynomial(t, 2);
        case easing::EasingType::OutCubic:
            return outPolynomial(t, 3);
        case easing::EasingType::OutQuartic:
            return outPolynomial(t, 4);
        case easing::EasingType::OutQuintic:
            return outPolynomial(t, 5);

        case easing::EasingType::InOutQuadratic:
            return inOutPolynomial(t, 2);
        case easing::EasingType::InOutCubic:
            return inOutPolynomial(t, 3);
        case easing::EasingType::InOutQuartic:
            return inOutPolynomial(t, 4);
        case easing::EasingType::InOutQuintic:
            return inOutPolynomial(t, 5);

        case easing::EasingType::InSine:
            return inSine(t);
        case easing::EasingType::OutSine:
            return outSine(t);
        case easing::EasingType::InOutSine:
            return inOutSine(t);

        case easing::EasingType::InExp:
            return inExp(t);
        case easing::EasingType::OutExp:
            return outExp(t);
        case easing::EasingType::InOutExp:
            return inOutExp(t);

        case easing::EasingType::InCircular:
            return inCircular(t);
        case easing::EasingType::OutCircular:
            return outCircular(t);
        case easing::EasingType::InOutCircular:
            return inOutCircular(t);

        case easing::EasingType::InBack:
            return inBack(t);
        case easing::EasingType::OutBack:
            return outBack(t);
        case easing::EasingType::InOutBack:
            return inOutBack(t);

        case easing::EasingType::InElastic:
            return inElastic(t);
        case easing::EasingType::OutElastic:
            return outElastic(t);
        case easing::EasingType::InOutElastic:
            return inOutElastic(t);

        case easing::EasingType::InBounce:
            return inBounce(t);
        case easing::EasingType::OutBounce:
            return outBounce(t);
        case easing::EasingType::InOutBounce:
            return inOutBounce(t);
    }
}

}  // namespace easing

}  // namespace animation

}  // namespace inviwo
