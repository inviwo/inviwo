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

#include <inviwo/core/interaction/events/event.h>
#include <inviwo/core/processors/processor.h>

namespace inviwo {

Event::Event() : Serializable(), used_(false) {}

Event::Event(const Event& rhs)
    : used_(rhs.used_)    {
}

Event& Event::operator=(const Event& that) {
    if (this != &that) {
        used_ = that.used_;
    }
    return *this;
}

Event* Event::clone() const {
    return new Event(*this);
}

Event::~Event() {}

void Event::markAsUsed(){
    used_ = true;
}

bool Event::hasBeenUsed(){
    return used_;
}

void Event::markAsVisited(Processor* p){
    visitedProcessors_.insert(p);
}

bool Event::hasVisitedProcessor(Processor* p){
    std::unordered_set<Processor*>::const_iterator it = visitedProcessors_.find(p);
    return (it != visitedProcessors_.end());
}

void Event::serialize(Serializer& s) const {}
void Event::deserialize(Deserializer& d) {}

} // namespace