/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2025 Inviwo Foundation
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

#include <modules/animation/animationmoduledefine.h>
#include <modules/animation/interpolation/interpolation.h>

#include <inviwo/core/util/defaultvalues.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/enumtraits.h>
#include <inviwo/core/io/serialization/serialization.h>

#include <algorithm>

#include <filesystem>

namespace inviwo::animation {

/** \class ConstantInterpolation
 * Interpolation function for key frames.
 * Returns left keyframe value until reaching right keyframe.
 *
 */
template <typename Key, typename Result = typename Key::value_type>
class ConstantInterpolation : public InterpolationTyped<Key, Result> {
public:
    ConstantInterpolation() = default;
    virtual ~ConstantInterpolation() = default;

    virtual ConstantInterpolation* clone() const override;

    virtual std::string getName() const override;

    static std::string_view classIdentifier();
    virtual std::string_view getClassIdentifier() const override;

    virtual bool equal(const Interpolation& other) const override;

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

    // keys should be sorted by time
    virtual void operator()(const std::vector<std::unique_ptr<Key>>& keys, Seconds from, Seconds to,
                            Easing easing, Result& out) const override;
};

template <typename Key, typename Result>
ConstantInterpolation<Key, Result>* ConstantInterpolation<Key, Result>::clone() const {
    return new ConstantInterpolation<Key, Result>(*this);
}

template <typename Key, typename Result>
std::string_view ConstantInterpolation<Key, Result>::classIdentifier() {
    using V = typename Key::value_type;
    if constexpr (std::is_enum_v<V>) {
        static const std::string identifier =
            fmt::format("org.inviwo.animation.constantinterpolation.enum.{}", util::enumName<V>());
        return identifier;
    } else if constexpr (std::is_same_v<V, std::filesystem::path>) {
        static const std::string identifier = "org.inviwo.animation.constantinterpolation.Path";
        return identifier;
    } else {
        static const auto identifier =
            "org.inviwo.animation.constantinterpolation." + Defaultvalues<V>::getName();
        return identifier;
    }
}

template <typename Key, typename Result>
std::string_view ConstantInterpolation<Key, Result>::getClassIdentifier() const {
    return classIdentifier();
}

template <typename Key, typename Result>
std::string ConstantInterpolation<Key, Result>::getName() const {
    return "Constant";
}

template <typename Key, typename Result>
bool ConstantInterpolation<Key, Result>::equal(const Interpolation& other) const {
    return classIdentifier() == other.getClassIdentifier();
}
template <typename Key, typename Result>
void ConstantInterpolation<Key, Result>::operator()(const std::vector<std::unique_ptr<Key>>& keys,
                                                    Seconds from, Seconds to, Easing,
                                                    Result& out) const {

    if (to > from) {
        auto it = std::upper_bound(
            keys.begin(), keys.end(), to,
            [](const auto& time, const auto& key) { return time < key->getTime(); });

        if (it == keys.begin()) {
            out = (*it)->getValue();
        } else {
            out = (*std::prev(it))->getValue();
        }

    } else {
        auto it = std::lower_bound(
            keys.begin(), keys.end(), to,
            [](const auto& key, const auto& time) { return key->getTime() < time; });

        if (it == keys.end()) {
            out = (*std::prev(it))->getValue();
        } else {
            out = (*it)->getValue();
        }
    }
}

template <typename Key, typename Result>
void ConstantInterpolation<Key, Result>::serialize(Serializer& s) const {
    s.serialize("type", getClassIdentifier(), SerializationTarget::Attribute);
}

template <typename Key, typename Result>
void ConstantInterpolation<Key, Result>::deserialize(Deserializer& d) {
    std::string className;
    d.deserialize("type", className, SerializationTarget::Attribute);
    if (className != getClassIdentifier()) {
        std::string_view cid = getClassIdentifier();
        throw SerializationException(SourceContext{},
                                     "Deserialized interpolation: {} from a serialized "
                                     "interpolation with a different class identifier: {}",
                                     cid, className);
    }
}

}  // namespace inviwo::animation
