/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2022 Inviwo Foundation
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

#include <modules/userinterfacegl/userinterfaceglmoduledefine.h>  // for IVW_MODULE_USERINTERFAC...

#include <inviwo/core/properties/property.h>                      // for PropertyTraits

#include <memory>                                                 // for unique_ptr, make_unique
#include <string>                                                 // for string

namespace inviwo {
class Processor;

namespace glui {
class Element;
class Renderer;

class IVW_MODULE_USERINTERFACEGL_API WidgetFactoryObject {
public:
    WidgetFactoryObject(const std::string& className);
    virtual ~WidgetFactoryObject();

    virtual std::unique_ptr<Element> create(Property&, Processor&, Renderer&) = 0;

    std::string getClassIdentifier() const;

private:
    std::string className_;
};

template <typename T, typename P>
class WidgetFactoryObjectTemplate : public WidgetFactoryObject {
public:
    WidgetFactoryObjectTemplate() : WidgetFactoryObject(PropertyTraits<P>::classIdentifier()) {}
    WidgetFactoryObjectTemplate(const std::string& classIdentifier)
        : WidgetFactoryObject(classIdentifier){};
    virtual ~WidgetFactoryObjectTemplate() = default;

    virtual std::unique_ptr<Element> create(Property& prop, Processor& proc,
                                            Renderer& renderer) override {
        return std::make_unique<T>(static_cast<P&>(prop), proc, renderer);
    }
};

}  // namespace glui

}  // namespace inviwo
