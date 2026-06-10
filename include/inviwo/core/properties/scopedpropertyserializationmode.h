/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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
#include <inviwo/core/properties/valuewrapper.h>

#include <vector>
#include <utility>

namespace inviwo {

class Property;
class Processor;
class ProcessorNetwork;

class IVW_CORE_API ScopedPropertySerializationMode {
public:
    ScopedPropertySerializationMode() = default;
    ScopedPropertySerializationMode(PropertySerializationMode Mode, Property& property);
    ScopedPropertySerializationMode(PropertySerializationMode Mode, Processor& processor);
    ScopedPropertySerializationMode(PropertySerializationMode Mode, ProcessorNetwork& network);

    ScopedPropertySerializationMode(const ScopedPropertySerializationMode&) = delete;
    ScopedPropertySerializationMode(ScopedPropertySerializationMode&&) = default;
    ScopedPropertySerializationMode& operator=(const ScopedPropertySerializationMode&) = delete;
    ScopedPropertySerializationMode& operator=(ScopedPropertySerializationMode&&) = default;
    ~ScopedPropertySerializationMode();

    void reset();

private:
    std::vector<std::pair<Property*, PropertySerializationMode>> properties_;
};

}  // namespace inviwo
