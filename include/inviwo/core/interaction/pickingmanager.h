/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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

#ifndef IVW_PICKINGMANAGER_H
#define IVW_PICKINGMANAGER_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/util/singleton.h>
#include <inviwo/core/util/callback.h>
#include <inviwo/core/interaction/pickingaction.h>

namespace inviwo {

class PickingEvent;

/** \class PickingManager
 * Manager for picking objects.
 */
class IVW_CORE_API PickingManager : public Singleton<PickingManager> {
public:
    struct Result {
        size_t index;
        const PickingAction* action;

        size_t getLocalPickingId() const { return action->getLocalPickingId(index); };
    };

    PickingManager();
    PickingManager(PickingManager const&) = delete;
    PickingManager& operator=(PickingManager const&) = delete;
    virtual ~PickingManager();

    // clang-format off
    template <typename T>
    [[deprecated("was declared deprecated. Use `registerPickingAction(Processor*, PickingAction::Callback, size_t)` instead")]]
    PickingAction* registerPickingAction(Processor* processor, T* o,
                                         void (T::*m)(PickingEvent*), size_t size = 1);
    // clang-format on

    PickingAction* registerPickingAction(Processor* processor, PickingAction::Callback callback,
                                         size_t size = 1);

    bool unregisterPickingAction(const PickingAction*);
    bool pickingEnabled();

    static uvec3 indexToColor(size_t index);
    static size_t colorToIndex(uvec3 color);

    Result getPickingActionFromColor(const uvec3& color);
    Result getPickingActionFromIndex(size_t index);

    bool isPickingActionRegistered(const PickingAction* action) const;

private:
    // start indexing at 1, 0 maps to black {0,0,0} and indicated no picking.
    size_t lastIndex_ = 1;
    // pickingObjects_ should be sorted on the start index.
    std::vector<std::unique_ptr<PickingAction>> pickingActions_;
    // unusedObjects_ should be sorted on capacity.
    std::vector<PickingAction*> unusedObjects_;

    bool enabled_ = false;
    const BaseCallBack* enableCallback_ = nullptr;

    friend Singleton<PickingManager>;
    static PickingManager* instance_;
};

// clang-format off
template <typename T>
[[deprecated("was declared deprecated. Use `registerPickingAction(Processor*, PickingAction::Callback, size_t)` instead")]]
PickingAction* PickingManager::registerPickingAction(Processor* processor, T* o,
                                                     void (T::*m)(PickingEvent*),
                                      size_t size) {
    using namespace std::placeholders;
    return registerPickingAction(processor, std::bind(m, o, _1), size);
}
// clang-format on
}  // namespace inviwo

#endif  // IVW_PICKINGMANAGER_H
