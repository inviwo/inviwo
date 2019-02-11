/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#include <inviwo/core/resourcemanager/resourcemanager.h>

namespace inviwo {

void ResourceManager::removeResource(const std::string &key, const std::type_index &type) {
    IVW_ASSERT(!key.empty(), "Key should not be empty string");
    auto it = resources_.find(std::make_pair(key, type));
    if (it != resources_.end()) {
        notifyResourceRemoved(key, type, it->second.get());
        resources_.erase(it);
    }
}

void ResourceManager::clear() {
    while (!resources_.empty()) {
        auto &p = resources_.begin()->first;
        removeResource(p.first, p.second);
    }
}

bool ResourceManager::isEnabled() const { return enabled_; }

void ResourceManager::setEnabled(bool enable) {
    if (enable != enabled_) {
        enabled_ = enable;
        notifyEnableChanged();
    }
}

size_t ResourceManager::numberOfResources() const { return resources_.size(); }

}  // namespace inviwo
