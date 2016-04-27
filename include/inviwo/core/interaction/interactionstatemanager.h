/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016 Inviwo Foundation
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

#ifndef IVW_INTERACTIONSTATEMANAGER_H
#define IVW_INTERACTIONSTATEMANAGER_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/util/dispatcher.h>

namespace inviwo {

/**
 * \class InteractionStateManager
 */
class IVW_CORE_API InteractionStateManager {
public:
    InteractionStateManager() = default;
    ~InteractionStateManager() = default;

    void beginInteraction();
    void endInteraction();

    bool isInteracting() const;

    template <typename T>
    std::shared_ptr<std::function<void()>> onInteractionBegin(T&& callback);
    template <typename T>
    std::shared_ptr<std::function<void()>> onInteractionEnd(T&& callback);

private:
    Dispatcher<void()> onInteractionBegin_;
    Dispatcher<void()> onInteractionEnd_;

    bool isInteracting_ = false;
};

template <typename T>
std::shared_ptr<std::function<void()>> InteractionStateManager::onInteractionBegin(T&& callback) {
    return onInteractionBegin_.add(std::forward<T>(callback));
}
template <typename T>
std::shared_ptr<std::function<void()>> InteractionStateManager::onInteractionEnd(T&& callback) {
    return onInteractionEnd_.add(std::forward<T>(callback));
}

} // namespace

#endif // IVW_INTERACTIONSTATEMANAGER_H

