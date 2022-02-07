
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
#pragma once

#include <modules/basegl/baseglmoduledefine.h>
#include <modules/basegl/shadercomponents/shadercomponent.h>
#include <inviwo/core/ports/volumeport.h>

#include <string_view>

namespace inviwo {

/**
 * Adds a Volume inport, binds that volume and assigns it the a sampler with `<name>` and
 * sets the `<name>Parameters` uniforms
 * It will sample the volume into `<name>Voxel` and keep the previous value in `<name>VoxelPrev`
 * If Gradients::Single is set the gradient for `channel` will be computed into `<name>Gradient`,
 * the previous gradient will be store in `<name>GradientPrev`.
 * If Gradients::All is set the gradients for all channels will be computed into
 * `<name>AllGradients`, the previous gradient will be store in `<name>AllGradientsPrev`
 */
class IVW_MODULE_BASEGL_API VolumeComponent : public ShaderComponent {
public:
    enum class Gradients { None, Single, All };
    VolumeComponent(std::string_view name, Gradients graidents = Gradients::Single);

    virtual std::string_view getName() const override;
    virtual void process(Shader& shader, TextureUnitContainer& cont) override;
    virtual std::vector<std::tuple<Inport*, std::string>> getInports() override;
    virtual std::vector<Segment> getSegments() override;

    VolumeInport volumePort;
    Gradients gradients;
};

}  // namespace inviwo
