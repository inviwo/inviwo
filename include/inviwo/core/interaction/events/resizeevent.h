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

#ifndef IVW_RESIZEEVENT_H
#define IVW_RESIZEEVENT_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/interaction/events/event.h>
#include <inviwo/core/util/constexprhash.h>

namespace inviwo {

class Inport;
class Processor;
class Outport;

/** \class ResizeEvent
 * Event propagating image/canvas size changes upwards in the network.
 * Image inports and outports within a processor must be in the same group for the event to be
 * propagated.
 *
 * As soon as the network adds or removes a connection, all the image sinks (processors that
 * consume images) are responsible for pushing a new resize event to the network to make sure that
 * all the image ports in the network above it have an up-to-date view on which image sizes to use.
 * (@see ImageExport, @see CanvasProcessorWidget)
 *
 * @see BaseImageInport
 * @see ImageOutport
 * @see Processor::addPort
 */
class IVW_CORE_API ResizeEvent : public Event {
public:
    ResizeEvent(size2_t newSize);
    ResizeEvent(size2_t newSize, size2_t previousSize);
    ResizeEvent(const ResizeEvent& rhs) = default;
    ResizeEvent& operator=(const ResizeEvent& that) = default;

    virtual ResizeEvent* clone() const override;
    virtual ~ResizeEvent() = default;

    virtual bool shouldPropagateTo(Inport* inport, Processor* processor, Outport* source) override;

    size2_t size() const;
    size2_t previousSize() const;
    void setSize(size2_t csize);
    void setPreviousSize(size2_t previousSize);

    virtual uint64_t hash() const override;
    static constexpr uint64_t chash();

    virtual void print(std::ostream& ss) const override;

private:
    size2_t size_;
    size2_t previousSize_;
};

constexpr uint64_t ResizeEvent::chash() { return util::constexpr_hash("org.inviwo.ResizeEvent"); }

}  // namespace inviwo

#endif  // IVW_RESIZEEVENT_H
