/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2026 Inviwo Foundation
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

#include <inviwo/core/properties/property.h>
#include <inviwo/core/ports/port.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/processorwidget.h>

#include <cstdint>
#include <string>
#include <functional>
#include <memory>
#include <cstdint>
#include <variant>

namespace inviwo {

/**
 * A Module can register ModuleAction with the InviwoApplication. These action will the
 * Be added a some places, for example as property widget context menu options.
 */
struct IVW_CORE_API ModuleAction {
    enum class Type : std::uint8_t { Property, Port, Processor, ProcessorWidget };
    enum class State : std::uint8_t { Default = 0, Enabled = 1, Disabled = 2, Custom = 3 };

    std::string name;
    std::string group;
    State state = State::Disabled;
    std::variant<std::weak_ptr<std::function<void(Property&)>>,
                 std::weak_ptr<std::function<void(Port&)>>,
                 std::weak_ptr<std::function<void(Processor&)>>,
                 std::weak_ptr<std::function<void(ProcessorWidget&)>>>
        callback;

    constexpr Type type() const { return static_cast<Type>(callback.index()); }
    bool expired() const {
        return std::visit([&](const auto& item) { return item.expired(); }, callback);
    }
    template <Type t>
    auto get() const -> decltype(std::get<static_cast<size_t>(t)>(callback).lock()) {
        if (auto* wPtr = std::get_if<static_cast<size_t>(t)>(&callback)) {
            if (auto funPtr = wPtr->lock()) {
                return funPtr;
            }
        }
        return nullptr;
    }
};

}  // namespace inviwo
