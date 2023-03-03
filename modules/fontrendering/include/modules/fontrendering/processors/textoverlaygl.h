/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2023 Inviwo Foundation
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
    virtual std::string getClassIdentifier() const override;
    static const std::string classIdentifier;

    TextOverlayProperty(std::string_view identifier, std::string_view displayName,
                        InvalidationLevel = InvalidationLevel::InvalidResources,
                        PropertySemantics semantics = PropertySemantics::Default);
    TextOverlayProperty(const TextOverlayProperty& rhs);
    virtual TextOverlayProperty* clone() const override;

    StringProperty text;
    FloatVec2Property position;
    IntVec2Property offset;
};

/** \docpage{org.inviwo.TextOverlayGL, Text Overlay}
 * ![](org.inviwo.TextOverlayGL.png?classIdentifier=org.inviwo.TextOverlayGL)
 *
 * Overlay text onto an image. The text can contain place markers indicated by '{}'.
 * These markers will be replaced with the contents of the corresponding \p Arguments properties.
 * The placemarkers uses standard fmt syntax and can either be numbered {0}, {1}, or
 * named {arg0} {arg1}.
 *
 * ### Inports
 *   * __Inport__ Input image (optional)
 *
 * ### Outports
 *   * __Outport__ Output image with overlayed text
 *
 * ### Properties
 *   * __Texts__ List of text items to overlay.
 *      * __Text__  The text with possible formatting
 *      * __Position__ Where to put the text, relative position from 0 to 1
 *      * __Offset__ Pixel offset for the text
 *   * __Arguments__ List of String, Int and Double Properties to get inserter at the markers
 *     indicated in the text strings
 *   * __Font__ Text font options
 *      * __Font Face__ Font face
 *      * __Font Size__ Size
 *      * __Line Spacing__ Line spacing ot the text
 *      * __Anchor__ What point of the text to put at "Position". Relative from -1,1. 0 means the
 *        text is centered on "Position".
 *      * __Color__ Foreground color of the text
 */

class IVW_MODULE_FONTRENDERING_API TextOverlayGL : public Processor {
public:
    TextOverlayGL();
    virtual ~TextOverlayGL() = default;

    virtual const ProcessorInfo getProcessorInfo() const override;
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
