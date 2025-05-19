/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2025 Inviwo Foundation
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

#include <inviwo/core/ports/imageport.h>              // for ImageInport, ImageOutport
#include <inviwo/core/processors/processor.h>         // for Processor
#include <inviwo/core/processors/processorinfo.h>     // for ProcessorInfo
#include <inviwo/core/properties/boolproperty.h>      // for BoolProperty
#include <inviwo/core/properties/ordinalproperty.h>   // for FloatProperty
#include <inviwo/core/util/glmvec.h>                  // for size2_t
#include <modules/opengl/buffer/framebufferobject.h>  // for FrameBufferObject
#include <modules/opengl/shader/shader.h>             // for Shader
#include <modules/opengl/texture/texture2d.h>         // for Texture2D

#include <array>  // for array

namespace inviwo {

/**
 * @brief Bloom filter with threshold
 */
class IVW_MODULE_POSTPROCESSING_API HdrBloom : public Processor {
public:
    HdrBloom();
    virtual ~HdrBloom();

    virtual void process() override;

    virtual const ProcessorInfo& getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    void resizeTextures(size2_t size);

    ImageInport inport_;
    ImageOutport outport_;

    BoolProperty enable_;
    FloatProperty threshold_;
    FloatProperty strength_;
    FloatProperty radius_;

    Shader highPass_;
    Shader blur_;
    Shader compose_;

    size2_t size_;

    // Sync in shader
    static constexpr int levels_ = 5;
    struct FBOTex {
        FrameBufferObject fbo;
        Texture2D tex;
    };
    std::array<FBOTex, levels_> horizontal_;
    std::array<FBOTex, levels_> vertical_;
    FBOTex bright_;
};

}  // namespace inviwo
