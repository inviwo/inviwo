/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2024 Inviwo Foundation
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

#include <modules/basegl/baseglmoduledefine.h>  // for IVW_MODULE_BASEGL_API

#include "inviwo/core/util/colorbrewer-generated.h"                    // for Family, operator<<
#include <inviwo/core/datastructures/image/layer.h>                    // for Layer
#include <inviwo/core/interaction/pickingmapper.h>                     // for PickingMapper
#include <inviwo/core/ports/volumeport.h>                              // for VolumeInport
#include <inviwo/core/properties/buttongroupproperty.h>                // for ButtonGroupProperty
#include <inviwo/core/properties/optionproperty.h>                     // for OptionProperty
#include <inviwo/core/properties/ordinalproperty.h>                    // for FloatProperty, Flo...
#include <inviwo/core/properties/transferfunctionproperty.h>           // for TransferFunctionPr...
#include <inviwo/core/util/staticstring.h>                             // for operator+
#include <modules/basegl/shadercomponents/shadercomponent.h>           // for ShaderComponent
#include <modules/brushingandlinking/ports/brushingandlinkingports.h>  // for BrushingAndLinking...

#include <functional>   // for __base
#include <string>       // for operator==, string
#include <string_view>  // for operator==, string...
#include <tuple>        // for tuple
#include <vector>       // for operator!=, vector

namespace inviwo {

class Inport;
class PickingEvent;
class Processor;
class Property;
class Shader;
class TextureUnitContainer;
class TimeComponent;

/**
 * Adds a atlas Volume inport, a BrushingAndLinking inport, and related functionality to do
 * segmented or "atlas" volume raycasting.
 */
class IVW_MODULE_BASEGL_API AtlasComponent : public ShaderComponent {
public:
    AtlasComponent(Processor* p, std::string_view color, TimeComponent* time);

    virtual std::string_view getName() const override;

    virtual void process(Shader& shader, TextureUnitContainer& cont) override;

    virtual std::vector<std::tuple<Inport*, std::string>> getInports() override;

    virtual std::vector<Property*> getProperties() override;

    virtual std::vector<Segment> getSegments() override;

    void onPickingEvent(PickingEvent* e);

    VolumeInport& getAtlasInport() { return atlas_; }

private:
    enum class ColoringGroup { All, Selected, Unselected, Filtered, Unfiltered, Zero };
    enum class ColoringAction { None, SetColor, SetAlpha, SetScheme };

    VolumeInport atlas_;
    BrushingAndLinkingInport brushing_;

    FloatVec3Property selectionColor_;
    FloatProperty selectionAlpha_;
    FloatProperty selectionMix_;

    FloatVec3Property filteredColor_;
    FloatProperty filteredAlpha_;
    FloatProperty filteredMix_;

    TransferFunctionProperty tf_;
    OptionProperty<ColoringGroup> coloringGroup_;
    FloatVec3Property coloringColor_;
    FloatProperty coloringAlpha_;
    OptionProperty<colorbrewer::Family> coloringScheme_;

    ButtonGroupProperty coloringApply_;
    ColoringAction coloringAction_;

    Layer colors_;
    std::string color_;
    PickingMapper picking_;
    int minSegmentId_;
    TimeComponent* time_;
};

}  // namespace inviwo
