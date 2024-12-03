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
#include <inviwo/core/io/serialization/serializable.h>
#include <inviwo/core/processors/processorfactoryobject.h>
#include <string>
#include <vector>

namespace inviwo {

class InviwoModule;
class InviwoApplication;

struct IVW_CORE_API InviwoSetupInfo {
    using allocator_type = std::pmr::polymorphic_allocator<std::byte>;

    struct ModuleSetupInfo {
        explicit ModuleSetupInfo(allocator_type alloc = {}) : name{alloc}, processors{alloc} {}

        ModuleSetupInfo(std::string_view aName, int aVersion,
                        std::pmr::vector<std::pmr::string> someProcessors,
                        allocator_type alloc = {})
            : name{aName, alloc}, version{aVersion}, processors{std::move(someProcessors)} {}

        void serialize(Serializer& s) const;
        void deserialize(Deserializer& d);
        std::pmr::string name;
        int version = 0;
        std::pmr::vector<std::pmr::string> processors;
    };

    explicit InviwoSetupInfo(allocator_type alloc = {});
    InviwoSetupInfo(const InviwoApplication& app, ProcessorNetwork& network,
                    allocator_type alloc = {});
    void serialize(Serializer& s) const;
    void deserialize(Deserializer& d);

    const InviwoSetupInfo::ModuleSetupInfo* getModuleInfo(std::string_view moduleName) const;
    const InviwoSetupInfo::ModuleSetupInfo* getModuleForProcessor(
        std::string_view processorClassIdentifier) const;

    std::pmr::vector<ModuleSetupInfo> modules_;
};

}  // namespace inviwo
