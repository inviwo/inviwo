/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2026 Inviwo Foundation
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

#include <modules/userinterfacegl/userinterfaceglmoduledefine.h>

#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/processorinfo.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/util/staticstring.h>
#include <modules/userinterfacegl/glui/element.h>
#include <modules/userinterfacegl/glui/layout/boxlayout.h>
#include <modules/userinterfacegl/glui/layout/vboxlayout.h>
#include <modules/userinterfacegl/glui/renderer.h>
#include <modules/userinterfacegl/glui/widgets/boolpropertywidget.h>
#include <modules/userinterfacegl/glui/widgets/buttonpropertywidget.h>
#include <modules/userinterfacegl/glui/widgets/floatpropertywidget.h>
#include <modules/userinterfacegl/glui/widgets/intminmaxpropertywidget.h>
#include <modules/userinterfacegl/glui/widgets/intpropertywidget.h>

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace inviwo {

/**
 * @brief provides a simple user interface based on GLUI
 */
class IVW_MODULE_USERINTERFACEGL_API GLUITestProcessor : public Processor {
public:
    GLUITestProcessor();
    virtual ~GLUITestProcessor() = default;

    virtual void process() override;

    virtual const ProcessorInfo& getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    ImageInport inport_;
    ImageOutport outport_;

    BoolProperty boolProperty_;
    IntProperty intProperty_;
    FloatProperty floatProperty_;
    IntMinMaxProperty intMinMaxProperty_;
    ButtonProperty buttonProperty_;

    BoolProperty readOnlyBoolProperty_;
    IntProperty readOnlyIntProperty_;
    ButtonProperty readOnlyButtonProperty_;

    CompositeProperty uiSettings_;
    BoolProperty uiVisible_;
    FloatProperty uiScaling_;
    FloatVec4Property uiColor_;
    FloatVec4Property uiSecondaryColor_;
    FloatVec4Property uiBorderColor_;
    FloatVec4Property uiDisabledColor_;
    FloatVec4Property uiTextColor_;
    FloatVec4Property hoverColor_;

    OptionProperty<glui::BoxLayout::LayoutDirection> layoutDirection_;

    BoolProperty intPropertyVertical_;

    IntProperty layoutSpacing_;
    IntVec4Property layoutMargins_;

    glui::Renderer uiRenderer_;
    glui::BoxLayout layout_;
    glui::VBoxLayout propertyLayout_;

    // GLUI widgets
    glui::BoolPropertyWidget boolPropertyUI_;
    glui::IntPropertyWidget intPropertyUI_;
    glui::FloatPropertyWidget floatPropertyUI_;
    glui::IntMinMaxPropertyWidget intMinMaxPropertyUI_;
    glui::ButtonPropertyWidget buttonPropertyUI_;
    glui::ToolButtonPropertyWidget toolButtonPropertyUI_;

    // read-only widgets
    glui::BoolPropertyWidget readOnlyBoolPropertyUI_;
    glui::IntPropertyWidget readOnlyIntPropertyUI_;
    glui::ButtonPropertyWidget readOnlyButtonPropertyUI_;

    // container for holding arbitrary GLUI element not connected to any properties
    std::vector<std::unique_ptr<glui::Element>> widgets_;
};

}  // namespace inviwo
