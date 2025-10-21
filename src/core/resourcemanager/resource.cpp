/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2025 Inviwo Foundation
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

#include <inviwo/core/resourcemanager/resource.h>

#include <inviwo/core/resourcemanager/resourcemanager.h>

#include <inviwo/core/common/factoryutil.h>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/settings/systemsettings.h>
#include <inviwo/core/util/threadutil.h>

namespace inviwo::resource {

namespace {

ResourceManager* getResourceManager() {
    if (!InviwoApplication::isInitialized()) return nullptr;
    auto* app = InviwoApplication::getPtr();
    if (!app->getSystemSettings().enableResourceTracking_) return nullptr;
    return util::getResourceManager(app);
}

const std::thread::id mainThread = std::this_thread::get_id();

inline void onMain(auto&& func, auto&&... args) {
    if (mainThread == std::this_thread::get_id()) {
        if (auto* rm = getResourceManager()) {
            std::invoke(func, *rm, std::forward<decltype(args)>(args)...);
        }
    } else {
        dispatchFrontAndForget([func = std::forward<decltype(func)>(func),
                                ... args = std::forward<decltype(args)>(args)]() {
            if (auto* rm = getResourceManager()) {
                std::invoke(func, *rm, std::move(args)...);
            }
        });
    }
}

}  // namespace

void add(const RAM& key, Resource resource) {
    onMain(&ResourceManager::add<RAM>, key, std::move(resource));
}

void move(const RAM& oldKey, const RAM& newKey, Resource resource) {
    onMain(&ResourceManager::move<RAM>, oldKey, newKey, std::move(resource));
}

void remove(const RAM& key) { onMain(&ResourceManager::remove<RAM>, key); }

void meta(const RAM& key, const ResourceMeta& meta) {
    onMain(&ResourceManager::meta<RAM>, key, meta);
}

void add(const GL& key, Resource resource) {
    onMain(&ResourceManager::add<GL>, key, std::move(resource));
}

void move(const GL& oldKey, const GL& newKey, Resource resource) {
    onMain(&ResourceManager::move<GL>, oldKey, newKey, std::move(resource));
}

void remove(const GL& key) { onMain(&ResourceManager::remove<GL>, key); }

void meta(const GL& key, const ResourceMeta& meta) {
    onMain(&ResourceManager::meta<GL>, key, meta);
}

void add(const PY& key, Resource resource) {
    onMain(&ResourceManager::add<PY>, key, std::move(resource));
}

void move(const PY& oldKey, const PY& newKey, Resource resource) {
    onMain(&ResourceManager::move<PY>, oldKey, newKey, std::move(resource));
}

void remove(const PY& key) { onMain(&ResourceManager::remove<PY>, key); }

void meta(const PY& key, const ResourceMeta& meta) {
    onMain(&ResourceManager::meta<PY>, key, meta);
}

RAM toRAM(const void* ptr) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return resource::RAM{reinterpret_cast<std::uintptr_t>(ptr)};
}

}  // namespace inviwo::resource
