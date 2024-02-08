/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2024 Inviwo Foundation
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

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/util/ostreamjoiner.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/util/fmtutils.h>

#include <array>
#include <ostream>

namespace inviwo {

enum class ImageType : unsigned char {
    ColorOnly = 0,
    ColorDepth = 1,
    ColorPicking = 2,
    ColorDepthPicking = 3,
    AllLayers = ColorDepthPicking
};

enum class InterpolationType : unsigned char { Linear, Nearest };
enum class Wrapping : unsigned char { Clamp = 0, Repeat, Mirror };

using Wrapping1D = std::array<Wrapping, 1>;
using Wrapping2D = std::array<Wrapping, 2>;
using Wrapping3D = std::array<Wrapping, 3>;

enum class LayerType : unsigned char { Color = 0, Depth = 1, Picking = 2 };

enum class ImageChannel : unsigned char { Red, Green, Blue, Alpha, Zero, One };

using SwizzleMask = std::array<ImageChannel, 4>;

IVW_CORE_API std::string_view enumToStr(ImageType b);
IVW_CORE_API std::string_view enumToStr(LayerType b);
IVW_CORE_API std::string_view enumToStr(ImageChannel b);
IVW_CORE_API std::string_view enumToStr(InterpolationType b);
IVW_CORE_API std::string_view enumToStr(Wrapping b);

IVW_CORE_API std::ostream& operator<<(std::ostream& ss, ImageType type);
IVW_CORE_API std::ostream& operator<<(std::ostream& ss, LayerType type);
IVW_CORE_API std::ostream& operator<<(std::ostream& ss, ImageChannel channel);
IVW_CORE_API std::istream& operator>>(std::istream& ss, ImageChannel& channel);
IVW_CORE_API std::ostream& operator<<(std::ostream& ss, SwizzleMask mask);
IVW_CORE_API std::istream& operator>>(std::istream& ss, SwizzleMask& mask);
IVW_CORE_API std::ostream& operator<<(std::ostream& ss, InterpolationType type);
IVW_CORE_API std::istream& operator>>(std::istream& ss, InterpolationType& interpolation);
IVW_CORE_API std::ostream& operator<<(std::ostream& ss, Wrapping type);
IVW_CORE_API std::istream& operator>>(std::istream& ss, Wrapping& wrapping);

template <size_t N>
std::ostream& operator<<(std::ostream& ss, const std::array<Wrapping, N>& wrapping) {
    std::copy(wrapping.begin(), wrapping.end(), util::make_ostream_joiner(ss, ", "));
    return ss;
}

template <size_t N>
std::istream& operator>>(std::istream& ss, std::array<Wrapping, N>& wrapping) {
    for (auto& w : wrapping) {
        ss >> w;
        if (!ss) return ss;
    }
    return ss;
}

namespace swizzlemasks {
constexpr SwizzleMask rgb = {
    {ImageChannel::Red, ImageChannel::Green, ImageChannel::Blue, ImageChannel::One}};
constexpr SwizzleMask rgba = {
    {ImageChannel::Red, ImageChannel::Green, ImageChannel::Blue, ImageChannel::Alpha}};
constexpr SwizzleMask rgbZeroAlpha = {
    {ImageChannel::Red, ImageChannel::Green, ImageChannel::Blue, ImageChannel::Zero}};
constexpr SwizzleMask luminance = {
    {ImageChannel::Red, ImageChannel::Red, ImageChannel::Red, ImageChannel::One}};
constexpr SwizzleMask luminanceAlpha = {
    {ImageChannel::Red, ImageChannel::Red, ImageChannel::Red, ImageChannel::Green}};
constexpr SwizzleMask redGreen = {
    {ImageChannel::Red, ImageChannel::Green, ImageChannel::Zero, ImageChannel::Zero}};
constexpr SwizzleMask depth = luminance;

constexpr SwizzleMask defaultColor(size_t numComponents) noexcept {
    switch (numComponents) {
        case 1:
            return swizzlemasks::luminance;
        case 2:
            return swizzlemasks::luminanceAlpha;
        case 3:
            return swizzlemasks::rgb;
        case 4:
            return swizzlemasks::rgba;
        default:
            return swizzlemasks::rgba;
    }
}
constexpr SwizzleMask defaultData(size_t numComponents) noexcept {
    switch (numComponents) {
        case 1:
            return swizzlemasks::luminance;
        case 2:
            return SwizzleMask{ImageChannel::Red, ImageChannel::Green, ImageChannel::Zero,
                               ImageChannel::One};
        case 3:
            return swizzlemasks::rgb;
        case 4:
            return swizzlemasks::rgba;
        default:
            return swizzlemasks::rgba;
    }
}
}  // namespace swizzlemasks

namespace wrapping2d {
constexpr Wrapping2D clampAll = {Wrapping::Clamp, Wrapping::Clamp};
constexpr Wrapping2D repeatAll = {Wrapping::Repeat, Wrapping::Repeat};
constexpr Wrapping2D mirrorAll = {Wrapping::Mirror, Wrapping::Mirror};
}  // namespace wrapping2d

namespace wrapping3d {
constexpr Wrapping3D clampAll = {Wrapping::Clamp, Wrapping::Clamp, Wrapping::Clamp};
constexpr Wrapping3D repeatAll = {Wrapping::Repeat, Wrapping::Repeat, Wrapping::Repeat};
constexpr Wrapping3D mirrorAll = {Wrapping::Mirror, Wrapping::Mirror, Wrapping::Mirror};
}  // namespace wrapping3d

#include <warn/push>
#include <warn/ignore/unused-function>
constexpr bool typeContainsColor(ImageType type) {
    return (type == ImageType::ColorOnly || type == ImageType::ColorDepth ||
            type == ImageType::ColorPicking || type == ImageType::ColorDepthPicking);
}

constexpr bool typeContainsDepth(ImageType type) {
    return (type == ImageType::ColorDepth || type == ImageType::ColorDepthPicking);
}

constexpr bool typeContainsPicking(ImageType type) {
    return (type == ImageType::ColorPicking || type == ImageType::ColorDepthPicking);
}
#include <warn/pop>

}  // namespace inviwo

#ifndef DOXYGEN_SHOULD_SKIP_THIS
template <>
struct fmt::formatter<inviwo::ImageType> : inviwo::FlagFormatter<inviwo::ImageType> {};
template <>
struct fmt::formatter<inviwo::LayerType> : inviwo::FlagFormatter<inviwo::LayerType> {};
template <>
struct fmt::formatter<inviwo::ImageChannel> : inviwo::FlagFormatter<inviwo::ImageChannel> {};
template <>
struct fmt::formatter<inviwo::InterpolationType>
    : inviwo::FlagFormatter<inviwo::InterpolationType> {};
template <>
struct fmt::formatter<inviwo::Wrapping> : inviwo::FlagFormatter<inviwo::Wrapping> {};

template <>
struct fmt::formatter<inviwo::SwizzleMask> : inviwo::FlagsFormatter<inviwo::SwizzleMask> {};
template <>
struct fmt::formatter<inviwo::Wrapping1D> : inviwo::FlagsFormatter<inviwo::Wrapping1D> {};
template <>
struct fmt::formatter<inviwo::Wrapping2D> : inviwo::FlagsFormatter<inviwo::Wrapping2D> {};
template <>
struct fmt::formatter<inviwo::Wrapping3D> : inviwo::FlagsFormatter<inviwo::Wrapping3D> {};
#endif
