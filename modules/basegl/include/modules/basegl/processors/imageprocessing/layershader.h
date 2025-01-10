/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2023-2025 Inviwo Foundation
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

#include <modules/basegl/baseglmoduledefine.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/ports/layerport.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/properties/stringsproperty.h>
#include <inviwo/core/properties/boolcompositeproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>
#include <modules/base/properties/datarangeproperty.h>
#include <inviwo/core/util/formats.h>
#include <modules/opengl/buffer/framebufferobject.h>
#include <modules/opengl/shader/shader.h>
#include <modules/basegl/processors/layerprocessing/layerglprocessor.h>

namespace inviwo {

class StringShaderResource;

class IVW_MODULE_BASEGL_API LayerShader : public LayerGLProcessor {
public:
    LayerShader();

    virtual const ProcessorInfo& getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    virtual void preProcess(TextureUnitContainer& cont, const Layer& input, Layer& output) override;
    virtual LayerConfig outputConfig(const Layer& input) const override;

private:
    LayerShader(std::shared_ptr<StringShaderResource> fragmentShader);
    std::shared_ptr<StringShaderResource> fragmentShader_;
    StringProperty fragmentShaderSource_;

    StringProperty inputFormat_;
    OptionProperty<DataFormatId> format_;
    OptionPropertyInt channels_;

    struct RangeOpts {
        CompositeProperty comp;
        DoubleMinMaxProperty input;
        OptionPropertyInt mode;
        DoubleMinMaxProperty output;
    };

    RangeOpts dataRange_;
    RangeOpts valueRange_;

    struct AxisOpts {
        CompositeProperty comp;
        StringsProperty<2> input;
        OptionPropertyInt mode;
        StringsProperty<2> output;
    };

    AxisOpts valueAxis_;
};

}  // namespace inviwo
