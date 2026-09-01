/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <inviwo/core/properties/ordinaloptproperty.h>

namespace inviwo {

// Scalar properties
template class IVW_CORE_TMPL_INST OrdinalOptProperty<float>;
template class IVW_CORE_TMPL_INST OrdinalOptProperty<int>;
template class IVW_CORE_TMPL_INST OrdinalOptProperty<size_t>;
template class IVW_CORE_TMPL_INST OrdinalOptProperty<glm::i64>;
template class IVW_CORE_TMPL_INST OrdinalOptProperty<double>;

// Vector properties
template class IVW_CORE_TMPL_INST OrdinalOptProperty<vec2>;
template class IVW_CORE_TMPL_INST OrdinalOptProperty<vec3>;
template class IVW_CORE_TMPL_INST OrdinalOptProperty<vec4>;

template class IVW_CORE_TMPL_INST OrdinalOptProperty<dvec2>;
template class IVW_CORE_TMPL_INST OrdinalOptProperty<dvec3>;
template class IVW_CORE_TMPL_INST OrdinalOptProperty<dvec4>;

template class IVW_CORE_TMPL_INST OrdinalOptProperty<ivec2>;
template class IVW_CORE_TMPL_INST OrdinalOptProperty<ivec3>;
template class IVW_CORE_TMPL_INST OrdinalOptProperty<ivec4>;

template class IVW_CORE_TMPL_INST OrdinalOptProperty<size2_t>;
template class IVW_CORE_TMPL_INST OrdinalOptProperty<size3_t>;
template class IVW_CORE_TMPL_INST OrdinalOptProperty<size4_t>;

// Matrix properties
template class IVW_CORE_TMPL_INST OrdinalOptProperty<mat2>;
template class IVW_CORE_TMPL_INST OrdinalOptProperty<mat3>;
template class IVW_CORE_TMPL_INST OrdinalOptProperty<mat4>;

template class IVW_CORE_TMPL_INST OrdinalOptProperty<dmat2>;
template class IVW_CORE_TMPL_INST OrdinalOptProperty<dmat3>;
template class IVW_CORE_TMPL_INST OrdinalOptProperty<dmat4>;

template class IVW_CORE_TMPL_INST OrdinalOptProperty<glm::dquat>;
template class IVW_CORE_TMPL_INST OrdinalOptProperty<glm::fquat>;

OrdinalOptPropertyState<vec4> util::ordinalOptColor(float r, float g, float b, float a,
                                                    InvalidationLevel invalidationLevel) {
    return ordinalOptColor(vec4(r, g, b, a), invalidationLevel);
}

OrdinalOptPropertyState<vec4> util::ordinalOptColor(const vec4& value,
                                                    InvalidationLevel invalidationLevel) {
    return ordinalOptColor(std::optional<vec4>{value}, invalidationLevel);
}

OrdinalOptPropertyState<vec3> util::ordinalOptColor(const vec3& value,
                                                    InvalidationLevel invalidationLevel) {
    return ordinalOptColor(std::optional<vec3>{value}, invalidationLevel);
}

OrdinalOptPropertyState<vec4> util::ordinalOptColor(const std::optional<vec4>& value,
                                                    InvalidationLevel invalidationLevel) {
    return {value,
            vec4{0.0f},
            ConstraintBehavior::Immutable,
            vec4{1.0f},
            ConstraintBehavior::Immutable,
            vec4{0.01f},
            invalidationLevel,
            PropertySemantics::Color};
}

OrdinalOptPropertyState<vec3> util::ordinalOptColor(const std::optional<vec3>& value,
                                                    InvalidationLevel invalidationLevel) {
    return {value,
            vec3{0.0f},
            ConstraintBehavior::Immutable,
            vec3{1.0f},
            ConstraintBehavior::Immutable,
            vec3{0.01f},
            invalidationLevel,
            PropertySemantics::Color};
}

OrdinalOptPropertyState<vec3> util::ordinalOptLight(const std::optional<vec3>& pos, float min,
                                                    float max,
                                                    InvalidationLevel invalidationLevel) {
    return {pos,
            vec3{min},
            ConstraintBehavior::Ignore,
            vec3{max},
            ConstraintBehavior::Ignore,
            vec3{0.1f},
            invalidationLevel,
            PropertySemantics::LightPosition};
}

}  // namespace inviwo
