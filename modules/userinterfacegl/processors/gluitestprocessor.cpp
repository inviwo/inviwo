/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017 Inviwo Foundation
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
#include <modules/userinterfacegl/glui/widgets/boolpropertywidget.h>
#include <modules/userinterfacegl/glui/widgets/buttonpropertywidget.h>
#include <modules/userinterfacegl/glui/widgets/intpropertywidget.h>

#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/openglutils.h>

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
    , boolProperty_("boolProperty", "Bool Property", true)
    , intProperty_("intProperty", "Int Property", 20, 0, 100)
    , buttonProperty_("buttonProperty", "Button Property")
    , uiSettings_("uiSettings", "UI Settings")
    , uiVisible_("uiVisibility", "UI Visible", true)
    , uiColor_("uiColor", "UI Color", vec4(vec3(1.0f), 1.0f), vec4(0.0f), vec4(1.0f))
    , uiModeColor_("uiModeColor", "UI Interaction Color", vec4(0.4f, 0.6f, 1.0f, 1.0f), vec4(0.0f),
                   vec4(1.0f))
    , uiTextColor_("uiTextColor", "Text Color", vec4(vec3(0.0f), 1.0f), vec4(0.0f), vec4(1.0f))
    , hoverColor_("hoverColor", "Hover Color", vec4(1.0f, 1.0f, 1.0f, 0.5f), vec4(0.0f), vec4(1.0f))
    , layoutDirection_("layoutDirection", "Layout Direction",
                       {{"horizontal", "Horizontal", glui::BoxLayout::LayoutDirection::Horizontal},
                        {"vertical", "Vertical", glui::BoxLayout::LayoutDirection::Vertical}},
                       1)
    , layoutSpacing_("layoutSpacing", "Layout Spacing", 5, 0, 50)
    , layoutMargins_("layoutMargins", "Layout Margins", ivec4(10), ivec4(0), ivec4(50))
    , uiManager_(this)
    , layout_(glui::BoxLayout::LayoutDirection::Vertical)
    , boolPropertyUI_(nullptr)
    , intPropertyUI_(nullptr)
    , buttonPropertyUI_(nullptr) {
    inport_.setOptional(true);

    addPort(inport_);
    addPort(outport_);

    // UI specific settings
    uiColor_.setSemantics(PropertySemantics::Color);
    uiModeColor_.setSemantics(PropertySemantics::Color);
    uiTextColor_.setSemantics(PropertySemantics::Color);
    hoverColor_.setSemantics(PropertySemantics::Color);

    uiSettings_.addProperty(uiVisible_);
    uiSettings_.addProperty(uiColor_);
    uiSettings_.addProperty(uiTextColor_);
    uiSettings_.addProperty(hoverColor_);
    uiSettings_.addProperty(uiModeColor_);
    uiSettings_.addProperty(layoutDirection_);
    uiSettings_.addProperty(layoutSpacing_);
    uiSettings_.addProperty(layoutMargins_);

    // regular properties
    buttonProperty_.onChange([&]() { LogInfo("button pressed"); });

    addProperty(boolProperty_);
    addProperty(intProperty_);
    addProperty(buttonProperty_);
    addProperty(uiSettings_);

    setAllPropertiesCurrentStateAsDefault();

    // plain GLUI widgets

    // create a small check box
    auto elem = uiManager_.createUIElement(glui::ItemType::Checkbox, "checkbox 1", ivec2(24, 24));
    layout_.addElement(elem);
    // create a larger check box
    elem = uiManager_.createUIElement(glui::ItemType::Checkbox, "checkbox 2", ivec2(32, 32));
    layout_.addElement(elem);
    // create a wide button
    elem = uiManager_.createUIElement(glui::ItemType::Button, "button", ivec2(100, 24));
    layout_.addElement(elem);
    // create a large button
    elem = uiManager_.createUIElement(glui::ItemType::Button, "button2", ivec2(70, 50));
    layout_.addElement(elem);

    // inviwo GLUI property widgets
    boolPropertyUI_ = new glui::BoolPropertyWidget(&uiManager_, &boolProperty_);
    uiManager_.addUIElement(boolPropertyUI_);

    intPropertyUI_ = new glui::IntPropertyWidget(&uiManager_, &intProperty_, ivec2(100, 24));
    uiManager_.addUIElement(intPropertyUI_);

    buttonPropertyUI_ =
        new glui::ButtonPropertyWidget(&uiManager_, &buttonProperty_, ivec2(150, 30));
    uiManager_.addUIElement(buttonPropertyUI_);

    propertyLayout_.addElement(boolPropertyUI_);
    propertyLayout_.addElement(intPropertyUI_);
    propertyLayout_.addElement(buttonPropertyUI_);

    // initialize color states
    uiManager_.setUIColor(uiColor_.get());
    uiManager_.setTextColor(uiTextColor_.get());
    uiManager_.setHoverColor(hoverColor_.get());
}

void GLUITestProcessor::process() {
    if (uiColor_.isModified()) {
        uiManager_.setUIColor(uiColor_.get());
    }
    if (uiTextColor_.isModified()) {
        uiManager_.setTextColor(uiTextColor_.get());
    }
    if (hoverColor_.isModified()) {
        uiManager_.setHoverColor(hoverColor_.get());
    }
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

    if (inport_.isConnected()) {
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
            ivec2 origin(ivec2(0, extent.y));

            uiManager_.renderLayout(layout_, origin, outport_);
        }

        {
            // render second layout in upper right corner
            const ivec2 extent(propertyLayout_.getExtent());
            const ivec2 outputDim(outport_.getDimensions());
            ivec2 origin(ivec2(outputDim.x - extent.x, outputDim.y));

            uiManager_.renderLayout(propertyLayout_, origin, outport_);
        }
    }

    utilgl::deactivateCurrentTarget();
}

}  // namespace inviwo
