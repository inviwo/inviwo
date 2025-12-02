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

#include <inviwo/core/ports/volumeport.h>      // for VolumeInport, VolumeOutport
#include <inviwo/core/processors/processor.h>  // for Processor
#include <inviwo/core/properties/boolproperty.h>
#include <modules/base/datastructures/volumereusecache.h>
#include <modules/base/properties/datarangeproperty.h>
#include <modules/basegl/algorithm/dataminmaxgl.h>
#include <modules/opengl/buffer/framebufferobject.h>  // for FrameBufferObject
#include <modules/opengl/shader/shader.h>             // for Shader

#include <memory>  // for shared_ptr
#include <string>  // for string

namespace inviwo {
class DataFormatBase;
class ShaderResource;
class TextureUnitContainer;
class Volume;

/*! @class VolumeGLProcessor
 *
 * @brief Base class for volume processing on the GPU using OpenGL.
 *
 * The VolumeGLProcessor provides the basic structure for volume processing on the GPU.
 * Derived shaders have to provide a custom fragment shader which is used during rendering.
 * Optionally, derived classes can override:
 * * initializeShader
 * * preProcess
 * * postProcess
 */
class IVW_MODULE_BASEGL_API VolumeGLProcessor : public Processor {
public:
    enum class UseInport : bool { Yes, No };

    explicit VolumeGLProcessor(std::shared_ptr<const ShaderResource> fragmentShaderResource = {},
                               VolumeConfig config = {}, UseInport useInport = UseInport::Yes);
    explicit VolumeGLProcessor(std::string_view fragmentShaderName, VolumeConfig config = {},
                               UseInport useInport = UseInport::Yes);

    virtual ~VolumeGLProcessor();

    virtual void initializeResources() final;

    virtual void process() final;

    struct FBO {
        FrameBufferObject fbo;
        size_t textureId;
        std::array<GLenum, 1> drawBuffers{0};
        std::chrono::high_resolution_clock::time_point lastUsed;
    };

protected:
    /**
     * @brief this function gets called during resource initialization.
     *
     * Override this to add any shader defines etc. The shader will be built automatically.
     */
    virtual void initializeShader(Shader& shader);

    /**
     * @brief this function gets called right before the actual processing but
     * after the shader has been activated.
     *
     * Overwrite this function in the derived class to perform things like custom shader setup
     * and binding texture etc.
     * The VolumeConfig, is based on the config from the volume inport, updated with the Config from
     * the constructor. This config can be customized further and is used to create the output
     * volume.
     */
    virtual void preProcess(TextureUnitContainer& cont, Shader& shader, VolumeConfig& config);

    /**
     * @brief this function gets called at the end of the process function
     *
     * Overwrite this function in the derived class to perform post-processing
     */
    virtual void postProcess(Volume& volume);

    void setFragmentShaderResource(std::shared_ptr<const ShaderResource> fragShaderResource);
    std::shared_ptr<const ShaderResource> getFragmentShaderResource();

    std::optional<VolumeInport> inport_;
    VolumeOutport outport_;
    BoolProperty calculateDataRange_;
    DataRangeProperty dataRange_;

private:
    VolumeConfig config_;
    VolumeReuseCache volumes_;

    Shader shader_;
    std::array<FBO, 3> fbos_;

    std::optional<utilgl::DataMinMaxGL> dataMinMaxGL_;
};

}  // namespace inviwo
