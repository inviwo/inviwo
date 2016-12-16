/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016 Inviwo Foundation
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

namespace inviwo {

namespace animation {

/** \class Easing
    \brief Provides easing functions.
    
    This class provides a function to ease animation between two keys.
    Different approaches are supported, including overshooting animations.
 */
class IVW_MODULE_ANIMATION_API Easing
{
//Friends
//Types
public:
    enum class EEasingType
    {
        None
        ,Linear

        //Polynomial Types
        ,InQuadratic
        ,InCubic
        ,InQuartic
        ,InQuintic
        ,OutQuadratic
        ,OutCubic
        ,OutQuartic
        ,OutQuintic
        ,InOutQuadratic
        ,InOutCubic
        ,InOutQuartic
        ,InOutQuintic

        //Trigonometric Types
        ,InSine
        ,OutSine
        ,InOutSine
    };

//Construction / Deconstruction
public:
    Easing(){}
    virtual ~Easing() = default;

//Methods
private:
    ///Polynomial In-Easing
    static double InPolynomial(const double t, const double Exponent)
    {return pow(t, Exponent);}

    ///Polynomial Out-Easing
    static double OutPolynomial(const double t, const double Exponent, const double Range = 1.0)
    {return Range - pow(Range - t, Exponent);}

    ///Polynomial In-Out-Easing
    static double InOutPolynomial(const double t, const double Exponent)
    {
        const double tstretched = 2.0 * t;
        if (tstretched < 1.0)
        {
            return InPolynomial(tstretched, Exponent) / 2.0;
        }
        return OutPolynomial(tstretched, Exponent, 2.0) / 2.0;
    }

    ///Trigonometric In-Easing
    static double InSine(const double t)
    {return 1 - sin((1-t) * M_PI_2);}

    ///Trigonometric Out-Easing
    static double OutSine(const double t)
    {return sin(t * M_PI_2);}

    ///Trigonometric In-Out-Easing
    static double InOutSine(const double t)
    {
        //This could be done in several ways, like the other sine easing as well.
        // With sin: 0.5 + sin((1.5-t) * M_PI)/2
        // We could also use a system similar to the polynomial easing,
        //  where we first call the InSine function and then the OutSine function.
        // However, this is most concise:
        return (1 - cos(t * M_PI)) / 2;
    }

    ///Exponential In-Easing
    static double InExp(const double t)
    {
        if (t == 0) return 0;
        if (t == 1) return 1;

        return exp(8 * (x - 1));
    }

    ///Exponential Out-Easing
    static double OutExp(const double t, const double Range = 1.0)
    {
        if (t == 0) return 0;
        if (t == 1) return 1;

        return Range - exp(-8*x);
    }

    ///Exponential In-Out-Easing
    static double InOutExp(const double t)
    {
        if (t == 0) return 0;
        if (t == 1) return 1;

        const double tstretched = 2.0 * t;
        if (tstretched < 1.0)
        {
            return InExp(tstretched, Exponent) / 2;
        }
        return OutExp(tstretched - 1, Exponent, 2.0) / 2;
    }



public:
    ///A time input t in [0,1] is assumed.
    /// Most easing functions return an value in [0,1] as well.
    /// Some functions overshoot, which means you get also values outside of [0,1].
    /// Most interpolations should be fine with that. Linear Interpolation is fine with that.
    static double Ease(const double t, EEasingType HowToEase)
    {
        ivwAssert(t >= 0 && t <= 1, "Normalized time required as easing input.");

        switch (HowToEase)
        {
            default:
            case EEasingType::None:
            case EEasingType::Linear:
            {
                return t;
            }

            case EEasingType::InQuadratic:    return InPolynomial(t, 2);
            case EEasingType::InCubic:        return InPolynomial(t, 3);
            case EEasingType::InQuartic:      return InPolynomial(t, 4);
            case EEasingType::InQuintic:      return InPolynomial(t, 5);

            case EEasingType::OutQuadratic:   return OutPolynomial(t, 2);
            case EEasingType::OutCubic:       return OutPolynomial(t, 3);
            case EEasingType::OutQuartic:     return OutPolynomial(t, 4);
            case EEasingType::OutQuintic:     return OutPolynomial(t, 5);

            case EEasingType::InOutQuadratic: return InOutPolynomial(t, 2);
            case EEasingType::InOutCubic:     return InOutPolynomial(t, 3);
            case EEasingType::InOutQuartic:   return InOutPolynomial(t, 4);
            case EEasingType::InOutQuintic:   return InOutPolynomial(t, 5);

            case EEasingType::InSine:    return InSine(t);
            case EEasingType::OutSine:   return OutSine(t);
            case EEasingType::InOutSine: return InOutSine(t);

            case EEasingType::InExp:     return InExp(t);
            case EEasingType::OutExp:    return OutExp(t);
            case EEasingType::InOutExp:  return InOutExp(t);
        }
    }

//Attributes
public:

};

} // namespace

} // namespace

#endif // IVW_KEYFRAME_EASING_H

