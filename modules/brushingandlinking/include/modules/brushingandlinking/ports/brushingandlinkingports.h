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

#ifndef IVW_BRUSHINGANDLINKINGOUTPORT_H
#define IVW_BRUSHINGANDLINKINGOUTPORT_H

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/ports/datainport.h>
#include <inviwo/core/ports/dataoutport.h>
#include <inviwo/core/ports/port.h>
#include <modules/brushingandlinking/brushingandlinkingmanager.h>
#include <modules/brushingandlinking/brushingandlinkingmoduledefine.h>
#include <modules/brushingandlinking/events/filteringevent.h>
#include <modules/brushingandlinking/events/hoverevent.h>
#include <modules/brushingandlinking/events/selectionevent.h>
#include <modules/brushingandlinking/events/columnselectionevent.h>
#include <inviwo/core/datastructures/datatraits.h>

namespace inviwo {

class IVW_MODULE_BRUSHINGANDLINKING_API BrushingAndLinkingInport
    : public DataInport<BrushingAndLinkingManager> {
public:
    BrushingAndLinkingInport(std::string identifier);
    virtual ~BrushingAndLinkingInport() = default;

    void sendFilterEvent(const std::unordered_set<size_t> &indices);

    void sendSelectionEvent(const std::unordered_set<size_t> &indices, bool append = false);

    void sendHoverEvent(const std::unordered_set<size_t> &indices);

    void sendColumnSelectionEvent(const std::unordered_set<size_t> &indices);

    bool isFiltered(size_t idx) const;
    bool isSelected(size_t idx) const;
    bool isHovered(size_t idx) const;

    bool isColumnSelected(size_t idx) const;

    const std::unordered_set<size_t> &getSelectedIndices() const;
    const std::unordered_set<size_t> &getHoveredIndices() const;
    const std::unordered_set<size_t> &getFilteredIndices() const;
    const std::unordered_set<size_t> &getSelectedColumns() const;

    virtual std::string getClassIdentifier() const override;

    std::unordered_set<size_t> filterCache_;
    std::unordered_set<size_t> hoverCache_;
    std::unordered_set<size_t> selectionCache_;
    std::unordered_set<size_t> selectionColumnCache_;
};

class IVW_MODULE_BRUSHINGANDLINKING_API BrushingAndLinkingOutport
    : public DataOutport<BrushingAndLinkingManager> {
public:
    BrushingAndLinkingOutport(std::string identifier);
    virtual ~BrushingAndLinkingOutport() = default;

    virtual std::string getClassIdentifier() const override;
};

template <>
struct PortTraits<BrushingAndLinkingInport> {
    static std::string classIdentifier() { return "BrushingAndLinkingInport"; }
};

template <>
struct PortTraits<BrushingAndLinkingOutport> {
    static std::string classIdentifier() { return "BrushingAndLinkingOutport"; }
};

template <>
struct DataTraits<BrushingAndLinkingManager> {
    static std::string classIdentifier() { return "BrushingAndLinkingManager"; }
    static std::string dataName() { return "BrushingAndLinkingManager"; }
    static uvec3 colorCode() { return uvec3(160, 182, 240); }
    static Document info(const BrushingAndLinkingManager &data) {
        Document doc;
        std::ostringstream oss;
        oss << "Number of selected indices: " << data.getNumberOfSelected() << std::endl;
        oss << "Number of hovered indices: " << data.getNumberOfHovered() << std::endl;
        oss << "Number of filtered indices: " << data.getNumberOfFiltered();
        doc.append("p", oss.str());
        return doc;
    }
};


inline bool BrushingAndLinkingInport::isFiltered(size_t idx) const {
    if (isConnected()) {
        return getData()->isFiltered(idx);
    } else {
        return filterCache_.find(idx) != filterCache_.end();
    }
}

inline bool BrushingAndLinkingInport::isSelected(size_t idx) const {
    if (isConnected()) {
        return getData()->isSelected(idx);
    } else {
        return selectionCache_.find(idx) != selectionCache_.end();
    }
}

inline bool BrushingAndLinkingInport::isHovered(size_t idx) const {
    if (isConnected()) {
        return getData()->isHovered(idx);
    } else {
        return hoverCache_.find(idx) != hoverCache_.end();
    }
}


}  // namespace inviwo

#endif  // IVW_BRUSHINGANDLINKINGOUTPORT_H
