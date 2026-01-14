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

#include <inviwo/core/processors/metadataprocessor.h>

namespace inviwo::detail {

template <typename... Types, typename F>
auto expand_each_type(const F& func) {
    (func.template operator()<Types>(), ...);
}

template <typename T>
struct TypeMap {
    using Property = OrdinalProperty<T>;
    using MetaData = MetaDataType<T>;
};

template <>
struct TypeMap<bool> {
    using Property = BoolProperty;
    using MetaData = MetaDataType<bool>;
};
template <>
struct TypeMap<std::string> {
    using Property = StringProperty;
    using MetaData = MetaDataType<std::string>;
};

std::vector<std::unique_ptr<Property>> createPrefabs() {
    std::vector<std::unique_ptr<Property>> vec;
    detail::expand_each_type<bool, int, float, double, vec2, vec3, vec4, dvec2, dvec3, dvec4, ivec2,
                             ivec3, ivec4, uvec2, uvec3, uvec4, mat2, mat3, mat4, dmat2, dmat3,
                             dmat4, std::string>([&]<typename Type>() {
        using Prop = detail::TypeMap<Type>::Property;
        vec.emplace_back(std::make_unique<Prop>("meta", Defaultvalues<Type>::getName()));
    });
    return vec;
}

void updateMetaData(ListProperty& metadata, const MetaDataOwner& data) {
    for (auto* property : metadata) {
        if (!data.hasMetaData(property->getDisplayName())) continue;

        detail::expand_each_type<bool, int, float, double, vec2, vec3, vec4, dvec2, dvec3, dvec4,
                                 ivec2, ivec3, ivec4, uvec2, uvec3, uvec4, mat2, mat3, mat4, dmat2,
                                 dmat3, dmat4, std::string>([&]<typename Type>() {
            using Prop = detail::TypeMap<Type>::Property;
            using Meta = detail::TypeMap<Type>::MetaData;
            if (auto* p = dynamic_cast<Prop*>(property)) {
                if (auto* meta = data.getMetaData<Meta>(p->getDisplayName())) {
                    p->set(meta->get());
                } else {
                    p->resetToDefaultState();
                }
            }
        });
    }
}

void clearMetaData(ListProperty& metadata) {
    for (auto* property : metadata) {
        property->resetToDefaultState();
    }
}

}  // namespace inviwo::detail
