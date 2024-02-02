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

#include <modules/meshrenderinggl/rasterizeevent.h>
#include <modules/meshrenderinggl/ports/rasterizationport.h>
#include <modules/meshrenderinggl/processors/rasterizationrenderer.h>

namespace inviwo {

void RasterizeHandle::configureShader(Shader& shader) const {
    if (auto proc = processor_.lock()) {
        proc->configureShader(shader);
    }
}

void RasterizeHandle::setUniforms(Shader& shader, UseFragmentList useFragmentList) const {
    if (auto proc = processor_.lock()) {
        proc->setUniforms(shader, useFragmentList);
    }
}

RasterizeEvent::RasterizeEvent(std::shared_ptr<RasterizationRenderer> processor)
    : Event(), processor_{processor} {}

RasterizeEvent* RasterizeEvent::clone() const { return new RasterizeEvent(*this); }

bool RasterizeEvent::shouldPropagateTo(Inport* inport, Processor*, Outport*) {
    return dynamic_cast<RasterizationInport*>(inport) != nullptr;
}

RasterizeHandle RasterizeEvent::addInitializeShaderCallback(std::function<void()> callback) const {
    if (auto proc = processor_.lock()) {
        return RasterizeHandle(proc->initializeShader_.add(callback), proc);
    } else {
        return {};
    }
}

uint64_t RasterizeEvent::hash() const { return chash(); }

}  // namespace inviwo
