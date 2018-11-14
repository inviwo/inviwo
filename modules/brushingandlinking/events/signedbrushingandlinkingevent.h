/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2018 Inviwo Foundation
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

#ifndef IVW_SIGNEDBRUSHINGANDLINKINGEVENT_H
#define IVW_SIGNEDBRUSHINGANDLINKINGEVENT_H

#include <modules/brushingandlinking/brushingandlinkingmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/interaction/events/event.h>
#include <inviwo/core/util/constexprhash.h>

namespace inviwo {

class BrushingAndLinkingInport;
/**
 * \class SignedBrushingAndLinkingEvent
 */
class IVW_MODULE_BRUSHINGANDLINKING_API SignedBrushingAndLinkingEvent : public Event {
public:
    SignedBrushingAndLinkingEvent(const BrushingAndLinkingInport* src,
                            const std::unordered_set<int>& indices);
    virtual ~SignedBrushingAndLinkingEvent() = default;

    virtual SignedBrushingAndLinkingEvent* clone() const override;

    const BrushingAndLinkingInport* getSource() const;

    const std::unordered_set<int>& getIndices() const;

    virtual uint64_t hash() const override;

    static constexpr uint64_t chash() {
        return util::constexpr_hash("org.inviwo.SignedBrushingAndLinkingEvent");
    }

private:
    const BrushingAndLinkingInport* source_;
    const std::unordered_set<int>& indices_;
};

}  // namespace

#endif // IVW_SIGNEDBRUSHINGANDLINKINGEVENT_H
