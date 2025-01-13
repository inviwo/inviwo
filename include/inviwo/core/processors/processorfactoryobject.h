/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2025 Inviwo Foundation
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
#include <inviwo/core/processors/processorinfo.h>
#include <inviwo/core/processors/processortraits.h>
#include <inviwo/core/processors/processor.h>

#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/utilities.h>
#include <inviwo/core/util/document.h>

#include <type_traits>
#include <string>

namespace inviwo {

class IVW_CORE_API ProcessorFactoryObject {
public:
    ProcessorFactoryObject(ProcessorInfo info, std::string_view typeName)
        : info_(std::move(info)), typeName_{typeName} {}
    virtual ~ProcessorFactoryObject() = default;

    virtual std::shared_ptr<Processor> create(InviwoApplication* app) const = 0;

    const ProcessorInfo& getProcessorInfo() const { return info_; }
    const std::string& getClassIdentifier() const { return info_.classIdentifier; }
    const std::string& getDisplayName() const { return info_.displayName; }
    const Tags& getTags() const { return info_.tags; }
    const std::string& getCategory() const { return info_.category; }
    CodeState getCodeState() const { return info_.codeState; }
    bool isVisible() { return info_.visible; }

    /**
     * Overload this function to provide additional information
     */
    virtual Document getMetaInformation() const { return Document(); }

    const std::string& getTypeName() const { return typeName_; }

private:
    ProcessorInfo info_;
    std::string typeName_;
};

#include <warn/push>
#include <warn/ignore/unused-parameter>
template <typename T>
class ProcessorFactoryObjectTemplate : public ProcessorFactoryObject {
public:
    ProcessorFactoryObjectTemplate()
        : ProcessorFactoryObject(ProcessorTraits<T>::getProcessorInfo(),
                                 util::demangle(typeid(T).name())) {}
    virtual ~ProcessorFactoryObjectTemplate() = default;

    virtual std::shared_ptr<Processor> create(
        [[maybe_unused]] InviwoApplication* app) const override {
        std::shared_ptr<Processor> p;

        if constexpr (std::is_constructible_v<T, const std::string&, const std::string&,
                                              InviwoApplication*>) {
            p = std::make_shared<T>(util::stripIdentifier(getDisplayName()), getDisplayName(), app);
        } else if constexpr (std::is_constructible_v<T, const std::string&, const std::string&>) {
            p = std::make_shared<T>(util::stripIdentifier(getDisplayName()), getDisplayName());
        } else if constexpr (std::is_constructible_v<T, const std::string&, InviwoApplication*>) {
            p = std::make_shared<T>(util::stripIdentifier(getDisplayName()), app);
        } else if constexpr (std::is_constructible_v<T, const std::string&>) {
            p = std::make_shared<T>(util::stripIdentifier(getDisplayName()));
        } else if constexpr (std::is_constructible_v<T, InviwoApplication*>) {
            p = std::make_shared<T>(app);
        } else {
            p = std::make_shared<T>();
        }

        if (p->getIdentifier().empty()) p->setIdentifier(util::stripIdentifier(getDisplayName()));
        if (p->getDisplayName().empty()) p->setDisplayName(getDisplayName());
        return p;
    }
};
#include <warn/pop>

}  // namespace inviwo
