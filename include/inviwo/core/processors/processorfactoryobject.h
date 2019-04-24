/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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

#include <inviwo/core/util/stdextensions.h>

#include <string>

namespace inviwo {

class Processor;

class IVW_CORE_API ProcessorFactoryObject {
public:
    ProcessorFactoryObject(ProcessorInfo info) : info_(std::move(info)) {}
    virtual ~ProcessorFactoryObject() = default;

    virtual std::unique_ptr<Processor> create(InviwoApplication* app) = 0;

    ProcessorInfo getProcessorInfo() const { return info_; }
    std::string getClassIdentifier() const { return info_.classIdentifier; }
    std::string getDisplayName() const { return info_.displayName; }
    Tags getTags() const { return info_.tags; }
    std::string getCategory() const { return info_.category; }
    CodeState getCodeState() const { return info_.codeState; }
    bool isVisible() { return info_.visible; }

private:
    const ProcessorInfo info_;
};

namespace detail {

struct empty_constructor {};
struct app_constructor {};
struct id_name_constructor {};
struct id_name_app_constructor {};

template <typename T>
struct constructor_picker {
private:
    using app = util::is_constructible<T, InviwoApplication*>;
    using id_name = util::is_constructible<T, const std::string&, const std::string&>;
    using id_name_app =
        util::is_constructible<T, const std::string&, const std::string&, InviwoApplication*>;

    using app_cond =
        typename std::conditional<app::value, app_constructor, empty_constructor>::type;
    using id_name_cond =
        typename std::conditional<id_name::value, id_name_constructor, app_cond>::type;

public:
    using type =
        typename std::conditional<id_name_app::value, id_name_app_constructor, id_name_cond>::type;
};

template <typename T>
std::unique_ptr<Processor> makeProcessor(const std::string& id, const std::string& name,
                                         InviwoApplication* app) {
    return makeProcessor<T>(typename constructor_picker<T>::type{}, id, name, app);
}

template <typename T>
std::unique_ptr<Processor> makeProcessor(empty_constructor, const std::string& id,
                                         const std::string& name, InviwoApplication*) {
    auto p = util::make_unique<T>();
    if (p->getIdentifier().empty()) p->setIdentifier(id);
    if (p->getDisplayName().empty()) p->setDisplayName(name);
    return std::move(p);
}

template <typename T>
std::unique_ptr<Processor> makeProcessor(app_constructor, const std::string& id,
                                         const std::string& name, InviwoApplication* app) {
    auto p = util::make_unique<T>(app);
    if (p->getIdentifier().empty()) p->setIdentifier(id);
    if (p->getDisplayName().empty()) p->setDisplayName(name);
    return std::move(p);
}

template <typename T>
std::unique_ptr<Processor> makeProcessor(id_name_constructor, const std::string& id,
                                         const std::string& name, InviwoApplication*) {
    return util::make_unique<T>(id, name);
}

template <typename T>
std::unique_ptr<Processor> makeProcessor(id_name_app_constructor, const std::string& id,
                                         const std::string& name, InviwoApplication* app) {
    return util::make_unique<T>(id, name, app);
}

}  // namespace detail

template <typename T>
class ProcessorFactoryObjectTemplate : public ProcessorFactoryObject {
public:
    ProcessorFactoryObjectTemplate()
        : ProcessorFactoryObject(ProcessorTraits<T>::getProcessorInfo()) {}
    virtual ~ProcessorFactoryObjectTemplate() = default;

    virtual std::unique_ptr<Processor> create(InviwoApplication* app) {
        return detail::makeProcessor<T>(getDisplayName(), getDisplayName(), app);
    }
};

}  // namespace inviwo

#endif  // IVW_PROCESSORFACTORYOBJECT_H
