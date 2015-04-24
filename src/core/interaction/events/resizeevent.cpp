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

#include <inviwo/core/interaction/events/resizeevent.h>

namespace inviwo {

ResizeEvent::ResizeEvent(uvec2 canvasSize)
    : Event(), size_(canvasSize), previousSize_(canvasSize) {}

ResizeEvent::ResizeEvent(uvec2 canvasSize, uvec2 previousSize)
    : Event(), size_(canvasSize), previousSize_(previousSize) {}

ResizeEvent::ResizeEvent(const ResizeEvent& rhs)
    : Event(rhs), size_(rhs.size_), previousSize_(rhs.previousSize_) {}

ResizeEvent& ResizeEvent::operator=(const ResizeEvent& that) {
    if (this != &that) {
        Event::operator=(that);
        size_ = that.size_;
        previousSize_ = that.previousSize_;
    }
    return *this;
}

ResizeEvent* ResizeEvent::clone() const { return new ResizeEvent(*this); }

ResizeEvent::~ResizeEvent() {}

}  // namespace