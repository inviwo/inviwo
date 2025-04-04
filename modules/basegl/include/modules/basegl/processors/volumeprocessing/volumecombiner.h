/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2025 Inviwo Foundation
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

#include <modules/basegl/baseglmoduledefine.h>  // for IVW_MODULE_BASEGL_API

#include <inviwo/core/datastructures/volume/volume.h>      // for Volume
#include <inviwo/core/ports/datainport.h>                  // for DataInport
#include <inviwo/core/ports/outportiterable.h>             // for OutportIterable
#include <inviwo/core/ports/volumeport.h>                  // for VolumeOutport
#include <inviwo/core/processors/processor.h>              // for Processor
#include <inviwo/core/processors/processorinfo.h>          // for ProcessorInfo
#include <inviwo/core/properties/boolcompositeproperty.h>  // for BoolCompositeProperty
#include <inviwo/core/properties/boolproperty.h>           // for BoolProperty
#include <inviwo/core/properties/buttonproperty.h>         // for ButtonProperty
#include <inviwo/core/properties/compositeproperty.h>      // for CompositeProperty
#include <inviwo/core/properties/minmaxproperty.h>         // for DoubleMinMaxProperty
#include <inviwo/core/properties/optionproperty.h>         // for OptionProperty, OptionPropertyInt
#include <inviwo/core/properties/ordinalproperty.h>        // for FloatVec4Property
#include <inviwo/core/properties/stringproperty.h>         // for StringProperty
#include <inviwo/core/util/glmvec.h>                       // for uvec3
#include <inviwo/core/util/staticstring.h>                 // for operator+
#include <modules/opengl/buffer/framebufferobject.h>       // for FrameBufferObject
#include <modules/opengl/shader/shader.h>                  // for Shader

#include <functional>   // for __base
#include <memory>       // for shared_ptr
#include <string>       // for operator==, string
#include <string_view>  // for operator==
#include <vector>       // for operator!=, vector, operator==

#include <fmt/core.h>  // for format_to, basic_string_view

namespace inviwo {
class StringShaderResource;

class IVW_MODULE_BASEGL_API VolumeCombiner : public Processor {
public:
    virtual const ProcessorInfo& getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    VolumeCombiner();
    virtual ~VolumeCombiner() = default;

    virtual void process() override;

private:
    enum class NormalizationMode { Normalized, SignedNormalized, NotNormalized };

    std::string buildEquation() const;
    void buildShader(const std::string& eqn);
    void updateProperties();

    void updateDataRange();

    DataInport<Volume, 0> inport_;
    VolumeOutport outport_;
    std::shared_ptr<Volume> volume_;
    StringProperty description_;
    StringProperty eqn_;
    OptionProperty<NormalizationMode> normalizationMode_;
    CompositeProperty scales_;
    ButtonProperty addScale_;
    ButtonProperty removeScale_;

    BoolCompositeProperty useWorldSpace_;
    FloatVec4Property borderValue_;

    CompositeProperty dataRange_;
    OptionPropertyInt rangeMode_;
    DoubleMinMaxProperty outputDataRange_;
    DoubleMinMaxProperty outputValueRange_;
    BoolProperty customRange_;
    DoubleMinMaxProperty customDataRange_;
    DoubleMinMaxProperty customValueRange_;

    std::shared_ptr<StringShaderResource> fragment_;
    Shader shader_;
    FrameBufferObject fbo_;

    bool dirty_ = true;
    bool valid_ = true;
};

}  // namespace inviwo
