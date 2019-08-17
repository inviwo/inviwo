/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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

#ifndef IVW_BRUSHINGANDLINKINGMANAGER_H
#define IVW_BRUSHINGANDLINKINGMANAGER_H

#include <inviwo/core/common/inviwo.h>
#include <modules/brushingandlinking/brushingandlinkingmoduledefine.h>
#include <modules/brushingandlinking/datastructures/indexlist.h>
#include <inviwo/core/properties/invalidationlevel.h>

namespace inviwo {
class BrushingAndLinkingInport;
class BrushingAndLinkingProcessor;
/**
 * \class BrushingAndLinkingManager
 * \brief Manages row filtering, row selection and column selection from multiple sources.
 */
class IVW_MODULE_BRUSHINGANDLINKING_API BrushingAndLinkingManager {
public:
    BrushingAndLinkingManager(Processor* p,
                              InvalidationLevel validationLevel = InvalidationLevel::InvalidOutput);
    virtual ~BrushingAndLinkingManager();
    /*
     * Return the number of selected items/rows.
     */
    size_t getNumberOfSelected() const;
    /*
     * Return the number of filtered items/rows, i.e. the number of items/rows that should not be
     * displayed.
     */
    size_t getNumberOfFiltered() const;

    void remove(const BrushingAndLinkingInport* src);

    bool isFiltered(size_t idx) const;
    bool isSelected(size_t idx) const;

    bool isColumnSelected(size_t column) const;

    void setSelected(const BrushingAndLinkingInport* src, const std::unordered_set<size_t>& idx);

    void setFiltered(const BrushingAndLinkingInport* src, const std::unordered_set<size_t>& idx);

    void setSelectedColumn(const BrushingAndLinkingInport* src,
                           const std::unordered_set<size_t>& columnIndices);

    const std::unordered_set<size_t>& getSelectedIndices() const;
    const std::unordered_set<size_t>& getFilteredIndices() const;
    const std::unordered_set<size_t>& getSelectedColumns() const;

private:
    std::unordered_set<size_t> selected_;
    std::unordered_set<size_t> selectedColumns_;
    IndexList filtered_;  // Use IndexList to be able to remove filtered rows on port disconnection
    std::shared_ptr<std::function<void()>> onFilteringChangeCallback_;

    Processor* owner_;  // Non-owning reference
    InvalidationLevel invalidationLevel_;
};

inline bool BrushingAndLinkingManager::isFiltered(size_t idx) const { return filtered_.has(idx); }

inline bool BrushingAndLinkingManager::isSelected(size_t idx) const {
    return selected_.find(idx) != selected_.end();
}

}  // namespace inviwo

#endif  // IVW_BRUSHINGANDLINKINGMANAGER_H
