/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022-2026 Inviwo Foundation
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

#include <inviwo/core/datastructures/image/imagetypes.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/stringconversion.h>

#include <ostream>

namespace inviwo {

std::string_view enumToStr(ImageType type) {
    switch (type) {
        case ImageType::ColorOnly:         return "Color Only";
        case ImageType::ColorDepth:        return "Color + Depth";
        case ImageType::ColorPicking:      return "Color + Picking";
        case ImageType::ColorDepthPicking: return "Color + Depth + Picking";
    }
    throw Exception(SourceContext{}, "Found invalid ImageType enum value '{}'",
                    static_cast<int>(type));
}
std::string_view enumToStr(LayerType type) {
    switch (type) {
        case LayerType::Color:   return "Color";
        case LayerType::Depth:   return "Depth";
        case LayerType::Picking: return "Picking";
    }
    throw Exception(SourceContext{}, "Found invalid LayerType enum value '{}'",
                    static_cast<int>(type));
}
std::string_view enumToStr(ImageChannel channel) {
    switch (channel) {
        case ImageChannel::Red:   return "r";
        case ImageChannel::Green: return "g";
        case ImageChannel::Blue:  return "b";
        case ImageChannel::Alpha: return "a";
        case ImageChannel::Zero:  return "0";
        case ImageChannel::One:   return "1";
    }
    throw Exception(SourceContext{}, "Found invalid ImageChannel enum value '{}'",
                    static_cast<int>(channel));
}

std::string_view enumToStr(InterpolationType type) {
    switch (type) {
        case InterpolationType::Nearest: return "Nearest";
        case InterpolationType::Linear:  return "Linear";
    }
    throw Exception(SourceContext{}, "Found invalid InterpolationType enum value '{}'",
                    static_cast<int>(type));
}
std::string_view enumToStr(Wrapping type) {
    switch (type) {
        case Wrapping::Mirror: return "Mirror";
        case Wrapping::Repeat: return "Repeat";
        case Wrapping::Clamp:  return "Clamp";
    }
    throw Exception(SourceContext{}, "Found invalid Wrapping enum value '{}'",
                    static_cast<int>(type));
}

std::optional<ImageChannel> charToImageChannel(char c) {
    switch (c) {
        case 'r': return ImageChannel::Red;
        case 'g': return ImageChannel::Green;
        case 'b': return ImageChannel::Blue;
        case 'a': return ImageChannel::Alpha;
        case '0': return ImageChannel::Zero;
        case '1': return ImageChannel::One;
        default: return std::nullopt;
    }
}

std::optional<SwizzleMask> strToSwizzleMask(std::string_view str) {
    SwizzleMask swizzleMask{};
    for (auto&& [m, c] : std::views::zip(swizzleMask, str)) {
        if (auto channel = charToImageChannel(c)) {
            m = *channel;
        } else {
            return std::nullopt;
        }
    }
    return swizzleMask;
}

std::optional<ImageType> strToImageType(std::string_view str) {
    for (auto e : {ImageType::ColorOnly, ImageType::ColorDepth, ImageType::ColorPicking,
                   ImageType::ColorDepthPicking}) {
        if (iCaseCmp(str, enumToStr(e))) {
            return e;
        }
    }
    return std::nullopt;
}

std::optional<LayerType> strToLayerType(std::string_view str) {
    for (auto e : {LayerType::Color, LayerType::Depth, LayerType::Picking}) {
        if (iCaseCmp(str, enumToStr(e))) {
            return e;
        }
    }
    return std::nullopt;
}

std::optional<InterpolationType> strToInterpolationType(std::string_view str) {
    for (auto e : {InterpolationType::Linear, InterpolationType::Nearest}) {
        if (iCaseCmp(str, enumToStr(e))) {
            return e;
        }
    }
    return std::nullopt;
}
std::optional<Wrapping> strToWrapping(std::string_view str) {
    for (auto e : {Wrapping::Clamp, Wrapping::Repeat, Wrapping::Mirror}) {
        if (iCaseCmp(str, enumToStr(e))) {
            return e;
        }
    }
    return std::nullopt;
}

namespace {
template <size_t N>
std::optional<std::array<Wrapping, N>> strToWrappings(std::string_view str) {
    std::array<Wrapping, N> wrappings;
    for (auto&& [wrapping, part] : std::views::zip(wrappings, util::splitIntoArray<N>(str))) {
        if (auto w = strToWrapping(part)) {
            wrapping = *w;
        } else {
            return std::nullopt;
        }
    }
    return wrappings;
}
}  // namespace

std::optional<Wrapping1D> strToWrapping1D(std::string_view str) { return strToWrappings<1>(str); }
std::optional<Wrapping2D> strToWrapping2D(std::string_view str) { return strToWrappings<2>(str); }
std::optional<Wrapping3D> strToWrapping3D(std::string_view str) { return strToWrappings<3>(str); }

}  // namespace inviwo
