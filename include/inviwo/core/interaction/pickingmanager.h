/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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
#include <inviwo/core/interaction/pickingobject.h>
#include <inviwo/core/util/singleton.h>

namespace inviwo {

/** \class PickingManager
 * Manager for picking objects.
 */
class IVW_CORE_API PickingManager : public Singleton<PickingManager> {
    friend class PickingContainer;

public:
    PickingManager() = default;
    PickingManager(PickingManager const&) = delete;
    PickingManager& operator=(PickingManager const&) = delete;
    virtual ~PickingManager();

    template <typename T>
    const PickingObject* registerPickingCallback(T* o, void (T::*m)(const PickingObject*),
                                                 size_t size = 1);
    const PickingObject* registerPickingCallback(std::function<void(const PickingObject*)> callback,
                                                 size_t size = 1);

    bool unregisterPickingObject(const PickingObject*);
    bool pickingEnabled();

    static uvec3 indexToColor(size_t index);
    static size_t colorToIndex(uvec3 color);

protected:
    PickingObject* getPickingObjectFromColor(const uvec3&);

private:
    size_t lastIndex_ = 0;
    // pickingObjects_ should be sorted on the start index.
    std::vector<std::unique_ptr<PickingObject>> pickingObjects_;
    // unusedObjects_ should be sorted on capacity.
    std::vector<PickingObject*> unusedObjects_;
};

template <typename T>
const PickingObject* PickingManager::registerPickingCallback(T* o,
                                                             void (T::*m)(const PickingObject*),
                                                             size_t size) {
    using namespace std::placeholders;
    return registerPickingCallback(std::bind(m, o, _1), size);
}

}  // namespace

#endif  // IVW_PICKINGMANAGER_H