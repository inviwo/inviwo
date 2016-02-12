/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#include <inviwo/core/util/rendercontext.h>
#include <inviwo/core/util/canvas.h>

namespace inviwo {

RenderContext::RenderContext() : defaultContext_(nullptr) {}

RenderContext::~RenderContext() {}

void RenderContext::setDefaultRenderContext(Canvas* canvas) {
    defaultContext_ = canvas;
    mainThread_ = std::this_thread::get_id();
}

void RenderContext::activateDefaultRenderContext() const {
    if (defaultContext_) defaultContext_->activate();
}

void RenderContext::activateLocalRenderContext() const {
    auto id = std::this_thread::get_id();

    if (id == mainThread_) {
        activateDefaultRenderContext();
        return;
    }

    Canvas* localContext = nullptr;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        auto it = contextMap_.find(id);
        if (it == contextMap_.end()) {
            auto canvas = defaultContext_->create();
            localContext = canvas.get();
            contextMap_[id] = std::move(canvas);
        } else {
            localContext = (*it).second.get();
        }
    }
    localContext->activate();
}

void RenderContext::clearContext(const std::thread::id& id) {
    std::unique_lock<std::mutex> lock(mutex_);
    contextMap_.erase(id);
}

void RenderContext::clearLocalContexts() {
    std::unique_lock<std::mutex> lock(mutex_);
    contextMap_.clear();
}

Canvas* RenderContext::getDefaultRenderContext() { return defaultContext_; }

}  // namespace
