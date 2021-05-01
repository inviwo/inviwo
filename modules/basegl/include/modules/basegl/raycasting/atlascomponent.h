/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021 Inviwo Foundation
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

#include <modules/basegl/baseglmoduledefine.h>

#include <modules/basegl/raycasting/raycastingcomponent.h>
#include <modules/opengl/volume/volumeutils.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/properties/transferfunctionproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/buttongroupproperty.h>
#include <inviwo/core/interaction/pickingmapper.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/util/colorbrewer.h>

#include <modules/brushingandlinking/ports/brushingandlinkingports.h>

#include <string>
#include <vector>

namespace inviwo {

class PickingEvent;
class Processor;
class TimeComponent;

class IVW_MODULE_BASEGL_API AtlasComponent : public RaycasterComponent {
public:
    AtlasComponent(Processor* p, std::string_view volume, TimeComponent* time);

    virtual std::string_view getName() const override;

    virtual void process(Shader& shader, TextureUnitContainer& cont) override;

    virtual std::vector<std::tuple<Inport*, std::string>> getInports() override;

    virtual std::vector<Property*> getProperties() override;

    virtual std::vector<Segment> getSegments() override;

    void onPickingEvent(PickingEvent* e);

private:
    enum class ColoringGroup { All, Selected, NotSelected, Filtered, NotFiltered, Zero };
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
    TemplateOptionProperty<ColoringGroup> coloringGroup_;
    FloatVec3Property coloringColor_;
    FloatProperty coloringAlpha_;
    TemplateOptionProperty<colorbrewer::Family> coloringScheme_;

    ButtonGroupProperty coloringApply_;
    ColoringAction coloringAction_;

    Layer colors_;
    std::string volume_;
    PickingMapper picking_;
    TimeComponent* time_;
};

}  // namespace inviwo
