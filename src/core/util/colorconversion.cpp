/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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

#include <algorithm>
#include <string>
#include <sstream>

namespace inviwo {

namespace color {

vec4 hex2rgba(std::string str) {
    vec4 result;
    str = trim(str);
    if (!str.empty() && (str[0] == '#') && (str.length() <= 9)) {
        // extract rgba values from HTML color code

        const auto numStr = [str]() {
            if ((str.length() == 4) || (str.length() == 5)) {
                // duplicate each character for hexcodes #RGB and #RGBA
                std::string result;
                for (auto c : str.substr(1)) {
                    result.append(std::string(2, c));
                }
                return result;
            }
            return str.substr(1);
        }();

        const unsigned long v = [numStr]() {
            unsigned long v = 0;
            std::istringstream stream("0x" + numStr);
            if (!(stream >> std::hex >> v)) {
                throw Exception("Invalid hex code \"#" + numStr + "\".",
                                IVW_CONTEXT_CUSTOM("color::hex2rgba"));
            }
            return v;
        }();
        auto *c = reinterpret_cast<const unsigned char *>(&v);
        switch (numStr.size()) {
            case 6:
                result = vec4(c[2], c[1], c[0], 255) / 255.0f;
                break;
            case 8:
                result = vec4(c[3], c[2], c[1], c[0]) / 255.0f;
                break;
            default:
                throw Exception("Invalid hex code \"" + str + "\".",
                                IVW_CONTEXT_CUSTOM("color::hex2rgba"));
        }

    } else {
        throw Exception("Invalid hex code \"" + str + "\".", IVW_CONTEXT_CUSTOM("color::hex2rgba"));
    }
    return result;
}

std::string rgba2hex(const vec4 &rgba) {
    glm::u8vec4 color(rgba * 255.0f);
    // change byte order
    std::swap(color.r, color.a);
    std::swap(color.g, color.b);

    std::ostringstream ss;
    ss << "#" << std::setw(8) << std::setfill('0') << std::hex
       << *reinterpret_cast<unsigned int *>(&color);
    return ss.str();
}

std::string rgb2hex(const vec3 &rgb) {
    glm::u8vec4 color(rgb * 255.0f, 0);
    // change byte order
    std::swap(color.r, color.b);

    std::ostringstream ss;
    ss << "#" << std::setw(6) << std::setfill('0') << std::hex
       << *reinterpret_cast<unsigned int *>(&color);
    return ss.str();
}

vec3 getD65WhitePoint() {
    // whiteD65 = rgb2XYZ(vec3(1.0f, 1.0f, 1.0f);
    return vec3(0.95047f, 1.0f, 1.08883f);
}

vec3 hsv2rgb(vec3 hsv) {
    double hue = hsv.x;
    double sat = hsv.y;
    double val = hsv.z;
    double r = 0.0;
    double g = 0.0;
    double b = 0.0;

    if (sat < 1.0e-8) {  // only value, no saturation
        r = val;
        g = val;
        b = val;
        return vec3(static_cast<float>(r), static_cast<float>(g), static_cast<float>(b));
        ;
    }

    hue *= 360.0;
    // divide hue into six segments, 60 degree each
    int h_i = static_cast<int>(std::floor(hue / 60.0)) % 6;
    double f = hue / 60.0 - std::floor(hue / 60.0);
    double p = val * (1.0 - sat);
    double q = val * (1.0 - f * sat);
    double t = val * (1.0 - (1.0 - f) * sat);

    switch (h_i) {
        case 0:
            r = val;
            g = t;
            b = p;
            break;
        case 1:
            r = q;
            g = val;
            b = p;
            break;
        case 2:
            r = p;
            g = val;
            b = t;
            break;
        case 3:
            r = p;
            g = q;
            b = val;
            break;
        case 4:
            r = t;
            g = p;
            b = val;
            break;
        case 5:
            r = val;
            g = p;
            b = q;
            break;
    }
    return vec3(static_cast<float>(r), static_cast<float>(g), static_cast<float>(b));
}

vec3 rgb2hsv(vec3 rgb) {
    double r = rgb.x;
    double g = rgb.y;
    double b = rgb.z;
    double val = std::max(std::max(r, g), b);
    double sat = std::min(std::min(r, g), b);
    double range = val - sat;

    // set hue to zero for undefined values
    bool notGray = (std::abs(val - sat) > 1.0e-8);
    double hue = 0.0;

    // blue hue
    if (notGray) {
        if (b == val)
            hue = 2.0 / 3.0 + 1.0 / 6.0 * (r - g) / range;
        else if (g == val)
            hue = 1.0 / 3.0 + 1.0 / 6.0 * (b - r) / range;
        else if (r == val)
            hue = 1.0 / 6.0 * (g - b) / range;
    }

    if (hue < 0.0) {
        hue += 1.0;
    }
    if (notGray) {
        sat = 1.0 - sat / val;
    } else {
        sat = 0.0;
    }
    return vec3(static_cast<float>(hue), static_cast<float>(sat), static_cast<float>(val));
}

vec3 XYZ2lab(vec3 xyz, vec3 whitePoint) {
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

vec3 lab2XYZ(const vec3 lab, const vec3 whitePoint) {

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

vec3 rgb2XYZ(const vec3 rgb) {
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

vec3 XYZ2rgb(const vec3 xyz) {
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

vec3 XYZ2xyY(vec3 xyz) {
    // if X, Y, and Z are 0, set xy to chromaticity (xy) of ref. white D65.
    if (glm::all(glm::lessThan(xyz, util::epsilon<vec3>()))) {
        // This xy value is obtained by calling XYZ2xyY(getD65WhitePoint())
        return vec3(0.3127266147f, 0.3290231303f, 0.0f);  // brucelindbloom.com D65
    } else {
        float &X = xyz.x;
        float &Y = xyz.y;
        float &Z = xyz.z;
        float sum = X + Y + Z;
        return vec3(X / sum, Y / sum, Y);
    }
}

vec3 xyY2XYZ(vec3 xyY) {
    // if y is 0, set X, Y, Z to 0
    if (xyY.y < glm::epsilon<float>()) {
        return vec3(0.0f);
    } else {
        auto &x = xyY.x;
        auto &y = xyY.y;
        auto &Y = xyY.z;

        return vec3(x * Y / y, Y, (1 - x - y) * Y / y);
    }
}

vec3 rgb2lab(const vec3 rgb) {
    vec3 xyz = rgb2XYZ(rgb);

    return XYZ2lab(xyz);
}

vec3 lab2rgb(const vec3 lab) {
    vec3 xyz = lab2XYZ(lab);

    return XYZ2rgb(xyz);
}

vec3 rgb2ycbcr(const vec3 &rgb) {
    double r = rgb.x;
    double g = rgb.y;
    double b = rgb.z;

    double y = 0.299 * r + 0.587 * g + 0.114 * b;
    double cb = (b - y) * 0.565;
    double cr = (r - y) * 0.713;

    return vec3(static_cast<float>(y), static_cast<float>(cb), static_cast<float>(cr));
}

vec3 ycbcr2rgb(const vec3 &ycbcr) {
    double y = ycbcr.x;
    double cb = ycbcr.y;
    double cr = ycbcr.z;

    double r = glm::clamp(y + 1.402 * cr, 0.0, 1.0);
    double g = glm::clamp(y - 0.344136 * cb - 0.714136 * cr, 0.0, 1.0);
    double b = glm::clamp(y + 1.772 * cb, 0.0, 1.0);

    return vec3(static_cast<float>(r), static_cast<float>(g), static_cast<float>(b));
}

vec3 LuvChromaticity2rgb(const vec3 &LuvChroma, bool clamp, vec3 whitePointXYZ) {
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

vec3 XYZ2LuvChromaticity(const vec3 &XYZ, vec3 whitePointXYZ) {
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

vec3 XYZ2Luv(const vec3 &XYZ, vec3 whitePointXYZ) {
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

vec3 Luv2XYZ(const vec3 &Luv, vec3 whitePointXYZ) {
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

vec3 rgb2LuvChromaticity(const vec3 &rgb, vec3 whitePointXYZ) {
    return XYZ2LuvChromaticity(rgb2XYZ(rgb), whitePointXYZ);
}

vec3 LuvChromaticity2XYZ(const vec3 &LuvChroma, vec3 whitePointXYZ) {
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

}  // namespace color

}  // namespace inviwo
