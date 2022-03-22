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

#include <modules/brushingandlinking/brushingandlinkingmanager.h>
#include <modules/brushingandlinking/processors/brushingandlinkingprocessor.h>
#include <modules/brushingandlinking/ports/brushingandlinkingports.h>

#include <inviwo/core/io/serialization/serializer.h>
#include <inviwo/core/io/serialization/deserializer.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/zip.h>
#include <inviwo/core/util/assertion.h>

#include <stack>

#include <fmt/format.h>

namespace inviwo {

BrushingAndLinkingManager::BrushingAndLinkingManager(
    BrushingAndLinkingInport* inport,
    std::vector<BrushingTargetsInvalidationLevel> invalidationLevels)
    : owner_(inport), invalidationLevels_(invalidationLevels) {

    inport->onConnect([&]() {
        setParent(&static_cast<BrushingAndLinkingOutport*>(
                       std::get<BrushingAndLinkingInport*>(owner_)->getConnectedOutport())
                       ->getManager());
    });
    inport->onDisconnect([&]() { setParent(nullptr); });
}

BrushingAndLinkingManager::BrushingAndLinkingManager(
    BrushingAndLinkingOutport* outport,
    std::vector<BrushingTargetsInvalidationLevel> invalidationLevels)
    : owner_(outport), invalidationLevels_(invalidationLevels) {}

BrushingAndLinkingManager::~BrushingAndLinkingManager() = default;

void BrushingAndLinkingManager::brush(BrushingAction action, BrushingTarget target,
                                      const BitSet& indices, std::string_view source) {
    if ((action == BrushingAction::Filter) && source.empty()) {
        throw Exception("BrushingAction::Filter requires a source", IVW_CONTEXT);
    }

    const int actionIdx = getActionIndex(action);

    const bool changed =
        std::visit(util::overloaded{[&](BitSetTargets& map) { return map[target].set(indices); },
                                    [&](IndexListTargets& map) {
                                        auto it = map.try_emplace(target, IndexList());
                                        return it.first->second.set(source, indices);
                                    }},
                   selections_[actionIdx]);

    if (onBrushCallback_) {
        std::invoke(onBrushCallback_, action, target, indices, source);
    }

    if (changed) {
        propagate(action, target);
    }
}

bool BrushingAndLinkingManager::isModified() const { return !modifications_.empty(); }

BrushingModifications BrushingAndLinkingManager::getModifiedActions() const {
    BrushingModifications modifiedActions(flags::empty);

    for (const auto& elem : modifications_) {
        modifiedActions |= elem.second;
    }
    return modifiedActions;
}

bool BrushingAndLinkingManager::isFilteringModified(BrushingTarget target) const {
    return isTargetModified(target, BrushingModification::Filtered);
}

bool BrushingAndLinkingManager::isSelectionModified(BrushingTarget target) const {
    return isTargetModified(target, BrushingModification::Selected);
}

bool BrushingAndLinkingManager::isHighlightModified(BrushingTarget target) const {
    return isTargetModified(target, BrushingModification::Highlighted);
}

std::vector<BrushingTarget> BrushingAndLinkingManager::getModifiedTargets() const {
    return util::transform(modifications_, [](const auto& elem) { return elem.first; });
}

bool BrushingAndLinkingManager::isTargetModified(BrushingTarget target,
                                                 BrushingAction action) const {
    return isTargetModified(target, fromAction(action));
}

bool BrushingAndLinkingManager::isTargetModified(BrushingTarget target,
                                                 BrushingModifications modifications) const {
    auto it = modifications_.find(target);
    if (it != modifications_.end()) {
        return !(it->second & modifications).empty();
    }

    return false;
}

bool BrushingAndLinkingManager::hasIndices(BrushingAction action, BrushingTarget target) const {
    if (parent_) {
        return parent_->hasIndices(action, target);
    }

    const int actionIdx = getActionIndex(action);
    return std::visit(
        util::overloaded{[&](const BitSetTargets& map) { return util::has_key(map, target); },
                         [&](const IndexListTargets& map) { return util::has_key(map, target); }},
        selections_[actionIdx]);
}

const BitSet& BrushingAndLinkingManager::getIndices(BrushingAction action,
                                                    BrushingTarget target) const {
    if (parent_) {
        return parent_->getIndices(action, target);
    }

    static const BitSet empty;

    if (auto indices = getBitSet(action, target)) {
        return *indices;
    } else {
        return empty;
    }
}

void BrushingAndLinkingManager::clearIndices(BrushingAction action, BrushingTarget target) {
    if (action == BrushingAction::Filter) {
        throw Exception(IVW_CONTEXT, "Clearing indices for action '{}' is not supported", action);
    }

    auto clearMap = [target, index = getActionIndex(action)](auto& selections) {
        return std::visit(
            [target](auto& map) {
                auto it = map.find(target);
                if (it == map.end()) {
                    return false;
                }
                if (it->second.empty()) return false;
                it->second.clear();
                return true;
            },
            selections[index]);
    };

    bool changed = false;

    // clear all children, set modification state if necessary, but avoid any propagation
    std::stack<BrushingAndLinkingManager*> stack;
    stack.push(this);
    while (!stack.empty()) {
        auto node = stack.top();
        stack.pop();
        if (clearMap(node->selections_)) {
            node->modifications_[target] |= fromAction(action);
            changed = true;
        }
        for (auto c : node->children_) {
            stack.push(c);
        }
    }

    if (changed && onBrushCallback_) {
        const std::string source = std::visit([](auto* p) { return p->getPath(); }, owner_);
        std::invoke(onBrushCallback_, action, target, BitSet(), source);
    }

    if (changed) {
        propagate(action, target);
    }
}

bool BrushingAndLinkingManager::contains(uint32_t idx, BrushingAction action,
                                         BrushingTarget target) const {
    if (parent_) {
        return parent_->contains(idx, action, target);
    }

    const int actionIdx = getActionIndex(action);
    return std::visit(
        [&](const auto& map) {
            auto it = map.find(target);
            if (it == map.end()) {
                return false;
            }
            return it->second.contains(idx);
        },
        selections_[actionIdx]);
}

std::vector<std::pair<BrushingAction, std::vector<BrushingTarget>>>
BrushingAndLinkingManager::getTargets() const {
    if (parent_) {
        return parent_->getTargets();
    }

    using ActionTarget = std::pair<BrushingAction, std::vector<BrushingTarget>>;

    std::vector<ActionTarget> targets;
    auto extractTargets = [&](BrushingAction action, const auto& map) {
        targets.emplace_back(action, util::transform(map, [](auto elem) { return elem.first; }));
    };

    for (auto&& [action, map] : util::zip(BrushingActions, selections_)) {
        std::visit(util::overloaded{[&, action = action](const BitSetTargets& set) {
                                        extractTargets(action, set);
                                    },
                                    [&, action = action](const IndexListTargets& list) {
                                        extractTargets(action, list);
                                    }},
                   map);
    }

    return targets;
}

std::vector<BrushingTarget> BrushingAndLinkingManager::getTargets(BrushingAction action) const {
    if (parent_) {
        return parent_->getTargets(action);
    }

    const int actionIdx = getActionIndex(action);
    return std::visit(
        [&](const auto& map) {
            return util::transform(map, [](const auto& elem) { return elem.first; });
        },
        selections_[actionIdx]);
}

void BrushingAndLinkingManager::filter(const BitSet& indices, BrushingTarget target,
                                       std::string_view source) {
    brush(BrushingAction::Filter, target, indices, source);
}

void BrushingAndLinkingManager::select(const BitSet& indices, BrushingTarget target) {
    brush(BrushingAction::Select, target, indices);
}

void BrushingAndLinkingManager::highlight(const BitSet& indices, BrushingTarget target) {
    brush(BrushingAction::Highlight, target, indices);
}

void BrushingAndLinkingManager::clearFiltered() {
    throw Exception("clearing filtered indices is no longer supported", IVW_CONTEXT);
}

void BrushingAndLinkingManager::clearSelected(BrushingTarget target) {
    clearIndices(BrushingAction::Select, target);
}

void BrushingAndLinkingManager::clearHighlighted(BrushingTarget target) {
    clearIndices(BrushingAction::Highlight, target);
}

size_t BrushingAndLinkingManager::getNumber(BrushingAction action, BrushingTarget target) const {
    return getIndices(action, target).size();
}

size_t BrushingAndLinkingManager::getNumberOfFiltered(BrushingTarget target) const {
    return getIndices(BrushingAction::Filter, target).size();
}

size_t BrushingAndLinkingManager::getNumberOfSelected(BrushingTarget target) const {
    return getIndices(BrushingAction::Select, target).size();
}

size_t BrushingAndLinkingManager::getNumberOfHighlighted(BrushingTarget target) const {
    return getIndices(BrushingAction::Highlight, target).size();
}

const BitSet& BrushingAndLinkingManager::getFilteredIndices(BrushingTarget target) const {
    return getIndices(BrushingAction::Filter, target);
}

const BitSet& BrushingAndLinkingManager::getSelectedIndices(BrushingTarget target) const {
    return getIndices(BrushingAction::Select, target);
}

const BitSet& BrushingAndLinkingManager::getHighlightedIndices(BrushingTarget target) const {
    return getIndices(BrushingAction::Highlight, target);
}

bool BrushingAndLinkingManager::isFiltered(uint32_t idx, BrushingTarget target) const {
    return contains(idx, BrushingAction::Filter, target);
}

bool BrushingAndLinkingManager::isSelected(uint32_t idx, BrushingTarget target) const {
    return contains(idx, BrushingAction::Select, target);
}

bool BrushingAndLinkingManager::isHighlighted(uint32_t idx, BrushingTarget target) const {
    return contains(idx, BrushingAction::Highlight, target);
}

void BrushingAndLinkingManager::setParent(BrushingAndLinkingManager* parent) {
    if (parent_) {
        parent_->removeChild(this);

        // set manager as modified since filter actions might have changed when removing the parent
        for (const auto& [action, targets] : parent_->getTargets()) {
            for (auto& t : targets) {
                modifications_[t] |= fromAction(action);
            }
        }

        if (std::holds_alternative<BrushingAndLinkingOutport*>(owner_)) {
            auto outport = std::get<BrushingAndLinkingOutport*>(owner_);
            outport->getProcessor()->invalidate(getInvalidationLevel());
        }
    }

    parent_ = parent;
    if (parent_) {
        parent_->addChild(this);

        auto propagateTargets = [&](BrushingAction action, const auto& map) {
            propagate(action, util::transform(map, [](auto elem) { return elem.first; }));
        };

        // propagate all selections of the manager to the parent
        for (auto&& [action, map] : util::zip(BrushingActions, selections_)) {
            std::visit(util::overloaded{[&, action = action](const BitSetTargets& set) {
                                            propagateTargets(action, set);
                                        },
                                        [&, action = action](const IndexListTargets& list) {
                                            propagateTargets(action, list);
                                        }},
                       map);
        }
    }
}

void BrushingAndLinkingManager::onBrush(
    std::function<void(BrushingAction, BrushingTarget, const BitSet&, std::string_view)> callback) {
    onBrushCallback_ = callback;
}

const std::vector<BrushingTargetsInvalidationLevel>&
BrushingAndLinkingManager::getInvalidationLevels() const {
    return invalidationLevels_;
}

InvalidationLevel BrushingAndLinkingManager::getInvalidationLevel() const {
    return getInvalidationLevel(modifications_);
}

void BrushingAndLinkingManager::setInvalidationLevels(
    std::vector<BrushingTargetsInvalidationLevel> invalidationLevels) {
    invalidationLevels_ = invalidationLevels;
}

void BrushingAndLinkingManager::propagateModifications() {
    for (auto c : children_) {
        for (const auto& [target, modification] : modifications_) {
            c->modifications_[target] |= modification;
        }
    }
}

void BrushingAndLinkingManager::clearModifications() { modifications_.clear(); }

void BrushingAndLinkingManager::serialize(Serializer& s) const {
    for (auto&& [action, targetmap] : util::zip(BrushingActions, selections_)) {
        if (std::holds_alternative<BitSetTargets>(targetmap)) {
            auto& map = std::get<BitSetTargets>(targetmap);
            if (map.empty()) continue;

            s.serialize(toString(action), map, "selection", {},
                        [](BrushingTarget t) { return t.getString(); }, {});
        } else if (std::holds_alternative<IndexListTargets>(targetmap)) {
            auto& map = std::get<BitSetTargets>(targetmap);
            if (map.empty()) continue;

            s.serialize(toString(action), map, "selection", {},
                        [](BrushingTarget t) { return t.getString(); }, {});
        }
    }
}

void BrushingAndLinkingManager::deserialize(Deserializer& d) {
    for (auto&& [action, targetmap] : util::zip(BrushingActions, selections_)) {
        if (std::holds_alternative<BitSetTargets>(targetmap)) {
            auto& map = std::get<BitSetTargets>(targetmap);

            auto des = util::MapDeserializer<BrushingTarget, BitSet>(toString(action), "selection")
                           .setMakeNew([]() { return BitSet(); })
                           .onNew([&](const BrushingTarget& key, BitSet& b) { map[key] = b; })
                           .onRemove([&](const BrushingTarget& key) { map.erase(key); });
            des(d, map);
        } else if (std::holds_alternative<IndexListTargets>(targetmap)) {
            auto& map = std::get<IndexListTargets>(targetmap);

            auto des =
                util::MapDeserializer<BrushingTarget, IndexList>(toString(action), "selection")
                    .setMakeNew([]() { return IndexList(); })
                    .onNew([&](const BrushingTarget& key, IndexList& l) { map[key] = l; })
                    .onRemove([&](const BrushingTarget& key) { map.erase(key); });
            des(d, map);
        }
    }
}

int BrushingAndLinkingManager::getActionIndex(BrushingAction action) {
    if (action == BrushingAction::NumberOfActions) {
        throw Exception(IVW_CONTEXT_CUSTOM("BrushingAndLinkingManager"),
                        "Invalid brushing action '{}'", action);
    }
    return static_cast<int>(action);
}

void BrushingAndLinkingManager::propagate(BrushingAction action, BrushingTarget target) {
    modifications_[target] |= fromAction(action);

    // Only invalidate the top level in the connected brushing manager network
    if (!parent_) {
        if (std::holds_alternative<BrushingAndLinkingOutport*>(owner_)) {
            auto outport = std::get<BrushingAndLinkingOutport*>(owner_);
            // Processor need to be invalidated for the network evaluation to be notified.
            outport->getProcessor()->invalidate(
                getInvalidationLevel(target, modifications_[target]));
        } else if (std::holds_alternative<BrushingAndLinkingInport*>(owner_)) {
            // Nothing is connected, invalidate the processor itself
            auto inport = std::get<BrushingAndLinkingInport*>(owner_);

            inport->invalidate(getInvalidationLevel(target, modifications_[target]));
        }
    }

    if (parent_) {
        const std::string source = std::visit([](auto* p) { return p->getPath(); }, owner_);

        auto localIndices = getBitSet(action, target);

        if (localIndices) {
            parent_->brush(action, target, *localIndices, source);
        }
    }
}

void BrushingAndLinkingManager::propagate(BrushingAction action,
                                          const std::vector<BrushingTarget>& targets) {
    for (auto t : targets) {
        modifications_[t] |= fromAction(action);
    }

    // Only invalidate the top level in the connected brushing manager network
    if (!parent_ && std::holds_alternative<BrushingAndLinkingOutport*>(owner_)) {
        auto outport = std::get<BrushingAndLinkingOutport*>(owner_);
        InvalidationLevel invalidationLevel(InvalidationLevel::Valid);
        for (auto& t : targets) {
            invalidationLevel =
                std::max(getInvalidationLevel(t, modifications_[t]), invalidationLevel);
        }
        outport->getProcessor()->invalidate(invalidationLevel);
    }

    if (parent_) {
        const std::string source = std::visit([](auto* p) { return p->getPath(); }, owner_);

        for (auto target : targets) {
            auto localIndices = getBitSet(action, target);

            if (localIndices) {
                parent_->brush(action, target, *localIndices, source);
            }
        }
    }
}

void BrushingAndLinkingManager::addChild(BrushingAndLinkingManager* child) {
    [[maybe_unused]] auto it = children_.insert(child);
    IVW_ASSERT(it.second, "child manager already added");
}

void BrushingAndLinkingManager::removeChild(BrushingAndLinkingManager* child) {
    if (child) {
        children_.erase(child);
        if (std::holds_alternative<BrushingAndLinkingInport*>(child->owner_)) {
            auto inport = std::get<BrushingAndLinkingInport*>(child->owner_);

            for (auto&& [action, targetmap] : util::zip(BrushingActions, selections_)) {
                if (std::holds_alternative<IndexListTargets>(targetmap)) {
                    for (auto& elem : std::get<IndexListTargets>(targetmap)) {
                        if (elem.second.removeSources({inport->getPath()})) {
                            // inform parent manager
                            propagate(action, elem.first);
                        }
                    }
                }
            }
        }
    }
}

const BitSet* BrushingAndLinkingManager::getBitSet(BrushingAction action,
                                                   BrushingTarget target) const {
    return std::visit(util::overloaded{[&](const BitSetTargets& map) -> const BitSet* {
                                           auto it = map.find(target);
                                           if (it == map.end()) {
                                               return nullptr;
                                           }
                                           return &it->second;
                                       },
                                       [&](const IndexListTargets& map) -> const BitSet* {
                                           auto it = map.find(target);
                                           if (it == map.end()) {
                                               return nullptr;
                                           }
                                           return &it->second.getIndices();
                                       }},
                      selections_[getActionIndex(action)]);
}

InvalidationLevel BrushingAndLinkingManager::getInvalidationLevel(
    const BrushingTarget& target, BrushingModifications mods) const {
    InvalidationLevel invalidationLevel(InvalidationLevel::Valid);
    for (const auto& l : invalidationLevels_) {
        if (l.contains(target, mods)) {
            invalidationLevel = std::max(invalidationLevel, l.invalidationLevel);
        }
    }
    return invalidationLevel;
}

InvalidationLevel BrushingAndLinkingManager::getInvalidationLevel(
    const std::unordered_map<BrushingTarget, BrushingModifications>& mods) const {
    InvalidationLevel invalidationLevel(InvalidationLevel::Valid);
    for (auto& m : mods) {
        invalidationLevel = std::max(getInvalidationLevel(m.first, m.second), invalidationLevel);
    }
    return invalidationLevel;
}

}  // namespace inviwo
