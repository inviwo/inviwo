/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
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

#ifndef IVW_TEMPLATE_RESOURCE_H
#define IVW_TEMPLATE_RESOURCE_H

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/properties/property.h>

namespace inviwo {

/** \class TemplateResource
 *
 * A TemplateResource is a container for data of type T.
 *
 * @see Resource
 * @see ResourceManager
 */
template<typename T>
class TemplateResource : public Resource {

public:
    /**
     * Resource will take ownership of data and then delete it upon destruction.
     *
     * @param value Pointer to allocated data.
     */
    TemplateResource(const std::string& identifier, T* value);

    virtual ~TemplateResource() { delete value_; }

    virtual T* getData() { return value_; };
    virtual const T* getData() const { return value_; };

    virtual const std::string& getIdentifier() const { return identifier_; };

protected:
    std::string identifier_;
    T* value_;
};

template <typename T>
TemplateResource<T>::TemplateResource(const std::string& identifier, T* value)
    : Resource(), identifier_(identifier), value_(value)
{}


} // namespace

#endif // IVW_TEMPLATE_RESOURCE_H
