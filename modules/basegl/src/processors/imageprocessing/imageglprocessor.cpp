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

#include <modules/basegl/processors/imageprocessing/imageglprocessor.h>

#include <inviwo/core/datastructures/image/image.h>       // for Image
#include <inviwo/core/datastructures/image/imagetypes.h>  // for SwizzleMask, ImageType, ImageTy...
#include <inviwo/core/datastructures/image/layer.h>       // for Layer
#include <inviwo/core/ports/imageport.h>                  // for ImageOutport, ImageInport, Base...
#include <inviwo/core/processors/processor.h>             // for Processor
#include <inviwo/core/properties/invalidationlevel.h>     // for InvalidationLevel, Invalidation...
#include <inviwo/core/util/glmvec.h>                      // for size2_t
#include <modules/opengl/shader/shader.h>                 // for Shader, Shader::Build
#include <modules/opengl/shader/shadertype.h>             // for ShaderType, ShaderType::Fragment
#include <modules/opengl/shader/shaderutils.h>            // for ImageInport, findShaderResource
#include <modules/opengl/shader/standardshaders.h>        // for imgQuadVert
#include <modules/opengl/texture/textureunit.h>           // for TextureUnitContainer, TextureUnit
#include <modules/opengl/texture/textureutils.h>          // for activateTargetAndCopySource

#include <array>        // for operator!=, array
#include <functional>   // for __base
#include <string_view>  // for string_view
#include <type_traits>  // for remove_extent_t
#include <utility>      // for pair, move

#include <glm/vec2.hpp>  // for operator!=

namespace inviwo {
class DataFormatBase;
class ShaderResource;

namespace detail {

void syncMetaDataAndColorLayerState(ImageInport& inport, Image& dstImage,
                                    std::optional<SwizzleMask> swizzleMask = std::nullopt) {
    const auto srcImage = inport.getData();
    const auto srcColorLayer = srcImage->getColorLayer();
    auto dstColorLayer = dstImage.getColorLayer();

    dstImage.copyMetaDataFrom(*srcImage);

    dstColorLayer->setSwizzleMask(swizzleMask.value_or(srcColorLayer->getSwizzleMask()));
    dstColorLayer->setWrapping(inport.getData()->getColorLayer()->getWrapping());
    dstColorLayer->setInterpolation(inport.getData()->getColorLayer()->getInterpolation());
}

bool equalColorLayerState(ImageInport& inport, ImageOutport& outport,
                          std::optional<SwizzleMask> swizzleMask = std::nullopt) {
    const auto source = inport.getData()->getColorLayer();
    auto dest = outport.getData()->getColorLayer();

    return swizzleMask.value_or(source->getSwizzleMask()) == dest->getSwizzleMask() &&
           source->getWrapping() == dest->getWrapping() &&
           source->getInterpolation() == dest->getInterpolation();
}

bool needsNewImage(ImageInport& inport, ImageOutport& outport, const size2_t& dstDim,
                   const DataFormatBase* dstDataFormat,
                   std::optional<SwizzleMask> swizzleMask = std::nullopt) {

    return dstDataFormat != outport.getData()->getDataFormat() ||
           dstDim != outport.getData()->getDimensions() ||
           !equalColorLayerState(inport, outport, swizzleMask);
}

}  // namespace detail

ImageGLProcessor::ImageGLProcessor(std::string_view fragmentShader, bool buildShader)
    : ImageGLProcessor(utilgl::findShaderResource(fragmentShader), buildShader) {}

ImageGLProcessor::ImageGLProcessor(std::shared_ptr<const ShaderResource> fragmentShader,
                                   bool buildShader)
    : Processor()
    , inport_("inputImage", "The input image"_help, OutportDeterminesSize::Yes)
    , outport_("outputImage", false)
    , dataFormat_(nullptr)
    , swizzleMask_(swizzlemasks::rgba)
    , internalInvalid_(false)
    , shader_({utilgl::imgQuadVert(), {ShaderType::Fragment, fragmentShader}},
              buildShader ? Shader::Build::Yes : Shader::Build::No) {

    addPort(inport_);
    addPort(outport_);
    outport_.setHelp("The output image"_help);

    inport_.onChange([this]() {
        markInvalid();
        afterInportChanged();
    });

    shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
}

ImageGLProcessor::~ImageGLProcessor() = default;

void ImageGLProcessor::initializeResources() {
    shader_.build();
    internalInvalid_ = true;
}

void ImageGLProcessor::process() {
    if (internalInvalid_) {
        internalInvalid_ = false;

        const size2_t dim(calcOutputDimensions());
        auto dataformat = getDestinationDataFormat();
        auto swizzlemask = getDestinationSwizzleMask();

        if (!outport_.hasEditableData() ||
            detail::needsNewImage(inport_, outport_, dim, dataformat, swizzlemask)) {
            auto img = std::make_shared<Image>(dim, dataformat);
            detail::syncMetaDataAndColorLayerState(inport_, *img, swizzlemask);

            outport_.setData(img);
        }
    }

    utilgl::activateTargetAndCopySource(outport_, inport_, ImageType::ColorOnly);
    shader_.activate();

    utilgl::setShaderUniforms(shader_, outport_, "outportParameters");

    // bind input image
    TextureUnitContainer cont;
    TextureUnit imgUnit;
    utilgl::bindColorTexture(inport_, imgUnit);
    shader_.setUniform("inport", imgUnit);
    cont.push_back(std::move(imgUnit));

    // trigger preprocessing
    preProcess(cont);

    utilgl::singleDrawImagePlaneRect();
    shader_.deactivate();
    utilgl::deactivateCurrentTarget();

    postProcess();
}

void ImageGLProcessor::markInvalid() { internalInvalid_ = true; }

void ImageGLProcessor::preProcess(TextureUnitContainer&) {}

void ImageGLProcessor::postProcess() {}

void ImageGLProcessor::afterInportChanged() {}

size2_t ImageGLProcessor::calcOutputDimensions() const {
    size2_t dimensions;
    if (outport_.isHandlingResizeEvents() || !inport_.isOutportDeterminingSize()) {
        dimensions = outport_.getData()->getDimensions();
    } else {
        dimensions = inport_.getData()->getDimensions();
    }
    return dimensions;
}

const DataFormatBase* ImageGLProcessor::getDestinationDataFormat() const {
    if (dataFormat_) {
        return dataFormat_;
    } else if (inport_.hasData()) {
        return inport_.getData()->getDataFormat();
    }
    return DataVec4UInt8::get();
}

SwizzleMask ImageGLProcessor::getDestinationSwizzleMask() const {
    if (dataFormat_) {
        return swizzleMask_;
    } else if (inport_.hasData()) {
        return inport_.getData()->getColorLayer()->getSwizzleMask();
    }
    return swizzlemasks::rgba;
}

}  // namespace inviwo
