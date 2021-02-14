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
#include <inviwo/core/ports/inport.h>
#include <inviwo/core/ports/outport.h>
#include <inviwo/core/ports/porttraits.h>

#include <modules/brushingandlinking/brushingandlinkingmanager.h>
#include <modules/brushingandlinking/datastructures/brushingaction.h>

namespace inviwo {

class BrushingAndLinkingOutport;

class IVW_MODULE_BRUSHINGANDLINKING_API BrushingAndLinkingInport : public Inport {
public:
    using type = void;

    BrushingAndLinkingInport(std::string identifier);
    virtual ~BrushingAndLinkingInport() = default;

    /**
     * check if the state of the manager was changed since the last network evaluation
     *
     * @return true if there have been recent changes
     *
     * \see getModifiedActions
     */
    virtual bool isChanged() const override;

    /**
     * return which actions were performed since the last network evaluation
     */
    BrushingModifications getModifiedActions() const;
    /**
     * return whether there was a filter action since the last network evaluation
     *
     * @return getModifiedActions() & BrushingAction::Filter
     */
    bool isFilteringModified() const;
    /**
     * return whether there was a select action since the last network evaluation
     *
     * @return getModifiedActions() & BrushingAction::Select
     */
    bool isSelectionModified() const;
    /**
     * return whether there was a highlight action since the last network evaluation
     *
     * @return getModifiedActions() & BrushingAction::Highlight
     */
    bool isHighlightModified() const;

    /**
     * Based on \p action update the internal selection with the given \p indices. For \p target
     * matching BrushingAction::Select or BrushingAction::Highlight, the indices will replace the
     * previous selection.
     *
     * In case of BrushingAction::Filter, a \p source must be provided. The indices are subsequently
     * marked as removed. Note that multiple filter actions might overlap hence the need for a
     * source.
     *
     * @param action   type of brushing action
     * @param target   target of the action, determines which brushing and linking state to update
     * @param indices  set of selected/filtered indices
     * @param source   must be provided if action is equal to BrushingAction::Filter
     *
     * @throw Exception if action is BrushingAction::Filter and no source is given
     *
     * \see BrushingAndLinkingManager::brush
     */
    void brush(BrushingAction action, BrushingTarget target, const BitSet& indices,
               std::string_view source = {});

    void filter(std::string_view source, const BitSet& indices,
                BrushingTarget target = BrushingTarget::Row);
    void select(const BitSet& indices, BrushingTarget target = BrushingTarget::Row);
    void highlight(const BitSet& indices, BrushingTarget target = BrushingTarget::Row);

    // clang-format off
    [[deprecated("use brush() or filter() instead")]] void sendFilterEvent(const BitSet& indices, std::string_view source);
    [[deprecated("use brush() or select() instead")]] void sendSelectionEvent(const BitSet& indices);
    [[deprecated("use brush() or select() with a column target instead")]] void sendColumnSelectionEvent(const BitSet& indices);
    // clang-format on

    bool isFiltered(uint32_t idx, BrushingTarget target = BrushingTarget::Row) const;
    bool isSelected(uint32_t idx, BrushingTarget target = BrushingTarget::Row) const;
    bool isHighlighted(uint32_t idx, BrushingTarget target = BrushingTarget::Row) const;

    // clang-format off
    [[deprecated("use isSelected() with a column target instead")]] bool isColumnSelected(uint32_t idx) const;
    // clang-format on

    const BitSet& getIndices(BrushingAction action,
                             BrushingTarget target = BrushingTarget::Row) const;

    const BitSet& getFilteredIndices(BrushingTarget target = BrushingTarget::Row) const;
    const BitSet& getSelectedIndices(BrushingTarget target = BrushingTarget::Row) const;
    const BitSet& getHighlightedIndices(BrushingTarget target = BrushingTarget::Row) const;

    // clang-format off
    [[deprecated("use getIndices() or getSelectedIndices() with a column target instead")]] const BitSet& getSelectedColumns() const;
    // clang-format on

    BrushingAndLinkingManager& getManager();
    const BrushingAndLinkingManager& getManager() const;

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

    virtual bool canConnectTo(const Port* port) const override;
    virtual size_t getMaxNumberOfConnections() const override { return 1; }

    virtual std::string getClassIdentifier() const override;
    virtual glm::uvec3 getColorCode() const override { return uvec3(160, 182, 240); }
    virtual Document getInfo() const override;

private:
    virtual void setChanged(bool changed = true, const Outport* source = nullptr) override;

    BrushingAndLinkingManager manager_;
};

class IVW_MODULE_BRUSHINGANDLINKING_API BrushingAndLinkingOutport : public Outport {
public:
    using type = void;

    BrushingAndLinkingOutport(std::string identifier);
    virtual ~BrushingAndLinkingOutport() = default;

    BrushingAndLinkingManager& getManager();
    const BrushingAndLinkingManager& getManager() const;

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

    virtual void setValid() override;

    virtual bool hasData() const override { return false; }
    virtual void clear() override {}

    virtual glm::uvec3 getColorCode() const override { return uvec3(160, 182, 240); }
    virtual Document getInfo() const override;

    virtual std::string getClassIdentifier() const override;

private:
    BrushingAndLinkingManager manager_;
};

template <>
struct PortTraits<BrushingAndLinkingInport> {
    static std::string classIdentifier() { return "BrushingAndLinkingInport"; }
};

template <>
struct PortTraits<BrushingAndLinkingOutport> {
    static std::string classIdentifier() { return "BrushingAndLinkingOutport"; }
};

}  // namespace inviwo
