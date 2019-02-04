/*********************************************************************************
*
* Inviwo - Interactive Visualization Workshop
*
* Copyright (c) 2015-2018 Inviwo Foundation
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

#include "imagetransform.h"
#include <memory>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/buffer/buffergl.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/image/imagegl.h>

namespace inviwo {

    // The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
    const ProcessorInfo ImageTransform::processorInfo_{
        "org.inviwo.ImageTransform", // Class identifier
        "Image Transform",           // Display name
        "Image Operation",           // Category
        CodeState::Experimental,     // Code state
        Tags::GL,                    // Tags
    };
    const ProcessorInfo ImageTransform::getProcessorInfo() const {
        return processorInfo_;
    }

    ImageTransform::ImageTransform() : Processor()
        , inport_("inport_")
        , outport_("outport_", DataVec4UInt8::get())
        , scaling_("scaling_", "Scale Factor", 1.0f, 0.0f, 10.0f, 0.01f)
        , rotation_("rotation_", "Rotation Angle", 0.0f, 0.0f, glm::two_pi<float>(), 1e-3f)
        , translation_("translation_", "Offset", vec2(0.0f), vec2(-2.0f), vec2(2.0f), vec2(1e-3f))
        , flipX_("flipX_", "Flip X", false)
        , flipY_("flipY_", "Flip Y", false)
        , shader_("img_transform.frag")
    {
        addProperty(scaling_);
        addProperty(rotation_);
        addProperty(translation_);
        addProperty(flipX_);
        addProperty(flipY_);

        addPort(inport_);
        addPort(outport_);

        shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
    }

    mat3 rotation2d(float angle)
    {
        return mat3{cos(angle), -sin(angle), 0.0f, sin(angle), cos(angle), 0.0f, 0.0f, 0.0f, 1.0f};
    }

    mat3 scaling2d(float scaling_factor)
    {
        return mat3{scaling_factor, 0.0f, 0.0f, 0.0f, scaling_factor, 0.0f, 0.0f, 0.0f, 1.0f};
    }

    mat3 translation2d(vec2 offset)
    {
        return glm::transpose(mat3{1.0f, 0.0f, offset.x, 0.0f, 1.0f, offset.y, 0.0f, 0.0f, 1.0f});
    }

    mat3 flipX2d()
    {
        return mat3{-1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    }

    mat3 flipY2d()
    {
        return mat3{1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    }

    void ImageTransform::process() {

        /*const auto S = scaling2d(1.0f / scaling_.get());
        const auto R = rotation2d(-rotation_.get());
        const auto T = translation2d(-translation_.get());
        const auto Fx = flipX_.get() ? flipX2d() : mat3(1.0f);
        const auto Fy = flipY_.get() ? flipY2d() : mat3(1.0f);
        const auto transformation = translation2d(vec2{0.5f}) * R * S * translation2d(vec2{-0.5f});*/

        utilgl::activateAndClearTarget(outport_, ImageType::ColorOnly);
        shader_.activate();

        const auto img_size = vec2(inport_.getData()->getDimensions());

        TextureUnit imgUnit;
        utilgl::bindColorTexture(inport_, imgUnit);
        shader_.setUniform("img", imgUnit);
        shader_.setUniform("scale", scaling_.get());
        shader_.setUniform("angle", rotation_.get());
        shader_.setUniform("offset", translation_.get());
        shader_.setUniform("img_size", img_size);
        shader_.setUniform("flip_x", flipX_ ? 1 : 0);
        shader_.setUniform("flip_y", flipY_ ? 1 : 0);

        utilgl::singleDrawImagePlaneRect();

        shader_.deactivate();
        utilgl::deactivateCurrentTarget();
    }

} // namespace
