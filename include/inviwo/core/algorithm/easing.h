/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2025 Inviwo Foundation
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

#include <inviwo/core/util/exception.h>
#include <cstdint>
#include <string_view>
#include <ranges>

#include <glm/gtx/easing.hpp>

namespace inviwo {

enum class EasingType : std::uint8_t {
    linear,
    quadratic,
    cubic,
    quartic,
    quintic,
    sine,
    circular,
    exponential,
    elastic,
    back,
    bounce
};

enum class EasingMode : std::uint8_t { in, out, inOut };

struct Easing {
    EasingType type;
    EasingMode mode;
    static constexpr size_t typeCount = 11;
    static constexpr size_t modeCount = 3;
};

constexpr std::string_view format_as(EasingType type) {
    using enum EasingType;
    switch (type) {
        case linear:
            return "linear";
        case quadratic:
            return "quadratic";
        case cubic:
            return "cubic";
        case quartic:
            return "quartic";
        case quintic:
            return "quintic";
        case sine:
            return "sine";
        case circular:
            return "circular";
        case exponential:
            return "exponential";
        case elastic:
            return "elastic";
        case back:
            return "back";
        case bounce:
            return "bounce";
    }
    throw Exception(SourceContext{}, "Got invalid EasingType {}", static_cast<int>(type));
}
constexpr std::string_view format_as(EasingMode mode) {
    switch (mode) {
        case EasingMode::in:
            return "in";
        case EasingMode::out:
            return "out";
        case EasingMode::inOut:
            return "inOut";
    }
    throw Exception(SourceContext{}, "Got invalid EasingMode {}", static_cast<int>(mode));
}

constexpr std::string_view format_as(Easing easing) {
    static constexpr auto toUpper = [](char c) {
        if (c < 'a' || c > 'z') return c;
        return static_cast<char>(c - ('a' - 'A'));
    };

    static constexpr auto storage = []() {
        std::array<std::array<std::pair<std::array<char, 20>, size_t>, Easing::modeCount>,
                   Easing::typeCount>
            tmp{};
        for (size_t t = 0; t < Easing::typeCount; ++t) {
            for (size_t m = 0; m < Easing::modeCount; ++m) {
                const auto name = format_as(static_cast<EasingType>(t));
                const auto mode = format_as(static_cast<EasingMode>(m));
                std::ranges::copy(name, tmp[t][m].first.begin());
                std::ranges::copy(mode, tmp[t][m].first.begin() + name.size());
                tmp[t][m].first[name.size()] = toUpper(tmp[t][m].first[name.size()]);
                tmp[t][m].second = name.size() + mode.size();
            }
        }
        return tmp;
    }();

    const auto t = static_cast<size_t>(easing.type);
    const auto m = static_cast<size_t>(easing.mode);
    if (t >= Easing::typeCount) {
        throw Exception(SourceContext{}, "Got invalid EasingType {}", t);
    }
    if (m > Easing::modeCount) {
        throw Exception(SourceContext{}, "Got invalid EasingMode {}", m);
    }
    return std::string_view{storage[t][m].first.data(), storage[t][m].second};
}

namespace util {

template <typename T>
constexpr T ease(const T& x, Easing easing) {
    switch (easing.type) {
        case EasingType::linear:
            return glm::linearInterpolation(x);
        case EasingType::quadratic: {
            switch (easing.mode) {
                case EasingMode::in:
                    return glm::quadraticEaseIn(x);
                case EasingMode::out:
                    return glm::quadraticEaseOut(x);
                case EasingMode::inOut:
                    return glm::quadraticEaseInOut(x);
            }
        }
        case EasingType::cubic: {
            switch (easing.mode) {
                case EasingMode::in:
                    return glm::cubicEaseIn(x);
                case EasingMode::out:
                    return glm::cubicEaseOut(x);
                case EasingMode::inOut:
                    return glm::cubicEaseInOut(x);
            }
        }
        case EasingType::quartic: {
            switch (easing.mode) {
                case EasingMode::in:
                    return glm::quarticEaseIn(x);
                case EasingMode::out:
                    return glm::quarticEaseOut(x);
                case EasingMode::inOut:
                    return glm::quarticEaseInOut(x);
            }
        }
        case EasingType::quintic: {
            switch (easing.mode) {
                case EasingMode::in:
                    return glm::quinticEaseIn(x);
                case EasingMode::out:
                    return glm::quinticEaseOut(x);
                case EasingMode::inOut:
                    return glm::quinticEaseInOut(x);
            }
        }
        case EasingType::sine: {
            switch (easing.mode) {
                case EasingMode::in:
                    return glm::sineEaseIn(x);
                case EasingMode::out:
                    return glm::sineEaseOut(x);
                case EasingMode::inOut:
                    return glm::sineEaseInOut(x);
            }
        }
        case EasingType::circular: {
            switch (easing.mode) {
                case EasingMode::in:
                    return glm::circularEaseIn(x);
                case EasingMode::out:
                    return glm::circularEaseOut(x);
                case EasingMode::inOut:
                    return glm::circularEaseInOut(x);
            }
        }
        case EasingType::exponential: {
            switch (easing.mode) {
                case EasingMode::in:
                    return glm::exponentialEaseIn(x);
                case EasingMode::out:
                    return glm::exponentialEaseOut(x);
                case EasingMode::inOut:
                    return glm::exponentialEaseInOut(x);
            }
        }
        case EasingType::elastic: {
            switch (easing.mode) {
                case EasingMode::in:
                    return glm::elasticEaseIn(x);
                case EasingMode::out:
                    return glm::elasticEaseOut(x);
                case EasingMode::inOut:
                    return glm::elasticEaseInOut(x);
            }
        }
        case EasingType::back: {
            switch (easing.mode) {
                case EasingMode::in:
                    return glm::backEaseIn(x);
                case EasingMode::out:
                    return glm::backEaseOut(x);
                case EasingMode::inOut:
                    return glm::backEaseInOut(x);
            }
        }
        case EasingType::bounce: {
            switch (easing.mode) {
                case EasingMode::in:
                    return glm::bounceEaseIn(x);
                case EasingMode::out:
                    return glm::bounceEaseOut(x);
                case EasingMode::inOut:
                    return glm::bounceEaseInOut(x);
            }
        }
    }
    throw Exception(SourceContext{}, "Got invalid Easing {}, {}", static_cast<int>(easing.mode),
                    static_cast<int>(easing.type));
}

}  // namespace util

}  // namespace inviwo
