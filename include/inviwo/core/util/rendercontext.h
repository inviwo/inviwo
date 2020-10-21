
/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2020 Inviwo Foundation
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

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/util/singleton.h>
#include <inviwo/core/util/canvas.h>

namespace inviwo {

/**
 * An abstraction for what we need from a "context".
 */
class IVW_CORE_API ContextHolder {

public:
    virtual ~ContextHolder() = default;
    virtual void activate() = 0;
    virtual std::unique_ptr<Canvas> createHiddenCanvas() = 0;
    virtual Canvas::ContextID activeContext() const = 0;
    virtual Canvas::ContextID contextId() const = 0;
};

class IVW_CORE_API CanvasContextHolder : public ContextHolder {
public:
    CanvasContextHolder(Canvas* canvas);
    virtual void activate() override;
    virtual std::unique_ptr<Canvas> createHiddenCanvas() override;
    virtual Canvas::ContextID activeContext() const override;
    virtual Canvas::ContextID contextId() const override;

    Canvas* canvas_;
};

/**
 * \class RenderContext
 * \brief Keeper of the default render context.
 */
class IVW_CORE_API RenderContext : public Singleton<RenderContext> {
public:
    RenderContext() = default;
    virtual ~RenderContext() = default;

    ContextHolder* getDefaultRenderContext();
    bool hasDefaultRenderContext() const;
    ContextHolder* setDefaultRenderContext(Canvas* canvas);
    ContextHolder* setDefaultRenderContext(std::unique_ptr<ContextHolder> context);
    void activateDefaultRenderContext() const;

    void activateLocalRenderContext() const;
    Canvas::ContextID activeContext() const;

    void clearContext();

    void registerContext(Canvas::ContextID id, std::string_view name,
                         std::unique_ptr<ContextHolder> context);
    void unRegisterContext(Canvas::ContextID id);

    std::string getContextName(Canvas::ContextID id) const;
    void setContextName(Canvas::ContextID id, std::string_view name);
    std::thread::id getContextThreadId(Canvas::ContextID id) const;
    void setContextThreadId(Canvas::ContextID id, std::thread::id);

    template <typename C>
    void forEachContext(C callback);

private:
    struct ContextInfo {
        std::string name;
        std::thread::id threadId;
        std::unique_ptr<ContextHolder> context;
    };

    std::unordered_map<Canvas::ContextID, ContextInfo> contextRegistry_;

    std::unique_ptr<ContextHolder> defaultContext_ = nullptr;
    std::thread::id mainThread_;
    mutable std::mutex mutex_;
    mutable std::unordered_map<std::thread::id, std::unique_ptr<Canvas>> contextMap_;

    friend Singleton<RenderContext>;
    static RenderContext* instance_;
};

template <typename C>
void RenderContext::forEachContext(C callback) {
    for (const auto& item : contextRegistry_) {
        callback(item.first, item.second.name, item.second.context.get(), item.second.threadId);
    }
}

}  // namespace inviwo
