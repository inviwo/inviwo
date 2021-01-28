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

#include <modules/basegl/processors/volumeprocessing/volumeregionshrink.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/volume/volumegl.h>
#include <modules/opengl/openglutils.h>
#include <modules/opengl/shader/shadermanager.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo VolumeRegionShrink::processorInfo_{
    "org.inviwo.VolumeRegionShrink",  // Class identifier
    "Volume Region Shrink",           // Display name
    "Volume Operation",               // Category
    CodeState::Stable,                // Code state
    Tags::GL,                         // Tags
};
const ProcessorInfo VolumeRegionShrink::getProcessorInfo() const { return processorInfo_; }

namespace {

constexpr std::string_view fragStr = R"(
#include "utils/structs.glsl"
#include "utils/sampler3d.glsl"

uniform sampler3D volume;
uniform VolumeParameters volumeParameters;

in vec4 texCoord_;

void main() {
    vec4 value = getVoxel(volume, volumeParameters, texCoord_.xyz);

    bool border = false;
    for (int z = -1; z < 1; z++) {
        for (int y = -1; y < 1; y++) {
            for (int x = -1; x < 1; x++) {
                border = border || value != getVoxel(volume, volumeParameters, 
                                            texCoord_.xyz + vec3(x,y,z) * 
                                            volumeParameters.reciprocalDimensions); 
            }
        }
    }
    FragData0 = mix(value, vec4(0.0), bvec4(border));
}   
)";

}

VolumeRegionShrink::VolumeRegionShrink()
    : Processor()
    , inport_{"inputVolume"}
    , outport_{"outputVolume"}
    , iterations_{"iterations", "Iterations", 3, 0, 25}
    , fragShader_{std::make_shared<StringShaderResource>("VolumeRegionShrink.frag", fragStr)}
    , shader_({{ShaderType::Vertex, utilgl::findShaderResource("volume_gpu.vert")},
               {ShaderType::Geometry, utilgl::findShaderResource("volume_gpu.geom")},
               {ShaderType::Fragment, fragShader_}},
              Shader::Build::Yes) {

    addPort(inport_);
    addPort(outport_);

    addProperty(iterations_);

    shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
    ShaderManager::getPtr()->addShaderResource(fragShader_);
}

void VolumeRegionShrink::process() {
    auto volume = inport_.getData();

    if (iterations_ == 0) {
        outport_.setData(volume);
        return;
    }

    if (!out_[0] || out_[0]->getDataFormat() != volume->getDataFormat() ||
        out_[0]->getDimensions() != volume->getDimensions()) {
        out_[0] = std::shared_ptr<Volume>(volume->clone());
    }

    out_[0]->setModelMatrix(volume->getModelMatrix());
    out_[0]->setWorldMatrix(volume->getWorldMatrix());
    out_[0]->setSwizzleMask(volume->getSwizzleMask());
    out_[0]->setWrapping(volume->getWrapping());
    out_[0]->setInterpolation(volume->getInterpolation());
    out_[0]->copyMetaDataFrom(*volume);
    out_[0]->dataMap_ = volume->dataMap_;

    const size3_t dim{volume->getDimensions()};
    glViewport(0, 0, static_cast<GLsizei>(dim.x), static_cast<GLsizei>(dim.y));

    fbo_.activate();
    VolumeGL* outGL0 = out_[0]->getEditableRepresentation<VolumeGL>();
    fbo_.attachColorTexture(outGL0->getTexture().get(), 0);
    out_[0]->invalidateHistogram();

    utilgl::Activate as{&shader_};

    // Iteration 1
    {
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        TextureUnitContainer cont;
        utilgl::bindAndSetUniforms(shader_, cont, *volume, "volume");
        utilgl::multiDrawImagePlaneRect(static_cast<int>(dim.z));
    }
    if (iterations_ == 1) {
        FrameBufferObject::deactivateFBO();
        outport_.setData(out_[0]);
        out_[1].reset();
        return;
    }

    if (!out_[1] || out_[1]->getDataFormat() != volume->getDataFormat() ||
        out_[1]->getDimensions() != volume->getDimensions()) {
        out_[1] = std::shared_ptr<Volume>(volume->clone());
    }

    out_[1]->setModelMatrix(volume->getModelMatrix());
    out_[1]->setWorldMatrix(volume->getWorldMatrix());
    out_[1]->setSwizzleMask(volume->getSwizzleMask());
    out_[1]->setWrapping(volume->getWrapping());
    out_[1]->setInterpolation(volume->getInterpolation());
    out_[1]->copyMetaDataFrom(*volume);
    out_[1]->dataMap_ = volume->dataMap_;

    VolumeGL* outGL1 = out_[1]->getEditableRepresentation<VolumeGL>();
    fbo_.attachColorTexture(outGL1->getTexture().get(), 1);
    out_[1]->invalidateHistogram();

    size_t src = 1;
    size_t dst = 0;
    for (int i = 1; i < iterations_; ++i) {
        std::swap(src, dst);
        glDrawBuffer(static_cast<GLenum>(GL_COLOR_ATTACHMENT0 + dst));
        TextureUnitContainer cont;
        utilgl::bindAndSetUniforms(shader_, cont, *out_[src], "volume");
        utilgl::multiDrawImagePlaneRect(static_cast<int>(dim.z));
    }

    FrameBufferObject::deactivateFBO();
    outport_.setData(out_[dst]);
}

void VolumeRegionShrink::initializeResources() { shader_.build(); }

}  // namespace inviwo
