/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2019 Inviwo Foundation
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

#ifndef IVW_PICKINGMAPPER_H
#define IVW_PICKINGMAPPER_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/interaction/pickingmanager.h>

namespace inviwo {

class Processor;
class PickingEvent;
class PickingAction;

/**
 * \class PickingMapper
 * \brief RAII tool for PickingActions
 */
class IVW_CORE_API PickingMapper {
public:
    PickingMapper(PickingManager* manager = PickingManager::getPtr());

    /**
     * Construct a picking mapper. This will register a range of colors in the PickingMangaer and
     * create a PickingAction to associate those indices the the supplied action. The processor
     * argument should be the processor where the picking colors are drawn.
     */
    PickingMapper(Processor* p, size_t size, std::function<void(PickingEvent*)> callback,
                  PickingManager* manager = PickingManager::getPtr());

    PickingMapper(const PickingMapper& rhs) = delete;
    PickingMapper& operator=(const PickingMapper& that) = delete;
    PickingMapper(PickingMapper&& rhs);
    PickingMapper& operator=(PickingMapper&& that);
    ~PickingMapper();

    /**
     * Resize the underlaying PickingAction. This will invalidate all old indices/colors
     */
    void resize(size_t newSize);

    /**
     * Enable or disable calling of the callback action.
     */
    bool isEnabled() const;
    void setEnabled(bool enabled);

    /**
     * Returns the global picking index, the global index can be used with the
     * PickingManager::indexToColor(size_t index) function to get a picking color.
     * \param id the local picking index
     */
    size_t getPickingId(size_t id = 0) const;

    /**
     *	The picking color to use for the object with local index id.
     *  This is eqvivalent to PickingManager::indexToColor(getPickingId(id))/255.0
     * \param id the local picking index
     */
    vec3 getColor(size_t id = 0) const;

    /**
     *	The number of picking indices in this picking object.
     */
    size_t getSize() const;

    /**
     *	Retrieve the underlaying picking action.
     */
    const PickingAction* getPickingAction() const;

private:
    PickingManager* manager_ = nullptr;  // Should never be null.
    Processor* processor_ = nullptr;
    std::function<void(PickingEvent*)> callback_;
    PickingAction* pickingAction_ = nullptr;
};

}  // namespace inviwo

#endif  // IVW_PICKINGMAPPER_H
