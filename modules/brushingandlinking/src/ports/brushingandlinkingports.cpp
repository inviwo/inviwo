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

#include <modules/brushingandlinking/ports/brushingandlinkingports.h>

#include <inviwo/core/datastructures/bitset.h>                         // for BitSet
#include <inviwo/core/io/serialization/deserializer.h>                 // for Deserializer
#include <inviwo/core/io/serialization/serializer.h>                   // for Serializer
#include <inviwo/core/network/networkutils.h>                          // for getPredecessors
#include <inviwo/core/ports/inport.h>                                  // for Inport
#include <inviwo/core/ports/outport.h>                                 // for Outport
#include <inviwo/core/ports/port.h>                                    // for Port
#include <inviwo/core/properties/invalidationlevel.h>                  // for InvalidationLevel
#include <inviwo/core/util/document.h>                                 // for Document, TableBui...
#include <inviwo/core/util/statecoordinator.h>                         // for StateCoordinator
#include <modules/brushingandlinking/brushingandlinkingmanager.h>      // for BrushingAndLinking...
#include <modules/brushingandlinking/datastructures/brushingaction.h>  // for BrushingAction

#include <functional>     // for __base
#include <unordered_map>  // for operator!=
#include <unordered_set>  // for unordered_set, uno...
#include <utility>        // for move, pair, tuple_...

namespace inviwo {

BrushingAndLinkingInport::BrushingAndLinkingInport(
    std::string_view identifier, std::vector<BrushingTargetsInvalidationLevel> invalidationLevels)
    : Inport(identifier, Document{}), manager_(this, invalidationLevels) {
    setOptional(true);
}
BrushingAndLinkingInport::BrushingAndLinkingInport(
    std::string_view identifier, Document help,
    std::vector<BrushingTargetsInvalidationLevel> invalidationLevels)
    : Inport(identifier, Document{std::move(help)}), manager_(this, invalidationLevels) {
    setOptional(true);
}

bool BrushingAndLinkingInport::isChanged() const { return manager_.isModified(); }

BrushingModifications BrushingAndLinkingInport::getModifiedActions() const {
    return manager_.getModifiedActions();
}

bool BrushingAndLinkingInport::isFilteringModified() const {
    return manager_.isFilteringModified();
}

bool BrushingAndLinkingInport::isSelectionModified() const {
    return manager_.isSelectionModified();
}

bool BrushingAndLinkingInport::isHighlightModified() const {
    return manager_.isHighlightModified();
}

void BrushingAndLinkingInport::brush(BrushingAction action, BrushingTarget target,
                                     const BitSet& indices, std::string_view source) {
    manager_.brush(action, target, indices, source);
}

void BrushingAndLinkingInport::filter(std::string_view source, const BitSet& indices,
                                      BrushingTarget target) {
    manager_.brush(BrushingAction::Filter, target, indices, source);
}

void BrushingAndLinkingInport::select(const BitSet& indices, BrushingTarget target) {
    manager_.brush(BrushingAction::Select, target, indices);
}

void BrushingAndLinkingInport::highlight(const BitSet& indices, BrushingTarget target) {
    manager_.brush(BrushingAction::Highlight, target, indices);
}

void BrushingAndLinkingInport::sendFilterEvent(const BitSet& indices, std::string_view source) {
    manager_.brush(BrushingAction::Filter, BrushingTarget::Row, indices, source);
}

void BrushingAndLinkingInport::sendSelectionEvent(const BitSet& indices) {
    manager_.brush(BrushingAction::Select, BrushingTarget::Row, indices);
}

void BrushingAndLinkingInport::sendColumnSelectionEvent(const BitSet& indices) {
    manager_.brush(BrushingAction::Select, BrushingTarget::Column, indices);
}

bool BrushingAndLinkingInport::isFiltered(uint32_t idx, BrushingTarget target) const {
    return manager_.contains(idx, BrushingAction::Filter, target);
}

bool BrushingAndLinkingInport::isSelected(uint32_t idx, BrushingTarget target) const {
    return manager_.contains(idx, BrushingAction::Select, target);
}

bool BrushingAndLinkingInport::isHighlighted(uint32_t idx, BrushingTarget target) const {
    return manager_.contains(idx, BrushingAction::Highlight, target);
}

bool BrushingAndLinkingInport::isColumnSelected(uint32_t idx) const {
    return manager_.contains(idx, BrushingAction::Select, BrushingTarget::Column);
}

const BitSet& BrushingAndLinkingInport::getIndices(BrushingAction action,
                                                   BrushingTarget target) const {
    return manager_.getIndices(action, target);
}

const BitSet& BrushingAndLinkingInport::getFilteredIndices(BrushingTarget target) const {
    return manager_.getIndices(BrushingAction::Filter, target);
}

const BitSet& BrushingAndLinkingInport::getSelectedIndices(BrushingTarget target) const {
    return manager_.getIndices(BrushingAction::Select, target);
}

const BitSet& BrushingAndLinkingInport::getHighlightedIndices(BrushingTarget target) const {
    return manager_.getIndices(BrushingAction::Highlight, target);
}

const BitSet& BrushingAndLinkingInport::getSelectedColumns() const {
    return manager_.getIndices(BrushingAction::Select, BrushingTarget::Column);
}

const std::vector<BrushingTargetsInvalidationLevel>&
BrushingAndLinkingInport::getInvalidationLevels() const {
    return manager_.getInvalidationLevels();
}

void BrushingAndLinkingInport::setInvalidationLevels(
    std::vector<BrushingTargetsInvalidationLevel> invalidationLevels) {
    manager_.setInvalidationLevels(invalidationLevels);
}

BrushingAndLinkingManager& BrushingAndLinkingInport::getManager() { return manager_; }

const BrushingAndLinkingManager& BrushingAndLinkingInport::getManager() const { return manager_; }

void BrushingAndLinkingInport::serialize(Serializer& s) const {
    Inport::serialize(s);
    s.serialize("manager", manager_);
}

void BrushingAndLinkingInport::deserialize(Deserializer& d) {
    Inport::deserialize(d);
    d.deserialize("manager", manager_);
}

bool BrushingAndLinkingInport::canConnectTo(const Port* port) const {
    if (!port || port->getProcessor() == getProcessor()) return false;

    // Check for circular depends.
    auto pd = util::getPredecessors(port->getProcessor());
    if (pd.find(getProcessor()) != pd.end()) return false;

    if (dynamic_cast<const BrushingAndLinkingOutport*>(port)) {
        return true;
    }

    return false;
}

std::string BrushingAndLinkingInport::getClassIdentifier() const {
    return PortTraits<BrushingAndLinkingInport>::classIdentifier();
}

Document BrushingAndLinkingInport::getInfo() const {
    using P = Document::PathComponent;
    using H = utildoc::TableBuilder::Header;
    Document doc;
    doc.append("b", "Brushing & Linking Inport", {{"style", "color:white;"}});

    if (!help_.empty()) {
        doc.append(help_);
    }

    utildoc::TableBuilder tb(doc.handle(), P::end());
    for (auto& [action, targets] : manager_.getTargets()) {
        for (auto& target : targets) {
            tb(H(action), target, manager_.getIndices(action, target).cardinality());
        }
    }
    return doc;
}

void BrushingAndLinkingInport::setChanged(bool changed, const Outport* source) {
    if (!changed) {
        manager_.clearModifications();
    }
    Inport::setChanged(changed, source);
}

void BrushingAndLinkingInport::invalidate(InvalidationLevel) {
    manager_.propagateModifications();
    // Override with the modification-based (brushing target and action) invalidation level.
    // This enables more selective invalidations that are dependent on this manager's settings
    // instead of the one of the top-most brushing manager that initiated the invalidation.
    Inport::invalidate(manager_.getInvalidationLevel());
}

BrushingAndLinkingOutport::BrushingAndLinkingOutport(std::string_view identifier, Document help)
    : Outport(identifier, std::move(help)), manager_(this) {
    isReady_.setUpdate([this]() { return invalidationLevel_ == InvalidationLevel::Valid; });
}

BrushingAndLinkingManager& BrushingAndLinkingOutport::getManager() { return manager_; }

const BrushingAndLinkingManager& BrushingAndLinkingOutport::getManager() const { return manager_; }

void BrushingAndLinkingOutport::invalidate(InvalidationLevel) {
    manager_.propagateModifications();
    // Override with the modification-based (brushing target and action) invalidation level.
    // This enables more selective invalidations that are dependent on this manager's settings
    // instead of the one of the top-most brushing manager that initiated the invalidation.
    Outport::invalidate(manager_.getInvalidationLevel());
}

const std::vector<BrushingTargetsInvalidationLevel>&
BrushingAndLinkingOutport::getInvalidationLevels() const {
    return manager_.getInvalidationLevels();
}

void BrushingAndLinkingOutport::setInvalidationLevels(
    std::vector<BrushingTargetsInvalidationLevel> invalidationLevels) {
    manager_.setInvalidationLevels(invalidationLevels);
}

void BrushingAndLinkingOutport::serialize(Serializer& s) const {
    Outport::serialize(s);
    manager_.serialize(s);
}

void BrushingAndLinkingOutport::deserialize(Deserializer& d) {
    Outport::deserialize(d);
    manager_.deserialize(d);
}

void BrushingAndLinkingOutport::setValid() {
    manager_.clearModifications();
    Outport::setValid();
}

Document BrushingAndLinkingOutport::getInfo() const {
    using P = Document::PathComponent;
    using H = utildoc::TableBuilder::Header;
    Document doc;
    doc.append("b", "Brushing & Linking Outport", {{"style", "color:white;"}});

    if (!help_.empty()) {
        doc.append(help_);
    }

    utildoc::TableBuilder tb(doc.handle(), P::end());
    for (auto& [action, targets] : manager_.getTargets()) {
        for (auto& target : targets) {
            tb(H(action), target, manager_.getIndices(action, target).cardinality());
        }
    }
    return doc;
}

std::string BrushingAndLinkingOutport::getClassIdentifier() const {
    return PortTraits<BrushingAndLinkingOutport>::classIdentifier();
}

}  // namespace inviwo
