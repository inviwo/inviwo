/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#ifndef IVW_TEXTOVERLAYGL_H
#define IVW_TEXTOVERLAYGL_H

#include <modules/fontrendering/fontrenderingmoduledefine.h>
#include <modules/fontrendering/textrenderer.h>

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>


namespace inviwo {

/** \docpage{org.inviwo.TextOverlayGL, Text Overlay}
 * ![](org.inviwo.TextOverlayGL.png?classIdentifier=org.inviwo.TextOverlayGL)
 *
 * Overlay text onto an image. 
 * 
 * ### Inports
 *   * __Inport__ Input image
 *
 * ### Outports
 *   * __Outport__ Output image with overlayed text
 * 
 * ### Properties
 *   * __Text__ Text to overlay
 *   * __Font size__ Text size
 *   * __Position__ Where to put the text, relative position from 0 to 1
 *   * __Anchor__ What point of the text to put at "Position". relative from -1,1. 0 meas the image
 *     is centered on "Position".
 */

class IVW_MODULE_FONTRENDERING_API TextOverlayGL : public Processor {
public:
    TextOverlayGL();
    ~TextOverlayGL();

    InviwoProcessorInfo();

    void initialize();
    void deinitialize();
protected:
    virtual void process();

private:
    void initMesh();

    ImageInport inport_;
    ImageOutport outport_;
    StringProperty text_;
    
    FloatVec4Property color_;
    OptionPropertyInt fontSize_;
    FloatVec2Property fontPos_;
    FloatVec2Property anchorPos_;

    TextRenderer* textRenderer_;

};

} // namespace

#endif // IVW_TEXTOVERLAYGL_H