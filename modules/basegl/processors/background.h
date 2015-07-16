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

#ifndef IVW_BACKGROUND_H
#define IVW_BACKGROUND_H

#include <modules/basegl/baseglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/ports/imageport.h>
#include <modules/opengl/inviwoopengl.h>
#include <modules/opengl/glwrap/shader.h>

namespace inviwo {

/** \docpage{org.inviwo.Background, Background}
 * ![](org.inviwo.Background.png?classIdentifier=org.inviwo.Background)
 * Adds a background to an image. 
 * The following mixing is applied
 * 
 *     out.rgb = in.rgb + color.rgb * color.a * (1.0 - in.a)
 *     out.a = in.a + color.a * (1.0 - in.a)
 * 
 * ### Inports
 *   * __ImageInport__ Input image.
 *
 * ### Outports
 *   * __ImageOutport__ Output image.
 * 
 * ### Properties
 *   * __Style__ The are three different styles to choose from Linear gradient, uniform color,
 *     or checker board.
 *   * __Color1__ Used as the uniform color and as color 1 in the gradient and checkerboard.
 *   * __Color2__ Used as color 2 the gradient and checkerboard.
 *   * __Checker Board Size__ The size of the rectangles in the checker board.
 *   * __Switch colors__ Button to switch color 1 and 2.
 */

/**
 * \brief Adds a background to an image.
 *
 */
class IVW_MODULE_BASEGL_API Background : public Processor {
public:
    InviwoProcessorInfo();
    
    Background();
    virtual ~Background();

    virtual void initializeResources();

protected:
    virtual void process();
    virtual bool isReady() const;

private:
    void switchColors();

    ImageInport inport_;
    ImageOutport outport_;

    OptionPropertyInt backgroundStyle_;
    FloatVec4Property color1_;
    FloatVec4Property color2_;
    IntVec2Property checkerBoardSize_;
    ButtonProperty switchColors_;
    Shader shader_;
    bool hadData_;
};

} // namespace

#endif // IVW_BACKGROUND_H
