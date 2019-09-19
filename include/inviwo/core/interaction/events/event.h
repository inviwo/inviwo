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

#ifndef IVW_EVENT_H
#define IVW_EVENT_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/io/serialization/serializable.h>
#include <vector>
#include <cstdint>
#include <ostream>

namespace inviwo {

class Processor;
class Inport;
class Outport;
/** \class Event
 * Events are propagated upwards in the ProcessorNetwork through ports of processors until
 * they are marked as used or reach a SourceProcessor.
 */
class IVW_CORE_API Event {
public:
    virtual ~Event() = default;
    virtual Event* clone() const = 0;
    virtual uint64_t hash() const = 0;

    /**
     * Determine if the event should be propagated upwards to inport.
     * Can be overloaded to limit the number or ports a event is propagated through.
     */
    virtual bool shouldPropagateTo(Inport* inport, Processor* processor, Outport* source);

    bool markAsUsed();
    /**
     * Returns the previous used state, and sets it to used;
     */
    bool hasBeenUsed() const;
    /**
     * Returns the previous used state, and sets it to unused;
     */
    bool markAsUnused();

    /**
     * Returns the previous used state, and sets it to 'isUsed';
     */
    bool setUsed(bool isUsed);

    /**
     * Returns false if the processor was already visited;
     */
    bool markAsVisited(Processor*);
    void markAsVisited(Event&);
    bool hasVisitedProcessor(Processor*) const;
    // Can be used to figure out where an event came from.
    // Processors are added in chronological order.
    const std::vector<Processor*>& getVisitedProcessors() const;

    template <typename EventType>
    EventType* getAs();
    template <typename EventType>
    const EventType* getAs() const;

    friend std::ostream& operator<<(std::ostream& ss, const Event& e) {
        e.print(ss);
        return ss;
    }

    virtual void print(std::ostream& ss) const;

protected:
    Event() = default;
    Event(const Event& rhs) = default;
    Event& operator=(const Event& that) = default;

private:
    bool used_ = false;
#include <warn/push>
#include <warn/ignore/dll-interface>
    std::vector<Processor*> visitedProcessors_;
#include <warn/pop>
};

template <typename EventType>
EventType* Event::getAs() {
    if (hash() == EventType::chash()) {
        return static_cast<EventType*>(this);
    }
    return nullptr;
}
template <typename EventType>
const EventType* Event::getAs() const {
    if (hash() == EventType::chash()) {
        return static_cast<const EventType*>(this);
    }
    return nullptr;
}

inline bool Event::markAsUsed() {
    const auto curr = used_;
    used_ = true;
    return curr;
}

inline bool Event::hasBeenUsed() const { return used_; }

inline bool Event::markAsUnused() {
    const auto curr = used_;
    used_ = false;
    return curr;
}

inline bool Event::setUsed(bool isUsed) {
    const auto curr = used_;
    used_ = isUsed;
    return curr;
}

}  // namespace inviwo

#endif  // IVW_EVENT_H
