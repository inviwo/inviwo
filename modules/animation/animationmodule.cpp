/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2018 Inviwo Foundation
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
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/fileproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/stringproperty.h>

#include <modules/animation/datastructures/constantinterpolation.h>
#include <modules/animation/datastructures/keyframe.h>
#include <modules/animation/datastructures/track.h>
#include <modules/animation/datastructures/propertytrack.h>

namespace inviwo {

template <typename PropertyType, typename ValueType, typename Interpolation>
auto trackAndInterpolationRegHelper(AnimationModule& am) {
    using namespace animation;
    // Fixme: TemplateOptionProperty uses valueType so we cannot use this line
    // using ValueType = typename PropertyType::value_type;
    // Register PropertyTrack and the KeyFrame it should use
    am.registerTrack<PropertyTrack<PropertyType, ValueKeyframe<ValueType>>>();
    am.registerPropertyTrackConnection(
        PropertyTraits<PropertyType>::classIdentifier(),
        PropertyTrack<PropertyType, ValueKeyframe<ValueType>>::classIdentifier());

    // Interpolation for Keyframe
    // No need to add existing interpolation method. Will produce a warning if adding a dupplicate
    if (!am.getAnimationManager().getInterpolationFactory().hasKey(
            Interpolation::classIdentifier())) {
        am.registerInterpolation<Interpolation>();
    }

    // Default interplation for this property
    am.registerPropertyInterpolationConnection(PropertyTraits<PropertyType>::classIdentifier(),
                                               Interpolation::classIdentifier());
}

struct OrdinalReghelper {
    template <typename T>
    auto operator()(AnimationModule& am) {
        using namespace animation;
        using PropertyType = OrdinalProperty<T>;
        using ValueType = typename OrdinalProperty<T>::value_type;  // will be T in this case
        trackAndInterpolationRegHelper<PropertyType, ValueType,
                                       LinearInterpolation<ValueKeyframe<ValueType>>>(am);
    }
};

struct MinMaxReghelper {
    template <typename T>
    auto operator()(AnimationModule& am) {
        using namespace animation;
        using PropertyType = MinMaxProperty<T>;
        using ValueType = typename MinMaxProperty<T>::value_type;  // tvec2<T>
        trackAndInterpolationRegHelper<PropertyType, ValueType,
                                       LinearInterpolation<ValueKeyframe<ValueType>>>(am);
    }
};

struct OptionReghelper {
    template <typename T>
    auto operator()(AnimationModule& am) {
        using namespace animation;
        using PropertyType = TemplateOptionProperty<T>;
        using ValueType = typename TemplateOptionProperty<T>::value_type;
        trackAndInterpolationRegHelper<PropertyType, ValueType,
                                       ConstantInterpolation<ValueKeyframe<ValueType>>>(am);
    }
};

struct ConstantInterpolationReghelper {
    template <typename PropertyType>
    auto operator()(AnimationModule& am) {
        using namespace animation;
        using ValueType = typename PropertyType::value_type;
        trackAndInterpolationRegHelper<PropertyType, ValueType,
                                       ConstantInterpolation<ValueKeyframe<ValueType>>>(am);
    }
};

AnimationModule::AnimationModule(InviwoApplication* app)
    : InviwoModule(app, "Animation"), animation::AnimationSupplier(manager_), manager_(app, this) {

    using namespace animation;

    // Register Ordinal properties
    using Types = std::tuple<float, vec2, vec3, vec4, mat2, mat3, mat4, double, dvec2, dvec3, dvec4,
                             dmat2, dmat3, dmat4, int, ivec2, ivec3, ivec4, unsigned int, uvec2,
                             uvec3, uvec4, size_t, size2_t, size3_t, size4_t>;
    util::for_each_type<Types>{}(OrdinalReghelper{}, *this);

    // Register MinMaxProperties
    using ScalarTypes = std::tuple<float, double, int, unsigned int, size_t>;
    util::for_each_type<ScalarTypes>{}(MinMaxReghelper{}, *this);

    // Register properties that should not interpolate
    // Todo: Add MultiFileProperty when we can deal with vector<T> data in animation
    using ConstantInterpolationProperties = std::tuple<BoolProperty, FileProperty, StringProperty>;
    util::for_each_type<ConstantInterpolationProperties>{}(ConstantInterpolationReghelper{}, *this);
    util::for_each_type<ScalarTypes>{}(OptionReghelper{}, *this);
    util::for_each_type<std::tuple<std::string>>{}(OptionReghelper{}, *this);
    // Todo: Add ButtonProperty. Have not tested but might work out of the box with constant
    // interpolation? Todo: Add support for TransferFunctionProperty (special interpolation)
}

AnimationModule::~AnimationModule() {
    // need to call that here since we will have to delete the manager before we call the destructor
    // of AnimationSupplier. Other modules that implement AnimationSupplier will not have this
    // problem.
    unRegisterAll();
}

animation::AnimationManager& AnimationModule::getAnimationManager() { return manager_; }

const animation::AnimationManager& AnimationModule::getAnimationManager() const { return manager_; }

}  // namespace inviwo
