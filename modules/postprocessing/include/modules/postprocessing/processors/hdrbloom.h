/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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

#ifndef IVW_HDR_BLOOM_H
#define IVW_HDR_BLOOM_H

#include <modules/postprocessing/postprocessingmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/ports/imageport.h>
#include <modules/opengl/inviwoopengl.h>
#include <modules/opengl/shader/shader.h>

#include <modules/opengl/texture/texture2d.h>
#include <modules/opengl/buffer/framebufferobject.h>

namespace inviwo {

/** \docpage{org.inviwo.HdrBloom, HdrBloom}
 * ![](org.inviwo.HdrBloom.png?classIdentifier=org.inviwo.HdrBloom)
 * Applies blooming for high intensities within HDR images
 *
 * ### Inports
 *   * __ImageInport__ Input image.
 *
 * ### Outports
 *   * __ImageOutport__ Output image.
 *
 * ### Properties
 *   * __Threshold__ All values above this will be blurred.
 *   * __Strength__ Sets the intensity of the bloom.
 *   * __Radius__ Sets the blur radius of the bloom.
 */

/**
 * \class HdrBloom
 * \brief Bloom filter with threshold
 */
class IVW_MODULE_POSTPROCESSING_API HdrBloom : public Processor {
public:
    HdrBloom();
    virtual ~HdrBloom();

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    void resizeTextures(int width, int height);

    ImageInport inport_;
    ImageOutport outport_;

    BoolProperty enable_;
    FloatProperty threshold_;
    FloatProperty strength_;
    FloatProperty radius_;

    Shader highPass_;
    Shader blur_;
    Shader compose_;

    int width_;
    int height_;

    // Sync in shader
    static constexpr int Levels = 5;

    FrameBufferObject fboHorizontal_[Levels];
    std::array<std::unique_ptr<Texture2D>, Levels> texHorizontal_;
    FrameBufferObject fboVertical_[Levels];
    std::array<std::unique_ptr<Texture2D>, Levels> texVertical_;
    FrameBufferObject fboBright_;
    Texture2D texBright_;
};

}  // namespace inviwo

#endif  // IVW_FXAA_H
