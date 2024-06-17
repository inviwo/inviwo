/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2024 Inviwo Foundation
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

#include <inviwo/core/links/linkevaluator.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/properties/propertyconvertermanager.h>
#include <inviwo/core/properties/propertyconverter.h>
#include <inviwo/core/util/raiiutils.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/properties/property.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/network/networklock.h>
#include <inviwo/core/properties/compositeproperty.h>

namespace inviwo {

namespace {

struct VisitedHelper {
    VisitedHelper(std::vector<Property*>& visited, std::vector<ConvertibleLink>& toVisit)
        : visited_(visited), toVisit_(toVisit) {
        for (auto& link : toVisit_) {
            util::push_back_unique(visited_, link.src);
            util::push_back_unique(visited_, link.dst);
        }
    }
    ~VisitedHelper() {
        for (auto& link : toVisit_) {
            std::erase(visited_, link.src);
            std::erase(visited_, link.dst);
        }
    }

private:
    std::vector<Property*>& visited_;
    std::vector<ConvertibleLink>& toVisit_;
};

}  // namespace

LinkEvaluator::LinkEvaluator(ProcessorNetwork* network) : network_(network) {}

void LinkEvaluator::addLink(const PropertyLink& propertyLink) {
    Property* src = propertyLink.getSource();
    Property* dst = propertyLink.getDestination();

    // Update ProcessorLink cache
    Processor* p1 = src->getOwner()->getProcessor();
    Processor* p2 = dst->getOwner()->getProcessor();
    processorLinksCache_[ProcessorPair(p1, p2)].push_back(propertyLink);

    // Update primary cache
    auto& cacheList = directLinkCache_[src];
    util::push_back_unique(cacheList, dst);

    if (cacheList.empty()) {
        directLinkCache_.erase(src);
    }

    transientLinkCache_.clear();
}

bool LinkEvaluator::canLink(const Property* src, const Property* dst) const {
    if (src == dst) return false;
    auto manager = network_->getApplication()->getPropertyConverterManager();
    return manager->canConvert(src, dst);
}

bool LinkEvaluator::canLink(const PropertyLink& propertyLink) const {
    return canLink(propertyLink.getSource(), propertyLink.getDestination());
}

void LinkEvaluator::removeLink(const PropertyLink& propertyLink) {
    Property* src = propertyLink.getSource();
    Property* dst = propertyLink.getDestination();

    // Update ProcessorLink cache
    Processor* p1 = src->getOwner()->getProcessor();
    Processor* p2 = dst->getOwner()->getProcessor();

    const auto it = processorLinksCache_.find(ProcessorPair(p1, p2));
    if (it != processorLinksCache_.end()) {
        std::erase(it->second, propertyLink);
        if (it->second.empty()) {
            processorLinksCache_.erase(it);
        }
    }

    // Update primary cache
    auto& cacheList = directLinkCache_[src];
    std::erase(cacheList, dst);
    if (cacheList.empty()) {
        directLinkCache_.erase(src);
    }

    transientLinkCache_.clear();
}

std::vector<PropertyLink> LinkEvaluator::getLinksBetweenProcessors(Processor* p1, Processor* p2) {
    const auto it = processorLinksCache_.find(ProcessorPair(p1, p2));
    if (it != processorLinksCache_.end()) {
        return it->second;
    } else {
        return std::vector<PropertyLink>();
    }
}

std::vector<ConvertibleLink>& LinkEvaluator::getTriggeredLinksForProperty(Property* property) {
    if (transientLinkCache_.find(property) != transientLinkCache_.end()) {
        return transientLinkCache_[property];
    } else {
        return addToTransientCache(property);
    }
}

std::vector<Property*> LinkEvaluator::getPropertiesLinkedTo(Property* property) {
    // check if link connectivity has been computed and cached already
    auto& list = (transientLinkCache_.find(property) != transientLinkCache_.end())
                     ? transientLinkCache_[property]
                     : addToTransientCache(property);

    return util::transform(list, [](const ConvertibleLink& link) { return link.dst; });
}

std::vector<ConvertibleLink>& LinkEvaluator::addToTransientCache(Property* src) {
    if (const auto it = directLinkCache_.find(src); it != directLinkCache_.end()) {
        for (auto& dst : it->second) {
            if (src != dst) {
                transientCacheHelper(transientLinkCache_[src], src, dst,
                                     network_->getApplication()->getPropertyConverterManager());
            }
        }
    }
    return transientLinkCache_[src];
}

void LinkEvaluator::transientCacheHelper(std::vector<ConvertibleLink>& links, Property* src,
                                         Property* dst, const PropertyConverterManager* manager) {
    // Check that we don't use a previous source or destination as the new destination.
    if (!util::contains_if(links, [dst](const ConvertibleLink& link) {
            return link.src == dst || link.dst == dst;
        })) {
        if (auto converter = manager->getConverter(src, dst)) {
            links.emplace_back(src, dst, converter);
        }

        // Follow the links of destination all links of all owners (CompositeProperties).
        for (Property* newSrc = dst; newSrc != nullptr;
             newSrc = dynamic_cast<Property*>(newSrc->getOwner())) {
            // Recurse over outgoing links.
            if (const auto it = directLinkCache_.find(newSrc); it != directLinkCache_.end()) {
                for (auto* elem : it->second) {
                    if (newSrc != elem) transientCacheHelper(links, newSrc, elem, manager);
                }
            }
        }

        // If we link to a CompositeProperty, make sure to evaluate sub-links.
        if (auto cp = dynamic_cast<CompositeProperty*>(dst)) {
            for (auto& srcProp : cp->getProperties()) {
                // Recurse over outgoing links.
                if (const auto it = directLinkCache_.find(srcProp); it != directLinkCache_.end()) {
                    for (auto* elem : it->second) {
                        if (srcProp != elem) transientCacheHelper(links, srcProp, elem, manager);
                    }
                }
            }
        }
    }
}

bool LinkEvaluator::isLinking() const { return !visited_.empty(); }

void LinkEvaluator::evaluateLinksFromProperty(Property* modifiedProperty) {
    if (util::contains(visited_, modifiedProperty)) return;

    NetworkLock lock(network_);

    auto& links = getTriggeredLinksForProperty(modifiedProperty);
    VisitedHelper helper(visited_, links);

    for (auto& link : links) {
        link.converter->convert(link.src, link.dst);
    }
}

}  // namespace inviwo
