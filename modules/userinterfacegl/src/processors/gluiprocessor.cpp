
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

#include <modules/userinterfacegl/processors/gluiprocessor.h>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>

#include <modules/userinterfacegl/userinterfaceglmodule.h>
#include <modules/userinterfacegl/glui/widgetfactory.h>
#include <modules/userinterfacegl/glui/widgets/boolpropertywidget.h>
#include <modules/userinterfacegl/glui/widgets/buttonpropertywidget.h>
#include <modules/userinterfacegl/glui/widgets/floatminmaxpropertywidget.h>
#include <modules/userinterfacegl/glui/widgets/floatpropertywidget.h>
#include <modules/userinterfacegl/glui/widgets/intminmaxpropertywidget.h>
#include <modules/userinterfacegl/glui/widgets/intpropertywidget.h>

namespace inviwo {

namespace glui {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo GLUIProcessor::processorInfo_{
    "org.inviwo.GLUIProcessor",  // Class identifier
    "GLUIProcessor",             // Display name
    "UI",                        // Category
    CodeState::Stable,           // Code state
    "GL, UI, Properties",        // Tags
};
const ProcessorInfo GLUIProcessor::getProcessorInfo() const { return processorInfo_; }

GLUIProcessor::GLUIProcessor()
    : Processor()
    , inport_("inport")
    , outport_("outport")

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
    , layoutSpacing_("layoutSpacing", "Layout Spacing", 5, 0, 50)
    , layoutMargins_("layoutMargins", "Layout Margins", ivec4(10), ivec4(0), ivec4(50))
    // list of dynamic properties
    , dynamicProperties_(
          "dynamicProperties", "Properties",
          []() {
              std::vector<std::unique_ptr<Property>> v;
              v.emplace_back(std::make_unique<BoolProperty>("boolProperty", "Boolean"));
              v.emplace_back(std::make_unique<ButtonProperty>("buttonProperty", "Button"));
              v.emplace_back(std::make_unique<FloatProperty>("floatProperty", "Float Value"));
              v.emplace_back(
                  std::make_unique<FloatMinMaxProperty>("floatMinMaxProperty", "Float MinMax"));
              v.emplace_back(std::make_unique<IntProperty>("intProperty", "Int Value"));
              v.emplace_back(
                  std::make_unique<IntMinMaxProperty>("intMinMaxProperty", "Int MinMax"));
              return v;
          }())
    , layout_(glui::BoxLayout::LayoutDirection::Vertical) {

    inport_.setOptional(true);

    addPort(inport_);
    addPort(outport_);

    dynamicProperties_.PropertyOwnerObservable::addObserver(this);

    // UI specific settings
    uiColor_.setSemantics(PropertySemantics::Color);
    uiSecondaryColor_.setSemantics(PropertySemantics::Color);
    uiBorderColor_.setSemantics(PropertySemantics::Color);
    uiDisabledColor_.setSemantics(PropertySemantics::Color);
    uiTextColor_.setSemantics(PropertySemantics::Color);
    hoverColor_.setSemantics(PropertySemantics::Color);

    uiSettings_.setCollapsed(true);
    uiSettings_.addProperty(uiVisible_);
    uiSettings_.addProperty(uiScaling_);
    uiSettings_.addProperty(uiColor_);
    uiSettings_.addProperty(uiSecondaryColor_);
    uiSettings_.addProperty(uiBorderColor_);
    uiSettings_.addProperty(uiTextColor_);
    uiSettings_.addProperty(hoverColor_);
    uiSettings_.addProperty(uiDisabledColor_);
    uiSettings_.addProperty(layoutDirection_);
    uiSettings_.addProperty(layoutSpacing_);
    uiSettings_.addProperty(layoutMargins_);

    addProperty(uiSettings_);
    addProperty(dynamicProperties_);

    // initialize color states
    uiRenderer_.setUIColor(uiColor_.get());
    uiRenderer_.setSecondaryUIColor(uiSecondaryColor_.get());
    uiRenderer_.setBorderColor(uiBorderColor_.get());
    uiRenderer_.setTextColor(uiTextColor_.get());
    uiRenderer_.setHoverColor(hoverColor_.get());
}

void GLUIProcessor::process() {
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
        layout_.setScalingFactor(uiScaling_.get());
    }

    // layout
    if (layoutDirection_.isModified()) {
        layout_.setDirection(layoutDirection_.get());
    }
    if (layoutSpacing_.isModified()) {
        layout_.setSpacing(layoutSpacing_.get());
    }
    if (layoutMargins_.isModified()) {
        const ivec4& m(layoutMargins_.get());
        layout_.setMargins(m.x, m.y, m.z, m.w);
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

        // TODO: make position/alignment accessible via properties
        {
            // put UI elements in lower left corner of the canvas
            const ivec2 extent(layout_.getExtent());
            ivec2 origin(0, extent.y);

            layout_.render(origin, outport_.getDimensions());
        }
    }

    utilgl::deactivateCurrentTarget();
}

void GLUIProcessor::onWillAddProperty(Property*, size_t) {}

void GLUIProcessor::onDidAddProperty(Property* property, size_t) {
    auto& factory = InviwoApplication::getPtr()
                        ->getModuleByType<UserInterfaceGLModule>()
                        ->getGLUIWidgetFactory();

    auto widget = factory.create(property->getClassIdentifier(), *property, *this, uiRenderer_);
    layout_.addElement(*widget.get());
    propertyWidgetMap_.insert(std::make_pair(property, std::move(widget)));

    invalidate(InvalidationLevel::InvalidOutput);
}

void GLUIProcessor::onWillRemoveProperty(Property* property, size_t) {
    // remove matching widget from map and the layout
    auto it = propertyWidgetMap_.find(property);
    if (it != propertyWidgetMap_.end()) {
        layout_.removeElement(*(it->second.get()));
        propertyWidgetMap_.erase(it);

        invalidate(InvalidationLevel::InvalidOutput);
    }
}

void GLUIProcessor::onDidRemoveProperty(Property*, size_t) {}

}  // namespace glui

}  // namespace inviwo
