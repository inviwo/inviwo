/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024-2025 Inviwo Foundation
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

#include <modules/oit/oitmoduledefine.h>

#include <inviwo/core/interaction/events/event.h>
#include <inviwo/core/util/constexprhash.h>
#include <inviwo/core/util/dispatcher.h>
#include <inviwo/core/util/fmtutils.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/sourcecontext.h>
#include <modules/oit/rendering/rasterizationrendererbase.h>

namespace inviwo {
class Rasterizer;
class Rasterization;
class Shader;

class IVW_MODULE_OIT_API RasterizeHandle {
public:
    RasterizeHandle() = default;
    RasterizeHandle(DispatcherHandle<void()> handle,
                    std::weak_ptr<RasterizationRendererBase> renderer)
        : handle_{handle}, rasterizationRenderer_{renderer} {}
    void configureShader(Shader&) const;
    void setUniforms(Shader&, UseFragmentList useFragmentList,
                     const Rasterization* rasterizer) const;

private:
    DispatcherHandle<void()> handle_;
    std::weak_ptr<RasterizationRendererBase> rasterizationRenderer_;
};

class IVW_MODULE_OIT_API RasterizeEvent : public Event {
public:
    RasterizeEvent(std::shared_ptr<RasterizationRendererBase> renderer);
    virtual ~RasterizeEvent() = default;

    virtual RasterizeEvent* clone() const override;

    virtual bool shouldPropagateTo(Inport* inport, Processor* processor, Outport* source) override;

    virtual uint64_t hash() const override;
    static constexpr uint64_t chash();

    RasterizeHandle addInitializeShaderCallback(std::function<void()> callback) const;

private:
    RasterizeEvent(const RasterizeEvent& rhs) = default;
    RasterizeEvent& operator=(const RasterizeEvent& that) = default;
    std::weak_ptr<RasterizationRendererBase> rasterizationRenderer_;
};

constexpr uint64_t RasterizeEvent::chash() {
    return util::constexpr_hash("org.inviwo.RasterizeEvent");
}

}  // namespace inviwo
