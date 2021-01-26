/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021 Inviwo Foundation
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
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/ports/volumeport.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/buffer/framebufferobject.h>
#include <modules/opengl/shader/shaderresource.h>

namespace inviwo {

/** \docpage{org.inviwo.VolumeRegionShrink, Volume Region Shrink}
 * ![](org.inviwo.VolumeRegionShrink.png?classIdentifier=org.inviwo.VolumeRegionShrink)
 * Shink all regions in the volume
 *
 * ### Inports
 *   * __inputVolume__ Input volume
 *
 * ### Outports
 *   * __outputVolume__ Output volume
 *
 * ### Properties
 *   * __iterations__ How many iterations to use
 */
class IVW_MODULE_BASEGL_API VolumeRegionShrink : public Processor {
public:
    VolumeRegionShrink();
    virtual ~VolumeRegionShrink() = default;

    virtual void process() override;

    virtual void initializeResources() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    VolumeInport inport_;
    VolumeOutport outport_;
    IntProperty iterations_;

    std::shared_ptr<StringShaderResource> fragShader_;
    Shader shader_;
    
    std::array<std::shared_ptr<Volume>,2> out_;
    std::array<FrameBufferObject, 2> fbo_;
};

}  // namespace inviwo
