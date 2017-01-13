/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2017 Inviwo Foundation
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

#include <modules/animation/animationmodule.h>

#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/properties/ordinalproperty.h>

#include <modules/animation/datastructures/keyframe.h>
#include <modules/animation/datastructures/track.h>
#include <modules/animation/datastructures/propertytrack.h>

namespace inviwo {

struct OrdinalReghelper {
    template <typename T>
    auto operator()(AnimationModule& am) {
        using namespace animation;
        am.registerTrack<PropertyTrack<OrdinalProperty<T>, ValueKeyframe<T>>>();
        am.registerInterpolation<LinearInterpolation<ValueKeyframe<T>>>();

        am.registerPropertyTrackConnection(
            OrdinalProperty<T>::CLASS_IDENTIFIER,
            PropertyTrack<OrdinalProperty<T>, ValueKeyframe<T>>::classIdentifier());
    }
};

AnimationModule::AnimationModule(InviwoApplication* app)
    : InviwoModule(app, "Animation"), animation::AnimationSupplier(manager_), manager_(app, this) {

    using namespace animation;

    using Types = std::tuple<float, vec2, vec3, vec4, mat2, mat3, mat4, double, dvec2, dvec3, dvec4,
                             dmat2, dmat3, dmat4>;

    util::for_each_type<Types>{}(OrdinalReghelper{}, *this);
}

AnimationModule::~AnimationModule() {
    // need to call that here since we will have to delete the manager before we call the destructor
    // of AnimationSupplier. Other modules that implement AnimationSupplier will not have this
    // problem.
    unRegisterAll();
}

animation::AnimationManager& AnimationModule::getAnimationManager() { return manager_; }

const animation::AnimationManager& AnimationModule::getAnimationManager() const { return manager_; }

} // namespace
