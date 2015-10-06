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

#include <inviwo/core/io/datareaderfactory.h>
#include <inviwo/core/common/inviwoapplication.h>

namespace inviwo {

bool DataReaderFactory::registerObject(DataReader* reader) {
    for (const auto& ext : reader->getExtensions()) {
        util::insert_unique(map_, ext, reader);
    }
    return true;
}

bool DataReaderFactory::unRegisterObject(DataReader* reader) {
    size_t removed = util::map_erase_remove_if(
        map_, [reader](Map::value_type& elem) { return elem.second == reader; });

    return removed > 0;
}

std::unique_ptr<DataReader> DataReaderFactory::create(const FileExtension& key) const {
    return std::unique_ptr<DataReader>(
        util::map_find_or_null(map_, key, [](DataReader* o) { return o->clone(); }));
}

std::unique_ptr<DataReader> DataReaderFactory::create(const std::string& key) const {
    auto lkey = toLower(key);
    for (auto& elem : map_) {
        if (toLower(elem.first.extension_) == toLower(lkey)) {
            return std::unique_ptr<DataReader>(elem.second->clone());
        }
    }
    return nullptr;
}

bool DataReaderFactory::hasKey(const std::string& key) const {
    auto lkey = toLower(key);
    for (auto& elem : map_) {
        if (toLower(elem.first.extension_) == toLower(lkey)) return true;
    }
    return false;
}

bool DataReaderFactory::hasKey(const FileExtension& key) const { return util::has_key(map_, key); }

}  // namespace
