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
#include <inviwo/core/properties/invalidationlevel.h>
#include <inviwo/core/datastructures/bitset.h>
#include <inviwo/core/io/serialization/serializable.h>

#include <modules/brushingandlinking/datastructures/indexlist.h>
#include <modules/brushingandlinking/datastructures/brushingaction.h>

#include <variant>
#include <array>
#include <unordered_set>

namespace inviwo {

class BrushingAndLinkingInport;
class BrushingAndLinkingOutport;
class Processor;
class Serializer;
class Deserializer;

/**
 * Manages brushing and linking events for filtering, selecting, and highlighting. When initialized
 * with a BrushingAndLinking inport, changes are propagated using this port if connected.
 */
class IVW_MODULE_BRUSHINGANDLINKING_API BrushingAndLinkingManager : public Serializable {
public:
    BrushingAndLinkingManager(BrushingAndLinkingInport* inport,
                              InvalidationLevel validationLevel = InvalidationLevel::InvalidOutput);
    BrushingAndLinkingManager(BrushingAndLinkingOutport* outport,
                              InvalidationLevel validationLevel = InvalidationLevel::InvalidOutput);
    virtual ~BrushingAndLinkingManager();

    /**
     * Based on \p action update the internal selection with the given \p indices. For \p target
     * matching BrushingAction::Select or BrushingAction::Highlight, the indices will replace the
     * previous selection.
     *
     * In case of BrushingAction::Filter, a \p source must be provided. The indices are subsequently
     * marked as removed. Note that multiple filter actions might overlap hence the need for a
     * source.
     *
     * For example, given indices {1, ..., 10} and two filter actions.
     *   + filter action A filters 1-5
     *   + filter action B filters 3-6
     * This results in indices {7, 8, 9, 10} remaining. If then action A is updated to filter out
     * indices 2-4, the overall result for filtering should be {1, 7, ..., 10}. Without different
     * sources it would not be possible to update the result accordingly.
     *
     * @param action   type of brushing action
     * @param target   target of the action, determines which brushing and linking state to update
     * @param indices  set of selected/filtered indices
     * @param source   must be provided if action is equal to BrushingAction::Filter
     *
     * @throw Exception if action is BrushingAction::Filter and no source is given
     *
     * \see BrushingAction
     */
    void brush(BrushingAction action, BrushingTarget target, const BitSet& indices,
               std::string_view source = {});

    //! convenience function for brush(BrushingAction::Filter, target, idx, source)
    void filter(const BitSet& idx, BrushingTarget target, std::string_view source);
    //! convenience function for brush(BrushingAction::Select, target, idx)
    void select(const BitSet& idx, BrushingTarget target = BrushingTarget::Row);
    //! convenience function for brush(BrushingAction::Highlight, target, idx)
    void highlight(const BitSet& idx, BrushingTarget target = BrushingTarget::Row);

    /**
     * check if the state of the manager was changed since the last network evaluation
     *
     * @return true if there have been recent changes
     *
     * \see getModifiedActions
     */
    bool isModified() const;

    /**
     * return which actions were performed since the last network evaluation
     */
    BrushingModifications getModifiedActions() const;
    /**
     * return whether there was a filter action on \p target since the last network evaluation
     *
     * @return isTargetModified(target, BrushingModification::Filtered)
     */
    bool isFilteringModified(BrushingTarget target = BrushingTarget::Row) const;
    /**
     * return whether there was a select action on \p target since the last network evaluation
     *
     * @return isTargetModified(target, BrushingModification::Selected)
     */
    bool isSelectionModified(BrushingTarget target = BrushingTarget::Row) const;
    /**
     * return whether there was a highlight action on \p target since the last network evaluation
     *
     * @return isTargetModified(target, BrushingModification::Highlighted)
     */
    bool isHighlightModified(BrushingTarget target = BrushingTarget::Row) const;
    /**
     * return which targets were changed since the last network evaluation
     */
    std::vector<BrushingTarget> getModifiedTargets() const;
    /**
     * return whether \p target was modified by any of \p modifications since the last network
     * evaluation
     */
    bool isTargetModified(BrushingTarget target, BrushingModifications modifications =
                                                     BrushingModifications(flags::any)) const;
    /**
     * return whether \p target was modified by \p action since the last network evaluation
     */
    bool isTargetModified(BrushingTarget target, BrushingAction action) const;

    /**
     * check whether the manager has an index set for \p target and \p action
     *
     * @param action   type of brushing action
     * @param target   target of the action
     * @return true if indices exist
     */
    bool hasIndices(BrushingAction action, BrushingTarget target = BrushingTarget::Row) const;

    /**
     * access indices for the given combination of \p action and \p target
     *
     * @param action    type of brushing action
     * @param target    target of the action
     * @return bitset corresponding to \p action and \p target
     * @throw Exception if \p target does not exist for \p action
     *
     * \see hasIndices
     */
    const BitSet& getIndices(BrushingAction action,
                             BrushingTarget target = BrushingTarget::Row) const;

    //! convenience function for getIndices(action, target).size()
    size_t getNumber(BrushingAction action, BrushingTarget target = BrushingTarget::Row) const;
    //! convenience function for getIndices(BrushingAction::Filter, target).size()
    size_t getNumberOfFiltered(BrushingTarget target = BrushingTarget::Row) const;
    //! convenience function for getIndices(BrushingAction::Select, target).size()
    size_t getNumberOfSelected(BrushingTarget target = BrushingTarget::Row) const;
    //! convenience function for getIndices(BrushingAction::Highlight, target).size()
    size_t getNumberOfHighlighted(BrushingTarget target = BrushingTarget::Row) const;

    /**
     * clear the selection for \p action and \p target. \p action must be different from
     * BrushingAction::Filter. Does nothing if \p target does not exist.
     *
     * @throw Exception if action is equal to BrushingAction::Filter since this may lead to an
     * inconsistent state
     */
    void clearIndices(BrushingAction action, BrushingTarget target);

    // clang-format off
    [[deprecated("clearing filtered indices is no longer supported. Use filter() with an empty BitSet")]] void clearFiltered();
    // clang-format on
    //! convenience function for clearIndices(BrushingAction::Select, target)
    void clearSelected(BrushingTarget target = BrushingTarget::Row);
    //! convenience function for clearIndices(BrushingAction::Highlight, target)
    void clearHighlighted(BrushingTarget target = BrushingTarget::Row);

    /**
     * check whether the selection for \p action and \p target contains index \p idx
     */
    bool contains(uint32_t idx, BrushingAction action,
                  BrushingTarget target = BrushingTarget::Row) const;

    //! convenience function for contains(idx, BrushingAction::Filter, target)
    //! \see contains
    bool isFiltered(uint32_t idx, BrushingTarget target = BrushingTarget::Row) const;
    //! convenience function for contains(idx, BrushingAction::Select, target)
    //!\see contains
    bool isSelected(uint32_t idx, BrushingTarget target = BrushingTarget::Row) const;
    //! convenience function for contains(idx, BrushingAction::Highlight, target)
    //! \see contains
    bool isHighlighted(uint32_t idx, BrushingTarget target = BrushingTarget::Row) const;

    std::vector<std::pair<BrushingAction, std::vector<BrushingTarget>>> getTargets() const;
    std::vector<BrushingTarget> getTargets(BrushingAction action) const;

    const BitSet& getFilteredIndices(BrushingTarget target = BrushingTarget::Row) const;
    const BitSet& getSelectedIndices(BrushingTarget target = BrushingTarget::Row) const;
    const BitSet& getHighlightedIndices(BrushingTarget target = BrushingTarget::Row) const;

    /**
     * register a parent manager for the propagation of brushing actions
     */
    void setParent(BrushingAndLinkingManager* parent);

    /**
     * propagates the modified state to all child managers and resets the state. Should only be
     * called by the brushing and linking ports _after_ the process() function has been called.
     */
    void markAsValid();

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

private:
    static int getActionIndex(BrushingAction action);
    void propagate(BrushingAction action, BrushingTarget target);
    void propagate(BrushingAction action, const std::vector<BrushingTarget>& targets);
    void addChild(BrushingAndLinkingManager* child);
    void removeChild(BrushingAndLinkingManager* child);
    const BitSet* getBitSet(BrushingAction action, BrushingTarget target) const;

    using BitSetTargets = std::unordered_map<BrushingTarget, BitSet>;
    using IndexListTargets = std::unordered_map<BrushingTarget, IndexList>;

    using SelectionMap = std::array<std::variant<BitSetTargets, IndexListTargets>,
                                    static_cast<size_t>(BrushingAction::NumberOfActions)>;

    SelectionMap selections_{{IndexListTargets(), BitSetTargets(), BitSetTargets()}};

    std::variant<BrushingAndLinkingInport*, BrushingAndLinkingOutport*> owner_;
    BrushingAndLinkingManager* parent_ = nullptr;
    std::unordered_set<BrushingAndLinkingManager*> children_;
    InvalidationLevel invalidationLevel_;

    std::unordered_map<BrushingTarget, BrushingModifications> modifications_;
};

}  // namespace inviwo
