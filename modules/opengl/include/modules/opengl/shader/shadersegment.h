/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2022 Inviwo Foundation
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

#include <modules/opengl/openglmoduledefine.h>

#include <string>
#include <string_view>
#include <functional>

namespace inviwo {

/**
 * Hold a small text snippet to be inserted into shader code
 */
struct IVW_MODULE_OPENGL_API ShaderSegment {

    /**
     * Represents a placeholder in shader code that will be replaced
     */
    struct IVW_MODULE_OPENGL_API Placeholder {
    public:
        Placeholder() = default;
        explicit constexpr Placeholder(std::string_view key, std::string_view name)
            : key{key}, name{name} {}

        inline friend bool operator==(const Placeholder& lhs, const Placeholder& rhs) {
            return lhs.key == rhs.key;
        }
        inline friend bool operator<(const Placeholder& lhs, const Placeholder& rhs) {
            return lhs.key < rhs.key;
        }
        inline friend bool operator!=(const Placeholder& lhs, const Placeholder& rhs) {
            return !operator==(lhs, rhs);
        }
        inline friend bool operator>(const Placeholder& lhs, const Placeholder& rhs) {
            return operator<(rhs, lhs);
        }
        inline friend bool operator<=(const Placeholder& lhs, const Placeholder& rhs) {
            return !operator>(lhs, rhs);
        }
        inline friend bool operator>=(const Placeholder& lhs, const Placeholder& rhs) {
            return !operator<(lhs, rhs);
        }

        IVW_MODULE_OPENGL_API friend std::ostream& operator<<(std::ostream& os,
                                                              const Placeholder& obj);

        std::string_view key;
        std::string_view name;
    };

    Placeholder placeholder;  //!< The placeholder that will be replaced
    /**
     * A name of the snippet, used by the LineNumberResolver and to identify the segment in the
     * ShaderObject
     */
    std::string name;
    std::string snippet;  //!< The replacement code
    /**
     * If there are multiple replacement with the same Placeholder, they will be sorted using
     * priority, lower goes first.
     */
    size_t priority = 1000;
};

}  // namespace inviwo

namespace std {
template <>
struct hash<typename inviwo::ShaderSegment::Placeholder> {
    size_t operator()(typename inviwo::ShaderSegment::Placeholder const& type) const {
        return std::hash<std::string_view>{}(type.key);
    }
};
}  // namespace std