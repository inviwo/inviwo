/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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
#pragma once

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/interaction/events/event.h>
#include <inviwo/core/util/constexprhash.h>
#include <inviwo/core/algorithm/camerautils.h>

#include <variant>

namespace inviwo {

class IVW_CORE_API ViewEvent : public Event {
public:
    struct FlipUp {};
    struct FitData {};

    using Action = std::variant<camerautil::Side, FlipUp, FitData>;

    ViewEvent(Action action = camerautil::Side::XNegative);
    ViewEvent(const ViewEvent&) = default;
    ViewEvent& operator=(const ViewEvent&) = default;

    virtual ~ViewEvent() = default;

    // Inherited via Event
    virtual Event* clone() const override;
    virtual uint64_t hash() const override;

    static constexpr uint64_t chash() { return util::constexpr_hash("org.inviwo.ViewEvent"); }

    virtual void print(std::ostream& ss) const override;

    Action getAction() const;

private:
    Action action_;
};

}  // namespace inviwo
