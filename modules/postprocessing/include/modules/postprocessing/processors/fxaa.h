/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2023 Inviwo Foundation
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

#include <modules/postprocessing/postprocessingmoduledefine.h>  // for IVW_MODULE_POSTPROCESSING...

#include <inviwo/core/ports/imageport.h>             // for ImageInport, ImageOutport
#include <inviwo/core/processors/processor.h>        // for Processor
#include <inviwo/core/processors/processorinfo.h>    // for ProcessorInfo
#include <inviwo/core/properties/boolproperty.h>     // for BoolProperty
#include <inviwo/core/properties/optionproperty.h>   // for OptionPropertyInt
#include <inviwo/core/properties/ordinalproperty.h>  // for FloatProperty
#include <modules/opengl/inviwoopengl.h>             // for GLuint
#include <modules/opengl/shader/shader.h>            // for Shader

namespace inviwo {

/** \docpage{org.inviwo.FXAA, FXAA}
 * ![](org.inviwo.FXAA.png?classIdentifier=org.inviwo.FXAA)
 * Applies Fast approximate anti-aliasing (FXAA) as a postprocessing operation
 *
 *
 * ### Inports
 *   * __ImageInport__ Input image.
 *
 * ### Outports
 *   * __ImageOutport__ Output image.
 *
 * ### Properties
 *   * __Dither__ Sets amount of dithering.
 *   * __Quality__ Sets the quality (number of samples) used. Performance vs. Quality
 */

/**
 * \class FXAA
 * \brief Anti-aliasing post process
 */
class IVW_MODULE_POSTPROCESSING_API FXAA : public Processor {
public:
    FXAA();
    virtual ~FXAA();

    virtual void initializeResources() override;
    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    void initFramebuffer(int width, int height);

    ImageInport inport_;
    ImageOutport outport_;

    BoolProperty enable_;

    OptionPropertyInt dither_;
    FloatProperty quality_;

    Shader fxaa_;
    Shader prepass_;

    struct {
        GLuint fbo = 0;
        GLuint tex = 0;
        int width = 0;
        int height = 0;
    } prepassFbo_;
};

}  // namespace inviwo
