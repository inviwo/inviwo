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

#include <modules/basegl/baseglmoduledefine.h>  // for IVW_MODULE_BASEG...

#include <inviwo/core/processors/processorinfo.h>                        // for ProcessorInfo
#include <inviwo/core/properties/optionproperty.h>                       // for OptionPropertyInt
#include <modules/basegl/processors/imageprocessing/imageglprocessor.h>  // for ImageGLProcessor

namespace inviwo {
class TextureUnitContainer;

namespace luminance {
enum class Model {
    PerceivedLum,  // Y = 0.299 R + 0.587 G + 0.114 B
    RelativeLum,   // Y = 0.2126 R + 0.7152 G + 0.0722 B
    AverageLum,    // Y = 0.3333 R + 0.3333 G + 0.3333 B
    RedOnly,       // Y = R
    GreenOnly,     // Y = G
    BlueOnly,      // Y = B
};

constexpr vec3 weights(Model model) {
    using enum Model;
    switch (model) {
        case PerceivedLum:
            return vec3(0.299f, 0.587f, 0.114f);
        case RelativeLum:
            return vec3(0.2126f, 0.7152f, 0.0722f);
        case AverageLum:
            return vec3(1.0f / 3.0f);
        case RedOnly:
            return vec3(1.0f, 0.0f, 0.0f);
        case GreenOnly:
            return vec3(0.0f, 1.0f, 0.0f);
        case BlueOnly:
            return vec3(0.0f, 0.0f, 1.0f);
    }
    return vec3(1.0f / 3.0f);
}

std::vector<OptionPropertyOption<Model>> options();

OptionPropertyState<Model> optionState();

}  // namespace luminance

/*! \class ImageGrayscale
 *
 * \brief Compute a gray-scale image from color input. Alpha channel is not touched.
 *
 * This processor computes the gray-scale image from a color image according to either
 * perceived luminance (Y = 0.299 R + 0.587 G + 0.114 B) or relative luminance for XYZ color
 * space (Y = 0.2126 R + 0.7152 G + 0.0722 B) utilizing the ImageGLProcessor.
 */
class IVW_MODULE_BASEGL_API ImageGrayscale : public ImageGLProcessor {
public:
    ImageGrayscale();
    virtual const ProcessorInfo& getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    virtual void preProcess(TextureUnitContainer& cont) override;

private:
    OptionProperty<luminance::Model> luminanceModel_;
};

}  // namespace inviwo
