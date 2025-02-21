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

layout(std430, binding = 13) buffer MomentSettings {
    vec4 wrapping_zone_parameters;
    float wrapping_zone_angle;
    float overestimation;
};

float approximate4PowerMoments(float b_0, vec2 b_even, vec2 b_odd, float depth, float bias,
                               float overestimation, vec4 bias_vector);
float approximate6PowerMoments(float b_0, vec3 b_even, vec3 b_odd, float depth, float bias,
                               float overestimation, float bias_vector[6]);
float approximate8PowerMoments(float b_0, vec4 b_even, vec4 b_odd, float depth, float bias,
                               float overestimation, float bias_vector[8]);

#ifdef COEFF_TEX_FIXED_POINT_FACTOR
void project(layout(r32i) iimage2DArray coeffTex, int N, float depth, float val)
#else
void project(layout(size1x32) image2DArray coeffTex, int N, float depth, float val)
#endif
{
    float m_n = 1;
    for (int i = 0; i < N; i++) {
        float projVal = val * m_n;
        m_n *= depth;

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
    if (N == 5) {
        float b_0;
        vec2 b_even;
        vec2 b_odd;

        for (int i = 0; i < N; i++) {
            ivec3 coord = ivec3(gl_FragCoord.xy, i);
            #ifdef COEFF_TEX_FIXED_POINT_FACTOR
                float coeff = float(imageLoad(coeffTex, coord).x) / COEFF_TEX_FIXED_POINT_FACTOR;
            #else
                float coeff = imageLoad(coeffTex, coord).x;
            #endif

            if (i == 0) b_0 = coeff;
            else if (i % 2 == 1) b_odd[(i - 1) / 2] = coeff / b_0;
            else b_even[(i - 2) / 2] = coeff / b_0;
        }

        float bias = 5e-7;
        const vec4 bias_vector = vec4(0, 0.375, 0, 0.375);
        return approximate4PowerMoments(b_0, b_even, b_odd, depth, bias, overestimation,
                                        bias_vector);
    } else if (N == 7) {
        float b_0;
        vec3 b_even;
        vec3 b_odd;

        for (int i = 0; i < N; i++) {
            ivec3 coord = ivec3(gl_FragCoord.xy, i);
            #ifdef COEFF_TEX_FIXED_POINT_FACTOR
                float coeff = float(imageLoad(coeffTex, coord).x) / COEFF_TEX_FIXED_POINT_FACTOR;
            #else
                float coeff = imageLoad(coeffTex, coord).x;
            #endif

            if (i == 0) b_0 = coeff;
            else if (i % 2 == 1) b_odd[(i - 1) / 2] = coeff / b_0;
            else b_even[(i - 2) / 2] = coeff / b_0;
        }

        float bias = 5e-6;
        const float bias_vector[6] = {0, 0.48, 0, 0.451, 0, 0.45};
        return approximate6PowerMoments(b_0, b_even, b_odd, depth, bias, overestimation,
                                        bias_vector);
    } else if (N == 9) {
        float b_0;
        vec4 b_even;
        vec4 b_odd;

        for (int i = 0; i < N; i++) {
            ivec3 coord = ivec3(gl_FragCoord.xy, i);
            #ifdef COEFF_TEX_FIXED_POINT_FACTOR
                float coeff = float(imageLoad(coeffTex, coord).x) / COEFF_TEX_FIXED_POINT_FACTOR;
            #else
                float coeff = imageLoad(coeffTex, coord).x;
            #endif

            if (i == 0) b_0 = coeff;
            else if (i % 2 == 1) b_odd[(i - 1) / 2] = coeff / b_0;
            else b_even[(i - 2) / 2] = coeff / b_0;
        }

        float bias = 5e-5;
        const float bias_vector[8] = {0, 0.75, 0, 0.67666666666666664,
                                      0, 0.63, 0, 0.60030303030303034};
        return approximate8PowerMoments(b_0, b_even, b_odd, depth, bias, overestimation,
                                        bias_vector);
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


float atan2(in float y, in float x) {
    bool s = (abs(x) > abs(y));
    return mix(PI / 2.0 - atan(x, y), atan(y, x), s);
}

vec2 solveQuadratic(vec3 coeffs) {
    coeffs[1] *= 0.5;

    float x1, x2, tmp;

    tmp = (coeffs[1] * coeffs[1] - coeffs[0] * coeffs[2]);
    if (coeffs[1] >= 0) {
        tmp = sqrt(tmp);
        x1 = (-coeffs[2]) / (coeffs[1] + tmp);
        x2 = (-coeffs[1] - tmp) / coeffs[0];
    } else {
        tmp = sqrt(tmp);
        x1 = (-coeffs[1] + tmp) / coeffs[0];
        x2 = coeffs[2] / (-coeffs[1] + tmp);
    }
    return vec2(x1, x2);
}

/*! Code taken from the blog "Moments in Graphics" by Christoph Peters.
    http://momentsingraphics.de/?p=105
    This function computes the three real roots of a cubic polynomial
    Coefficient[0]+Coefficient[1]*x+Coefficient[2]*x^2+Coefficient[3]*x^3.*/
vec3 SolveCubic(vec4 Coefficient) {
    // Normalize the polynomial
    Coefficient.xyz /= Coefficient.w;
    // Divide middle coefficients by three
    Coefficient.yz /= 3.0f;
    // Compute the Hessian and the discrimant
    vec3 Delta = vec3(fma(-Coefficient.z, Coefficient.z, Coefficient.y),
                      fma(-Coefficient.y, Coefficient.z, Coefficient.x),
                      dot(vec2(Coefficient.z, -Coefficient.y), Coefficient.xy));
    float Discriminant = dot(vec2(4.0f * Delta.x, -Delta.y), Delta.zy);
    // Compute coefficients of the depressed cubic
    // (third is zero, fourth is one)
    vec2 Depressed = vec2(fma(-2.0f * Coefficient.z, Delta.x, Delta.y), Delta.x);
    // Take the cubic root of a normalized complex number
    float Theta = atan2(sqrt(Discriminant), -Depressed.x) / 3.0f;
    vec2 CubicRoot = vec2(cos(Theta), sin(Theta));
    // Compute the three roots, scale appropriately and
    // revert the depression transform
    vec3 Root = vec3(CubicRoot.x, dot(vec2(-0.5f, -0.5f * sqrt(3.0f)), CubicRoot),
                     dot(vec2(-0.5f, 0.5f * sqrt(3.0f)), CubicRoot));
    Root = fma(vec3(2.0f * sqrt(-Depressed.y)), Root, vec3(-Coefficient.z));
    return Root;
}

/*! Given coefficients of a cubic polynomial
    coeffs[0]+coeffs[1]*x+coeffs[2]*x^2+coeffs[3]*x^3 with three real roots,
    this function returns the root of least magnitude.*/
float solveCubicBlinnSmallest(vec4 coeffs) {
    coeffs.xyz /= coeffs.w;
    coeffs.yz /= 3.0;

    vec3 delta = vec3(fma(-coeffs.z, coeffs.z, coeffs.y), fma(-coeffs.z, coeffs.y, coeffs.x),
                      coeffs.z * coeffs.x - coeffs.y * coeffs.y);
    float discriminant = 4.0 * delta.x * delta.z - delta.y * delta.y;

    vec2 depressed = vec2(delta.z, -coeffs.x * delta.y + 2.0 * coeffs.y * delta.z);
    float theta = abs(atan2(coeffs.x * sqrt(discriminant), -depressed.y)) / 3.0;
    vec2 sin_cos = vec2(sin(theta), cos(theta));
    float tmp = 2.0 * sqrt(-depressed.x);
    vec2 x = vec2(tmp * sin_cos.y, tmp * (-0.5 * sin_cos.y - 0.5 * sqrt(3.0) * sin_cos.x));
    vec2 s = (x.x + x.y < 2.0 * coeffs.y) ? vec2(-coeffs.x, x.x + coeffs.y)
                                          : vec2(-coeffs.x, x.y + coeffs.y);

    return s.x / s.y;
}

/*! Given coefficients of a quartic polynomial
    coeffs[0]+coeffs[1]*x+coeffs[2]*x^2+coeffs[3]*x^3+coeffs[4]*x^4 with four
    real roots, this function returns all roots.*/
vec4 solveQuarticNeumark(float coeffs[5]) {
    // Normalization
    float B = coeffs[3] / coeffs[4];
    float C = coeffs[2] / coeffs[4];
    float D = coeffs[1] / coeffs[4];
    float E = coeffs[0] / coeffs[4];

    // Compute coefficients of the cubic resolvent
    float P = -2.0 * C;
    float Q = C * C + B * D - 4.0 * E;
    float R = D * D + B * B * E - B * C * D;

    // Obtain the smallest cubic root
    float y = solveCubicBlinnSmallest(vec4(R, Q, P, 1.0));

    float BB = B * B;
    float fy = 4.0 * y;
    float BB_fy = BB - fy;

    float Z = C - y;
    float ZZ = Z * Z;
    float fE = 4.0 * E;
    float ZZ_fE = ZZ - fE;

    float G, g, H, h;
    // Compute the coefficients of the quadratics adaptively using the two
    // proposed factorizations by Neumark. Choose the appropriate
    // factorizations using the heuristic proposed by Herbison-Evans.
    if (y < 0 || (ZZ + fE) * BB_fy > ZZ_fE * (BB + fy)) {
        float tmp = sqrt(BB_fy);
        G = (B + tmp) * 0.5;
        g = (B - tmp) * 0.5;

        tmp = (B * Z - 2.0 * D) / (2.0 * tmp);
        H = fma(Z, 0.5, tmp);
        h = fma(Z, 0.5, -tmp);
    } else {
        float tmp = sqrt(ZZ_fE);
        H = (Z + tmp) * 0.5;
        h = (Z - tmp) * 0.5;

        tmp = (B * Z - 2.0 * D) / (2.0 * tmp);
        G = fma(B, 0.5, tmp);
        g = fma(B, 0.5, -tmp);
    }
    // Solve the quadratics
    return vec4(solveQuadratic(vec3(1.0, G, H)), solveQuadratic(vec3(1.0, g, h)));
}

/*! This function reconstructs the transmittance at the given depth from four
    normalized power moments and the given zeroth moment.*/
float approximate4PowerMoments(float b_0, vec2 b_even, vec2 b_odd, float depth,
                                                   float bias, float overestimation,
                                                   vec4 bias_vector) {
    vec4 b = vec4(b_odd.x, b_even.x, b_odd.y, b_even.y);
    // Bias input data to avoid artifacts
    b = mix(b, bias_vector, bias);
    vec3 z;
    z[0] = depth;

    // Compute a Cholesky factorization of the Hankel matrix B storing only non-
    // trivial entries or related products
    float L21D11 = fma(-b[0], b[1], b[2]);
    float D11 = fma(-b[0], b[0], b[1]);
    float InvD11 = 1.0f / D11;
    float L21 = L21D11 * InvD11;
    float SquaredDepthVariance = fma(-b[1], b[1], b[3]);
    float D22 = fma(-L21D11, L21, SquaredDepthVariance);

    // Obtain a scaled inverse image of bz=(1,z[0],z[0]*z[0])^T
    vec3 c = vec3(1.0f, z[0], z[0] * z[0]);
    // Forward substitution to solve L*c1=bz
    c[1] -= b.x;
    c[2] -= b.y + L21 * c[1];
    // Scaling to solve D*c2=c1
    c[1] *= InvD11;
    c[2] /= D22;
    // Backward substitution to solve L^T*c3=c2
    c[1] -= L21 * c[2];
    c[0] -= dot(c.yz, b.xy);
    // Solve the quadratic equation c[0]+c[1]*z+c[2]*z^2 to obtain solutions
    // z[1] and z[2]
    float InvC2 = 1.0f / c[2];
    float p = c[1] * InvC2;
    float q = c[0] * InvC2;
    float D = (p * p * 0.25f) - q;
    float r = sqrt(D);
    z[1] = -p * 0.5f - r;
    z[2] = -p * 0.5f + r;
    // Compute the absorbance by summing the appropriate weights
    vec3 polynomial;
    vec3 weigth_factor =
        vec3(overestimation, (z[1] < z[0]) ? 1.0f : 0.0f, (z[2] < z[0]) ? 1.0f : 0.0f);
    float f0 = weigth_factor[0];
    float f1 = weigth_factor[1];
    float f2 = weigth_factor[2];
    float f01 = (f1 - f0) / (z[1] - z[0]);
    float f12 = (f2 - f1) / (z[2] - z[1]);
    float f012 = (f12 - f01) / (z[2] - z[0]);
    polynomial[0] = f012;
    polynomial[1] = polynomial[0];
    polynomial[0] = f01 - polynomial[0] * z[1];
    polynomial[2] = polynomial[1];
    polynomial[1] = polynomial[0] - polynomial[1] * z[0];
    polynomial[0] = f0 - polynomial[0] * z[0];
    float absorbance = polynomial[0] + dot(b.xy, polynomial.yz);
    ;
    // Turn the normalized absorbance into transmittance
    return b_0 * absorbance;
}

/*! This function reconstructs the transmittance at the given depth from six
    normalized power moments and the given zeroth moment.*/
float approximate6PowerMoments(float b_0, vec3 b_even, vec3 b_odd, float depth,
                                                   float bias, float overestimation,
                                                   float bias_vector[6]) {
    float b[6] = {b_odd.x, b_even.x, b_odd.y, b_even.y, b_odd.z, b_even.z};
    // Bias input data to avoid artifacts
    //[unroll]
    for (int i = 0; i != 6; ++i) {
        b[i] = mix(b[i], bias_vector[i], bias);
    }

    vec4 z;
    z[0] = depth;

    // Compute a Cholesky factorization of the Hankel matrix B storing only non-
    // trivial entries or related products
    float InvD11 = 1.0f / fma(-b[0], b[0], b[1]);
    float L21D11 = fma(-b[0], b[1], b[2]);
    float L21 = L21D11 * InvD11;
    float D22 = fma(-L21D11, L21, fma(-b[1], b[1], b[3]));
    float L31D11 = fma(-b[0], b[2], b[3]);
    float L31 = L31D11 * InvD11;
    float InvD22 = 1.0f / D22;
    float L32D22 = fma(-L21D11, L31, fma(-b[1], b[2], b[4]));
    float L32 = L32D22 * InvD22;
    float D33 = fma(-b[2], b[2], b[5]) - dot(vec2(L31D11, L32D22), vec2(L31, L32));
    float InvD33 = 1.0f / D33;

    // Construct the polynomial whose roots have to be points of support of the
    // canonical distribution: bz=(1,z[0],z[0]*z[0],z[0]*z[0]*z[0])^T
    vec4 c;
    c[0] = 1.0f;
    c[1] = z[0];
    c[2] = c[1] * z[0];
    c[3] = c[2] * z[0];
    // Forward substitution to solve L*c1=bz
    c[1] -= b[0];
    c[2] -= fma(L21, c[1], b[1]);
    c[3] -= b[2] + dot(vec2(L31, L32), c.yz);
    // Scaling to solve D*c2=c1
    c.yzw *= vec3(InvD11, InvD22, InvD33);
    // Backward substitution to solve L^T*c3=c2
    c[2] -= L32 * c[3];
    c[1] -= dot(vec2(L21, L31), c.zw);
    c[0] -= dot(vec3(b[0], b[1], b[2]), c.yzw);

    // Solve the cubic equation
    z.yzw = SolveCubic(c);

    // Compute the absorbance by summing the appropriate weights
    vec4 weigth_factor;
    weigth_factor[0] = overestimation;
    // weigth_factor.yzw = (z.yzw > z.xxx) ? vec3 (0.0f, 0.0f, 0.0f) : vec3 (1.0f, 1.0f, 1.0f);
    // weigth_factor = vec4(overestimation, (z[1] < z[0])?1.0f:0.0f, (z[2] < z[0])?1.0f:0.0f, (z[3]
    // < z[0])?1.0f:0.0f);
    weigth_factor.yzw =
        mix(vec3(1.0f, 1.0f, 1.0f), vec3(0.0f, 0.0f, 0.0f), ivec3(greaterThan(z.yzw, z.xxx)));
    // Construct an interpolation polynomial
    float f0 = weigth_factor[0];
    float f1 = weigth_factor[1];
    float f2 = weigth_factor[2];
    float f3 = weigth_factor[3];
    float f01 = (f1 - f0) / (z[1] - z[0]);
    float f12 = (f2 - f1) / (z[2] - z[1]);
    float f23 = (f3 - f2) / (z[3] - z[2]);
    float f012 = (f12 - f01) / (z[2] - z[0]);
    float f123 = (f23 - f12) / (z[3] - z[1]);
    float f0123 = (f123 - f012) / (z[3] - z[0]);
    vec4 polynomial;
    // f012+f0123 *(z-z2)
    polynomial[0] = fma(-f0123, z[2], f012);
    polynomial[1] = f0123;
    // *(z-z1) +f01
    polynomial[2] = polynomial[1];
    polynomial[1] = fma(polynomial[1], -z[1], polynomial[0]);
    polynomial[0] = fma(polynomial[0], -z[1], f01);
    // *(z-z0) +f0
    polynomial[3] = polynomial[2];
    polynomial[2] = fma(polynomial[2], -z[0], polynomial[1]);
    polynomial[1] = fma(polynomial[1], -z[0], polynomial[0]);
    polynomial[0] = fma(polynomial[0], -z[0], f0);
    float absorbance = dot(polynomial, vec4(1.0, b[0], b[1], b[2]));
    // Turn the normalized absorbance into transmittance
    return b_0 * absorbance;
}

/*! This function reconstructs the transmittance at the given depth from eight
    normalized power moments and the given zeroth moment.*/
float approximate8PowerMoments(float b_0, vec4 b_even, vec4 b_odd, float depth,
                                                   float bias, float overestimation,
                                                   float bias_vector[8]) {
    float b[8] = {b_odd.x, b_even.x, b_odd.y, b_even.y, b_odd.z, b_even.z, b_odd.w, b_even.w};
    // Bias input data to avoid artifacts
    //[unroll]
    for (int i = 0; i != 8; ++i) {
        b[i] = mix(b[i], bias_vector[i], bias);
    }

    float z[5];
    z[0] = depth;

    // Compute a Cholesky factorization of the Hankel matrix B storing only non-trivial entries or
    // related products
    float D22 = fma(-b[0], b[0], b[1]);
    float InvD22 = 1.0 / D22;
    float L32D22 = fma(-b[1], b[0], b[2]);
    float L32 = L32D22 * InvD22;
    float L42D22 = fma(-b[2], b[0], b[3]);
    float L42 = L42D22 * InvD22;
    float L52D22 = fma(-b[3], b[0], b[4]);
    float L52 = L52D22 * InvD22;

    float D33 = fma(-L32, L32D22, fma(-b[1], b[1], b[3]));
    float InvD33 = 1.0 / D33;
    float L43D33 = fma(-L42, L32D22, fma(-b[2], b[1], b[4]));
    float L43 = L43D33 * InvD33;
    float L53D33 = fma(-L52, L32D22, fma(-b[3], b[1], b[5]));
    float L53 = L53D33 * InvD33;

    float D44 = fma(-b[2], b[2], b[5]) - dot(vec2(L42, L43), vec2(L42D22, L43D33));
    float InvD44 = 1.0 / D44;
    float L54D44 = fma(-b[3], b[2], b[6]) - dot(vec2(L52, L53), vec2(L42D22, L43D33));
    float L54 = L54D44 * InvD44;

    float D55 = fma(-b[3], b[3], b[7]) - dot(vec3(L52, L53, L54), vec3(L52D22, L53D33, L54D44));
    float InvD55 = 1.0 / D55;

    // Construct the polynomial whose roots have to be points of support of the
    // Canonical distribution:
    // bz = (1,z[0],z[0]^2,z[0]^3,z[0]^4)^T
    float c[5];
    c[0] = 1.0;
    c[1] = z[0];
    c[2] = c[1] * z[0];
    c[3] = c[2] * z[0];
    c[4] = c[3] * z[0];

    // Forward substitution to solve L*c1 = bz
    c[1] -= b[0];
    c[2] -= fma(L32, c[1], b[1]);
    c[3] -= b[2] + dot(vec2(L42, L43), vec2(c[1], c[2]));
    c[4] -= b[3] + dot(vec3(L52, L53, L54), vec3(c[1], c[2], c[3]));

    // Scaling to solve D*c2 = c1
    // c = c .*[1, InvD22, InvD33, InvD44, InvD55];
    c[1] *= InvD22;
    c[2] *= InvD33;
    c[3] *= InvD44;
    c[4] *= InvD55;

    // Backward substitution to solve L^T*c3 = c2
    c[3] -= L54 * c[4];
    c[2] -= dot(vec2(L53, L43), vec2(c[4], c[3]));
    c[1] -= dot(vec3(L52, L42, L32), vec3(c[4], c[3], c[2]));
    c[0] -= dot(vec4(b[3], b[2], b[1], b[0]), vec4(c[4], c[3], c[2], c[1]));

    // Solve the quartic equation
    vec4 zz = solveQuarticNeumark(c);
    z[1] = zz[0];
    z[2] = zz[1];
    z[3] = zz[2];
    z[4] = zz[3];

    // Compute the absorbance by summing the appropriate weights
    // vec4 weigth_factor = (vec4(z[1], z[2], z[3], z[4]) <= z[0].xxxx);
    vec4 weigth_factor = vec4(lessThanEqual(vec4(z[1], z[2], z[3], z[4]), z[0].xxxx));
    // Construct an interpolation polynomial
    float f0 = overestimation;
    float f1 = weigth_factor[0];
    float f2 = weigth_factor[1];
    float f3 = weigth_factor[2];
    float f4 = weigth_factor[3];
    float f01 = (f1 - f0) / (z[1] - z[0]);
    float f12 = (f2 - f1) / (z[2] - z[1]);
    float f23 = (f3 - f2) / (z[3] - z[2]);
    float f34 = (f4 - f3) / (z[4] - z[3]);
    float f012 = (f12 - f01) / (z[2] - z[0]);
    float f123 = (f23 - f12) / (z[3] - z[1]);
    float f234 = (f34 - f23) / (z[4] - z[2]);
    float f0123 = (f123 - f012) / (z[3] - z[0]);
    float f1234 = (f234 - f123) / (z[4] - z[1]);
    float f01234 = (f1234 - f0123) / (z[4] - z[0]);

    float Polynomial_0;
    vec4 Polynomial;
    // f0123 + f01234 * (z - z3)
    Polynomial_0 = fma(-f01234, z[3], f0123);
    Polynomial[0] = f01234;
    // * (z - z2) + f012
    Polynomial[1] = Polynomial[0];
    Polynomial[0] = fma(-Polynomial[0], z[2], Polynomial_0);
    Polynomial_0 = fma(-Polynomial_0, z[2], f012);
    // * (z - z1) + f01
    Polynomial[2] = Polynomial[1];
    Polynomial[1] = fma(-Polynomial[1], z[1], Polynomial[0]);
    Polynomial[0] = fma(-Polynomial[0], z[1], Polynomial_0);
    Polynomial_0 = fma(-Polynomial_0, z[1], f01);
    // * (z - z0) + f1
    Polynomial[3] = Polynomial[2];
    Polynomial[2] = fma(-Polynomial[2], z[0], Polynomial[1]);
    Polynomial[1] = fma(-Polynomial[1], z[0], Polynomial[0]);
    Polynomial[0] = fma(-Polynomial[0], z[0], Polynomial_0);
    Polynomial_0 = fma(-Polynomial_0, z[0], f0);
    float absorbance = Polynomial_0 + dot(Polynomial, vec4(b[0], b[1], b[2], b[3]));
    // Turn the normalized absorbance into transmittance
    return b_0 * absorbance;
}
