/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2025 Inviwo Foundation
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

#include <inviwo/core/processors/processorinfo.h>
#include <tuple>

namespace inviwo {

ProcessorInfo::ProcessorInfo(std::string aClassIdentifier, std::string aDisplayName,
                             std::string aCategory, CodeState aCodeState, Tags someTags)
    : classIdentifier{std::move(aClassIdentifier)}
    , displayName{std::move(aDisplayName)}
    , category{std::move(aCategory)}
    , codeState(aCodeState)
    , tags{std::move(someTags)}
    , help{}
    , visible{true} {}

ProcessorInfo::ProcessorInfo(std::string aClassIdentifier, std::string aDisplayName,
                             std::string aCategory, CodeState aCodeState, Tags someTags,
                             Document help, bool isVisible)
    : classIdentifier{std::move(aClassIdentifier)}
    , displayName{std::move(aDisplayName)}
    , category{std::move(aCategory)}
    , codeState(aCodeState)
    , tags{std::move(someTags)}
    , help{std::move(help)}
    , visible{isVisible} {}

bool operator==(const ProcessorInfo& a, const ProcessorInfo& b) {
    return std::tie(a.classIdentifier, a.displayName, a.category, a.codeState, a.tags, a.visible) ==
           std::tie(b.classIdentifier, b.displayName, b.category, b.codeState, b.tags, b.visible);
}

bool operator!=(const ProcessorInfo& a, const ProcessorInfo& b) { return !(a == b); }

}  // namespace inviwo
