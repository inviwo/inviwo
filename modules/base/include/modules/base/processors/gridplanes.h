/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2024 Inviwo Foundation
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

#include <modules/base/basemoduledefine.h>  // for IVW_MODULE_BASE_API

#include <inviwo/core/ports/meshport.h>                    // for MeshOutport
#include <inviwo/core/ports/volumeport.h>                  // for VolumeInport
#include <inviwo/core/processors/processor.h>              // for Processor
#include <inviwo/core/processors/processorinfo.h>          // for ProcessorInfo
#include <inviwo/core/properties/boolcompositeproperty.h>  // for BoolCompositeProperty
#include <inviwo/core/properties/boolproperty.h>           // for BoolProperty
#include <inviwo/core/properties/minmaxproperty.h>         // for FloatMinMaxProperty
#include <inviwo/core/properties/ordinalproperty.h>        // for FloatProperty, FloatVec4Property
#include <inviwo/core/properties/property.h>               // for Property::OnChangeBlocker
#include <inviwo/core/util/stdextensions.h>                // for make_array

#include <array>        // for array
#include <cstddef>      // for size_t
#include <string_view>  // for string_view

namespace inviwo {

template <typename Prop, unsigned N>
class SyncedListProperty : public BoolCompositeProperty {
public:
    template <typename... DefaultArgs>
    SyncedListProperty(std::string_view identifier, std::string_view displayName,
                       DefaultArgs... defaultArgs)
        : BoolCompositeProperty(identifier, displayName)

        , props{util::make_array<size_t{N}>([&](auto index) {
            constexpr std::array<std::string_view, 4> names = {"x", "y", "z", "w"};

            return Prop(fmt::format("axis_{}", names[index]),
                        fmt::format("{} {}", displayName, names[index]), defaultArgs...);
        })} {

        getBoolProperty()->setDisplayName("Sync");

        auto sync = [&](Prop* current) {
            return [this, current]() {
                if (!getBoolProperty()->get()) return;
                for (auto& prop : props) {
                    if (&prop != current) {
                        OnChangeBlocker block(prop);
                        prop.set(current->get());
                    }
                }
            };
        };

        for (auto& prop : props) {
            prop.onChange(sync(&prop));
            addProperty(prop);
        }

        setAllPropertiesCurrentStateAsDefault();
    }

    auto get(unsigned i) const { return props[i].get(); }

    virtual ~SyncedListProperty() = default;

    std::array<Prop, N> props;

private:
};

class IVW_MODULE_BASE_API GridPlanes : public Processor {
public:
    GridPlanes();
    virtual ~GridPlanes() = default;

    virtual void process() override;

    virtual const ProcessorInfo& getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    VolumeInport basis_;
    MeshOutport grid_;

    SyncedListProperty<BoolProperty, 3> enable_;
    SyncedListProperty<FloatProperty, 3> spacing_;

    SyncedListProperty<FloatMinMaxProperty, 3> extent_;
    SyncedListProperty<FloatVec4Property, 3> color_;
};

}  // namespace inviwo
