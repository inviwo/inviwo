/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024 Inviwo Foundation
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
#include <inviwo/core/ports/layerport.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/buffer/framebufferobject.h>

#include <optional>

namespace inviwo {

class Layer;
class DataFormatBase;
class ShaderResource;
class TextureUnitContainer;

/**
 * @brief Base class for layer processing on the GPU using OpenGL.
 *
 * The LayerGLProcessor provides the basic structure for layer processing on the GPU.
 * Derived processors have to provide a custom fragment shader which is used during rendering.
 * Optionally, derived classes can overwrite LayerGLProcessor::preProcess() and
 * LayerGLProcessor::postProcess() to perform pre- and post-processing of the Layer directly before
 * and after rendering. Furthermore, it is possible to specify the data format of the output
 * Layer by overriding LayerGLProcessor::outputConfig().
 *
 * @see ImageGLProcessor
 */
class IVW_MODULE_BASEGL_API LayerGLProcessor : public Processor {
public:
    LayerGLProcessor(std::shared_ptr<const ShaderResource> fragmentShader);
    LayerGLProcessor(Shader shader);

    virtual void initializeResources() override;

    virtual void process() override;

protected:
    /** @brief this function gets called right before the actual processing but
     *         after the shader has been activated
     *
     * Overwrite this function in the derived class to perform things like custom shader setup.
     */
    virtual void preProcess(TextureUnitContainer& cont, const Layer& input, Layer& output);

    /** @brief this function gets called at the end of the process function
     *
     * Overwrite this function in the derived class to perform post-processing.
     */
    virtual void postProcess(const Layer& input, Layer& output);

    /** @brief Returns the output configuration for the layer.
     *
     * By default the output layer will have the same config (@see LayerConfig) as the input layer.
     * This function should be overridden in derived classes to provide the specific configuration
     * for the output layer.
     *
     * @param input the input data given
     * @return The output configuration for the layer.
     */
    virtual LayerConfig outputConfig([[maybe_unused]] const Layer& input) const;

    LayerInport inport_;
    LayerOutport outport_;
    Shader shader_;

private:
    LayerConfig config;
    std::vector<std::pair<FrameBufferObject, std::shared_ptr<Layer>>> cache_;
};

}  // namespace inviwo
