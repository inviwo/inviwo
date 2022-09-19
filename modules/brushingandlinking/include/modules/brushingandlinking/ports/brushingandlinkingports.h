/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2022 Inviwo Foundation
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

#include <modules/brushingandlinking/brushingandlinkingmoduledefine.h>  // for IVW_MODULE_BRUSHI...

#include <inviwo/core/datastructures/bitset.h>                         // for BitSet
#include <inviwo/core/ports/inport.h>                                  // for Inport
#include <inviwo/core/ports/outport.h>                                 // for Outport
#include <inviwo/core/ports/porttraits.h>                              // for PortTraits
#include <inviwo/core/properties/invalidationlevel.h>                  // for InvalidationLevel
#include <inviwo/core/util/document.h>                                 // for Document
#include <inviwo/core/util/glmvec.h>                                   // for uvec3
#include <modules/brushingandlinking/brushingandlinkingmanager.h>      // for BrushingTargetsIn...
#include <modules/brushingandlinking/datastructures/brushingaction.h>  // for BrushingTarget

#include <cstddef>      // for size_t
#include <cstdint>      // for uint32_t
#include <string>       // for string
#include <string_view>  // for string_view
#include <vector>       // for vector

#include <flags/flags.h>             // for any
#include <glm/ext/vector_uint3.hpp>  // for uvec3

namespace inviwo {
class Deserializer;
class Port;
class Serializer;

/**
 * \ingroup ports
 * Enables selection/filtering/highlighting between processors.
 * The inport has it's own BrushingManager and therefore does
 * not need to be connected to a BrushingAndLinkingOutport to be valid.
 *
 * Use setInvalidationLevels if you only want Processor::process to be called for a subset of
 * brushing targets or actions. Use getModifiedActions if you want to know which
 * BrushingModifications caused a Processor::process call.
 * @see BrushingAndLinkingManager
 */
class IVW_MODULE_BRUSHINGANDLINKING_API BrushingAndLinkingInport : public Inport {
public:
    friend class BrushingAndLinkingManager;  // Allow calls to invalidate during brushing
                                             // propagation
    using type = void;
    /**
     * Inport constructor for BrushingAndLinkingManager.
     *
     * @code
     *  // Only invalidate processor on row filtering and column selection.
     *  BrushingAndLinkingInport("brushing",
     *  {
     *    {{BrushingTarget::Row}, BrushingModification::Filtered, InvalidationLevel::InvalidOutput},
     *    {{BrushingTarget::Column}, BrushingModification::Selected,
     * InvalidationLevel::InvalidOutput}
     *  }
     *  );
     *  // Invalidate processor on filtering or selection for any target.
     *  BrushingAndLinkingInport("brushing",
     *  {
     *    {BrushingAndLinkingManager::AnyBrushingTarget, BrushingModification::Filtered |
     * BrushingModification::Selected, InvalidationLevel::InvalidOutput}
     *  }
     *  );
     * @param identifier of port.
     * @param invalidationLevels for brushing targets and actions that should invalidate the
     * processor (Processor::process will be called for those). Defaults to InvalidOutput for all
     * targets (Row, Column) and all actions (filtering/selection/highlight).
     */
    BrushingAndLinkingInport(std::string_view identifier,
                             std::vector<BrushingTargetsInvalidationLevel> invalidationLevels = {
                                 {BrushingAndLinkingManager::AnyBrushingTarget,
                                  BrushingModifications(flags::any),
                                  InvalidationLevel::InvalidOutput}});
    BrushingAndLinkingInport(std::string_view identifier, Document help,
                             std::vector<BrushingTargetsInvalidationLevel> invalidationLevels = {
                                 {BrushingAndLinkingManager::AnyBrushingTarget,
                                  BrushingModifications(flags::any),
                                  InvalidationLevel::InvalidOutput}});
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
     * @see isFilteringModified
     * @see isSelectionModified
     * @see isHighlightModified
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

    /**
     * Returns the types of targets and actions causing the owning processor to invalidate.
     */
    const std::vector<BrushingTargetsInvalidationLevel>& getInvalidationLevels() const;

    /**
     * Set the types of brushing targets and actions that should invalidate the owning processor.
     * Enables processors to only handle certain types brushing targets (row, column) and actions
     * (filter/selection/highlight).
     * @code
     *  // Only invalidate processor on row filtering and column selection.
     *  setInvalidationLevels(
     *  {
     *    {{BrushingTarget::Row}, BrushingModification::Filtered, InvalidationLevel::InvalidOutput},
     *    {{BrushingTarget::Column}, BrushingModification::Selected,
     * InvalidationLevel::InvalidOutput}
     *  }
     *  );
     *  // Invalidate processor on filtering or selection for any target.
     *  setInvalidationLevels(
     *  {
     *    {BrushingAndLinkingManager::AnyBrushingTarget, BrushingModification::Filtered |
     * BrushingModification::Selected, InvalidationLevel::InvalidOutput}
     *  }
     *  );
     * @endcode
     */
    void setInvalidationLevels(std::vector<BrushingTargetsInvalidationLevel> invalidationLevels);

    BrushingAndLinkingManager& getManager();
    const BrushingAndLinkingManager& getManager() const;

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

    virtual bool canConnectTo(const Port* port) const override;
    virtual size_t getMaxNumberOfConnections() const override { return 1; }

    virtual std::string getClassIdentifier() const override;
    virtual glm::uvec3 getColorCode() const override { return uvec3(160, 182, 240); }
    virtual Document getInfo() const override;

protected:
    virtual void invalidate(InvalidationLevel invalidationLevel) override;

private:
    virtual void setChanged(bool changed = true, const Outport* source = nullptr) override;

    BrushingAndLinkingManager manager_;
};
/**
 * \ingroup ports
 * Enables selection/filtering/highlighting between processors.
 * The BrushingAndLinkingOutport can be connected to several BrushingAndLinkingInports in order to
 * share information.
 * @see BrushingAndLinkingManager
 */
class IVW_MODULE_BRUSHINGANDLINKING_API BrushingAndLinkingOutport : public Outport {
public:
    using type = void;
    BrushingAndLinkingOutport(std::string_view identifier, Document help = {});
    virtual ~BrushingAndLinkingOutport() = default;

    BrushingAndLinkingManager& getManager();
    const BrushingAndLinkingManager& getManager() const;

    /**
     * Overrides the provided invalidationLevel with that of the BrushingAngLinkingManager.
     * Will invalidate its connected inports if any of the BrushingModifications overlap with
     * the modified brushing targets and actions provided by getInvalidationLevels
     * @param invalidationLevel unused!
     * @note Port is set to valid after its processor successfully finished processing.
     */
    virtual void invalidate(InvalidationLevel invalidationLevel) override;

    /**
     * Returns the types of targets and actions causing the owning processor to invalidate.
     */
    const std::vector<BrushingTargetsInvalidationLevel>& getInvalidationLevels() const;

    /**
     * Set the types of brushing targets and actions that should invalidate the owning processor.
     * Enables processors to only handle certain types of filter/selection/highlight actions.
     * @code
     *  // Only invalidate processor on row filtering and column selection.
     *  setInvalidationLevels(
     *  {
     *    {{BrushingTarget::Row}, BrushingModification::Filtered, InvalidationLevel::InvalidOutput},
     *    {{BrushingTarget::Column}, BrushingModification::Selected,
     * InvalidationLevel::InvalidOutput}
     *  }
     *  );
     *  // Invalidate processor on any type of action for filtering or column selection.
     *  setInvalidationLevels(
     *  {
     *    {BrushingModification::Filtered | BrushingModification::Selected,
     * InvalidationLevel::InvalidOutput}
     *  }
     *  );
     * @endcode
     */
    void setInvalidationLevels(std::vector<BrushingTargetsInvalidationLevel> invalidationLevels);

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
