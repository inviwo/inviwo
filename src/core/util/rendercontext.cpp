/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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
#include <inviwo/core/common/inviwoapplication.h>

namespace inviwo {

RenderContext* RenderContext::instance_ = nullptr;

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
            auto& canvas = contextMap_[id];
            lock.unlock();

            canvas = defaultContext_->createHiddenCanvas();
            localContext = canvas.get();
        } else {
            localContext = (*it).second.get();
        }
    }
    localContext->activate();
}

void RenderContext::clearContext() {
    auto id = std::this_thread::get_id();
    if (id == mainThread_) {
        std::unique_lock<std::mutex> lock(mutex_);
        contextMap_.erase(id);
    } else {
        std::unique_lock<std::mutex> lock(mutex_);
        auto it = contextMap_.find(id);
        if (it != contextMap_.end()) {
            auto canvas = it->second.release();
            contextMap_.erase(it);
            lock.unlock();

            canvas->releaseContext();
            dispatchFront([canvas]() { delete canvas; });
        }
    }
}

void RenderContext::registerContext(Canvas* canvas, const std::string& name) {
    std::unique_lock<std::mutex> lock(mutex_);
    contextRegistry_[canvas->contextId()] = {name, canvas, std::this_thread::get_id()};
}

void RenderContext::unRegisterContext(Canvas* canvas) {
    std::unique_lock<std::mutex> lock(mutex_);
    contextRegistry_.erase(canvas->contextId());
}

std::string RenderContext::getContextName(Canvas::ContextID id) const {
    std::unique_lock<std::mutex> lock(mutex_);
    auto it = contextRegistry_.find(id);
    if (it != contextRegistry_.end()) {
        return it->second.name;
    } else {
        return "";
    }
}

void RenderContext::setContextName(Canvas::ContextID id, const std::string& name) {
    std::unique_lock<std::mutex> lock(mutex_);
    auto it = contextRegistry_.find(id);
    if (it != contextRegistry_.end()) {
        it->second.name = name;
    }
}

Canvas* RenderContext::getCanvas(Canvas::ContextID id) const {
    std::unique_lock<std::mutex> lock(mutex_);
    auto it = contextRegistry_.find(id);
    if (it != contextRegistry_.end()) {
        return it->second.canvas;
    } else {
        return nullptr;
    }
}

std::thread::id RenderContext::getContextThreadId(Canvas::ContextID id) const {
    std::unique_lock<std::mutex> lock(mutex_);
    auto it = contextRegistry_.find(id);
    if (it != contextRegistry_.end()) {
        return it->second.threadId;
    } else {
        return std::thread::id{};
    }
}

void RenderContext::setContextThreadId(Canvas::ContextID id, std::thread::id threadId) {
    std::unique_lock<std::mutex> lock(mutex_);
    auto it = contextRegistry_.find(id);
    if (it != contextRegistry_.end()) {
        it->second.threadId = threadId;
    }
}

Canvas::ContextID RenderContext::activeContext() const {
    return defaultContext_ ? defaultContext_->activeContext() : nullptr;
}

Canvas* RenderContext::getDefaultRenderContext() { return defaultContext_; }

}  // namespace inviwo
