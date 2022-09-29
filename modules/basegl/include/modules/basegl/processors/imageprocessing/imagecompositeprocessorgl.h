/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2022 Inviwo Foundation
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

#include <modules/basegl/baseglmoduledefine.h>     // for IVW_MODULE_BASEGL_API

#include <inviwo/core/ports/imageport.h>           // for ImageInport, ImageOutport
#include <inviwo/core/processors/processor.h>      // for Processor
#include <inviwo/core/processors/processorinfo.h>  // for ProcessorInfo
#include <modules/opengl/image/imagecompositor.h>  // for ImageCompositor

namespace inviwo {

/** \docpage{org.inviwo.ImageCompositeProcessorGL, ImageCompositeProcessorGL}
 * ![](org.inviwo.ImageCompositeProcessorGL.png?classIdentifier=org.inviwo.ImageCompositeProcessorGL)
 * Do a depth based compositing of two images
 *
 * ### Inports
 *   * __imageInport1__ Image 1.
 *   * __imageInport2__ Image 2.
 *
 * ### Outports
 *   * __outport__ Output image.
 *
 */

class IVW_MODULE_BASEGL_API ImageCompositeProcessorGL : public Processor {
public:
    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;
    ImageCompositeProcessorGL();
    virtual ~ImageCompositeProcessorGL() = default;

protected:
    virtual void process() override;

private:
    ImageInport imageInport1_;
    ImageInport imageInport2_;
    ImageOutport outport_;
    ImageCompositor compositor_;
};

}  // namespace inviwo
