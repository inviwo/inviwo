/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2025 Inviwo Foundation
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

#include <inviwo/core/network/autolinker.h>
#include <inviwo/core/network/networkutils.h>
#include <inviwo/core/network/networklock.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/settings/linksettings.h>

#include <algorithm>

namespace inviwo {

void AutoLinker::update(ProcessorNetwork& network, Processor& target, Processor* source,
                        const std::vector<Processor*>& ignore) {
    autoLinkCandidates_.clear();

    std::vector<Property*> sourceProperties;
    const auto collectProperties = [&](const Processor* p) {
        if (auto it = std::ranges::find(ignore, p); it == ignore.end()) {
            p->getPropertiesRecursive(sourceProperties);
        }
    };

    if (source) {
        for (auto* p : util::getPredecessors(source)) {
            collectProperties(p);
        }
    } else {
        network.forEachProcessor([&](Processor* p) {
            if (p != &target) collectProperties(p);
        });
    }

    // auto link based on global settings
    auto* app = network.getApplication();
    const auto* linkSettings = app->getSettingsByType<LinkSettings>();

    const auto isGlobalAutoLinkAble = [&](const Property* targetProperty) {
        return [&, targetProperty](const Property* p) {
            return linkSettings->isLinkable(p) && network.canLink(p, targetProperty) &&
                   p->getClassIdentifier() == targetProperty->getClassIdentifier() &&
                   p->getIdentifier() == targetProperty->getIdentifier();
        };
    };

    const auto isLocalAutoLinkAble = [&](const auto& item) {
        return [&, item](const Property* p) {
            const auto& [processorClassId, propertyPath] = item;
            const auto* proc = p->getOwner()->getProcessor();
            return proc != nullptr && proc->getClassIdentifier() == processorClassId &&
                   p->getPath() == propertyPath;
        };
    };

    const auto targetProperties = target.getPropertiesRecursive();
    for (auto* targetProperty : targetProperties) {

        std::vector<Property*> candidates;
        if (linkSettings->isLinkable(targetProperty)) {
            std::copy_if(sourceProperties.begin(), sourceProperties.end(),
                         std::back_inserter(candidates), isGlobalAutoLinkAble(targetProperty));
        }

        for (const auto& item : targetProperty->getAutoLinkToProperty()) {
            std::copy_if(sourceProperties.begin(), sourceProperties.end(),
                         std::back_inserter(candidates), isLocalAutoLinkAble(item));
        }

        if (!candidates.empty()) {
            autoLinkCandidates_[targetProperty] = std::move(candidates);
        }
    }
}

void AutoLinker::sortAutoLinkCandidates() {
    util::PropertyDistanceSorter distSorter;
    for (auto& item : autoLinkCandidates_) {
        distSorter.setTarget(item.first);
        std::sort(item.second.begin(), item.second.end(), distSorter);
    }
}

void AutoLinker::addLinksToClosestCandidates(ProcessorNetwork& network, bool bidirectional) {
    const NetworkLock lock(&network);
    sortAutoLinkCandidates();
    for (auto& item : getAutoLinkCandidates()) {
        const auto existingSources = network.getPropertiesLinkedTo(item.first);
        if (std::ranges::find(existingSources, item.second.front()) == existingSources.end()) {
            network.addLink(item.second.front(), item.first);
            // Propagate the link to the new Processor.
            network.evaluateLinksFromProperty(item.second.front());
            if (bidirectional) {
                network.addLink(item.first, item.second.front());
            }
        }
    }
}

void AutoLinker::clear() {
    autoLinkCandidates_.clear();
}

const std::unordered_map<Property*, std::vector<Property*>>& AutoLinker::getAutoLinkCandidates()
    const {
    return autoLinkCandidates_;
}

void AutoLinker::addLinks(ProcessorNetwork& network, Processor& target, Processor* source,
                          const std::vector<Processor*>& ignore) {
    AutoLinker al{};
    al.update(network, target, source, ignore);
    al.addLinksToClosestCandidates(network, true);
}

}  // namespace inviwo
