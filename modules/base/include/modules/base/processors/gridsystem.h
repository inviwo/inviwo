/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020 Inviwo Foundation
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

#include <modules/base/basemoduledefine.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/ports/meshport.h>

namespace inviwo {

namespace detail {
template <unsigned N, typename T>
constexpr T passthrough_the_value_helper(const T& t) {
    return t;
}

template <typename T, unsigned... I>
constexpr auto copyInitFilledArrayHelper(const T& t, std::index_sequence<I...>) {
    return std::array<T, sizeof...(I)>{passthrough_the_value_helper<I>(t)...};
}

template <unsigned N, typename T, typename Indices = std::make_index_sequence<N>>
constexpr std::array<T, N> copyInitFilledArray(const T& t) {
    return copyInitFilledArrayHelper(t, Indices{});
}
}  // namespace detail

enum class ListOrSingleValuePropertyState { Single, List };

template <typename Prop, unsigned N>
class ListOrSingleValueProperty : public CompositeProperty {
public:
    const static inline std::array<std::string, 4> xyzwAxisNames = {"x", "y", "z", "w"};

    using State = ListOrSingleValuePropertyState;

    template <typename... DefaultArgs>
    ListOrSingleValueProperty(const std::string& identifier, const std::string& displayName,
                              State initState = State::Single, DefaultArgs... defaultArgs)
        : CompositeProperty(identifier, displayName)
        , state_{"state",
                 "State",
                 {{"single", "Single", State::Single}, {"list", "List", State::List}},
                 initState == State::Single ? 0 : 1}
        , single_{"single", displayName, defaultArgs...}
        , list_{detail::copyInitFilledArray<N, Prop>(single_)}

    {
        addProperties(state_, single_);

        auto getId = [](auto i) {
            if constexpr (N <= 4) {
                return xyzwAxisNames[i];
            }
            return toString(i);
        };

        for (unsigned i = 0; i < N; i++) {
            const std::string id = getId(i);
            list_[i].setIdentifier("axis_" + id);
            list_[i].setDisplayName(displayName + " " + id);
            addProperty(list_[i]);

            list_[i].visibilityDependsOn(state_, [](auto& p) { return p.get() == State::List; });
        }
        single_.visibilityDependsOn(state_, [](auto& p) { return p.get() == State::Single; });

        setAllPropertiesCurrentStateAsDefault();
    }

    auto get(unsigned i) const {
        if (state_.get() == State::Single) {
            return single_.get();
        } else {
            return list_[i].get();
        }
    }

    virtual ~ListOrSingleValueProperty() = default;

    TemplateOptionProperty<State> state_;
    Prop single_;
    std::array<Prop, N> list_;
};

/** \docpage{org.inviwo.GridSystem, Grid System}
 * ![](org.inviwo.GridSystem.png?classIdentifier=org.inviwo.GridSystem)
 * Explanation of how to use the processor.
 *
 * ### Inports
 *   * __<Inport1>__ <description>.
 *
 * ### Outports
 *   * __<Outport1>__ <description>.
 *
 * ### Properties
 *   * __<Prop1>__ <description>.
 *   * __<Prop2>__ <description>
 */
class IVW_MODULE_BASE_API GridSystem : public Processor {
public:
    GridSystem();
    virtual ~GridSystem() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    VolumeInport transform_{"transform"};
    MeshOutport grid_{"grid"};

    ListOrSingleValueProperty<BoolProperty, 3> enable_{"enable", "Enable",
                                                       ListOrSingleValuePropertyState::List, true};

    ListOrSingleValueProperty<FloatProperty, 3> spacing_{
        "spacing", "Spacing", ListOrSingleValuePropertyState::Single, 0.1f};

    ListOrSingleValueProperty<FloatMinMaxProperty, 3> extent_{
        "extent", "Extent", ListOrSingleValuePropertyState::Single, -1.05f, 1.05f, -100.f, 100.f};

    ListOrSingleValueProperty<FloatVec4Property, 3> color_{"color",
                                                           "Color",
                                                           ListOrSingleValuePropertyState::Single,
                                                           vec4{0.5f, 0.5f, 0.5f, 1.f},
                                                           vec4(0.f),
                                                           vec4(1.f),
                                                           vec4(0.05f),
                                                           InvalidationLevel::InvalidOutput,
                                                           PropertySemantics::Color};
};

}  // namespace inviwo
