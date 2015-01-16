/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
 *
 * Copyright (c) 2015 Inviwo Foundation
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

#ifndef IVW_IMAGECOMPOSITEPROCESSORGL_H
#define IVW_IMAGECOMPOSITEPROCESSORGL_H

#include <modules/basegl/baseglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <modules/opengl/image/compositeprocessorgl.h>
#include <inviwo/core/ports/imageport.h>

namespace inviwo {

/** \docpage{"org.inviwo.ImageCompositeProcessorGL, ImageCompositeProcessorGL}
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


/**
 * \class ImageCompositeProcessorGL
 *
 * \brief <brief description> 
 *
 * <Detailed description from a developer prespective>
 */
class IVW_MODULE_BASEGL_API ImageCompositeProcessorGL : public CompositeProcessorGL { 
public:
	InviwoProcessorInfo();
    ImageCompositeProcessorGL();
    virtual ~ImageCompositeProcessorGL(){}
	 
protected:
    virtual void process();

private:
    ImageInport imageInport1_;
    ImageInport imageInport2_;
    ImageOutport outport_;
};

} // namespace

#endif // IVW_IMAGECOMPOSITEPROCESSORGL_H

