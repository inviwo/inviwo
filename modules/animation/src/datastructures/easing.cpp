/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2023 Inviwo Foundation
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

#include <modules/animation/datastructures/easing.h>

#include <inviwo/core/util/exception.h>      // for Exception
#include <inviwo/core/util/sourcecontext.h>  // for IVW_CONTEXT_CUSTOM

#include <ostream>      // for operator<<
#include <string>       // for string
#include <string_view>  // for string_view
#include <type_traits>  // for __underlying_type_impl<>::type, underlying_type

namespace inviwo {
namespace animation {

namespace easing {

EasingType& operator++(EasingType& e) {
    using IntType = typename std::underlying_type<EasingType>::type;
    e = static_cast<EasingType>(static_cast<IntType>(e) + 1);
    return e;
}

EasingType operator++(EasingType& e, int) {
    EasingType result = e;
    ++e;
    return result;
}

std::ostream& operator<<(std::ostream& os, EasingType type) {
    switch (type) {
        case EasingType::None:
            os << "None";
            break;
        case EasingType::Linear:
            os << "Linear";
            break;
        case EasingType::InQuadratic:
            os << "InQuadratic";
            break;
        case EasingType::InCubic:
            os << "InCubic";
            break;
        case EasingType::InQuartic:
            os << "InQuartic";
            break;
        case EasingType::InQuintic:
            os << "InQuintic";
            break;
        case EasingType::OutQuadratic:
            os << "OutQuadratic";
            break;
        case EasingType::OutCubic:
            os << "OutCubic";
            break;
        case EasingType::OutQuartic:
            os << "OutQuartic";
            break;
        case EasingType::OutQuintic:
            os << "OutQuintic";
            break;
        case EasingType::InOutQuadratic:
            os << "InOutQuadratic";
            break;
        case EasingType::InOutCubic:
            os << "InOutCubic";
            break;
        case EasingType::InOutQuartic:
            os << "InOutQuartic";
            break;
        case EasingType::InOutQuintic:
            os << "InOutQuintic";
            break;
        case EasingType::InSine:
            os << "InSine";
            break;
        case EasingType::OutSine:
            os << "OutSine";
            break;
        case EasingType::InOutSine:
            os << "InOutSine";
            break;
        case EasingType::InExp:
            os << "InExp";
            break;
        case EasingType::OutExp:
            os << "OutExp";
            break;
        case EasingType::InOutExp:
            os << "InOutExp";
            break;
        case EasingType::InCircular:
            os << "InCircular";
            break;
        case EasingType::OutCircular:
            os << "OutCircular";
            break;
        case EasingType::InOutCircular:
            os << "InOutCircular";
            break;
        case EasingType::InBack:
            os << "InBack";
            break;
        case EasingType::OutBack:
            os << "OutBack";
            break;
        case EasingType::InOutBack:
            os << "InOutBack";
            break;
        case EasingType::InElastic:
            os << "InElastic";
            break;
        case EasingType::OutElastic:
            os << "OutElastic";
            break;
        case EasingType::InOutElastic:
            os << "InOutElastic";
            break;
        case EasingType::InBounce:
            os << "InBounce";
            break;
        case EasingType::OutBounce:
            os << "OutBounce";
            break;
        case EasingType::InOutBounce:
            os << "InOutBounce";
            break;
        default:
            throw Exception("Unknown Easing type", IVW_CONTEXT_CUSTOM("Easing::operator<<"));
    }

    return os;
}

}  // namespace easing
}  // namespace animation
}  // namespace inviwo
