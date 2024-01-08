/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2024 Inviwo Foundation
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
#include <inviwo/core/util/hashcombine.h>

namespace inviwo {

class Processor;
class Inport;
class Outport;

class IVW_CORE_API PortConnection {

public:
    PortConnection();
    PortConnection(Outport* outport, Inport* inport);
    PortConnection(const PortConnection&) = default;
    PortConnection(PortConnection&&) noexcept = default;
    PortConnection& operator=(const PortConnection&) = default;
    PortConnection& operator=(PortConnection&&) noexcept = default;
    ~PortConnection() = default;

    /**
     * Method to test if both outport and inport is valid, eg not nullptr
     * @return false if the outport and/or the inport is nullptr
     */
    operator bool() const;

    Inport* getInport() const { return inport_; }
    Outport* getOutport() const { return outport_; }

    bool involvesProcessor(Processor* processor) const;

    friend bool IVW_CORE_API operator==(const PortConnection& lhs, const PortConnection& rhs);
    friend bool IVW_CORE_API operator!=(const PortConnection& lhs, const PortConnection& rhs);
    friend bool IVW_CORE_API operator<(const PortConnection& lhs, const PortConnection& rhs);

private:
    Inport* inport_;
    Outport* outport_;
};

}  // namespace inviwo

namespace std {

template <>
struct hash<inviwo::PortConnection> {
    size_t operator()(const inviwo::PortConnection& p) const {
        size_t h = 0;
        inviwo::util::hash_combine(h, p.getOutport());
        inviwo::util::hash_combine(h, p.getInport());
        return h;
    }
};

}  // namespace std
