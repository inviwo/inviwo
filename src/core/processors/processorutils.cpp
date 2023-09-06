/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2023 Inviwo Foundation
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

#include <inviwo/core/processors/processorutils.h>

#include <inviwo/core/processors/processor.h>
#include <inviwo/core/metadata/processormetadata.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/common/inviwomodule.h>
#include <inviwo/core/common/modulemanager.h>

#include <algorithm>

namespace inviwo {
namespace util {

const ProcessorMetaData* getMetaData(const Processor* processor) {
    if (processor) {
        return processor->getMetaData<ProcessorMetaData>(ProcessorMetaData::CLASS_IDENTIFIER);
    }
    return nullptr;
}

ProcessorMetaData* getMetaData(Processor* processor) {
    if (processor) {
        return processor->getMetaData<ProcessorMetaData>(ProcessorMetaData::CLASS_IDENTIFIER);
    }
    return nullptr;
}

ivec2 getPosition(const Processor* processor) {
    if (auto meta = getMetaData(processor)) {
        return meta->getPosition();
    }
    return {0, 0};
}

void setPosition(Processor* processor, ivec2 pos) {
    if (auto meta = getMetaData(processor)) {
        meta->setPosition(pos);
    }
}

bool isSelected(const Processor* processor) {
    if (auto meta = getMetaData(processor)) {
        return meta->isSelected();
    }
    return false;
}

void setSelected(Processor* processor, bool selected) {
    if (auto meta = getMetaData(processor)) {
        meta->setSelected(selected);
    }
}

InviwoModule* getProcessorModule(const Processor* processor, InviwoApplication& app) {
    if (!processor) return nullptr;
    return getProcessorModule(processor->getClassIdentifier(), app);
}

InviwoModule* getProcessorModule(std::string_view classIdentifier, InviwoApplication& app) {

    auto inviwoModules = app.getModuleManager().getInviwoModules();
    const auto it = std::ranges::find_if(
        inviwoModules, [&](const InviwoModule& inviwoModule) {
            const auto processors = inviwoModule.getProcessors();
            return std::ranges::find_if(processors, [&](const ProcessorFactoryObject* pfo) {
                       return pfo->getClassIdentifier() == classIdentifier;
                   }) != processors.end();
        });
    return it != inviwoModules.end() ? &(*it) : nullptr;
}

std::optional<std::string> getProcessorModuleIdentifier(std::string_view classIdentifier,
                                                        InviwoApplication& app) {

    if (auto m = getProcessorModule(classIdentifier, app)) {
        return m->getIdentifier();
    } else {
        return std::nullopt;
    }
}

}  // namespace util

}  // namespace inviwo
