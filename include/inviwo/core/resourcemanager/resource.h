/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017 Inviwo Foundation
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

#ifndef IVW_RESOURCE_H
#define IVW_RESOURCE_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>

namespace inviwo {

/**
 * \class Resource
 * \brief Base class for resources. 
 * @see TypedResource<T> 
 * @see ResourceManager
 */
class IVW_CORE_API Resource {
public:
    Resource();
    virtual ~Resource() = default;
    
    Resource(const Resource &r) = delete;
    Resource(Resource &&r) = delete;

    Resource &operator=(const Resource &r) = delete;
    Resource &operator=(const Resource &&r) = delete;

};

/**
 * \class TypedResource
 * \brief Class used by ResourceManager to wrap a shared_ptr in a resource
 * @see ResourceManager
 */
template <typename T>
class TypedResource : public Resource {
public:
    TypedResource(std::shared_ptr<T> resource) : resource_(resource) {}
    virtual ~TypedResource() = default;

    operator std::shared_ptr<T>() const { return resource_; }
    std::shared_ptr<T> getData() { return resource_; }

private:
    std::shared_ptr<T> resource_;
};

}  // namespace inviwo

#endif  // IVW_RESOURCE_H
