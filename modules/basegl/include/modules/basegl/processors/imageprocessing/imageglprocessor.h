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

#include <inviwo/core/datastructures/image/imagetypes.h>  // for SwizzleMask
#include <inviwo/core/ports/imageport.h>                  // for ImageInport, ImageOutport
#include <inviwo/core/processors/processor.h>             // for Processor
#include <inviwo/core/util/glmvec.h>                      // for size2_t
#include <modules/opengl/shader/shader.h>                 // for Shader

#include <memory>  // for shared_ptr
#include <string>  // for string

namespace inviwo {
class DataFormatBase;
class ShaderResource;
class TextureUnitContainer;

/*! \class ImageGLProcessor
 *
 * \brief Base class for image processing on the GPU using OpenGL.
 *
 * The ImageGLProcessor provides the basic structure for image processing on the GPU.
 * Derived shaders have to provide a custom fragment shader which is used during rendering.
 * Optionally, derived classes can overwrite ImageGLProcessor::postProcess() to perform
 * post-processing of the image data set in the outport. Furthermore, it is possible to
 * be notified of changes in the input image by overwriting ImageGLProcessor::afterInportChanged().
 *
 * \see VolumeGLProcessor
 */
class IVW_MODULE_BASEGL_API ImageGLProcessor : public Processor {
public:
    ImageGLProcessor(std::shared_ptr<const ShaderResource> fragmentShader, bool buildShader = true);
    ImageGLProcessor(std::string_view fragmentShader, bool buildShader = true);
    virtual ~ImageGLProcessor();

    virtual void initializeResources() override;

    virtual void process() override;

protected:
    void markInvalid();

    /*! \brief this function gets called right before the actual processing but
     *         after the shader has been activated
     *
     * overwrite this function in the derived class to perform things like custom shader setup
     */
    virtual void preProcess(TextureUnitContainer& cont);

    /*! \brief this function gets called at the end of the process function
     *
     * overwrite this function in the derived class to perform post-processing
     */
    virtual void postProcess();

    /*! \brief this function gets called whenever the inport changes
     *
     * overwrite this function in the derived class to be notified of inport onChange events
     */
    virtual void afterInportChanged();

    size2_t calcOutputDimensions() const;
    const DataFormatBase* getDestinationDataFormat() const;
    SwizzleMask getDestinationSwizzleMask() const;

    ImageInport inport_;
    ImageOutport outport_;

    const DataFormatBase* dataFormat_;
    // if a custom data format is specified, i.e. dataFormat_ != nullptr, this swizzle mask is used
    SwizzleMask swizzleMask_;

    bool internalInvalid_;

    Shader shader_;
};

}  // namespace inviwo
