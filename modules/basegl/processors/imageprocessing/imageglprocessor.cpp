/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2017 Inviwo Foundation
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

#include "imageglprocessor.h"
#include <modules/opengl/image/imagegl.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/buffer/framebufferobject.h>

namespace inviwo {

ImageGLProcessor::ImageGLProcessor(std::string fragmentShader)
    : Processor()
    , inport_("inputImage")
    , outport_("outputImage")
    , dataFormat_(nullptr)
    , swizzleMask_(swizzlemasks::rgba)
    , internalInvalid_(false)
    , fragmentShader_(fragmentShader)
    , shader_(fragmentShader, false) {
    addPort(inport_);
    addPort(outport_);

    inport_.onChange(this, &ImageGLProcessor::inportChanged);
    inport_.setOutportDeterminesSize(true);
    outport_.setHandleResizeEvents(false);
    shader_.onReload([this](){invalidate(InvalidationLevel::InvalidResources);});
}

ImageGLProcessor::~ImageGLProcessor() {}

void ImageGLProcessor::initializeResources() {
    shader_.build();
    internalInvalid_ = true;
}

void ImageGLProcessor::process() {
    if (internalInvalid_) {
        internalInvalid_ = false;

        const size2_t dim(calcOutputDimensions());
        if (dataFormat_) {
            createCustomImage(dim, dataFormat_, swizzleMask_, inport_, outport_);
        } else {
            createDefaultImage(dim, inport_, outport_);
        }
    }

    utilgl::activateTargetAndCopySource(outport_, inport_, ImageType::ColorOnly);
    shader_.activate();

    utilgl::setShaderUniforms(shader_, outport_, "outportParameters_");

    // bind input image
    TextureUnitContainer cont;
    TextureUnit imgUnit;
    utilgl::bindColorTexture(inport_, imgUnit);
    shader_.setUniform("inport_", imgUnit);
    cont.push_back(std::move(imgUnit));
    
    // trigger preprocessing
    preProcess(cont);

    utilgl::singleDrawImagePlaneRect();
    shader_.deactivate();
    utilgl::deactivateCurrentTarget();

    postProcess();
}

void ImageGLProcessor::createCustomImage(const size2_t &dim, const DataFormatBase *dataFormat,
                                         const SwizzleMask &swizzleMask, ImageInport &inport, ImageOutport &outport) {

    if (!outport.hasEditableData() || dataFormat != outport.getData()->getDataFormat()
        || dim != outport.getData()->getDimensions()) {
        Image *img = new Image(dim, dataFormat);
        img->copyMetaDataFrom(*inport.getData());
        img->getColorLayer()->setSwizzleMask(swizzleMask);
        outport.setData(img);
    } else if (outport.hasEditableData() && outport.getData()->getColorLayer()->getSwizzleMask() != swizzleMask) {
        outport.getEditableData()->getColorLayer()->setSwizzleMask(swizzleMask);
    }
}

void ImageGLProcessor::createDefaultImage(const size2_t &dim, ImageInport &inport, ImageOutport &outport) {
    const DataFormatBase* format = inport.getData()->getDataFormat();

    const auto swizzleMask = inport.getData()->getColorLayer()->getSwizzleMask();

    if (!outport.hasEditableData() || format != outport.getData()->getDataFormat() ||
        dim != outport.getData()->getDimensions() ||
        swizzleMask != outport.getData()->getColorLayer()->getSwizzleMask()) {
        Image *img = new Image(dim, format);
        img->copyMetaDataFrom(*inport.getData());
        // forward swizzle mask of the input
        img->getColorLayer()->setSwizzleMask(swizzleMask);

        outport.setData(img);
    }
}

size2_t ImageGLProcessor::calcOutputDimensions() const {
    size2_t dimensions;
    if (outport_.isHandlingResizeEvents() || !inport_.isOutportDeterminingSize()) {
        dimensions = outport_.getData()->getDimensions();
    } else {
        dimensions = inport_.getData()->getDimensions();
    }
    return dimensions;
}

}  // namespace
