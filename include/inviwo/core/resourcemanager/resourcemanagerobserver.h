/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2024 Inviwo Foundation
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
#include <inviwo/core/util/observer.h>

namespace inviwo {

struct Resource;

class IVW_CORE_API ResourceManagerObserver : public Observer {
public:
    virtual void onWillAddResource(size_t group, size_t index, const Resource& resource);
    virtual void onDidAddResource(size_t group, size_t index, const Resource& resource);
    virtual void onWillUpdateResource(size_t group, size_t index, const Resource& resource);
    virtual void onDidUpdateResource(size_t group, size_t index, const Resource& resource);
    virtual void onWillRemoveResource(size_t group, size_t index, const Resource& resource);
    virtual void onDidRemoveResource(size_t group, size_t index, const Resource& resource);
};

class IVW_CORE_API ResourceManagerObservable : public Observable<ResourceManagerObserver> {
protected:
    void notifyWillAddResource(size_t group, size_t index, const Resource& resource);
    void notifyDidAddResource(size_t group, size_t index, const Resource& resource);
    void notifyWillUpdateResource(size_t group, size_t index, const Resource& resource);
    void notifyDidUpdateResource(size_t group, size_t index, const Resource& resource);
    void notifyWillRemoveResource(size_t group, size_t index, const Resource& resource);
    void notifyDidRemoveResource(size_t group, size_t index, const Resource& resource);
};

inline void ResourceManagerObserver::onWillAddResource(size_t, size_t, const Resource&) {}
inline void ResourceManagerObserver::onDidAddResource(size_t, size_t, const Resource&) {}
inline void ResourceManagerObserver::onWillUpdateResource(size_t, size_t, const Resource&) {}
inline void ResourceManagerObserver::onDidUpdateResource(size_t, size_t, const Resource&) {}
inline void ResourceManagerObserver::onWillRemoveResource(size_t, size_t, const Resource&) {}
inline void ResourceManagerObserver::onDidRemoveResource(size_t, size_t, const Resource&) {}

}  // namespace inviwo
