/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#ifndef IVW_PROCESSORFACTORYOBJECT_H
#define IVW_PROCESSORFACTORYOBJECT_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/processors/processorstate.h>
#include <inviwo/core/processors/processortags.h>
#include <string>

namespace inviwo {

class Processor;

class IVW_CORE_API ProcessorFactoryObject {
public:
    ProcessorFactoryObject(const std::string &classIdentifier, const std::string &displayName, Tags tags,
                           const std::string &category, CodeState codeState)
        : classIdentifier_(classIdentifier)
        , displayName_(displayName)
        , tags_(tags)
        , category_(category)
        , codeState_(codeState) {}
    virtual ~ProcessorFactoryObject() {}

    virtual Processor* create() = 0;

    std::string getClassIdentifier() const { return classIdentifier_; }
    std::string getDisplayName() const { return displayName_; }
    Tags getTags() const { return tags_; }
    std::string getCategory() const { return category_; }
    CodeState getCodeState() const { return codeState_; }

private:
    const std::string classIdentifier_;
    const std::string displayName_;
    const Tags tags_;
    const std::string category_;
    const CodeState codeState_;
};

template <typename T>
class ProcessorFactoryObjectTemplate : public ProcessorFactoryObject {
public:
    ProcessorFactoryObjectTemplate()
        : ProcessorFactoryObject(T::CLASS_IDENTIFIER, T::DISPLAY_NAME, T::TAGS, T::CATEGORY,
                                 T::CODE_STATE) {}
    virtual ~ProcessorFactoryObjectTemplate() {}

    virtual Processor* create() { return static_cast<Processor*>(new T()); }
};

} // namespace

#endif // IVW_PROCESSORFACTORYOBJECT_H
