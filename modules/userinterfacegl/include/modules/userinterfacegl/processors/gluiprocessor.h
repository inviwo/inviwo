/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/listproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/ports/imageport.h>

/*
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>
*/

#include <modules/userinterfacegl/glui/renderer.h>
#include <modules/userinterfacegl/glui/layout/boxlayout.h>

#include <unordered_map>

namespace inviwo {

class InviwoApplication;

namespace glui {

class Element;

/** \docpage{org.inviwo.GLUIProcessor, GLUIProcessor}
 * ![](org.inviwo.GLUIProcessor.png?classIdentifier=org.inviwo.GLUIProcessor)
 * provides a simple, adpative user interface based on GLUI. Properties can be added to and
 * removed from the list property.
 *
 * ### Inports
 *   * __inport__  Input image
 *
 * ### Outports
 *   * __outport__ Input image with the UI rendered on top.
 *
 * ### UI Properties
 *   * __UI Visible__   UI visibility, i.e. whether the UI is rendered or not
 *   * __Position__ Where to put the text, relative position from 0 to 1
 *   * __Anchor__ What point of the text to put at "Position". relative from -1,1. 0 means the image
 *     is centered on "Position".
 *   * __UI Color__     main color of the UI
 *   * __UI Interaction Color__
 *   * __Text Color__   color of the text labels
 *   * __Hover Color__  highlight color when hovering UI elements
 *   * __Layout Spacing__      spacing in between UI elements
 *   * __Layout Margins__      margins applied to the layout (top, left, bottom, right)
 */

/**
 * \brief provides a simple user interface based on GLUI
 */
class IVW_MODULE_USERINTERFACEGL_API GLUIProcessor : public Processor,
                                                     public PropertyOwnerObserver {
public:
    GLUIProcessor(InviwoApplication* app = InviwoApplication::getPtr());
    virtual ~GLUIProcessor() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    virtual void onWillAddProperty(Property* property, size_t index) override;
    virtual void onDidAddProperty(Property* property, size_t index) override;

    virtual void onWillRemoveProperty(Property* property, size_t index) override;
    virtual void onDidRemoveProperty(Property* property, size_t index) override;

private:
    ImageInport inport_;
    ImageOutport outport_;

    CompositeProperty uiSettings_;
    BoolProperty uiVisible_;
    CompositeProperty positioning_;
    FloatVec2Property position_;
    FloatVec2Property anchorPos_;
    IntVec2Property offset_;
    FloatProperty uiScaling_;
    FloatVec4Property uiColor_;
    FloatVec4Property uiSecondaryColor_;
    FloatVec4Property uiBorderColor_;
    FloatVec4Property uiDisabledColor_;
    FloatVec4Property uiTextColor_;
    FloatVec4Property hoverColor_;

    TemplateOptionProperty<glui::BoxLayout::LayoutDirection> layoutDirection_;

    IntProperty layoutSpacing_;
    IntVec4Property layoutMargins_;

    std::unordered_map<Property*, std::unique_ptr<glui::Element>> propertyWidgetMap_;
    ListProperty dynamicProperties_;

    glui::Renderer uiRenderer_;
    glui::BoxLayout layout_;

    InviwoApplication* app_;
};

}  // namespace glui

}  // namespace inviwo
