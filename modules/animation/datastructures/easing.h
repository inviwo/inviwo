/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2018 Inviwo Foundation
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

#ifndef IVW_KEYFRAME_EASING_H
#define IVW_KEYFRAME_EASING_H

#include <modules/animation/animationmoduledefine.h>
#include <modules/animation/datastructures/animationtime.h>
#include <inviwo/core/util/formats.h>
#include <inviwo/core/util/assertion.h>

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
const EasingType FirstEasingType = EasingType::None;
const EasingType LastEasingType = EasingType::InOutBounce;

IVW_MODULE_ANIMATION_API EasingType& operator++( EasingType &e );

IVW_MODULE_ANIMATION_API EasingType operator++( EasingType &e , int);

template <class Elem, class Traits>
std::basic_ostream<Elem, Traits> &operator<<(std::basic_ostream<Elem, Traits> &os,
    EasingType type) {
    switch (type)
    {
    case inviwo::animation::easing::EasingType::None:
        os << "None";
        break;
    case inviwo::animation::easing::EasingType::Linear:
        os << "Linear";
        break;
    case inviwo::animation::easing::EasingType::InQuadratic:
        os << "InQuadratic";
        break;
    case inviwo::animation::easing::EasingType::InCubic:
        os << "InCubic";
        break;
    case inviwo::animation::easing::EasingType::InQuartic:
        os << "InQuartic";
        break;
    case inviwo::animation::easing::EasingType::InQuintic:
        os << "InQuintic";
        break;
    case inviwo::animation::easing::EasingType::OutQuadratic:
        os << "OutQuadratic";
        break;
    case inviwo::animation::easing::EasingType::OutCubic:
        os << "OutCubic";
        break;
    case inviwo::animation::easing::EasingType::OutQuartic:
        os << "OutQuartic";
        break;
    case inviwo::animation::easing::EasingType::OutQuintic:
        os << "OutQuintic";
        break;
    case inviwo::animation::easing::EasingType::InOutQuadratic:
        os << "InOutQuadratic";
        break;
    case inviwo::animation::easing::EasingType::InOutCubic:
        os << "InOutCubic";
        break;
    case inviwo::animation::easing::EasingType::InOutQuartic:
        os << "InOutQuartic";
        break;
    case inviwo::animation::easing::EasingType::InOutQuintic:
        os << "InOutQuintic";
        break;
    case inviwo::animation::easing::EasingType::InSine:
        os << "InSine";
        break;
    case inviwo::animation::easing::EasingType::OutSine:
        os << "OutSine";
        break;
    case inviwo::animation::easing::EasingType::InOutSine:
        os << "InOutSine";
        break;
    case inviwo::animation::easing::EasingType::InExp:
        os << "InExp";
        break;
    case inviwo::animation::easing::EasingType::OutExp:
        os << "OutExp";
        break;
    case inviwo::animation::easing::EasingType::InOutExp:
        os << "InOutExp";
        break;
    case inviwo::animation::easing::EasingType::InCircular:
        os << "InCircular";
        break;
    case inviwo::animation::easing::EasingType::OutCircular:
        os << "OutCircular";
        break;
    case inviwo::animation::easing::EasingType::InOutCircular:
        os << "InOutCircular";
        break;
    case inviwo::animation::easing::EasingType::InBack:
        os << "InBack";
        break;
    case inviwo::animation::easing::EasingType::OutBack:
        os << "OutBack";
        break;
    case inviwo::animation::easing::EasingType::InOutBack:
        os << "InOutBack";
        break;
    case inviwo::animation::easing::EasingType::InElastic:
        os << "InElastic";
        break;
    case inviwo::animation::easing::EasingType::OutElastic:
        os << "OutElastic";
        break;
    case inviwo::animation::easing::EasingType::InOutElastic:
        os << "InOutElastic";
        break;
    case inviwo::animation::easing::EasingType::InBounce:
        os << "InBounce";
        break;
    case inviwo::animation::easing::EasingType::OutBounce:
        os << "OutBounce";
        break;
    case inviwo::animation::easing::EasingType::InOutBounce:
        os << "InOutBounce";
        break;
    default:
        throw inviwo::Exception("Unknown Easing type" , IvwContextCustom("Easing::operator<<"));
        break;
    }


    return os;

}


}  // namespace easing

/** \class Easing
    \brief Provides easing functions.
    

    This class provides a function to ease animation between two keys.
    Different approaches are supported, including overshooting animations.
 */
class IVW_MODULE_ANIMATION_API Easing {
public:
public:
    Easing() {}
    virtual ~Easing() = default;

private:
    /// Polynomial In-Easing
    static double InPolynomial(const double t, const double Exponent) { return pow(t, Exponent); }

    /// Polynomial Out-Easing
    static double OutPolynomial(const double t, const double Exponent, const double Range = 1.0) {
        return Range - pow(Range - t, Exponent);
    }

    /// Polynomial In-Out-Easing
    static double InOutPolynomial(const double t, const double Exponent) {
        const double tstretched = 2.0 * t;
        if (tstretched < 1.0) {
            return InPolynomial(tstretched, Exponent) / 2.0;
        }
        return OutPolynomial(tstretched, Exponent, 2.0) / 2.0;
    }

    /// Trigonometric In-Easing
    static double InSine(const double t) { return 1 - sin((1 - t) * M_PI_2); }

    /// Trigonometric Out-Easing
    static double OutSine(const double t) { return sin(t * M_PI_2); }

    /// Trigonometric In-Out-Easing
    static double InOutSine(const double t) {
        // This could be done in several ways, like the other sine easing as well.
        // With sin: 0.5 + sin((1.5-t) * M_PI)/2
        // We could also use a system similar to the polynomial easing,
        //  where we first call the InSine function and then the OutSine function.
        // However, this is most concise:
        return (1 - cos(t * M_PI)) / 2;
    }

    /// Exponential In-Easing
    static double InExp(const double t) {
        if (t == 0) return 0;
        if (t == 1) return 1;

        return exp(8 * (t - 1));
    }

    /// Exponential Out-Easing
    static double OutExp(const double t, const double Range = 1.0) {
        if (t == 0) return 0;
        if (t == 1) return 1;

        return Range - exp(-8 * t);
    }

    /// Exponential In-Out-Easing
    static double InOutExp(const double t) {
        if (t == 0) return 0;
        if (t == 0.5) return 0.5;
        if (t == 1) return 1;

        const double tstretched = 2.0 * t;
        if (tstretched < 1.0) {
            return InExp(tstretched) / 2;
        }
        return OutExp(tstretched - 1, 2.0) / 2;
    }

    /// Circular In-Easing
    static double InCircular(const double t) { return 1 - sqrt(1 - pow(t, 2)); }

    /// Circular Out-Easing
    static double OutCircular(const double t, const double Range = 1.0) {
        return sqrt(1 - pow(Range - t, 2));
    }

    /// Circular In-Out-Easing
    static double InOutCircular(const double t) {
        const double tstretched = 2.0 * t;
        if (tstretched < 1.0) {
            return InCircular(tstretched) / 2;
        }
        return (1 + OutCircular(tstretched, 2.0)) / 2;
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
    static double InBack(const double t, const double OvershootStrength = 1.70154) {
        return ((OvershootStrength + 1) * t - OvershootStrength) * pow(t, 2);
    }

    /// Overshooting Out-Back-Easing
    static double OutBack(const double t, const double OvershootStrength = 1.70154,
                          const double Range = 1.0) {
        return Range -
               ((OvershootStrength + 1) * (Range - t) - OvershootStrength) * pow(Range - t, 2);
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
    static double InOutBack(const double t, const double OvershootStrength = 2.59239) {
        const double tstretched = 2.0 * t;
        if (tstretched < 1.0) {
            return InBack(tstretched, OvershootStrength) / 2;
        }
        return OutBack(tstretched, OvershootStrength, 2.0) / 2;
    }

    /// Overshooting Elastic In-Easing
    static double InElastic(const double t) { return InExp(t) * sin(t * 3.25 * 2 * M_PI); }

    /// Overshooting Elastic Out-Easing
    static double OutElastic(const double t) {
        return 1 - exp(-8 * t) * sin((1 - t) * 3.25 * 2 * M_PI);
    }

    /// Overshooting Elastic In-Out-Easing
    static double InOutElastic(const double t) {
        if (t == 0.5) return 0.5;
        if (t == 1) return 1;

        const double tstretched = 2.0 * t;
        if (tstretched < 1.0) {
            return InElastic(tstretched) / 2;
        }
        return (2 - exp(-8 * (tstretched - 1)) * sin((2 - tstretched) * 3.25 * 2 * M_PI)) / 2;
    }

    static double InBounce(const double t) { return 1 - OutBounce(1 - t); }

    static double OutBounce(const double t) {
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

    static double InOutBounce(const double t) {
        const double tstretched = 2.0 * t;
        if (t < 0.5) {
            return InBounce(tstretched) / 2;
        }
        return 0.5 + OutBounce(tstretched - 1) / 2;
    }

public:
    /// A time input t in [0,1] is assumed.
    /// Most easing functions return a value in [0,1] as well.
    /// Some functions overshoot, which means you get also values outside of [0,1].
    /// Most interpolations should be fine with that. Linear Interpolation is fine with that.
    static double Ease(const double t, easing::EasingType HowToEase) {
        ivwAssert(t >= 0 && t <= 1, "Normalized time required as easing input.");

        switch (HowToEase) {
            default:
            case easing::EasingType::None:
            case easing::EasingType::Linear: {
                return t;
            }

            case easing::EasingType::InQuadratic:
                return InPolynomial(t, 2);
            case easing::EasingType::InCubic:
                return InPolynomial(t, 3);
            case easing::EasingType::InQuartic:
                return InPolynomial(t, 4);
            case easing::EasingType::InQuintic:
                return InPolynomial(t, 5);

            case easing::EasingType::OutQuadratic:
                return OutPolynomial(t, 2);
            case easing::EasingType::OutCubic:
                return OutPolynomial(t, 3);
            case easing::EasingType::OutQuartic:
                return OutPolynomial(t, 4);
            case easing::EasingType::OutQuintic:
                return OutPolynomial(t, 5);

            case easing::EasingType::InOutQuadratic:
                return InOutPolynomial(t, 2);
            case easing::EasingType::InOutCubic:
                return InOutPolynomial(t, 3);
            case easing::EasingType::InOutQuartic:
                return InOutPolynomial(t, 4);
            case easing::EasingType::InOutQuintic:
                return InOutPolynomial(t, 5);

            case easing::EasingType::InSine:
                return InSine(t);
            case easing::EasingType::OutSine:
                return OutSine(t);
            case easing::EasingType::InOutSine:
                return InOutSine(t);

            case easing::EasingType::InExp:
                return InExp(t);
            case easing::EasingType::OutExp:
                return OutExp(t);
            case easing::EasingType::InOutExp:
                return InOutExp(t);

            case easing::EasingType::InCircular:
                return InCircular(t);
            case easing::EasingType::OutCircular:
                return OutCircular(t);
            case easing::EasingType::InOutCircular:
                return InOutCircular(t);

            case easing::EasingType::InBack:
                return InBack(t);
            case easing::EasingType::OutBack:
                return OutBack(t);
            case easing::EasingType::InOutBack:
                return InOutBack(t);

            case easing::EasingType::InElastic:
                return InElastic(t);
            case easing::EasingType::OutElastic:
                return OutElastic(t);
            case easing::EasingType::InOutElastic:
                return InOutElastic(t);

            case easing::EasingType::InBounce:
                return InBounce(t);
            case easing::EasingType::OutBounce:
                return OutBounce(t);
            case easing::EasingType::InOutBounce:
                return InOutBounce(t);
        }
    }
};

}  // namespace animation

}  // namespace inviwo

#endif  // IVW_KEYFRAME_EASING_H
