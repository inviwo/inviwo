/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2025 Inviwo Foundation
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

#include <inviwo/core/util/colorconversion.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/util/glm.h>

#include <algorithm>
#include <fmt/format.h>
#include <charconv>

namespace inviwo::color {

vec4 hex2rgba(std::string_view str) {
    vec4 result;

    str = util::trim(str);
    if (!str.empty() && (str[0] == '#') && (str.length() <= 9)) {
        // extract rgba values from HTML color code

        // remove '#'
        str = str.substr(1);

        StrBuffer buf;
        if ((str.length() == 3) || (str.length() == 4)) {
            // duplicate each character for hexcodes #RGB and #RGBA
            for (const auto& c : str) {
                buf.append("{0}{0}", c);
            }
            str = buf.view();
        }

        std::uint64_t v{};
        const auto* const end = str.data() + str.size();
        if (auto [p, ec] = std::from_chars(str.data(), str.data() + str.size(), v, 16);
            ec != std::errc() || p != end) {
            throw Exception(SourceContext{}, R"(Invalid hex code "{}".)", str);
        }

        const auto* c = reinterpret_cast<const unsigned char*>(&v);
        switch (str.length()) {
            case 6:
                result = vec4(c[2], c[1], c[0], 255) / 255.0f;
                break;
            case 8:
                result = vec4(c[3], c[2], c[1], c[0]) / 255.0f;
                break;
            default:
                throw Exception(SourceContext{}, R"(Invalid hex code "{}".)", str);
        }
    } else {
        throw Exception(SourceContext{}, R"(Invalid hex code "{}".)", str);
    }
    return result;
}

std::string rgba2hex(const vec4& rgba) {
    glm::u8vec4 color(rgba * 255.0f);
    return fmt::format("#{:02x}{:02x}{:02x}{:02x}", color.r, color.g, color.b, color.a);
}

std::string rgb2hex(const vec3& rgb) {
    glm::u8vec4 color(rgb * 255.0f, 0);
    return fmt::format("#{:02x}{:02x}{:02x}", color.r, color.g, color.b);
}

vec3 hsl2rgb(const vec3& hsl) {
    const double hue = hsl.x;
    const double sat = hsl.y;
    const double lum = hsl.z;

    auto f = [lum, a = sat * std::min(lum, 1.0 - lum), h = hue * 360.0 / 30.0](double n) {
        const double k = std::fmod(n + h, 12.0);
        return lum - a * std::max(-1.0, std::min(k - 3.0, std::min(9.0 - k, 1.0)));
    };
    return vec3{f(0.0), f(8.0), f(4.0)};
}

vec3 rgb2hsl(const vec3& rgb) {
    const double max = glm::compMax(rgb);
    const double min = glm::compMin(rgb);
    const double range = max - min;
    const double lum = (max + min) * 0.5;

    const bool notGray = (std::abs(range) > 1.0e-8);
    double hue = 0.0;
    double sat = 0.0;

    if (notGray) {
        if (rgb.b == max) {
            hue = 2.0 / 3.0 + 1.0 / 6.0 * (rgb.r - rgb.g) / range;
        } else if (rgb.g == max) {
            hue = 1.0 / 3.0 + 1.0 / 6.0 * (rgb.b - rgb.r) / range;
        } else if (rgb.r == max) {
            hue = 1.0 / 6.0 * (rgb.g - rgb.b) / range;
        }
    }

    if (hue < 0.0) {
        hue += 1.0;
    }
    if (lum > util::epsilon<double>() && lum < 1.0 - util::epsilon<float>()) {
        sat = range / (1.0 - std::abs(2.0 * lum - 1.0));
    }
    return {static_cast<float>(hue), static_cast<float>(sat), static_cast<float>(lum)};
}

vec3 XYZ2lab(const vec3& xyz, const vec3& whitePoint) {
    static const float epsilon = 0.008856f;
    static const float kappa = 903.3f;
    vec3 lab;
    vec3 t = xyz / whitePoint;
    vec3 f;
    for (int i = 0; i < 3; ++i) {
        if (t[i] > epsilon) {
            f[i] = std::pow(t[i], 1.f / 3.f);
        } else {
            f[i] = (kappa * t[i] + 16.f) / 116.f;
        }
    }
    lab.x = 116.f * f.y - 16.f;
    lab.y = 500.f * (f.x - f.y);
    lab.z = 200.f * (f.y - f.z);
    return lab;
}

vec3 lab2XYZ(const vec3& lab, const vec3& whitePoint) {

    vec3 t((1.f / 116.f) * (lab.x + 16.f));
    t.y += (1.f / 500.f) * lab.y;
    t.z -= (1.f / 200.f) * lab.z;
    vec3 f;
    for (int i = 0; i < 3; ++i) {
        const float sixDivTwentyNine = 6.f / 29.f;
        if (t[i] > sixDivTwentyNine) {
            f[i] = std::pow(t[i], 3.f);
        } else {
            // f(t) = 3*(6/29)^2 * (t - 4/29)
            f[i] = 3.f * sixDivTwentyNine * sixDivTwentyNine * (t[i] - 4.f / 29.f);
        }
    }
    vec3 xyz;
    xyz.x = whitePoint.x * f.y;
    xyz.y = whitePoint.y * f.x;
    xyz.z = whitePoint.z * f.z;
    return xyz;
}

vec3 rgb2XYZ(const vec3& rgb) {
    // Conversion matrix for sRGB, D65 white point
    static const mat3 rgb2XYZD65Mat(0.4124564f, 0.2126729f, 0.0193339f, 0.3575761f, 0.7151522f,
                                    0.1191920f, 0.1804375f, 0.0721750f, 0.9503041f);
    // Inverse sRGB companding
    vec3 v;
    for (int i = 0; i < 3; ++i) {
        if (rgb[i] > 0.04045f) {
            v[i] = std::pow((rgb[i] + 0.055f) / 1.055f, 2.4f);
        } else {
            v[i] = rgb[i] / 12.92f;
        }
    }
    return rgb2XYZD65Mat * v;
}

vec3 XYZ2rgb(const vec3& xyz) {
    // Conversion matrix for sRGB, D65 white point
    static const mat3 XYZ2rgbD65Mat(3.2404542f, -0.9692660f, 0.0556434, -1.5371385f, 1.8760108f,
                                    -0.2040259f, -0.4985314f, 0.0415560f, 1.0572252f);
    vec3 v = XYZ2rgbD65Mat * xyz;
    // sRGB companding
    vec3 rgb;
    for (int i = 0; i < 3; ++i) {
        if (v[i] > 0.0031308f) {
            rgb[i] = std::pow(v[i], 1.f / 2.4f) * 1.055f - 0.055f;
        } else {
            rgb[i] = v[i] * 12.92f;
        }
    }
    return rgb;
}

vec3 XYZ2xyY(const vec3& xyz) {
    // if X, Y, and Z are 0, set xy to chromaticity (xy) of ref. white D65.
    if (glm::all(glm::lessThan(xyz, util::epsilon<vec3>()))) {
        // This xy value is obtained by calling XYZ2xyY(D65WhitePoint)
        return vec3(0.3127266147f, 0.3290231303f, 0.0f);  // brucelindbloom.com D65
    } else {
        auto& X = xyz.x;
        auto& Y = xyz.y;
        auto& Z = xyz.z;
        float sum = X + Y + Z;
        return vec3(X / sum, Y / sum, Y);
    }
}

vec3 xyY2XYZ(const vec3& xyY) {
    // if y is 0, set X, Y, Z to 0
    if (xyY.y < glm::epsilon<float>()) {
        return vec3(0.0f);
    } else {
        auto& x = xyY.x;
        auto& y = xyY.y;
        auto& Y = xyY.z;

        return vec3(x * Y / y, Y, (1 - x - y) * Y / y);
    }
}

vec3 rgb2lab(const vec3& rgb) {
    vec3 xyz = rgb2XYZ(rgb);

    return XYZ2lab(xyz);
}

vec3 lab2rgb(const vec3& lab) {
    vec3 xyz = lab2XYZ(lab);

    return XYZ2rgb(xyz);
}

vec3 rgb2ycbcr(const vec3& rgb) {
    double r = rgb.x;
    double g = rgb.y;
    double b = rgb.z;

    double y = 0.299 * r + 0.587 * g + 0.114 * b;
    double cb = (b - y) * 0.565;
    double cr = (r - y) * 0.713;

    return vec3(static_cast<float>(y), static_cast<float>(cb), static_cast<float>(cr));
}

vec3 ycbcr2rgb(const vec3& ycbcr) {
    double y = ycbcr.x;
    double cb = ycbcr.y;
    double cr = ycbcr.z;

    double r = glm::clamp(y + 1.402 * cr, 0.0, 1.0);
    double g = glm::clamp(y - 0.344136 * cb - 0.714136 * cr, 0.0, 1.0);
    double b = glm::clamp(y + 1.772 * cb, 0.0, 1.0);

    return vec3(static_cast<float>(r), static_cast<float>(g), static_cast<float>(b));
}

vec3 LuvChromaticity2rgb(const vec3& LuvChroma, bool clamp, const vec3& whitePointXYZ) {
    vec3 rgb(XYZ2rgb(LuvChromaticity2XYZ(LuvChroma, whitePointXYZ)));
    if (clamp) {
        // determine largest component
        int index = 0;
        for (int i = 1; i < 3; ++i) {
            if (rgb[i] > rgb[index]) {
                index = i;
            }
        }
        if (rgb[index] > 1.0f) {
            // renormalize rgb values
            rgb /= rgb[index];
        }
        // get rid of negative values
        rgb = glm::max(rgb, vec3(0.0f));
    }
    return rgb;
}

vec3 XYZ2LuvChromaticity(const vec3& XYZ, const vec3& whitePointXYZ) {
    // see http://www.brucelindbloom.com/index.html?Eqn_XYZ_to_Luv.html

    const double epsilon = 216.0 / 24389.0;
    const double kappa = 24389.0 / 27.0;

    // compute u' and v' for XYZ color value
    double u_prime = 4 * XYZ.x / (XYZ.x + 15 * XYZ.y + 3 * XYZ.z);
    double v_prime = 9 * XYZ.y / (XYZ.x + 15 * XYZ.y + 3 * XYZ.z);

    double yr = XYZ.y / whitePointXYZ.y;
    // L <- ifelse(yr > epsilon, 116 * yr^(1/3) - 16, kappa * yr);
    double L = ((yr > epsilon) ? 116.0 * std::pow(yr, 1.0 / 3.0) - 16.0 : kappa * yr);

    return vec3(L, u_prime, v_prime);
}

vec3 XYZ2Luv(const vec3& XYZ, const vec3& whitePointXYZ) {
    // see http://www.brucelindbloom.com/index.html?Eqn_XYZ_to_Luv.html

    const double epsilon = 216.0 / 24389.0;
    const double kappa = 24389.0 / 27.0;

    // compute u' and v' for reference white point
    // u' <- 4 * X_r / (X_r + 15 * Y_r + 3 * Z_r);
    // v' <- 9 * Y_r / (X_r + 15 * Y_r + 3 * Z_r);
    double u0_prime =
        4 * whitePointXYZ.x / (whitePointXYZ.x + 15 * whitePointXYZ.y + 3 * whitePointXYZ.z);
    double v0_prime =
        9 * whitePointXYZ.y / (whitePointXYZ.x + 15 * whitePointXYZ.y + 3 * whitePointXYZ.z);

    // compute u' and v' for XYZ color value
    double u_prime = 4 * XYZ.x / (XYZ.x + 15 * XYZ.y + 3 * XYZ.z);
    double v_prime = 9 * XYZ.y / (XYZ.x + 15 * XYZ.y + 3 * XYZ.z);

    double yr = XYZ.y / whitePointXYZ.y;
    // L <- ifelse(yr > epsilon, 116 * yr^(1/3) - 16, kappa * yr);
    double L = ((yr > epsilon) ? 116.0 * std::pow(yr, 1.0 / 3.0) - 16.0 : kappa * yr);
    // u <- 13 * L * (u_prime - u0_prime);
    double u = 13.0 * L * (u_prime - u0_prime);
    // v <- 13 * L * (v_prime - v0_prime);
    double v = 13.0 * L * (v_prime - v0_prime);

    return vec3(L, u, v);
}

vec3 Luv2XYZ(const vec3& Luv, const vec3& whitePointXYZ) {
    // see http://www.brucelindbloom.com/index.html?Eqn_Luv_to_XYZ.html

    const double epsilon = 216.0 / 24389.0;
    const double kappa = 24389.0 / 27.0;

    // compute u and v for reference white point
    // u0 <- 4 * X_r / (X_r + 15 * Y_r + 3 * Z_r);
    // v0 <- 9 * Y_r / (X_r + 15 * Y_r + 3 * Z_r);
    double u0 =
        4 * whitePointXYZ.x / (whitePointXYZ.x + 15 * whitePointXYZ.y + 3 * whitePointXYZ.z);
    double v0 =
        9 * whitePointXYZ.y / (whitePointXYZ.x + 15 * whitePointXYZ.y + 3 * whitePointXYZ.z);

    double L = Luv.x;
    double u = Luv.y;
    double v = Luv.z;

    // Y <- ifelse(L > kappa * epsilon, ((L + 16) / 116) ^ 3, L / kappa);
    double Y = ((L > kappa * epsilon) ? std::pow((L + 16) / 116, 3) : L / kappa);

    // a <- 1 / 3 * (52 * L / (u + 13 * L * u0) - 1);
    // b <- -5*Y;
    // c <- -1/3;
    // d <- Y * (39 * L / (v + 13 * L * v0) - 5);
    double a = 1.0 / 3.0 * (52.0 * L / (u + 13 * L * u0) - 1.0);
    double b = -5.0 * Y;
    double c = -1.0 / 3.0;
    double d = Y * (39.0 * L / (v + 13 * L * v0) - 5.0);
    // X <- (d - b) / (a - c);
    double X = (d - b) / (a - c);

    // Z <- X * a + b;
    double Z = X * a + b;

    return vec3(X, Y, Z);
}

vec3 rgb2LuvChromaticity(const vec3& rgb, const vec3& whitePointXYZ) {
    return XYZ2LuvChromaticity(rgb2XYZ(rgb), whitePointXYZ);
}

vec3 LuvChromaticity2XYZ(const vec3& LuvChroma, const vec3& whitePointXYZ) {
    // compute u and v for reference white point
    // u0 <- 4 * X_r / (X_r + 15 * Y_r + 3 * Z_r);
    // v0 <- 9 * Y_r / (X_r + 15 * Y_r + 3 * Z_r);
    double u0_prime =
        4 * whitePointXYZ.x / (whitePointXYZ.x + 15 * whitePointXYZ.y + 3 * whitePointXYZ.z);
    double v0_prime =
        9 * whitePointXYZ.y / (whitePointXYZ.x + 15 * whitePointXYZ.y + 3 * whitePointXYZ.z);

    double L = LuvChroma.x;
    double u_prime = LuvChroma.y;
    double v_prime = LuvChroma.z;

    // convert chromaticity to CIE Luv
    double u = 13.0 * L * (u_prime - u0_prime);
    double v = 13.0 * L * (v_prime - v0_prime);

    return Luv2XYZ(vec3(L, u, v));
}

}  // namespace inviwo::color
