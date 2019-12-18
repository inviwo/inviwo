/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2020 Inviwo Foundation
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

#include <modules/basegl/raycasting/isotfcomponent.h>

#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/texture/textureutils.h>


namespace inviwo {

IsoTFComponent::IsoTFComponent(VolumeInport* volumeInport)
    : RaycasterComponent()
    , isotfComposite_("isotfComposite", "TF & Isovalues", volumeInport,
                      InvalidationLevel::InvalidResources) {}

std::string IsoTFComponent::getName() const { return isotfComposite_.getIdentifier(); }

void IsoTFComponent::setUniforms(Shader& shader, TextureUnitContainer& cont) const {
    utilgl::bindAndSetUniforms(shader, cont, isotfComposite_);
    utilgl::setUniforms(shader, isotfComposite_);
}

void IsoTFComponent::setDefines(Shader& shader) const {
    // need to ensure there is always at least one isovalue due to the use of the macro
    // as array size in IsovalueParameters
    shader.getFragmentShaderObject()->addShaderDefine(
        "MAX_ISOVALUE_COUNT",
        toString(std::max<size_t>(1, isotfComposite_.isovalues_.get().size())));
}

std::vector<Property*> IsoTFComponent::getProperties() { return {&isotfComposite_}; }

auto IsoTFComponent::getSegments() const -> std::vector<Segment> {
    return {Segment{"uniform sampler2D transferFunction;", Segment::uniform, 1000}};
}
}  // namespace inviwo
