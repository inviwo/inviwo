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

#ifndef IVW_EVENT_H
#define IVW_EVENT_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/io/serialization/serializable.h>
#include <unordered_set>

namespace inviwo {

class Processor;

class IVW_CORE_API Event : public Serializable {
public:
    Event();
    Event(const Event& rhs);
    Event& operator=(const Event& that);
    virtual Event* clone() const;
    virtual ~Event();

    // Check if this event has the same type and selectors as aEvent.
    // this should be the selector, and aEvent the "real" event.
    virtual bool matching(const Event* aEvent) const { return false; }
    virtual bool equalSelectors(const Event* aEvent) const { return false; }

    void markAsUsed();
    bool hasBeenUsed();

    void markAsVisited(Processor*);
    bool hasVisitedProcessor(Processor*);

    virtual void serialize(Serializer& s) const;
    virtual void deserialize(Deserializer& d);

private:
    bool used_;
    #include <warn/push>
    #include <warn/ignore/dll-interface>
    std::unordered_set<Processor*> visitedProcessors_;
    #include <warn/pop>
};

}  // namespace

#endif  // IVW_EVENT_H