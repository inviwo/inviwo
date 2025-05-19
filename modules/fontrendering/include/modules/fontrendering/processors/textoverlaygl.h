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

#pragma once

#include <modules/fontrendering/fontrenderingmoduledefine.h>  // for IVW_MODULE_FONTRENDERING_API

#include <inviwo/core/ports/imageport.h>                    // for ImageInport, ImageOutport
#include <inviwo/core/processors/processor.h>               // for Processor
#include <inviwo/core/processors/processorinfo.h>           // for ProcessorInfo
#include <inviwo/core/properties/boolproperty.h>            // for BoolProperty
#include <inviwo/core/properties/compositeproperty.h>       // for CompositeProperty
#include <inviwo/core/properties/invalidationlevel.h>       // for InvalidationLevel, Invalida...
#include <inviwo/core/properties/listproperty.h>            // for ListProperty
#include <inviwo/core/properties/ordinalproperty.h>         // for FloatVec2Property, FloatVec...
#include <inviwo/core/properties/propertysemantics.h>       // for PropertySemantics, Property...
#include <inviwo/core/properties/stringproperty.h>          // for StringProperty
#include <modules/fontrendering/properties/fontproperty.h>  // for FontProperty
#include <modules/fontrendering/textrenderer.h>             // for TextTextureObject, TextRend...
#include <modules/opengl/rendering/texturequadrenderer.h>   // for TextureQuadRenderer

#include <string>       // for string
#include <string_view>  // for string_view
#include <vector>       // for vector

namespace inviwo {

class IVW_MODULE_FONTRENDERING_API TextOverlayProperty : public CompositeProperty {
public:
    virtual std::string_view getClassIdentifier() const override;
    static constexpr std::string_view classIdentifier{"org.inviwo.TextOverlayProperty"};

    TextOverlayProperty(std::string_view identifier, std::string_view displayName,
                        InvalidationLevel = InvalidationLevel::InvalidResources,
                        PropertySemantics semantics = PropertySemantics::Default);
    TextOverlayProperty(const TextOverlayProperty& rhs);
    virtual TextOverlayProperty* clone() const override;

    StringProperty text;
    FloatVec2Property position;
    IntVec2Property offset;
};

class IVW_MODULE_FONTRENDERING_API TextOverlayGL : public Processor {
public:
    TextOverlayGL();
    virtual ~TextOverlayGL() = default;

    virtual const ProcessorInfo& getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    virtual void process() override;
    void updateCache();

private:
    ImageInport inport_;
    ImageOutport outport_;

    BoolProperty enable_;
    ListProperty texts_;

    FloatVec4Property color_;
    FontProperty font_;

    ListProperty args_;

    TextRenderer textRenderer_;
    std::vector<TextTextureObject> textObjects_;
    TextureQuadRenderer textureRenderer_;
};

}  // namespace inviwo
