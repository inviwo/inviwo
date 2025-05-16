/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2025 Inviwo Foundation
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

#include <modules/fontrendering/processors/textoverlaygl.h>

#include <inviwo/core/datastructures/image/imagetypes.h>              // for ImageType, ImageTyp...
#include <inviwo/core/ports/imageport.h>                              // for ImageInport, ImageO...
#include <inviwo/core/processors/processor.h>                         // for Processor
#include <inviwo/core/processors/processorinfo.h>                     // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                    // for CodeState, CodeStat...
#include <inviwo/core/processors/processortags.h>                     // for Tags
#include <inviwo/core/properties/boolproperty.h>                      // for BoolProperty
#include <inviwo/core/properties/compositeproperty.h>                 // for CompositeProperty
#include <inviwo/core/properties/invalidationlevel.h>                 // for InvalidationLevel
#include <inviwo/core/properties/listproperty.h>                      // for ListProperty, ListP...
#include <inviwo/core/properties/ordinalproperty.h>                   // for IntProperty, Double...
#include <inviwo/core/properties/property.h>                          // for Property
#include <inviwo/core/properties/propertysemantics.h>                 // for PropertySemantics
#include <inviwo/core/properties/stringproperty.h>                    // for StringProperty
#include <inviwo/core/util/glmvec.h>                                  // for vec2, ivec2, vec4
#include <inviwo/core/util/logcentral.h>                              // for LogCentral
#include <inviwo/core/util/rendercontext.h>                           // for RenderContext
#include <inviwo/core/util/stdextensions.h>                           // for any_of
#include <inviwo/core/util/zip.h>                                     // for zip, zipIterator
#include <modules/fontrendering/datastructures/textboundingbox.h>     // for TextBoundingBox
#include <modules/fontrendering/properties/fontfaceoptionproperty.h>  // for FontFaceOptionProperty
#include <modules/fontrendering/properties/fontproperty.h>            // for FontProperty
#include <modules/fontrendering/textrenderer.h>                       // for TextTextureObject
#include <modules/opengl/inviwoopengl.h>                              // for GL_ALWAYS, GL_ONE
#include <modules/opengl/openglutils.h>                               // for BlendModeState, Dep...
#include <modules/opengl/rendering/texturequadrenderer.h>             // for TextureQuadRenderer
#include <modules/opengl/texture/textureutils.h>                      // for activateAndClearTarget

#include <memory>  // for unique_ptr, make_un...

#include <flags/flags.h>  // for operator|
#include <fmt/core.h>     // for arg, format_context
#include <fmt/format.h>   // for format_error
#include <glm/vec2.hpp>   // for operator*, operator+

#if __has_include(<fmt/args.h>)  // New in fmt v8
#include <fmt/args.h>            // for dynamic_format_arg_...
#endif

namespace inviwo {

std::string_view TextOverlayProperty::getClassIdentifier() const { return classIdentifier; }

TextOverlayProperty::TextOverlayProperty(std::string_view identifier, std::string_view displayName,
                                         InvalidationLevel invalidationLevel,
                                         PropertySemantics semantics)
    : CompositeProperty(identifier, displayName, invalidationLevel, semantics)
    , text("text", "Text", "The text with possible formatting"_help, "Lorem ipsum etc.",
           InvalidationLevel::InvalidOutput, PropertySemantics::TextEditor)
    , position("position", "Position", "Where to put the text, relative position from 0 to 1"_help,
               vec2(0.0f), {vec2(-1.0f), ConstraintBehavior::Immutable},
               {vec2(2.0f), ConstraintBehavior::Immutable}, vec2(0.01f))
    , offset("offset", "Offset (Pixel)", "Pixel offset for the text"_help, ivec2(0),
             {ivec2(-100), ConstraintBehavior::Ignore}, {ivec2(100), ConstraintBehavior::Ignore}) {
    addProperties(text, position, offset);
}
TextOverlayProperty::TextOverlayProperty(const TextOverlayProperty& rhs)
    : CompositeProperty(rhs), text{rhs.text}, position{rhs.position}, offset{rhs.offset} {
    addProperties(text, position, offset);
}
TextOverlayProperty* TextOverlayProperty::clone() const { return new TextOverlayProperty(*this); }

const ProcessorInfo TextOverlayGL::processorInfo_{
    "org.inviwo.TextOverlayGL",  // Class identifier
    "Text Overlay",              // Display name
    "Drawing",                   // Category
    CodeState::Stable,           // Code state
    "GL, Font, Text",            // Tags
    R"(Overlay text onto an image. The text can contain place markers indicated by '{}'.
    These markers will be replaced with the contents of the corresponding  Arguments properties.
    The place markers uses standard fmt syntax and can either be numbered {0}, {1}, or
    named {arg0} {arg1}.)"_unindentHelp,
};
const ProcessorInfo& TextOverlayGL::getProcessorInfo() const { return processorInfo_; }

TextOverlayGL::TextOverlayGL()
    : Processor()
    , inport_("inport", "Input image (optional)"_help)
    , outport_("outport", "Output image with overlayed text"_help)
    , enable_("enable", "Enabled", true)
    , texts_{"texts",
             "Texts",
             "List of text items to overlay"_help,
             []() {
                 std::vector<std::unique_ptr<Property>> res;
                 res.emplace_back(std::make_unique<TextOverlayProperty>("text0", "Text 0"));
                 return res;
             }(),
             0,
             ListPropertyUIFlag::Add | ListPropertyUIFlag::Remove}
    , color_("color", "Color",
             util::ordinalColor(vec4(1.f)).set("Foreground color of the text"_help))
    , font_("font", "Font Settings")
    , args_{"args", "Arguments",
            []() {
                std::vector<std::unique_ptr<Property>> props;
                props.emplace_back(std::make_unique<StringProperty>("arg0", "String Arg 0"));
                props.emplace_back(std::make_unique<IntProperty>("arg0", "Int Arg 0"));
                props.emplace_back(std::make_unique<DoubleProperty>("arg0", "Double Arg 0"));
                return props;
            }(),
            0, ListPropertyUIFlag::Add | ListPropertyUIFlag::Remove}
    , textRenderer_{[]() {
        // ensure the default context is active when creating the TextRenderer
        RenderContext::getPtr()->activateDefaultRenderContext();
        return TextRenderer{};
    }()} {

    addPorts(inport_, outport_);
    addProperties(enable_, font_, texts_, args_);
    font_.addProperties(color_);
    inport_.setOptional(true);

    texts_.constructProperty(0);
    font_.fontFace_.setSelectedIdentifier("arial");
    font_.fontSize_.set(14);

    setAllPropertiesCurrentStateAsDefault();
}

void TextOverlayGL::process() {
    if (!enable_.get()) {
        outport_.setData(inport_.getData());
        return;
    }

    if (font_.fontFace_.isModified()) {
        textRenderer_.setFont(font_.fontFace_.get());
    }

    // check whether a property was modified
    if (textObjects_.size() != texts_.size() || texts_.isModified() || color_.isModified() ||
        font_.isModified() || util::any_of(args_.getProperties(), &Property::isModified)) {
        updateCache();
    }

    // draw cached overlay on top of the input image
    if (inport_.isReady()) {
        utilgl::activateTargetAndCopySource(outport_, inport_, ImageType::ColorDepth);
    } else {
        utilgl::activateAndClearTarget(outport_, ImageType::ColorDepth);
    }

    utilgl::DepthFuncState depthFunc(GL_ALWAYS);
    utilgl::BlendModeState blending(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    for (auto&& [item, tex] : util::zip(texts_, textObjects_)) {
        if (auto tp = dynamic_cast<const TextOverlayProperty*>(item)) {
            // use integer position for best results
            vec2 size(tex.bbox.textExtent);
            vec2 shift = 0.5f * size * (font_.anchorPos_.get() + vec2(1.0f, 1.0f));

            ivec2 pos(tp->position.get() * vec2(outport_.getDimensions()));
            pos += tp->offset.get() - ivec2(shift);

            // render texture containing the text onto the current canvas
            textureRenderer_.render(tex.texture, pos + tex.bbox.glyphsOrigin,
                                    outport_.getDimensions());
        }
    }

    utilgl::deactivateCurrentTarget();
}

void TextOverlayGL::updateCache() {
    textRenderer_.setFontSize(font_.fontSize_.get());
    textRenderer_.setLineSpacing(font_.lineSpacing_.get());

    auto store = fmt::dynamic_format_arg_store<fmt::format_context>();
    for (auto item : args_) {
        if (auto sp = dynamic_cast<const StringProperty*>(item)) {
            store.push_back(fmt::arg(sp->getIdentifier().c_str(), sp->get()));
        } else if (auto ip = dynamic_cast<const IntProperty*>(item)) {
            store.push_back(fmt::arg(ip->getIdentifier().c_str(), ip->get()));
        } else if (auto dp = dynamic_cast<const DoubleProperty*>(item)) {
            store.push_back(fmt::arg(dp->getIdentifier().c_str(), dp->get()));
        }
    }

    textObjects_.resize(texts_.size());
    for (auto&& [item, tex] : util::zip(texts_, textObjects_)) {
        if (auto tp = dynamic_cast<const TextOverlayProperty*>(item)) {
            std::string text;
            try {
                text = fmt::vformat(tp->text.get(), store);
            } catch (const fmt::format_error&) {
                log::warn(
                    "Invalid formatting string {}:'{}'\nFormat uses fmt syntax, args "
                    "can be named by index {{0}} or by name {{arg0}}",
                    tp->getPath(), tp->text.get());
                text = "<Invalid Format!>";
            }
            tex = util::createTextTextureObject(textRenderer_, text, color_.get(), tex.texture);
        }
    }
}

}  // namespace inviwo
