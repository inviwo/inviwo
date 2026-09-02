/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <modules/hdf5/properties/dimselectionsproperty.h>
#include <modules/hdf5/hdf5utils.h>

#include <inviwo/core/network/networklock.h>
#include <inviwo/core/util/zip.h>

#include <algorithm>

#include <fmt/format.h>

namespace inviwo::hdf5 {

std::string_view DimSelectionsProperty::getClassIdentifier() const { return classIdentifier; }

DimSelectionsProperty::DimSelectionsProperty(std::string_view identifier,
                                             std::string_view displayName, size_t maxRank,
                                             InvalidationLevel invalidationLevel)
    : CompositeProperty(identifier, displayName, invalidationLevel)
    , maxRank_(maxRank)
    , rank_(maxRank) {

    constexpr char last = 'Z';
    for (size_t i = 0; i < maxRank_; ++i) {
        const auto ind = fmt::to_string(static_cast<char>(last - maxRank_ + i + 1));
        selection_.push_back(std::make_unique<DimSelectionProperty>("dim" + ind, ind));
        addProperty(selection_[i].get(), false);
    }
}

DimSelectionsProperty::DimSelectionsProperty(const DimSelectionsProperty& rhs)
    : CompositeProperty(rhs), maxRank_(rhs.maxRank_), rank_(rhs.rank_) {

    for (const auto& sel : rhs.selection_) {
        selection_.push_back(std::unique_ptr<DimSelectionProperty>(sel->clone()));
        addProperty(selection_.back().get(), false);
    }
}

DimSelectionsProperty* DimSelectionsProperty::clone() const {
    return new DimSelectionsProperty(*this);
}

void DimSelectionsProperty::update(const DataSetInfo& dataSetInfo) {
    const NetworkLock lock{this};
    const auto cmdims = dataSetInfo.getColumnMajorDimensions();
    rank_ = std::min(cmdims.size(), maxRank_);

    for (auto&& [index, selection] : inviwo::util::enumerate(selection_)) {
        selection->setVisible(index >= maxRank_ - rank_);
    }

    for (size_t i = 0; i < rank_; ++i) {
        selection_[i + maxRank_ - rank_]->update(cmdims[i]);
    }
}

std::vector<Selection> DimSelectionsProperty::getSelection() const {
    std::vector<Selection> selection;
    for (size_t i = maxRank_ - rank_; i < maxRank_; ++i) {
        selection.push_back(selection_[i]->getSelection());
    }
    return selection;
}

}  // namespace inviwo::hdf5
