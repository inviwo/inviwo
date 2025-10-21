/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2025 Inviwo Foundation
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

ContextHolder* RenderContext::setDefaultRenderContext(Canvas* canvas) {
    if (canvas) {
        return setDefaultRenderContext(std::make_unique<CanvasContextHolder>(canvas));
    } else {
        return setDefaultRenderContext(std::unique_ptr<ContextHolder>{});
    }
}

ContextHolder* RenderContext::setDefaultRenderContext(std::unique_ptr<ContextHolder> context) {
    defaultContext_ = std::move(context);
    mainThread_ = std::this_thread::get_id();

    return defaultContext_.get();
}

void RenderContext::activateDefaultRenderContext() const {
    if (defaultContext_) defaultContext_->activate();
}

void RenderContext::activateLocalRenderContext() const {
    if (!defaultContext_) return;

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

void RenderContext::registerContext(Canvas::ContextID id, std::string_view name,
                                    std::unique_ptr<ContextHolder> context) {
    std::unique_lock<std::mutex> lock(mutex_);
    contextRegistry_[id] =
        ContextInfo{std::string(name), std::this_thread::get_id(), std::move(context)};
}

void RenderContext::unRegisterContext(Canvas::ContextID id) {
    std::unique_lock<std::mutex> lock(mutex_);
    contextRegistry_.erase(id);
}

std::string RenderContext::getContextName(Canvas::ContextID id) const {
    std::unique_lock<std::mutex> lock(mutex_);
    auto it = contextRegistry_.find(id);
    if (it != contextRegistry_.end()) {
        return it->second.name;
    } else {
        return "Unknown";
    }
}

void RenderContext::setContextName(Canvas::ContextID id, std::string_view name) {
    std::unique_lock<std::mutex> lock(mutex_);
    auto it = contextRegistry_.find(id);
    if (it != contextRegistry_.end()) {
        it->second.name = name;
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

ContextHolder* RenderContext::getDefaultRenderContext() { return defaultContext_.get(); }

bool RenderContext::hasDefaultRenderContext() const { return defaultContext_ != nullptr; }

CanvasContextHolder::CanvasContextHolder(Canvas* canvas) : canvas_{canvas} {}

void CanvasContextHolder::activate() { return canvas_->activate(); }

std::unique_ptr<Canvas> CanvasContextHolder::createHiddenCanvas() {
    return canvas_->createHiddenCanvas();
}

Canvas::ContextID CanvasContextHolder::activeContext() const { return canvas_->activeContext(); }

Canvas::ContextID CanvasContextHolder::contextId() const { return canvas_->contextId(); }

}  // namespace inviwo
