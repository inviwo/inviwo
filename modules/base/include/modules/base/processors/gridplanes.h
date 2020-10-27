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

#include <inviwo/core/util/stdextensions.h>

#include <array>

namespace inviwo {

enum class ListOrSingleValuePropertyState { Single, List };

template <typename Prop, unsigned N>
class ListOrSingleValueProperty : public CompositeProperty {
public:
    using State = ListOrSingleValuePropertyState;

    template <typename... DefaultArgs>
    ListOrSingleValueProperty(const std::string& identifier, const std::string& displayName,
                              State initState, DefaultArgs... defaultArgs)
        : CompositeProperty(identifier, displayName)
        , state_{"state",
                 "State",
                 {{"single", "Single", State::Single}, {"list", "List", State::List}},
                 initState == State::Single ? size_t{0} : size_t{1}}
        , single_{"single", displayName, defaultArgs...}
        , list_{util::make_array<size_t{N}>([&](auto index) {
            auto id = [](auto i) {
                if constexpr (N <= 4) {
                    return xyzwAxisNames[i];
                } else {
                    return i;
                }
            }(index);

            return Prop(fmt::format("axis_{}", id), fmt::format("{} {}", displayName, id),
                        defaultArgs...);
        })}

    {
        addProperties(state_, single_);

        for (auto& prop : list_) {
            addProperty(prop);
            prop.visibilityDependsOn(state_, [](auto& p) { return p.get() == State::List; });
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

private:
    const static inline std::array<std::string, 4> xyzwAxisNames = {"x", "y", "z", "w"};
};

/** \docpage{org.inviwo.GridPlanes, Grid Planes}
 * ![](org.inviwo.GridSystem.png?classIdentifier=org.inviwo.GridPlanes)
 *
 * Creates a mesh that can be used to draw grid planes for the current coordinate system. 
 *
 * ### Inports
 *   * __transform__ Optional volume inport. If a volume is connected the grid will be aligned to that volume.
 *
 * ### Outports
 *   * __grid__ A mesh containing the grid planes, can be rendered using, for example, the Mesh Renderer, Line Renderer or Tube Renderer.
 *
 * ### Properties
 * Each property can be toggled between having one value for each individual plane or a single value used for all planes
 *   * __Enable__ Toggles wether or not a given grid plane should be visible
 *   * __Spacing__ Set the distance between the each line along the given axis
 *   * __Extent__ Set the extent of the grid along the given axis. 
 *   * __Color__ Set the color of each grid plane.
 *
 */
class IVW_MODULE_BASE_API GridPlanes : public Processor {
public:
    GridPlanes();
    virtual ~GridPlanes() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    VolumeInport basis_{"basis"};
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
