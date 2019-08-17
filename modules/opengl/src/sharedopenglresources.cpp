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

#include <modules/opengl/sharedopenglresources.h>

#include <modules/opengl/geometry/meshgl.h>
#include <modules/opengl/texture/textureutils.h>

#include <modules/opengl/shader/shader.h>

namespace inviwo {

SharedOpenGLResources* SharedOpenGLResources::instance_ = nullptr;

const MeshGL* SharedOpenGLResources::imagePlaneRect() {
    if (!planeRectMesh_) {
        planeRectMesh_ = utilgl::planeRect();
        planeRectMeshGl_ = planeRectMesh_->getRepresentation<MeshGL>();
    }
    return planeRectMeshGl_;
}

Shader* SharedOpenGLResources::getTextureShader() {
    if (!textureShader_) {
        textureShader_ = std::make_unique<Shader>("img_texturequad.vert", "img_texturequad.frag");
    }
    return textureShader_.get();
}

Shader* SharedOpenGLResources::getNoiseShader() {
    if (!noiseShader_) {
        noiseShader_ = std::make_unique<Shader>("img_texturequad.vert", "img_noise.frag");
    }
    return noiseShader_.get();
}

Shader* SharedOpenGLResources::getImageCopyShader(size_t colorLayers) {
    auto& elem = imgCopyShaders_[colorLayers];
    if (!elem) {
        auto shader = std::make_unique<Shader>("standard.vert", "img_copy.frag", false);

        std::stringstream ssUniform;
        for (size_t i = 1; i < colorLayers; ++i) {
            ssUniform << "layout(location = " << i + 1 << ") out vec4 FragData" << i << ";";
        }
        for (size_t i = 1; i < colorLayers; ++i) {
            ssUniform << "uniform sampler2D color" << i << ";";
        }
        shader->getFragmentShaderObject()->addShaderDefine("ADDITIONAL_COLOR_LAYER_OUT_UNIFORMS",
                                                           ssUniform.str());

        std::stringstream ssWrite;
        for (size_t i = 1; i < colorLayers; ++i) {
            ssWrite << "FragData" << i << " = texture(color" << i << ", texCoord_.xy);";
        }
        shader->getFragmentShaderObject()->addShaderDefine("ADDITIONAL_COLOR_LAYER_WRITE",
                                                           ssWrite.str());

        if (colorLayers > 1) {
            shader->getFragmentShaderObject()->addShaderDefine("ADDITIONAL_COLOR_LAYERS");
        } else {
            shader->getFragmentShaderObject()->removeShaderDefine("ADDITIONAL_COLOR_LAYERS");
        }

        shader->build();

        elem = std::move(shader);
    }
    return elem.get();
}

void SharedOpenGLResources::reset() {
    planeRectMesh_ = nullptr;
    textureShader_ = nullptr;
    noiseShader_ = nullptr;
    imgCopyShaders_.clear();
}

}  // namespace inviwo
