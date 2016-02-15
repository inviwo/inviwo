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
#include <inviwo/core/processors/processorinfo.h>
#include <inviwo/core/processors/processortraits.h>

#include <string>

namespace inviwo {

class Processor;

class IVW_CORE_API ProcessorFactoryObject {
public:
    ProcessorFactoryObject(const ProcessorInfo info) : info_(info) {}
    virtual ~ProcessorFactoryObject() {}

    virtual std::unique_ptr<Processor> create() = 0;

    ProcessorInfo getProcessorInfo() const { return info_; }
    std::string getClassIdentifier() const { return info_.classIdentifier; }
    std::string getDisplayName() const { return info_.displayName; }
    Tags getTags() const { return info_.tags; }
    std::string getCategory() const { return info_.category; }
    CodeState getCodeState() const { return info_.codeState; }

private:
    const ProcessorInfo info_;
};

template <typename T>
class ProcessorFactoryObjectTemplate : public ProcessorFactoryObject {
public:
    ProcessorFactoryObjectTemplate()
        : ProcessorFactoryObject(ProcessorTraits<T>::getProcessorInfo()) {}
    virtual ~ProcessorFactoryObjectTemplate() {}

    virtual std::unique_ptr<Processor> create() { return util::make_unique<T>(); }
};

}  // namespace

#endif  // IVW_PROCESSORFACTORYOBJECT_H
