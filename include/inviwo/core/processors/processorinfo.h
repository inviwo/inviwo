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

#pragma once

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/processors/processorstate.h>
#include <inviwo/core/processors/processortags.h>
#include <inviwo/core/util/document.h>

#include <string>

namespace inviwo {

/**
 * \class ProcessorInfo
 * \brief Static information about a processor. Needed mostly for the processor list and factories
 */
struct IVW_CORE_API ProcessorInfo {
public:
    ProcessorInfo(std::string aClassIdentifier, std::string aDisplayName, std::string aCategory,
                  CodeState aCodeState, Tags someTags);
    ProcessorInfo(std::string aClassIdentifier, std::string aDisplayName, std::string aCategory,
                  CodeState aCodeState, Tags someTags, Document help, bool visible = true);

    /// Identifier must be unique for all processors, example org.inviwo.yourprocessor
    std::string classIdentifier;
    std::string displayName;  ///< Processor::getDisplayName
    std::string category;     ///< Used for grouping processors in ProcessorTreeWidget
    CodeState codeState;
    Tags tags;      ///< Searchable tags, platform tags are shown in ProcessorTreeWidget
    Document help;  ///< Help text for the processor
    bool visible;   ///< Show in processor list (ProcessorTreeWidget), enabling drag&drop

    IVW_CORE_API friend bool operator==(const ProcessorInfo& a, const ProcessorInfo& b);
    IVW_CORE_API friend bool operator!=(const ProcessorInfo& a, const ProcessorInfo& b);
};

}  // namespace inviwo
