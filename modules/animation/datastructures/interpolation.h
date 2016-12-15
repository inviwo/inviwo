/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016 Inviwo Foundation
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

#ifndef IVW_KEYFRAME_INTERPOLATION_H
#define IVW_KEYFRAME_INTERPOLATION_H

#include <modules/animation/animationmoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <modules/animation/datastructures/keyframe.h>

#include <algorithm>

namespace inviwo {

namespace animation {

/**
 * \class Interpolation
 * \brief VERY_BRIEFLY_DESCRIBE_THE_CLASS
 * DESCRIBE_THE_CLASS
 */
class IVW_MODULE_ANIMATION_API Interpolation { 
public:
    Interpolation() = default;
    virtual ~Interpolation() = default;

    virtual std::string getClassIdentifier() const = 0;
};

template <typename Key>
class  InterpolationTyped : public Interpolation {
public:
    InterpolationTyped() = default;
    virtual ~InterpolationTyped() = default;

    virtual std::string getClassIdentifier() const override = 0;

    // keys should be sorted by time
    virtual auto evaluate(const std::vector<Key>& key, Time t) const -> typename Key::value_type = 0;
};


template <typename Key>
class  LinearInterpolation : public InterpolationTyped<Key> {
public:
    LinearInterpolation() = default;
    virtual ~LinearInterpolation() = default;

    virtual std::string getClassIdentifier() const override { return "org.inviwo.linearinterpolation"; }

    // keys should be sorted by time
    virtual auto evaluate(const std::vector<Key>& key, Time t) const ->
        typename Key::value_type override {
        auto it = std::upper_bound(key.begin(), key.end(), t, [](const auto& time, const auto& key) {
            return time < key.getTime();
        });

        const auto& v1 = std::prev(it)->getValue();
        const auto& t1 = std::prev(it)->getTime();

        const auto& v2 = it->getValue();
        const auto& t2 = it->getTime();

        return glm::mix(v1, v2, (t - t1) / (t2 - t1));
    }
};


} // namespace

} // namespace

#endif // IVW_KEYFRAME_INTERPOLATION_H

