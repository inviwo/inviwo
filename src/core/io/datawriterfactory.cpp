/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2025 Inviwo Foundation
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

#include <inviwo/core/io/datawriterfactory.h>
#include <inviwo/core/common/inviwoapplication.h>

#include <inviwo/core/io/datawriter.h>
#include <inviwo/core/util/stdextensions.h>

namespace inviwo {

bool DataWriterFactory::registerObject(DataWriter* writer) {
    size_t added = 0;
    for (const auto& ext : writer->getExtensions()) {
        if (util::insert_unique(map_, ext, writer)) {
            ++added;
        } else {
            log::warn("Failed to register DataWriter for \"{}\", already registered", ext);
        }
    }
    if (added > 0) {
        this->notifyObserversOnRegister(writer);
    }
    return added > 0;
}

bool DataWriterFactory::unRegisterObject(DataWriter* writer) {
    size_t removed =
        std::erase_if(map_, [writer](const auto& elem) { return elem.second == writer; });

    if (removed > 0) {
        this->notifyObserversOnUnRegister(writer);
    }

    return removed > 0;
}

std::unique_ptr<DataWriter> DataWriterFactory::create(std::string_view key) const {
    for (auto& elem : map_) {
        if (iCaseCmp(elem.first.extension_, key)) {
            return std::unique_ptr<DataWriter>(elem.second->clone());
        }
    }
    return nullptr;
}

std::unique_ptr<DataWriter> DataWriterFactory::create(const FileExtension& key) const {
    return std::unique_ptr<DataWriter>(
        util::map_find_or_null(map_, key, [](DataWriter* o) { return o->clone(); }));
}

bool DataWriterFactory::hasKey(const FileExtension& key) const { return util::has_key(map_, key); }

bool DataWriterFactory::hasKey(std::string_view key) const {
    for (auto& elem : map_) {
        if (iCaseCmp(elem.first.extension_, key)) return true;
    }
    return false;
}

}  // namespace inviwo
