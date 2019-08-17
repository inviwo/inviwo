/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
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

#include <modules/userinterfacegl/processors/gluitestprocessor.h>
#include <modules/userinterfacegl/glui/element.h>

#include <inviwo/core/util/moduleutils.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/openglutils.h>

#include <modules/userinterfacegl/glui/widgets/button.h>
#include <modules/userinterfacegl/glui/widgets/checkbox.h>
#include <modules/userinterfacegl/glui/widgets/slider.h>
#include <modules/userinterfacegl/glui/widgets/rangeslider.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo GLUITestProcessor::processorInfo_{
    "org.inviwo.GLUITestProcessor",  // Class identifier
    "GLUITest Processor",            // Display name
    "UI",                            // Category
    CodeState::Experimental,         // Code state
    "GL, UI",                        // Tags
};
const ProcessorInfo GLUITestProcessor::getProcessorInfo() const { return processorInfo_; }

GLUITestProcessor::GLUITestProcessor()
    : Processor()
    , inport_("inport")
    , outport_("outport")
    // properties represented by GLUI
    , boolProperty_("boolProperty", "Bool Property", true)
    , intProperty_("intProperty", "Int Property", 20, 0, 100)
    , floatProperty_("floatProperty", "Float Property", 0.5f, 0.0f, 1.0f)
    , intMinMaxProperty_("intMinMaxProperty", "Int Min Max Property", 10, 50, 0, 100,
                         Defaultvalues<int>::getInc(), 30)
    , buttonProperty_("buttonProperty", "Button Property")
    , readOnlyBoolProperty_("readOnlyBoolProperty", "Read-Only Bool Property", true)
    , readOnlyIntProperty_("readOnlyIntProperty", "Read-Only Int Property", 75, 0, 100)
    , readOnlyButtonProperty_("readOnlyButtonProperty", "Read-Only Button")

    // settings properties for GLUI
    , uiSettings_("uiSettings", "UI Settings")
    , uiVisible_("uiVisibility", "UI Visible", true)
    , uiScaling_("uiScaling", "UI Scaling", 1.0f, 0.0f, 4.0f)
    , uiColor_("uiColor", "UI Color", vec4(0.51f, 0.64f, 0.91f, 1.0f), vec4(0.0f), vec4(1.0f))
    , uiSecondaryColor_("uiSecondaryColor", "UI Secondary Color", vec4(0.4f, 0.4f, 0.45f, 1.0f),
                        vec4(0.0f), vec4(1.0f))
    , uiBorderColor_("uiBorderColor", "UI Border Color", vec4(vec3(0.1f), 1.0f), vec4(0.0f),
                     vec4(1.0f))
    , uiDisabledColor_("uiDisabledColor", "UI Disabled Color", vec4(0.6f, 0.6f, 0.63f, 1.0f),
                       vec4(0.0f), vec4(1.0f))
    , uiTextColor_("uiTextColor", "Text Color", vec4(vec3(0.0f), 1.0f), vec4(0.0f), vec4(1.0f))
    , hoverColor_("hoverColor", "Hover Color", vec4(1.0f, 1.0f, 1.0f, 0.5f), vec4(0.0f), vec4(1.0f))
    , layoutDirection_("layoutDirection", "Layout Direction",
                       {{"horizontal", "Horizontal", glui::BoxLayout::LayoutDirection::Horizontal},
                        {"vertical", "Vertical", glui::BoxLayout::LayoutDirection::Vertical}},
                       1)
    , intPropertyVertical_("intPropertyVertical", "Int Property Vertical", false)
    , layoutSpacing_("layoutSpacing", "Layout Spacing", 5, 0, 50)
    , layoutMargins_("layoutMargins", "Layout Margins", ivec4(10), ivec4(0), ivec4(50))
    , layout_(glui::BoxLayout::LayoutDirection::Vertical)
    // GLUI property widgets
    , boolPropertyUI_(boolProperty_, *this, uiRenderer_)
    , intPropertyUI_(intProperty_, *this, uiRenderer_, ivec2(100, 24))
    , floatPropertyUI_(floatProperty_, *this, uiRenderer_, ivec2(100, 24))
    , intMinMaxPropertyUI_(intMinMaxProperty_, *this, uiRenderer_, ivec2(100, 24))
    , buttonPropertyUI_(buttonProperty_, *this, uiRenderer_, ivec2(150, 30))
    , toolButtonPropertyUI_(buttonProperty_, nullptr, *this, uiRenderer_, ivec2(32, 32))

    , readOnlyBoolPropertyUI_(readOnlyBoolProperty_, *this, uiRenderer_)
    , readOnlyIntPropertyUI_(readOnlyIntProperty_, *this, uiRenderer_, ivec2(100, 24))
    , readOnlyButtonPropertyUI_(readOnlyButtonProperty_, *this, uiRenderer_, ivec2(150, 30)) {

    inport_.setOptional(true);

    addPort(inport_);
    addPort(outport_);

    // UI specific settings
    uiColor_.setSemantics(PropertySemantics::Color);
    uiSecondaryColor_.setSemantics(PropertySemantics::Color);
    uiBorderColor_.setSemantics(PropertySemantics::Color);
    uiDisabledColor_.setSemantics(PropertySemantics::Color);
    uiTextColor_.setSemantics(PropertySemantics::Color);
    hoverColor_.setSemantics(PropertySemantics::Color);

    uiSettings_.addProperty(uiVisible_);
    uiSettings_.addProperty(uiScaling_);
    uiSettings_.addProperty(uiColor_);
    uiSettings_.addProperty(uiSecondaryColor_);
    uiSettings_.addProperty(uiBorderColor_);
    uiSettings_.addProperty(uiTextColor_);
    uiSettings_.addProperty(hoverColor_);
    uiSettings_.addProperty(uiDisabledColor_);
    uiSettings_.addProperty(intPropertyVertical_);
    uiSettings_.addProperty(layoutDirection_);
    uiSettings_.addProperty(layoutSpacing_);
    uiSettings_.addProperty(layoutMargins_);

    // regular properties
    buttonProperty_.onChange([&]() { LogInfo("Property button pressed"); });

    addProperty(boolProperty_);
    addProperty(intProperty_);
    addProperty(floatProperty_);
    addProperty(intMinMaxProperty_);
    addProperty(buttonProperty_);
    // read-only properties
    readOnlyBoolProperty_.setReadOnly(true);
    readOnlyIntProperty_.setReadOnly(true);
    readOnlyButtonProperty_.setReadOnly(true);
    addProperty(readOnlyBoolProperty_);
    addProperty(readOnlyIntProperty_);
    addProperty(readOnlyButtonProperty_);
    // settings
    addProperty(uiSettings_);

    setAllPropertiesCurrentStateAsDefault();

    // inviwo GLUI property widgets
    propertyLayout_.addElement(boolPropertyUI_);
    propertyLayout_.addElement(readOnlyBoolPropertyUI_);
    propertyLayout_.addElement(intPropertyUI_);
    propertyLayout_.addElement(readOnlyIntPropertyUI_);
    propertyLayout_.addElement(floatPropertyUI_);
    propertyLayout_.addElement(intMinMaxPropertyUI_);
    propertyLayout_.addElement(buttonPropertyUI_);
    propertyLayout_.addElement(readOnlyButtonPropertyUI_);
    propertyLayout_.addElement(toolButtonPropertyUI_);

    toolButtonPropertyUI_.setImage(module::getModulePath("UserInterfaceGL", ModulePath::Images) +
                                   "/home.png");

    // plain GLUI widgets w/o connection to any Inviwo property
    //
    // create a small check box
    widgets_.emplace_back(
        std::make_unique<glui::CheckBox>("checkbox 1", *this, uiRenderer_, ivec2(24, 24)));
    // create a larger check box
    widgets_.emplace_back(
        std::make_unique<glui::CheckBox>("checkbox 2", *this, uiRenderer_, ivec2(32, 32)));
    // create a slider
    auto slider =
        std::make_unique<glui::Slider>("slider", 0, 0, 100, *this, uiRenderer_, ivec2(100, 24));
    slider->setAction([&, p = slider.get()]() { LogInfo("UI slider changed: " << p->get()); });
    widgets_.emplace_back(std::move(slider));
    // create a range slider
    auto rangeslider = std::make_unique<glui::RangeSlider>("rangeslider", ivec2(10, 70), 0, 100, 40,
                                                           *this, uiRenderer_, ivec2(100, 24));
    rangeslider->setAction(
        [&, p = rangeslider.get()]() { LogInfo("UI range slider changed: " << p->get()); });
    widgets_.emplace_back(std::move(rangeslider));
    // create a wide button
    auto button = std::make_unique<glui::Button>("button 1", *this, uiRenderer_, ivec2(100, 28));
    button->setAction([&]() { LogInfo("UI button pressed"); });
    widgets_.emplace_back(std::move(button));
    // create a large button
    widgets_.emplace_back(
        std::make_unique<glui::Button>("button 2", *this, uiRenderer_, ivec2(80, 50)));

    for (auto &widget : widgets_) {
        layout_.addElement(*widget);
    }

    // initialize color states
    uiRenderer_.setUIColor(uiColor_.get());
    uiRenderer_.setSecondaryUIColor(uiSecondaryColor_.get());
    uiRenderer_.setBorderColor(uiBorderColor_.get());
    uiRenderer_.setTextColor(uiTextColor_.get());
    uiRenderer_.setHoverColor(hoverColor_.get());

    auto updateSliderOrientation = [this]() {
        if (intPropertyVertical_.get()) {
            // vertical
            intPropertyUI_.setOrientation(glui::UIOrientation::Vertical);
            intPropertyUI_.setWidgetExtent(ivec2(24, 100));

            intMinMaxPropertyUI_.setOrientation(glui::UIOrientation::Vertical);
            intMinMaxPropertyUI_.setWidgetExtent(ivec2(24, 100));
        } else {
            // horizontal
            intPropertyUI_.setOrientation(glui::UIOrientation::Horizontal);
            intPropertyUI_.setWidgetExtent(ivec2(100, 24));

            intMinMaxPropertyUI_.setOrientation(glui::UIOrientation::Horizontal);
            intMinMaxPropertyUI_.setWidgetExtent(ivec2(100, 24));
        }
    };

    intPropertyVertical_.onChange(updateSliderOrientation);
    updateSliderOrientation();
}

void GLUITestProcessor::process() {
    if (uiColor_.isModified()) {
        uiRenderer_.setUIColor(uiColor_.get());
    }
    if (uiSecondaryColor_.isModified()) {
        uiRenderer_.setSecondaryUIColor(uiSecondaryColor_.get());
    }
    if (uiBorderColor_.isModified()) {
        uiRenderer_.setBorderColor(uiBorderColor_.get());
    }
    if (uiTextColor_.isModified()) {
        uiRenderer_.setTextColor(uiTextColor_.get());
    }
    if (hoverColor_.isModified()) {
        uiRenderer_.setHoverColor(hoverColor_.get());
    }
    if (uiDisabledColor_.isModified()) {
        uiRenderer_.setDisabledColor(uiDisabledColor_.get());
    }
    if (uiScaling_.isModified()) {
        // scaling will affect all glui elements, the change is propagated by the layout
        propertyLayout_.setScalingFactor(uiScaling_.get());
    }

    // layout
    if (layoutDirection_.isModified()) {
        layout_.setDirection(layoutDirection_.get());
        propertyLayout_.setDirection(layoutDirection_.get());
    }
    if (layoutSpacing_.isModified()) {
        layout_.setSpacing(layoutSpacing_.get());
        propertyLayout_.setSpacing(layoutSpacing_.get());
    }
    if (layoutMargins_.isModified()) {
        const ivec4 &m(layoutMargins_.get());
        layout_.setMargins(m.x, m.y, m.z, m.w);
        propertyLayout_.setMargins(m.x, m.y, m.z, m.w);
    }

    if (inport_.isReady()) {
        utilgl::activateTargetAndCopySource(outport_, inport_);
    } else {
        utilgl::activateAndClearTarget(outport_);
    }
    utilgl::DepthFuncState depthFunc(GL_ALWAYS);
    utilgl::BlendModeState blending(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    if (uiVisible_.get()) {
        // coordinate system defined in screen coords with origin in the top-left corner

        {
            // put UI elements in lower left corner of the canvas
            const ivec2 extent(layout_.getExtent());
            ivec2 origin(0, extent.y);

            layout_.render(origin, outport_.getDimensions());
        }

        {
            // render second layout in upper right corner
            const ivec2 extent(propertyLayout_.getExtent());
            const ivec2 outputDim(outport_.getDimensions());
            ivec2 origin(outputDim.x - extent.x, outputDim.y);

            propertyLayout_.render(origin, outport_.getDimensions());
        }
    }

    utilgl::deactivateCurrentTarget();
}

}  // namespace inviwo
