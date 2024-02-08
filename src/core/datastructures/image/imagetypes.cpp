/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022-2024 Inviwo Foundation
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

namespace inviwo {

std::string_view enumToStr(ImageType type) {
    switch (type) {
        case ImageType::ColorOnly:
            return "Color Only";
        case ImageType::ColorDepth:
            return "Color + Depth";
        case ImageType::ColorPicking:
            return "Color + Picking";
        case ImageType::ColorDepthPicking:
            return "Color + Depth + Picking";
    }
    throw Exception(IVW_CONTEXT_CUSTOM("enumName"), "Found invalid ImageType enum value '{}'",
                    static_cast<int>(type));
}
std::string_view enumToStr(LayerType type) {
    switch (type) {
        case LayerType::Color:
            return "Color";
        case LayerType::Depth:
            return "Depth";
        case LayerType::Picking:
            return "Picking";
    }
    throw Exception(IVW_CONTEXT_CUSTOM("enumName"), "Found invalid LayerType enum value '{}'",
                    static_cast<int>(type));
}
std::string_view enumToStr(ImageChannel channel) {
    switch (channel) {
        case ImageChannel::Red:
            return "r";
        case ImageChannel::Green:
            return "g";
        case ImageChannel::Blue:
            return "b";
        case ImageChannel::Alpha:
            return "a";
        case ImageChannel::Zero:
            return "0";
        case ImageChannel::One:
            return "1";
    }
    throw Exception(IVW_CONTEXT_CUSTOM("enumName"), "Found invalid ImageChannel enum value '{}'",
                    static_cast<int>(channel));
}

std::string_view enumToStr(InterpolationType type) {
    switch (type) {
        case InterpolationType::Nearest:
            return "Nearest";
        case InterpolationType::Linear:
            return "Linear";
    }
    throw Exception(IVW_CONTEXT_CUSTOM("enumName"),
                    "Found invalid InterpolationType enum value '{}'", static_cast<int>(type));
}
std::string_view enumToStr(Wrapping type) {
    switch (type) {
        case Wrapping::Mirror:
            return "Mirror";
        case Wrapping::Repeat:
            return "Repeat";
        case Wrapping::Clamp:
            return "Clamp";
    }
    throw Exception(IVW_CONTEXT_CUSTOM("enumName"), "Found invalid Wrapping enum value '{}'",
                    static_cast<int>(type));
}

std::ostream& operator<<(std::ostream& ss, ImageType type) { return ss << enumToStr(type); }

std::ostream& operator<<(std::ostream& ss, LayerType type) { return ss << enumToStr(type); }

std::ostream& operator<<(std::ostream& ss, ImageChannel channel) {
    return ss << enumToStr(channel);
}

std::istream& operator>>(std::istream& ss, ImageChannel& channel) {
    char c{0};
    ss >> c;
    switch (c) {
        case 'r':
            channel = ImageChannel::Red;
            break;
        case 'g':
            channel = ImageChannel::Green;
            break;
        case 'b':
            channel = ImageChannel::Blue;
            break;
        case 'a':
            channel = ImageChannel::Alpha;
            break;
        case '0':
            channel = ImageChannel::Zero;
            break;
        case '1':
            channel = ImageChannel::One;
            break;
        default:
            ss.setstate(std::ios_base::failbit);
            break;
    }
    return ss;
}

std::ostream& operator<<(std::ostream& ss, SwizzleMask mask) {
    for (const auto c : mask) {
        ss << c;
    }
    return ss;
}

std::istream& operator>>(std::istream& ss, SwizzleMask& mask) {
    for (auto& c : mask) {
        ss >> c;
        if (!ss) return ss;
    }
    return ss;
}

std::ostream& operator<<(std::ostream& ss, InterpolationType type) { return ss << enumToStr(type); }

std::istream& operator>>(std::istream& ss, InterpolationType& interpolation) {
    std::string str;
    ss >> str;
    str = toLower(str);

    if (str == toLower(toString(InterpolationType::Nearest))) {
        interpolation = InterpolationType::Nearest;
    } else if (str == toLower(toString(InterpolationType::Linear))) {
        interpolation = InterpolationType::Linear;
    } else {
        ss.setstate(std::ios_base::failbit);
    }

    return ss;
}

std::ostream& operator<<(std::ostream& ss, Wrapping type) { return ss << enumToStr(type); }

std::istream& operator>>(std::istream& ss, Wrapping& wrapping) {
    std::string str;
    ss >> str;
    str = toLower(str);

    if (str == toLower(toString(Wrapping::Mirror))) {
        wrapping = Wrapping::Mirror;
    } else if (str == toLower(toString(Wrapping::Repeat))) {
        wrapping = Wrapping::Repeat;
    } else if (str == toLower(toString(Wrapping::Clamp))) {
        wrapping = Wrapping::Clamp;
    } else {
        ss.setstate(std::ios_base::failbit);
    }

    return ss;
}

}  // namespace inviwo
