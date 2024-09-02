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

#include <modules/oit/processors/rasterizer.h>
#include <modules/oit/datastructures/rasterization.h>
#include <modules/oit/rendering/fragmentlistrenderer.h>
#include <modules/oit/rasterizeevent.h>

namespace inviwo {

Rasterizer::Rasterizer(std::string_view identifier, std::string_view displayName)
    : Processor(identifier, displayName), outport_{"image"} {
    addPort(outport_);
}

void Rasterizer::initializeResources() {}

void Rasterizer::configureShader(Shader& shader) {
    const bool set = usesFragmentLists() == UseFragmentList::Yes &&
                     FragmentListRenderer::supportsFragmentLists();
    constexpr auto Enable = ShaderObject::ExtensionBehavior::Enable;
    auto* fso = shader.getFragmentShaderObject();
    fso->setShaderExtension("GL_NV_gpu_shader5", Enable, set);
    fso->setShaderExtension("GL_EXT_shader_image_load_store", Enable, set);
    fso->setShaderExtension("GL_NV_shader_buffer_load", Enable, set);
    fso->setShaderExtension("GL_EXT_bindable_uniform", Enable, set);

    fso->setShaderDefine("USE_FRAGMENT_LIST", set);

    handle_.configureShader(shader);
}

void Rasterizer::setUniforms(Shader& shader) {
    handle_.setUniforms(shader, usesFragmentLists(), rasterization_.get());
}

void Rasterizer::propagateEvent(Event* event, Outport* source) {
    if (event->hash() == RasterizeEvent::chash()) {
        auto e = static_cast<RasterizeEvent*>(event);
        handle_ = e->addInitializeShaderCallback([this]() { initializeResources(); });
    }

    Processor::propagateEvent(event, source);
}

void Rasterizer::process() {
    rasterization_ =
        std::make_shared<Rasterization>(std::dynamic_pointer_cast<Rasterizer>(shared_from_this()));
    outport_.setData(rasterization_);
}

void Rasterizer::setValid() {
    // override setValid() to maintain isModified() state of properties and isChanged() for inports
    // as these might be used later during rasterization in a subsequent rasterization renderer.
    // Note: need to set this processor to valid _after_ rasterization.
    for (auto outport : getOutports()) outport->setValid();
}

}  // namespace inviwo
