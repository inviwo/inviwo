/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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

#ifndef IVW_PICKINGACTION_H
#define IVW_PICKINGACTION_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>

namespace inviwo {

class PickingEvent;
class Processor;

/**
 * \class PickingAction
 * Associate a range of picking colors / indices to a callback function. Created and handled by the
 * PickingManger. Use a PickingMapper to ask the PickingManager for a PickingAction.
 */
class IVW_CORE_API PickingAction {
public:
    friend class PickingManager;
    using Callback = std::function<void(PickingEvent*)>;

    PickingAction(size_t start, size_t size = 1);
    virtual ~PickingAction();

    /**
     * Returns the global picking index, the global index can be used with the
     * PickingManager::indexToColor(size_t index) function to get a picking color.
     * \param id the local picking index
     */
    size_t getPickingId(size_t id = 0) const;

    /**
     * Check if a global picking index belongs to this picking action.
     */
    bool isIndex(size_t globalId) const;

    /**
     * Returns the local picking index, the local index converted from the global index by
     * subtracting the start id making it range from 0 to size-1
     * \param globalId the global picking index
     */
    size_t getLocalPickingId(size_t globalId = 0) const;

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
     * Enable or disable calling of the callback action.
     */
    void setEnabled(bool enabled);
    bool isEnabled() const;

    /**
     *	Set the callback action
     */
    void setAction(Callback action);

    /**
     *	Set the processor where the picking colors is drawn.
     */
    void setProcessor(Processor* processor);
    Processor* getProcessor() const;

    void operator()(PickingEvent*) const;

private:
    size_t getCapacity() const;
    void setSize(size_t size);

    size_t start_;
    size_t size_;
    size_t capacity_;

    Callback action_;
    Processor* processor_;

    bool enabled_ = true;
};

}  // namespace inviwo

#endif  // IVW_PICKINGACTION_H
