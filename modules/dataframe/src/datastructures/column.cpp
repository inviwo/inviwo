/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
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

#include <inviwo/dataframe/datastructures/column.h>

namespace inviwo {

CategoricalColumn::CategoricalColumn(const std::string &header)
    : TemplateColumn<std::uint32_t>(header) {}

CategoricalColumn *CategoricalColumn::clone() const { return new CategoricalColumn(*this); }

std::string CategoricalColumn::getAsString(size_t idx) const {
    auto index = getTypedBuffer()->getRAMRepresentation()->getDataContainer()[idx];
    return lookUpTable_[index];
}

std::shared_ptr<DataPointBase> CategoricalColumn::get(size_t idx, bool getStringsAsStrings) const {
    if (getStringsAsStrings) {
        return std::make_shared<DataPoint<std::string>>(getAsString(idx));
    } else {
        return TemplateColumn<std::uint32_t>::get(idx, getStringsAsStrings);
    }
}

void CategoricalColumn::set(size_t idx, const std::string &str) {
    auto id = addOrGetID(str);
    getTypedBuffer()->getEditableRAMRepresentation()->set(idx, id);
}

void CategoricalColumn::add(const std::string &value) {
    auto id = addOrGetID(value);
    getTypedBuffer()->getEditableRAMRepresentation()->add(id);
}

glm::uint32_t CategoricalColumn::addOrGetID(const std::string &str) {
    auto it = std::find(lookUpTable_.begin(), lookUpTable_.end(), str);
    if (it != lookUpTable_.end()) {
        return static_cast<glm::uint32_t>(std::distance(lookUpTable_.begin(), it));
    }
    lookUpTable_.push_back(str);
    return static_cast<glm::uint32_t>(lookUpTable_.size() - 1);
}

}  // namespace inviwo
