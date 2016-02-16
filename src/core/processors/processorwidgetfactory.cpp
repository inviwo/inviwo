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

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/common/inviwomodule.h>
#include <inviwo/core/processors/processorwidgetfactory.h>

namespace inviwo {

bool ProcessorWidgetFactory::registerObject(M* obj) {
    if (util::insert_unique(map_, obj->getClassIdentifier(), obj)) {
        return true;
    } else {
        LogWarn("Failed to register object " << obj->getClassIdentifier()
                                             << ", already registered");
        return false;
    }
}

bool ProcessorWidgetFactory::unRegisterObject(M* obj) {
    size_t removed = util::map_erase_remove_if(
        map_, [obj](Map::value_type& elem) { return elem.second == obj; });

    return removed > 0;
}

std::unique_ptr<ProcessorWidget> ProcessorWidgetFactory::create(Processor* processor) const {
    auto it = map_.find(processor->getClassIdentifier());
    if (it != end(map_)) {
        return it->second->create(processor);
    } else {
        return nullptr;
    }
}

auto ProcessorWidgetFactory::create(K key, Processor* processor) const -> std::unique_ptr<T> {
    auto it = map_.find(key);
    if (it != end(map_)) {
        return it->second->create(processor);
    } else {
        return nullptr;
    }
}

bool ProcessorWidgetFactory::hasKey(K key) const { return util::has_key(map_, key); }

auto ProcessorWidgetFactory::getKeys() const -> std::vector<Key> {
    auto res = std::vector<Key>();
    for (auto& elem : map_) res.push_back(elem.first);
    return res;
}

}  // namespace
