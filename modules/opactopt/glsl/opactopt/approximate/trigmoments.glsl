/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2024 Inviwo Foundation
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

#define PI 3.1415926535897932384626433832795f
#define TWOPI 6.283185307179586476925286766559f

layout(std430, binding = 13) buffer MomentSettings {
    vec4 wrapping_zone_parameters;
    float wrapping_zone_angle;
    float overestimation;
};

float approximate2TrigonometricMoments(float b_0, vec2 trig_b[2], float depth, float bias,
                                       float overestimation, vec4 wrapping_zone_parameters);
float approximate3TrigonometricMoments(float b_0, vec2 trig_b[3], float depth, float bias,
                                       float overestimation, vec4 wrapping_zone_parameters);
float approximate4TrigonometricMoments(float b_0, vec2 trig_b[4], float depth, float bias,
                                       float overestimation, vec4 wrapping_zone_parameters);

#ifdef COEFF_TEX_FIXED_POINT_FACTOR
void project(layout(r32i) iimage2DArray coeffTex, int N, float depth, float val)
#else
void project(layout(size1x32) image2DArray coeffTex, int N, float depth, float val)
#endif
{
    float phase = fma(wrapping_zone_parameters.y, depth, wrapping_zone_parameters.y);
    float costheta = cos(phase);
    float sintheta = sin(phase);
    float coskm1theta = 1.0;
    float sinkm1theta = 0.0;
    float cosktheta = 0.0;
    float sinktheta = 0.0;

    for (int i = 0; i < N; i++) {
        float projVal = 0.0;
        if (i == 0) {
            projVal = val;
        } else if (i % 2 == 0) {
            cosktheta = coskm1theta * costheta - sinkm1theta * sintheta;
            projVal = val * cosktheta;
        } else {
            sinktheta = sinkm1theta * costheta + coskm1theta * sintheta;
            projVal = val * sinktheta;
        }

        // increment k
        if (i % 2 == 0 && i != 0) {
            coskm1theta = cosktheta;
            sinkm1theta = sinktheta;
        }

        ivec3 coord = ivec3(gl_FragCoord.xy, i);
#if defined(COEFF_TEX_FIXED_POINT_FACTOR)
        imageAtomicAdd(coeffTex, coord, int(projVal * COEFF_TEX_FIXED_POINT_FACTOR));
#elif defined(COEFF_TEX_ATOMIC_FLOAT)
        imageAtomicAdd(coeffTex, coord, projVal);
#else
        float currVal = imageLoad(coeffTex, coord).x;
        imageStore(coeffTex, coord, vec4(currVal + projVal));
#endif
    }
}

#ifdef COEFF_TEX_FIXED_POINT_FACTOR
float approximate(layout(r32i) iimage2DArray coeffTex, int N, float depth)
#else
float approximate(layout(size1x32) image2DArray coeffTex, int N, float depth)
#endif
{
    float b_0;
    int k = 0;
    float bias;

    if (N == 5) {
        vec2 trig_b[2];

        for (int i = 0; i < N; i++) {
            ivec3 coord = ivec3(gl_FragCoord.xy, i);
#ifdef COEFF_TEX_FIXED_POINT_FACTOR
            float coeff = float(imageLoad(coeffTex, coord).x) / COEFF_TEX_FIXED_POINT_FACTOR;
#else
            float coeff = imageLoad(coeffTex, coord).x;
#endif
            if (i == 0)
                b_0 = coeff;
            else if (i % 2 == 1)
                trig_b[k - 1].y = coeff / b_0;
            else
                trig_b[k - 1].x = coeff / b_0;

            if (i % 2 == 0) k++;
        }

        bias = 4e-7;
        return approximate2TrigonometricMoments(b_0, trig_b, depth, bias, overestimation,
                                                wrapping_zone_parameters);
    } else if (N == 7) {
        vec2 trig_b[3];

        for (int i = 0; i < N; i++) {
            ivec3 coord = ivec3(gl_FragCoord.xy, i);
#ifdef COEFF_TEX_FIXED_POINT_FACTOR
            float coeff = float(imageLoad(coeffTex, coord).x) / COEFF_TEX_FIXED_POINT_FACTOR;
#else
            float coeff = imageLoad(coeffTex, coord).x;
#endif
            if (i == 0)
                b_0 = coeff;
            else if (i % 2 == 1)
                trig_b[k - 1].y = coeff / b_0;
            else
                trig_b[k - 1].x = coeff / b_0;

            if (i % 2 == 0) k++;
        }

        bias = 8e-7;
        return approximate3TrigonometricMoments(b_0, trig_b, depth, bias, overestimation,
                                                wrapping_zone_parameters);
    } else if (N == 9) {
        vec2 trig_b[4];

        for (int i = 0; i < N; i++) {
            ivec3 coord = ivec3(gl_FragCoord.xy, i);
#ifdef COEFF_TEX_FIXED_POINT_FACTOR
            float coeff = float(imageLoad(coeffTex, coord).x) / COEFF_TEX_FIXED_POINT_FACTOR;
#else
            float coeff = imageLoad(coeffTex, coord).x;
#endif
            if (i == 0)
                b_0 = coeff;
            else if (i % 2 == 1)
                trig_b[k - 1].y = coeff / b_0;
            else
                trig_b[k - 1].x = coeff / b_0;

            if (i % 2 == 0) k++;
        }

        bias = 1.5e-6;
        return approximate4TrigonometricMoments(b_0, trig_b, depth, bias, overestimation,
                                                wrapping_zone_parameters);
    } else {
        return 0;
    }
}

#ifdef COEFF_TEX_FIXED_POINT_FACTOR
float total(layout(r32i) iimage2DArray coeffTex, int N)
#else
float total(layout(size1x32) image2DArray coeffTex, int N)
#endif
{
#ifdef COEFF_TEX_FIXED_POINT_FACTOR
    return float(imageLoad(coeffTex, ivec3(gl_FragCoord.xy, 0)).x) / COEFF_TEX_FIXED_POINT_FACTOR;
#else
    return imageLoad(coeffTex, ivec3(gl_FragCoord.xy, 0)).x;
#endif
}

/**
 * The code below is adapted from LineVis (https://github.com/chrismile/LineVis/),
 * which in turn adapted their code from HLSL code accompanying the paper "Moment-Based
 * Order-Independent Transparency" by Münstermann, Krumpen, Klein, and Peters
 * (http://momentsingraphics.de/?page_id=210). The original code was released in accordance to CC0
 * (https://creativecommons.org/publicdomain/zero/1.0/).
 */

/*
* BSD 2-Clause License
* 
* Copyright (c) 2020-2022, Christoph Neuhauser, Michael Kern, Felix Brendel
* All rights reserved.
* 
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
* 
* 1. Redistributions of source code must retain the above copyright notice, this
*    list of conditions and the following disclaimer.
* 
* 2. Redistributions in binary form must reproduce the above copyright notice,
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution.
* 
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
* 
* 
* ----------------------------------------------------------------------------
* 
* The following license applies to
* src/Renderers/Vulkan/Scattering/PathTracer/VolumetricPathTracingPass.{hpp,cpp}
* and src/Data/Shaders/Vulkan/Scattering/Clouds/Clouds.glsl.
* 
* MIT License
* 
* Copyright (c) 2021, Christoph Neuhauser, Ludwig Leonard
* 
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

float rsqrt(float x) { return 1.0 / x; }

float saturate(float x) {
    if (isinf(x)) x = 1.0;
    return clamp(x, 0.0, 1.0);
}

/**
 * The OpenGL implementation of atan changes the order of x and y (compared to DirectX),
 * and is unstable for x close to 0.
 *
 * For more details see:
 * https://stackoverflow.com/questions/26070410/robust-atany-x-on-glsl-for-converting-xy-coordinate-to-angle
 */
float atan2(in float y, in float x) {
    bool s = (abs(x) > abs(y));
    return mix(PI / 2.0 - atan(x, y), atan(y, x), s);
}

vec2 mul(vec2 v, mat2 m) { return m * v; }

vec3 mul(vec3 v, mat3 m) { return m * v; }

vec4 mul(vec4 v, mat4 m) { return m * v; }

vec2 mul(mat2 m, vec2 v) { return v * m; }

vec3 mul(mat3 m, vec3 v) { return v * m; }

vec4 mul(mat4 m, vec4 v) { return v * m; }

/*! \file
   This header defines utility functions to deal with complex numbers and
   complex polynomials.*/

/*! Returns the complex conjugate of the given complex number (i.e. it changes
    the sign of the y-component).*/
vec2 Conjugate(vec2 Z) { return vec2(Z.x, -Z.y); }
/*! This function implements complex multiplication.*/
vec2 Multiply(vec2 LHS, vec2 RHS) {
    return vec2(LHS.x * RHS.x - LHS.y * RHS.y, LHS.x * RHS.y + LHS.y * RHS.x);
}
/*! This function computes the magnitude of the given complex number.*/
float Magnitude(vec2 Z) { return sqrt(dot(Z, Z)); }
/*! This function computes the quotient of two complex numbers. The denominator
    must not be zero.*/
vec2 Divide(vec2 Numerator, vec2 Denominator) {
    return vec2(Numerator.x * Denominator.x + Numerator.y * Denominator.y,
                -Numerator.x * Denominator.y + Numerator.y * Denominator.x) /
           dot(Denominator, Denominator);
}
/*! This function divides a real number by a complex number. The denominator
    must not be zero.*/
vec2 Divide(float Numerator, vec2 Denominator) {
    return vec2(Numerator * Denominator.x, -Numerator * Denominator.y) /
           dot(Denominator, Denominator);
}
/*! This function implements computation of the reciprocal of the given non-
    zero complex number.*/
vec2 Reciprocal(vec2 Z) { return vec2(Z.x, -Z.y) / dot(Z, Z); }
/*! This utility function implements complex squaring.*/
vec2 Square(vec2 Z) { return vec2(Z.x * Z.x - Z.y * Z.y, 2.0f * Z.x * Z.y); }
/*! This utility function implements complex computation of the third power.*/
vec2 Cube(vec2 Z) { return Multiply(Square(Z), Z); }
/*! This utility function computes one square root of the given complex value.
    The other one can be found using the unary minus operator.
  \warning This function is continuous but not defined on the negative real
            axis (and cannot be continued continuously there).
  \sa SquareRoot() */
vec2 SquareRootUnsafe(vec2 Z) {
    float ZLengthSq = dot(Z, Z);
    float ZLengthInv = 1.0 / sqrt(ZLengthSq);
    vec2 UnnormalizedRoot = Z * ZLengthInv + vec2(1.0f, 0.0f);
    float UnnormalizedRootLengthSq = dot(UnnormalizedRoot, UnnormalizedRoot);
    float NormalizationFactorInvSq = UnnormalizedRootLengthSq * ZLengthInv;
    float NormalizationFactor = 1.0 / sqrt(NormalizationFactorInvSq);
    return NormalizationFactor * UnnormalizedRoot;
}
/*! This utility function computes one square root of the given complex value.
    The other one can be found using the unary minus operator.
  \note This function has discontinuities for values with real part zero.
  \sa SquareRootUnsafe() */
vec2 SquareRoot(vec2 Z) {
    vec2 ZPositiveRealPart = vec2(abs(Z.x), Z.y);
    vec2 ComputedRoot = SquareRootUnsafe(ZPositiveRealPart);
    return (Z.x >= 0.0) ? ComputedRoot : ComputedRoot.yx;
}
/*! This utility function computes one cubic root of the given complex value. The
   other roots can be found by multiplication by cubic roots of unity.
  \note This function has various discontinuities.*/
vec2 CubicRoot(vec2 Z) {
    float Argument = atan2(Z.y, Z.x);
    float NewArgument = Argument / 3.0f;
    vec2 NormalizedRoot = vec2(cos(NewArgument), sin(NewArgument));
    return NormalizedRoot * pow(dot(Z, Z), 1.0f / 6.0f);
}

/*! @{
   Returns the complex conjugate of the given complex vector (i.e. it changes the
   second column resp the y-component).*/
mat2x2 Conjugate(mat2x2 Vector) {
    return mat2x2(Vector[0].x, -Vector[0].y, Vector[1].x, -Vector[1].y);
}
mat3x2 Conjugate(mat3x2 Vector) {
    return mat3x2(Vector[0].x, -Vector[0].y, Vector[1].x, -Vector[1].y, Vector[2].x, -Vector[2].y);
}
mat4x2 Conjugate(mat4x2 Vector) {
    return mat4x2(Vector[0].x, -Vector[0].y, Vector[1].x, -Vector[1].y, Vector[2].x, -Vector[2].y,
                  Vector[3].x, -Vector[3].y);
}
void Conjugate(out vec2 OutConjugateVector[5], vec2 Vector[5]) {
    //[unroll]
    for (int i = 0; i != 5; ++i) {
        OutConjugateVector[i] = vec2(Vector[i].x, -Vector[i].x);
    }
}
//!@}

/*! Returns the real part of a complex number as real.*/
float RealPart(vec2 Z) { return Z.x; }

/*! Given coefficients of a quadratic polynomial A*x^2+B*x+C, this function
    outputs its two complex roots.*/
void SolveQuadratic(out vec2 pOutRoot[2], vec2 A, vec2 B, vec2 C) {
    // Normalize the coefficients
    vec2 InvA = Reciprocal(A);
    B = Multiply(B, InvA);
    C = Multiply(C, InvA);
    // Divide the middle coefficient by two
    B *= 0.5f;
    // Apply the quadratic formula
    vec2 DiscriminantRoot = SquareRoot(Square(B) - C);
    pOutRoot[0] = -B - DiscriminantRoot;
    pOutRoot[1] = -B + DiscriminantRoot;
}

/*! Given coefficients of a cubic polynomial A*x^3+B*x^2+C*x+D, this function
    outputs its three complex roots.*/
void SolveCubicBlinn(out vec2 pOutRoot[3], vec2 A, vec2 B, vec2 C, vec2 D) {
    // Normalize the polynomial
    vec2 InvA = Reciprocal(A);
    B = Multiply(B, InvA);
    C = Multiply(C, InvA);
    D = Multiply(D, InvA);
    // Divide middle coefficients by three
    B /= 3.0f;
    C /= 3.0f;
    // Compute the Hessian and the discriminant
    vec2 Delta00 = -Square(B) + C;
    vec2 Delta01 = -Multiply(C, B) + D;
    vec2 Delta11 = Multiply(B, D) - Square(C);
    vec2 Discriminant = 4.0f * Multiply(Delta00, Delta11) - Square(Delta01);
    // Compute coefficients of the depressed cubic
    // (third is zero, fourth is one)
    vec2 DepressedD = -2.0f * Multiply(B, Delta00) + Delta01;
    vec2 DepressedC = Delta00;
    // Take the cubic root of a complex number avoiding cancellation
    vec2 DiscriminantRoot = SquareRoot(-Discriminant);
    DiscriminantRoot = faceforward(DiscriminantRoot, DiscriminantRoot, DepressedD);
    vec2 CubedRoot = DiscriminantRoot - DepressedD;
    vec2 FirstRoot = CubicRoot(0.5f * CubedRoot);
    vec2 pCubicRoot[3] = {FirstRoot, Multiply(vec2(-0.5f, -0.5f * sqrt(3.0f)), FirstRoot),
                          Multiply(vec2(-0.5f, 0.5f * sqrt(3.0f)), FirstRoot)};
    // Also compute the reciprocal cubic roots
    vec2 InvFirstRoot = Reciprocal(FirstRoot);
    vec2 pInvCubicRoot[3] = {InvFirstRoot, Multiply(vec2(-0.5f, 0.5f * sqrt(3.0f)), InvFirstRoot),
                             Multiply(vec2(-0.5f, -0.5f * sqrt(3.0f)), InvFirstRoot)};
    // Turn them into roots of the depressed cubic and revert the depression
    // transform
    //[unroll]
    for (int i = 0; i != 3; ++i) {
        pOutRoot[i] = pCubicRoot[i] - Multiply(DepressedC, pInvCubicRoot[i]) - B;
    }
}

/*! Given coefficients of a quartic polynomial A*x^4+B*x^3+C*x^2+D*x+E, this
    function outputs its four complex roots.*/
void SolveQuarticNeumark(out vec2 pOutRoot[4], vec2 A, vec2 B, vec2 C, vec2 D, vec2 E) {
    // Normalize the polynomial
    vec2 InvA = Reciprocal(A);
    B = Multiply(B, InvA);
    C = Multiply(C, InvA);
    D = Multiply(D, InvA);
    E = Multiply(E, InvA);
    // Construct a normalized cubic
    vec2 P = -2.0f * C;
    vec2 Q = Square(C) + Multiply(B, D) - 4.0f * E;
    vec2 R = Square(D) + Multiply(Square(B), E) - Multiply(Multiply(B, C), D);
    // Compute a root that is not the smallest of the cubic
    vec2 pCubicRoot[3];
    SolveCubicBlinn(pCubicRoot, vec2(1.0f, 0.0f), P, Q, R);
    vec2 y = (dot(pCubicRoot[1], pCubicRoot[1]) > dot(pCubicRoot[0], pCubicRoot[0]))
                 ? pCubicRoot[1]
                 : pCubicRoot[0];

    // Solve a quadratic to obtain linear coefficients for quadratic polynomials
    vec2 BB = Square(B);
    vec2 fy = 4.0f * y;
    vec2 BB_fy = BB - fy;
    vec2 tmp = SquareRoot(BB_fy);
    vec2 G = (B + tmp) * 0.5f;
    vec2 g = (B - tmp) * 0.5f;
    // Construct the corresponding constant coefficients
    vec2 Z = C - y;
    tmp = Divide(0.5f * Multiply(B, Z) - D, tmp);
    vec2 H = Z * 0.5f + tmp;
    vec2 h = Z * 0.5f - tmp;

    // Compute the roots
    vec2 pQuadraticRoot[2];
    SolveQuadratic(pQuadraticRoot, vec2(1.0f, 0.0f), G, H);
    pOutRoot[0] = pQuadraticRoot[0];
    pOutRoot[1] = pQuadraticRoot[1];
    SolveQuadratic(pQuadraticRoot, vec2(1.0f, 0.0f), g, h);
    pOutRoot[2] = pQuadraticRoot[0];
    pOutRoot[3] = pQuadraticRoot[1];
}

/*! \file
    This header provides the utility functions to reconstruct the transmittance
    from a given vector of trigonometric moments (2, 3 or 4 trigonometric
    moments) at a specified depth.*/

/*! This utility function turns a point on the unit circle into a scalar
    parameter. It is guaranteed to grow monotonically for (cos(phi),sin(phi))
    with phi in 0 to 2*pi. There are no other guarantees. In particular it is
    not an arclength parametrization. If you change this function, you must
    also change circleToParameter() in MomentOIT.cpp.*/
float circleToParameter(vec2 circle_point) {
    float result = abs(circle_point.y) - abs(circle_point.x);
    result = (circle_point.x < 0.0f) ? (2.0f - result) : result;
    return (circle_point.y < 0.0f) ? (6.0f - result) : result;
}

/*! This utility function returns the appropriate weight factor for a root at
    the given location. Both inputs are supposed to be unit vectors. If a
    circular arc going counter clockwise from (1.0,0.0) meets root first, it
    returns 1.0, otherwise 0.0 or a linear ramp in the wrapping zone.*/
float getRootWeightFactor(float reference_parameter, float root_parameter,
                          vec4 wrapping_zone_parameters) {
    float binary_weigth_factor = (root_parameter < reference_parameter) ? 1.0f : 0.0f;
    float linear_weigth_factor = clamp(
        fma(root_parameter, wrapping_zone_parameters.z, wrapping_zone_parameters.w), 0.0, 1.0);
    return binary_weigth_factor + linear_weigth_factor;
}

/*! This function reconstructs the transmittance at the given depth from two
    normalized trigonometric moments.*/
float approximate2TrigonometricMoments(float b_0, vec2 trig_b[2], float depth, float bias,
                                       float overestimation, vec4 wrapping_zone_parameters) {
    // Apply biasing and reformat the inputs a little bit
    float moment_scale = 1.0f - bias;
    vec2 b[3] = {vec2(1.0f, 0.0f), trig_b[0] * moment_scale, trig_b[1] * moment_scale};
    // Compute a Cholesky factorization of the Toeplitz matrix
    float D00 = RealPart(b[0]);
    float InvD00 = 1.0f / D00;
    vec2 L10 = (b[1]) * InvD00;
    float D11 = RealPart(b[0] - D00 * Multiply(L10, Conjugate(L10)));
    float InvD11 = 1.0f / D11;
    vec2 L20 = (b[2]) * InvD00;
    vec2 L21 = (b[1] - D00 * Multiply(L20, Conjugate(L10))) * InvD11;
    float D22 =
        RealPart(b[0] - D00 * Multiply(L20, Conjugate(L20)) - D11 * Multiply(L21, Conjugate(L21)));
    float InvD22 = 1.0f / D22;
    // Solve a linear system to get the relevant polynomial
    float phase = fma(depth, wrapping_zone_parameters.y, wrapping_zone_parameters.y);
    vec2 circle_point = vec2(cos(phase), sin(phase));
    vec2 c[3] = {vec2(1.0f, 0.0f), circle_point, Multiply(circle_point, circle_point)};
    c[1] -= Multiply(L10, c[0]);
    c[2] -= Multiply(L20, c[0]) + Multiply(L21, c[1]);
    c[0] *= InvD00;
    c[1] *= InvD11;
    c[2] *= InvD22;
    c[1] -= Multiply(Conjugate(L21), c[2]);
    c[0] -= Multiply(Conjugate(L10), c[1]) + Multiply(Conjugate(L20), c[2]);
    // Compute roots of the polynomial
    vec2 pRoot[2];
    SolveQuadratic(pRoot, Conjugate(c[2]), Conjugate(c[1]), Conjugate(c[0]));
    // Figure out how to weight the weights
    float depth_parameter = circleToParameter(circle_point);
    vec3 weigth_factor;
    weigth_factor[0] = overestimation;
    //[unroll]
    for (int i = 0; i != 2; ++i) {
        float root_parameter = circleToParameter(pRoot[i]);
        weigth_factor[i + 1] =
            getRootWeightFactor(depth_parameter, root_parameter, wrapping_zone_parameters);
    }
    // Compute the appropriate linear combination of weights
    vec2 z[3] = {circle_point, pRoot[0], pRoot[1]};
    float f0 = weigth_factor[0];
    float f1 = weigth_factor[1];
    float f2 = weigth_factor[2];
    vec2 f01 = Divide(f1 - f0, z[1] - z[0]);
    vec2 f12 = Divide(f2 - f1, z[2] - z[1]);
    vec2 f012 = Divide(f12 - f01, z[2] - z[0]);
    vec2 polynomial[3];
    polynomial[0] = f012;
    polynomial[1] = polynomial[0];
    polynomial[0] = f01 - Multiply(polynomial[0], z[1]);
    polynomial[2] = polynomial[1];
    polynomial[1] = polynomial[0] - Multiply(polynomial[1], z[0]);
    polynomial[0] = f0 - Multiply(polynomial[0], z[0]);
    float weight_sum = 0.0f;
    weight_sum += RealPart(Multiply(b[0], polynomial[0]));
    weight_sum += RealPart(Multiply(b[1], polynomial[1]));
    weight_sum += RealPart(Multiply(b[2], polynomial[2]));
    // Turn the normalized absorbance into transmittance
    return b_0 * weight_sum;
}

/*! This function reconstructs the transmittance at the given depth from three
    normalized trigonometric moments. */
float approximate3TrigonometricMoments(float b_0, vec2 trig_b[3], float depth, float bias,
                                       float overestimation, vec4 wrapping_zone_parameters) {
    // Apply biasing and reformat the inputs a little bit
    float moment_scale = 1.0f - bias;
    vec2 b[4] = {vec2(1.0f, 0.0f), trig_b[0] * moment_scale, trig_b[1] * moment_scale,
                 trig_b[2] * moment_scale};
    // Compute a Cholesky factorization of the Toeplitz matrix
    float D00 = RealPart(b[0]);
    float InvD00 = 1.0f / D00;
    vec2 L10 = (b[1]) * InvD00;
    float D11 = RealPart(b[0] - D00 * Multiply(L10, Conjugate(L10)));
    float InvD11 = 1.0f / D11;
    vec2 L20 = (b[2]) * InvD00;
    vec2 L21 = (b[1] - D00 * Multiply(L20, Conjugate(L10))) * InvD11;
    float D22 =
        RealPart(b[0] - D00 * Multiply(L20, Conjugate(L20)) - D11 * Multiply(L21, Conjugate(L21)));
    float InvD22 = 1.0f / D22;
    vec2 L30 = (b[3]) * InvD00;
    vec2 L31 = (b[2] - D00 * Multiply(L30, Conjugate(L10))) * InvD11;
    vec2 L32 =
        (b[1] - D00 * Multiply(L30, Conjugate(L20)) - D11 * Multiply(L31, Conjugate(L21))) * InvD22;
    float D33 = RealPart(b[0] - D00 * Multiply(L30, Conjugate(L30)) -
                         D11 * Multiply(L31, Conjugate(L31)) - D22 * Multiply(L32, Conjugate(L32)));
    float InvD33 = 1.0f / D33;
    // Solve a linear system to get the relevant polynomial
    float phase = fma(depth, wrapping_zone_parameters.y, wrapping_zone_parameters.y);
    vec2 circle_point = vec2(cos(phase), sin(phase));
    vec2 circle_point_pow2 = Multiply(circle_point, circle_point);
    vec2 c[4] = {vec2(1.0f, 0.0f), circle_point, circle_point_pow2,
                 Multiply(circle_point, circle_point_pow2)};
    c[1] -= Multiply(L10, c[0]);
    c[2] -= Multiply(L20, c[0]) + Multiply(L21, c[1]);
    c[3] -= Multiply(L30, c[0]) + Multiply(L31, c[1]) + Multiply(L32, c[2]);
    c[0] *= InvD00;
    c[1] *= InvD11;
    c[2] *= InvD22;
    c[3] *= InvD33;
    c[2] -= Multiply(Conjugate(L32), c[3]);
    c[1] -= Multiply(Conjugate(L21), c[2]) + Multiply(Conjugate(L31), c[3]);
    c[0] -= Multiply(Conjugate(L10), c[1]) + Multiply(Conjugate(L20), c[2]) +
            Multiply(Conjugate(L30), c[3]);
    // Compute roots of the polynomial
    vec2 pRoot[3];
    SolveCubicBlinn(pRoot, Conjugate(c[3]), Conjugate(c[2]), Conjugate(c[1]), Conjugate(c[0]));
    // The roots are known to be normalized but for reasons of numerical
    // stability it can be better to enforce that
    // pRoot[0]=normalize(pRoot[0]);
    // pRoot[1]=normalize(pRoot[1]);
    // pRoot[2]=normalize(pRoot[2]);
    // Figure out how to weight the weights
    float depth_parameter = circleToParameter(circle_point);
    vec4 weigth_factor;
    weigth_factor[0] = overestimation;
    //[unroll]
    for (int i = 0; i != 3; ++i) {
        float root_parameter = circleToParameter(pRoot[i]);
        weigth_factor[i + 1] =
            getRootWeightFactor(depth_parameter, root_parameter, wrapping_zone_parameters);
    }
    // Compute the appropriate linear combination of weights
    vec2 z[4] = {circle_point, pRoot[0], pRoot[1], pRoot[2]};
    float f0 = weigth_factor[0];
    float f1 = weigth_factor[1];
    float f2 = weigth_factor[2];
    float f3 = weigth_factor[3];
    vec2 f01 = Divide(f1 - f0, z[1] - z[0]);
    vec2 f12 = Divide(f2 - f1, z[2] - z[1]);
    vec2 f23 = Divide(f3 - f2, z[3] - z[2]);
    vec2 f012 = Divide(f12 - f01, z[2] - z[0]);
    vec2 f123 = Divide(f23 - f12, z[3] - z[1]);
    vec2 f0123 = Divide(f123 - f012, z[3] - z[0]);
    vec2 polynomial[4];
    polynomial[0] = f0123;
    polynomial[1] = polynomial[0];
    polynomial[0] = f012 - Multiply(polynomial[0], z[2]);
    polynomial[2] = polynomial[1];
    polynomial[1] = polynomial[0] - Multiply(polynomial[1], z[1]);
    polynomial[0] = f01 - Multiply(polynomial[0], z[1]);
    polynomial[3] = polynomial[2];
    polynomial[2] = polynomial[1] - Multiply(polynomial[2], z[0]);
    polynomial[1] = polynomial[0] - Multiply(polynomial[1], z[0]);
    polynomial[0] = f0 - Multiply(polynomial[0], z[0]);
    float weight_sum = 0;
    weight_sum += RealPart(Multiply(b[0], polynomial[0]));
    weight_sum += RealPart(Multiply(b[1], polynomial[1]));
    weight_sum += RealPart(Multiply(b[2], polynomial[2]));
    weight_sum += RealPart(Multiply(b[3], polynomial[3]));
    // Turn the normalized absorbance into transmittance
    return b_0 * weight_sum;
}

/*! This function reconstructs the transmittance at the given depth from four
    normalized trigonometric moments.*/
float approximate4TrigonometricMoments(float b_0, vec2 trig_b[4], float depth, float bias,
                                       float overestimation, vec4 wrapping_zone_parameters) {
    // Apply biasing and reformat the inputs a little bit
    float moment_scale = 1.0f - bias;
    vec2 b[5] = {vec2(1.0f, 0.0f), trig_b[0] * moment_scale, trig_b[1] * moment_scale,
                 trig_b[2] * moment_scale, trig_b[3] * moment_scale};
    // Compute a Cholesky factorization of the Toeplitz matrix
    float D00 = RealPart(b[0]);
    float InvD00 = 1.0 / D00;
    vec2 L10 = (b[1]) * InvD00;
    float D11 = RealPart(b[0] - D00 * Multiply(L10, Conjugate(L10)));
    float InvD11 = 1.0 / D11;
    vec2 L20 = (b[2]) * InvD00;
    vec2 L21 = (b[1] - D00 * Multiply(L20, Conjugate(L10))) * InvD11;
    float D22 =
        RealPart(b[0] - D00 * Multiply(L20, Conjugate(L20)) - D11 * Multiply(L21, Conjugate(L21)));
    float InvD22 = 1.0 / D22;
    vec2 L30 = (b[3]) * InvD00;
    vec2 L31 = (b[2] - D00 * Multiply(L30, Conjugate(L10))) * InvD11;
    vec2 L32 =
        (b[1] - D00 * Multiply(L30, Conjugate(L20)) - D11 * Multiply(L31, Conjugate(L21))) * InvD22;
    float D33 = RealPart(b[0] - D00 * Multiply(L30, Conjugate(L30)) -
                         D11 * Multiply(L31, Conjugate(L31)) - D22 * Multiply(L32, Conjugate(L32)));
    float InvD33 = 1.0 / D33;
    vec2 L40 = (b[4]) * InvD00;
    vec2 L41 = (b[3] - D00 * Multiply(L40, Conjugate(L10))) * InvD11;
    vec2 L42 =
        (b[2] - D00 * Multiply(L40, Conjugate(L20)) - D11 * Multiply(L41, Conjugate(L21))) * InvD22;
    vec2 L43 = (b[1] - D00 * Multiply(L40, Conjugate(L30)) - D11 * Multiply(L41, Conjugate(L31)) -
                D22 * Multiply(L42, Conjugate(L32))) *
               InvD33;
    float D44 =
        RealPart(b[0] - D00 * Multiply(L40, Conjugate(L40)) - D11 * Multiply(L41, Conjugate(L41)) -
                 D22 * Multiply(L42, Conjugate(L42)) - D33 * Multiply(L43, Conjugate(L43)));
    float InvD44 = 1.0 / D44;
    // Solve a linear system to get the relevant polynomial
    float phase = fma(depth, wrapping_zone_parameters.y, wrapping_zone_parameters.y);
    vec2 circle_point = vec2(cos(phase), sin(phase));
    vec2 circle_point_pow2 = Multiply(circle_point, circle_point);
    vec2 c[5] = {vec2(1.0f, 0.0f), circle_point, circle_point_pow2,
                 Multiply(circle_point, circle_point_pow2),
                 Multiply(circle_point_pow2, circle_point_pow2)};
    c[1] -= Multiply(L10, c[0]);
    c[2] -= Multiply(L20, c[0]) + Multiply(L21, c[1]);
    c[3] -= Multiply(L30, c[0]) + Multiply(L31, c[1]) + Multiply(L32, c[2]);
    c[4] -= Multiply(L40, c[0]) + Multiply(L41, c[1]) + Multiply(L42, c[2]) + Multiply(L43, c[3]);
    c[0] *= InvD00;
    c[1] *= InvD11;
    c[2] *= InvD22;
    c[3] *= InvD33;
    c[4] *= InvD44;
    c[3] -= Multiply(Conjugate(L43), c[4]);
    c[2] -= Multiply(Conjugate(L32), c[3]) + Multiply(Conjugate(L42), c[4]);
    c[1] -= Multiply(Conjugate(L21), c[2]) + Multiply(Conjugate(L31), c[3]) +
            Multiply(Conjugate(L41), c[4]);
    c[0] -= Multiply(Conjugate(L10), c[1]) + Multiply(Conjugate(L20), c[2]) +
            Multiply(Conjugate(L30), c[3]) + Multiply(Conjugate(L40), c[4]);
    // Compute roots of the polynomial
    vec2 pRoot[4];
    SolveQuarticNeumark(pRoot, Conjugate(c[4]), Conjugate(c[3]), Conjugate(c[2]), Conjugate(c[1]),
                        Conjugate(c[0]));
    // Figure out how to weight the weights
    float depth_parameter = circleToParameter(circle_point);
    float weigth_factor[5];
    weigth_factor[0] = overestimation;
    //[unroll]
    for (int i = 0; i != 4; ++i) {
        float root_parameter = circleToParameter(pRoot[i]);
        weigth_factor[i + 1] =
            getRootWeightFactor(depth_parameter, root_parameter, wrapping_zone_parameters);
    }
    // Compute the appropriate linear combination of weights
    vec2 z[5] = {circle_point, pRoot[0], pRoot[1], pRoot[2], pRoot[3]};
    float f0 = weigth_factor[0];
    float f1 = weigth_factor[1];
    float f2 = weigth_factor[2];
    float f3 = weigth_factor[3];
    float f4 = weigth_factor[4];
    vec2 f01 = Divide(f1 - f0, z[1] - z[0]);
    vec2 f12 = Divide(f2 - f1, z[2] - z[1]);
    vec2 f23 = Divide(f3 - f2, z[3] - z[2]);
    vec2 f34 = Divide(f4 - f3, z[4] - z[3]);
    vec2 f012 = Divide(f12 - f01, z[2] - z[0]);
    vec2 f123 = Divide(f23 - f12, z[3] - z[1]);
    vec2 f234 = Divide(f34 - f23, z[4] - z[2]);
    vec2 f0123 = Divide(f123 - f012, z[3] - z[0]);
    vec2 f1234 = Divide(f234 - f123, z[4] - z[1]);
    vec2 f01234 = Divide(f1234 - f0123, z[4] - z[0]);
    vec2 polynomial[5];
    polynomial[0] = f01234;
    polynomial[1] = polynomial[0];
    polynomial[0] = f0123 - Multiply(polynomial[0], z[3]);
    polynomial[2] = polynomial[1];
    polynomial[1] = polynomial[0] - Multiply(polynomial[1], z[2]);
    polynomial[0] = f012 - Multiply(polynomial[0], z[2]);
    polynomial[3] = polynomial[2];
    polynomial[2] = polynomial[1] - Multiply(polynomial[2], z[1]);
    polynomial[1] = polynomial[0] - Multiply(polynomial[1], z[1]);
    polynomial[0] = f01 - Multiply(polynomial[0], z[1]);
    polynomial[4] = polynomial[3];
    polynomial[3] = polynomial[2] - Multiply(polynomial[3], z[0]);
    polynomial[2] = polynomial[1] - Multiply(polynomial[2], z[0]);
    polynomial[1] = polynomial[0] - Multiply(polynomial[1], z[0]);
    polynomial[0] = f0 - Multiply(polynomial[0], z[0]);
    float weight_sum = 0;
    weight_sum += RealPart(Multiply(b[0], polynomial[0]));
    weight_sum += RealPart(Multiply(b[1], polynomial[1]));
    weight_sum += RealPart(Multiply(b[2], polynomial[2]));
    weight_sum += RealPart(Multiply(b[3], polynomial[3]));
    weight_sum += RealPart(Multiply(b[4], polynomial[4]));
    // Turn the normalized absorbance into transmittance
    return b_0 * weight_sum;
}
