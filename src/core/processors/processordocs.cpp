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

#include <inviwo/core/processors/processordocs.h>

#include <inviwo/core/common/inviwomodule.h>
#include <inviwo/core/processors/processorfactory.h>

namespace inviwo::help {

const HelpProcessor* ProcessorDocs::get(std::string_view classId) const {
    if (auto it = map.find(classId); it != map.end()) {
        return &it->second;
    } else {
        return nullptr;
    }
}

ProcessorDocs generateDocs(ProcessorFactory& pf) {
    UnorderedStringMap<HelpProcessor> docs;

    log::SuppressLoggingLocal suppress{};
    for (auto& classId : pf.getKeyView()) {
        try {
            if (auto processor = pf.createShared(classId)) {
                docs.try_emplace(classId, buildProcessorHelp(*processor));
            }
        } catch (...) {
        }
    }

    return {docs};
}

bool matchOutportToInports(std::string_view outportClassId, const HelpProcessor& processor) {
    static constexpr std::string_view outportPostFix = ".outport";
    static constexpr std::string_view inportPostFix = ".inport";
    static constexpr std::string_view flatInportPostFix = ".flat.inport";
    static constexpr std::string_view multiInportPostFix = ".multi.inport";
    static constexpr std::string_view flatMultiInportPostFix = ".flat.multi.inport";

    if (outportClassId.ends_with(outportPostFix)) {
        const auto base = outportClassId.substr(0, outportClassId.size() - outportPostFix.size());
        return std::ranges::any_of(processor.inports, [&](const help::HelpInport& inport) {
            if (inport.classIdentifier.starts_with(base)) {
                const auto inportKind = inport.classIdentifier.substr(base.size());
                return inportKind == inportPostFix || inportKind == flatInportPostFix ||
                       inportKind == multiInportPostFix || inportKind == flatMultiInportPostFix;
            }
            return false;
        });
    }
    return false;
}

}  // namespace inviwo::help
