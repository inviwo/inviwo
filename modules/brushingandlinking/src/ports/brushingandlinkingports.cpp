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

#include <modules/brushingandlinking/ports/brushingandlinkingports.h>

#include <inviwo/core/network/networkutils.h>

namespace inviwo {

BrushingAndLinkingInport::BrushingAndLinkingInport(std::string identifier)
    : Inport(identifier), manager_(this) {
    setOptional(true);
}

bool BrushingAndLinkingInport::isChanged() const { return manager_.isModified(); }

BrushingModifications BrushingAndLinkingInport::modifiedActions() const {
    return manager_.modifiedActions();
}

bool BrushingAndLinkingInport::modifiedFiltering() const { return manager_.modifiedFiltering(); }

bool BrushingAndLinkingInport::modifiedSelection() const { return manager_.modifiedSelection(); }

bool BrushingAndLinkingInport::modifiedHighlight() const { return manager_.modifiedHighlight(); }

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
    utildoc::TableBuilder tb(doc.handle(), P::end());
    for (auto& [action, target] : manager_.getTargets()) {
        tb(H(action), target, manager_.getIndices(action, target).cardinality());
    }
    return doc;
}

void BrushingAndLinkingInport::setChanged(bool changed, const Outport* source) {
    if (!changed) {
        manager_.markAsValid();
    }
    Inport::setChanged(changed, source);
}

BrushingAndLinkingOutport::BrushingAndLinkingOutport(std::string identifier)
    : Outport(identifier), manager_(this) {}

BrushingAndLinkingManager& BrushingAndLinkingOutport::getManager() { return manager_; }

const BrushingAndLinkingManager& BrushingAndLinkingOutport::getManager() const { return manager_; }

void BrushingAndLinkingOutport::serialize(Serializer& s) const {
    Outport::serialize(s);
    manager_.serialize(s);
}

void BrushingAndLinkingOutport::deserialize(Deserializer& d) {
    Outport::deserialize(d);
    manager_.deserialize(d);
}

void BrushingAndLinkingOutport::setValid() {
    manager_.markAsValid();
    Outport::setValid();
}

Document BrushingAndLinkingOutport::getInfo() const {
    using P = Document::PathComponent;
    using H = utildoc::TableBuilder::Header;
    Document doc;
    doc.append("b", "Brushing & Linking Outport", {{"style", "color:white;"}});
    utildoc::TableBuilder tb(doc.handle(), P::end());
    for (auto& [action, target] : manager_.getTargets()) {
        tb(H(action), target, manager_.getIndices(action, target).cardinality());
    }
    return doc;
}

std::string BrushingAndLinkingOutport::getClassIdentifier() const {
    return PortTraits<BrushingAndLinkingOutport>::classIdentifier();
}

}  // namespace inviwo
