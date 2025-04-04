/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2025 Inviwo Foundation
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

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/processors/processorfactory.h>
#include <inviwo/core/io/serialization/serializable.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/processors/processorutils.h>

#include <fmt/format.h>

namespace inviwo {

ProcessorFactory::ProcessorFactory(InviwoApplication* app) : app_{app} {}

bool ProcessorFactory::registerObject(ProcessorFactoryObject* processor) {
    auto moduleId = [&]() {
        auto optId = util::getProcessorModuleIdentifier(processor->getClassIdentifier(), *app_);
        return optId.value_or(std::string_view("Unknown"));
    };

    if (!Register::registerObject(processor)) {
        log::warn(
            "Processor with class name: '{}' is already registered by module '{}'. This "
            "processor will be ignored",
            processor->getClassIdentifier(), moduleId());
        return false;
    }

    if (util::splitStringView(processor->getClassIdentifier(), '.').size() < 3) {
        log::warn(
            "All processor classIdentifiers should be named using reverse DNS "
            "(org.inviwo.processor) not like: '{}' in module {}",
            processor->getClassIdentifier(), moduleId());
    }
    if (processor->getCategory().empty()) {
        log::warn("Processor '{}' in module '{}' has no category", processor->getClassIdentifier(),
                  moduleId());
    } else if (processor->getCategory() == "Undefined") {
        log::warn("Processor '{}' in module '{}' has category \"Undefined\"",
                  processor->getClassIdentifier(), moduleId());
    }
    if (processor->getDisplayName().empty()) {
        log::warn("Processor '{}' in module '{}' has no display name",
                  processor->getClassIdentifier(), moduleId());
    }

    return true;
}

std::unique_ptr<Processor> ProcessorFactory::create(std::string_view, InviwoApplication*) const {
    throw Exception("Processors can only be created using createShared");
}
std::unique_ptr<Processor> ProcessorFactory::create(std::string_view) const {
    throw Exception("Processors can only be created using createShared");
}

std::shared_ptr<Processor> ProcessorFactory::createShared(std::string_view key) const {
    return createShared(key, app_);
};
std::shared_ptr<Processor> ProcessorFactory::createShared(std::string_view key,
                                                          InviwoApplication* app) const {
    auto it = this->map_.find(key);
    if (it != end(this->map_)) {
        return it->second->create(app);
    } else {
        return nullptr;
    }
};

bool ProcessorFactory::hasKey(std::string_view key) const { return Register::hasKey(key); }

}  // namespace inviwo
