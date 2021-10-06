/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2021 Inviwo Foundation
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

#include <modules/brushingandlinking/brushingandlinkingmoduledefine.h>
#include <modules/brushingandlinking/datastructures/indexlist.h>
#include <inviwo/core/properties/invalidationlevel.h>
#include <inviwo/core/datastructures/bitset.h>

#include <unordered_set>

namespace inviwo {

class BrushingAndLinkingInport;
class Processor;
class Serializer;
class Deserializer;

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

    size_t getNumberOfHighlighted() const;

    /*
     * Return the number of selected columns.
     */
    size_t getNumberOfSelectedColumns() const;

    void remove(const BrushingAndLinkingInport* src);

    bool isFiltered(uint32_t idx) const;
    bool isSelected(uint32_t idx) const;
    bool isHighlighted(uint32_t idx) const;

    bool isColumnSelected(uint32_t column) const;

    void setSelected(const BrushingAndLinkingInport* src, const BitSet& idx);
    void clearSelected();

    void setFiltered(const BrushingAndLinkingInport* src, const BitSet& idx);
    void clearFiltered();

    void setHighlighted(const BrushingAndLinkingInport* src, const BitSet& idx);
    void clearHighlighted();

    void setSelectedColumn(const BrushingAndLinkingInport* src, const BitSet& columnIndices);
    void clearColumns();

    const BitSet& getSelectedIndices() const;
    const BitSet& getFilteredIndices() const;
    const BitSet& getHighlightedIndices() const;
    const BitSet& getSelectedColumns() const;

    void serialize(Serializer& s) const;
    void deserialize(Deserializer& d, const BrushingAndLinkingOutport& port);

private:
    BitSet selected_;
    BitSet highlighted_;
    BitSet selectedColumns_;
    IndexList filtered_;  // Use IndexList to be able to remove filtered rows on port disconnection
    std::shared_ptr<std::function<void()>> onFilteringChangeCallback_;

    Processor* owner_;  // Non-owning reference
    InvalidationLevel invalidationLevel_;
};

inline bool BrushingAndLinkingManager::isFiltered(uint32_t idx) const {
    return filtered_.contains(idx);
}

inline bool BrushingAndLinkingManager::isSelected(uint32_t idx) const {
    return selected_.contains(idx);
}

inline bool BrushingAndLinkingManager::isHighlighted(uint32_t idx) const {
    return highlighted_.contains(idx);
}

}  // namespace inviwo
