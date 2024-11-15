
/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2024 Inviwo Foundation
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

#include <inviwo/core/common/inviwoapplication.h>           // for InviwoApplication
#include <inviwo/core/ports/imageport.h>                    // for ImageInport, ImageOutport
#include <inviwo/core/processors/processor.h>               // for Processor
#include <inviwo/core/processors/processorinfo.h>           // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>          // for CodeState, CodeState::Stable
#include <inviwo/core/processors/processortags.h>           // for Tags
#include <inviwo/core/properties/boolproperty.h>            // for BoolProperty
#include <inviwo/core/properties/compositeproperty.h>       // for CompositeProperty
#include <inviwo/core/properties/invalidationlevel.h>       // for InvalidationLevel, Invalidati...
#include <inviwo/core/properties/listproperty.h>            // for ListProperty
#include <inviwo/core/properties/optionproperty.h>          // for OptionProperty, OptionPropert...
#include <inviwo/core/properties/ordinalproperty.h>         // for FloatVec4Property, FloatProperty
#include <inviwo/core/properties/property.h>                // for Property
#include <inviwo/core/properties/propertyfactory.h>         // for PropertyFactory
#include <inviwo/core/properties/propertyownerobserver.h>   // for PropertyOwnerObservable
#include <inviwo/core/properties/propertysemantics.h>       // for PropertySemantics, PropertySe...
#include <inviwo/core/util/glmvec.h>                        // for vec4, vec2, ivec2, ivec4, vec3
#include <inviwo/core/util/iterrange.h>                     // for iter_range
#include <inviwo/core/util/staticstring.h>                  // for operator+
#include <inviwo/core/util/stringconversion.h>              // for splitByLast
#include <inviwo/core/util/transformiterator.h>             // for TransformIterator
#include <modules/opengl/inviwoopengl.h>                    // for GL_ALWAYS, GL_ONE, GL_ONE_MIN...
#include <modules/opengl/openglutils.h>                     // for BlendModeState, DepthFuncState
#include <modules/opengl/texture/textureutils.h>            // for activateTargetAndClearOrCopyS...
#include <modules/userinterfacegl/glui/element.h>           // for Element
#include <modules/userinterfacegl/glui/layout/boxlayout.h>  // for BoxLayout::LayoutDirection
#include <modules/userinterfacegl/glui/renderer.h>          // for Renderer
#include <modules/userinterfacegl/glui/widgetfactory.h>     // for WidgetFactory
#include <modules/userinterfacegl/userinterfaceglmodule.h>  // for UserInterfaceGLModule

#include <cctype>   // for tolower
#include <utility>  // for pair, make_pair, move

#include <glm/vec2.hpp>  // for operator*, operator+, vec

namespace inviwo::glui {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo GLUIProcessor::processorInfo_{
    "org.inviwo.GLUIProcessor",                // Class identifier
    "GLUIProcessor",                           // Display name
    "UI",                                      // Category
    CodeState::Stable,                         // Code state
    Tags::GL | Tag{"UI"} | Tag{"Properties"},  // Tags
    R"(Provides a simple, adpative OpenGL user interface based on GLUI. Properties can be
       added to and removed from a list property.)"_unindentHelp,
};

const ProcessorInfo GLUIProcessor::getProcessorInfo() const { return processorInfo_; }

GLUIProcessor::GLUIProcessor(InviwoApplication* app)
    : Processor()
    , inport_("inport", "Input image"_help)
    , outport_("outport", "Input image with the UI rendered on top"_help)

    // settings properties for GLUI
    , uiSettings_("uiSettings", "UI Settings")
    , uiVisible_("uiVisibility", "UI Visible",
                 "UI visibility, i.e. whether the UI is rendered or not"_help, true)
    , positioning_("positioning", "Positioning")
    , position_("position", "Position", vec2(0.0f), vec2(0.0f), vec2(1.0f), vec2(0.01f))
    , anchorPos_("anchor", "Anchor",
                 util::ordinalSymmetricVector(vec2{-1.0f}, vec2{1.0f})
                     .set("Relative alignments of labels"_help))
    , offset_("offset", "Offset (Pixel)", ivec2(0), ivec2(-100), ivec2(100))
    , uiScaling_("uiScaling", "UI Scaling", util::ordinalScale(1.0f, 4.0f))
    , uiColor_("uiColor", "UI Color",
               util::ordinalColor(vec4{0.51f, 0.64f, 0.91f, 1.0f}).set("Main color of the UI"_help))
    , uiSecondaryColor_(
          "uiSecondaryColor", "UI Secondary Color",
          util::ordinalColor(vec4{0.4f, 0.4f, 0.45f, 1.0f}).set("Secondary color of the UI"_help))
    , uiBorderColor_("uiBorderColor", "UI Border Color",
                     util::ordinalColor(vec4{vec3{0.1f}, 1.0f})
                         .set("Color of the border around UI elements"_help))
    , uiDisabledColor_("uiDisabledColor", "UI Disabled Color",
                       util::ordinalColor(vec4{0.6f, 0.6f, 0.63f, 1.0f})
                           .set("Used for disabled/read-only elements"_help))
    , uiTextColor_("uiTextColor", "Text Color",
                   util::ordinalColor(vec4{vec3{0.0f}, 1.0f}).set("Color of the text labels"_help))
    , hoverColor_("hoverColor", "Hover Color",
                  util::ordinalColor(vec4{1.0f, 1.0f, 1.0f, 0.5f})
                      .set("Highlight color when hovering UI elements"_help))
    , layoutDirection_("layoutDirection", "Layout Direction",
                       {{"horizontal", "Horizontal", glui::BoxLayout::LayoutDirection::Horizontal},
                        {"vertical", "Vertical", glui::BoxLayout::LayoutDirection::Vertical}},
                       1)
    , layoutSpacing_("layoutSpacing", "Layout Spacing",
                     util::ordinalLength(5, 50).set("Spacing in between UI elements"_help))
    , layoutMargins_("layoutMargins", "Layout Margins",
                     util::ordinalLength(ivec4(10), ivec4(50))
                         .set("Margins applied to the layout (top, left, bottom, right)"_help))
    // list of dynamic properties
    , dynamicProperties_(
          "dynamicProperties", "Properties",
          [app]() {
              std::vector<std::unique_ptr<Property>> v;
              const auto& factory =
                  app->getModuleByType<UserInterfaceGLModule>()->getGLUIWidgetFactory();
              auto propertyFactory = app->getPropertyFactory();

              for (auto&& key : factory.getKeyView()) {
                  auto displayName = std::string{util::splitByLast(key, '.').second};
                  auto identifier = displayName;
                  identifier[0] = static_cast<char>(std::tolower(identifier[0]));
                  v.emplace_back(propertyFactory->create(key, identifier, displayName));
              }
              return v;
          }())
    , layout_(glui::BoxLayout::LayoutDirection::Vertical)
    , app_(app) {

    inport_.setOptional(true);

    addPorts(inport_, outport_);

    dynamicProperties_.PropertyOwnerObservable::addObserver(this);

    // UI specific settings
    positioning_.setCollapsed(true);
    positioning_.addProperties(position_, anchorPos_, offset_);

    uiSettings_.setCollapsed(true);
    uiSettings_.addProperties(uiVisible_, positioning_, uiScaling_, uiColor_, uiSecondaryColor_,
                              uiBorderColor_, uiTextColor_, hoverColor_, uiDisabledColor_,
                              layoutDirection_, layoutSpacing_, layoutMargins_);

    addProperties(uiSettings_, dynamicProperties_);

    // initialize color states
    uiRenderer_.setUIColor(uiColor_.get());
    uiRenderer_.setSecondaryUIColor(uiSecondaryColor_.get());
    uiRenderer_.setBorderColor(uiBorderColor_.get());
    uiRenderer_.setTextColor(uiTextColor_.get());
    uiRenderer_.setHoverColor(hoverColor_.get());
}

namespace {

template <typename P, typename T, typename Arg>
void sync(T& obj, const P& property, void (T::*setter)(Arg)) {
    if (property.isModified()) {
        (obj.*setter)(property.get());
    }
}
template <typename P, typename T, typename V>
void sync(T& obj, const P& property, V T::*member) {
    if (property.isModified()) {
        obj.*member = property.get();
    }
}

}  // namespace

void GLUIProcessor::process() {
    sync(uiRenderer_, uiColor_, &Renderer::setUIColor);
    sync(uiRenderer_, uiSecondaryColor_, &Renderer::setSecondaryUIColor);
    sync(uiRenderer_, uiBorderColor_, &Renderer::setBorderColor);
    sync(uiRenderer_, uiTextColor_, &Renderer::setTextColor);
    sync(uiRenderer_, hoverColor_, &Renderer::setHoverColor);
    sync(uiRenderer_, uiDisabledColor_, &Renderer::setDisabledColor);
    // layout
    sync(layout_, layoutDirection_, &BoxLayout::setDirection);
    sync(layout_, layoutSpacing_, &BoxLayout::setSpacing);
    sync(layout_, layoutMargins_,
         static_cast<void (BoxLayout::*)(const ivec4&)>(&BoxLayout::setMargins));
    // scaling will affect all glui elements, the change is propagated by the layout
    sync(layout_, uiScaling_, &BoxLayout::setScalingFactor);

    utilgl::activateTargetAndClearOrCopySource(outport_, inport_);
    utilgl::DepthFuncState depthFunc(GL_ALWAYS);
    utilgl::BlendModeState blending(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    if (uiVisible_.get()) {
        // coordinate system defined in screen coords with origin in the bottom-left corner
        // Note: the layout needs to be positioned with respect to its top-left corner

        const ivec2 extent(layout_.getExtent());
        // use integer position for best results
        vec2 shift = 0.5f * vec2(extent) * (anchorPos_.get() + vec2(1.0f));
        ivec2 origin(position_.get() * vec2(outport_.getDimensions()));
        origin += offset_.get() - ivec2(shift) + ivec2(0, extent.y);

        layout_.render(origin, outport_.getDimensions());
    }

    utilgl::deactivateCurrentTarget();
}

void GLUIProcessor::onWillAddProperty(PropertyOwner*, Property*, size_t) {}

void GLUIProcessor::onDidAddProperty(Property* property, size_t) {
    const auto& factory = app_->getModuleByType<UserInterfaceGLModule>()->getGLUIWidgetFactory();

    auto widget = factory.create(property->getClassIdentifier(), *property, *this, uiRenderer_);
    layout_.addElement(*widget.get());
    layout_.setScalingFactor(uiScaling_);
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

void GLUIProcessor::onDidRemoveProperty(PropertyOwner*, Property*, size_t) {}

}  // namespace inviwo::glui
