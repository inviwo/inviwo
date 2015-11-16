/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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
#include <inviwo/core/links/linkconditions.h>
#include <inviwo/core/properties/propertyconvertermanager.h>
#include <inviwo/core/properties/propertyconverter.h>
#include <inviwo/core/util/raiiutils.h>
#include <inviwo/core/properties/property.h>
#include <inviwo/core/links/propertylink.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/network/networklock.h>
#include <inviwo/core/properties/compositeproperty.h>

namespace inviwo {

LinkEvaluator::LinkEvaluator(ProcessorNetwork* network) : network_(network) {}

void LinkEvaluator::addLink(PropertyLink* propertyLink) {
    Property* src = propertyLink->getSourceProperty();
    Property* dst = propertyLink->getDestinationProperty();

    // Update ProcessorLink cache
    Processor* p1 = src->getOwner()->getProcessor();
    Processor* p2 = dst->getOwner()->getProcessor();
    processorLinksCache_[ProcessorPair(p1, p2)].push_back(propertyLink);

    // Update primary cache
    auto& cachelist = propertyLinkPrimaryCache_[src];
    util::push_back_unique(cachelist, dst);

    if (cachelist.empty()) {
        propertyLinkPrimaryCache_.erase(src);
    }

    propertyLinkSecondaryCache_.clear();
}

void LinkEvaluator::removeLink(PropertyLink* propertyLink) {
    Property* src = propertyLink->getSourceProperty();
    Property* dst = propertyLink->getDestinationProperty();

    // Update ProcessorLink cache
    Processor* p1 = src->getOwner()->getProcessor();
    Processor* p2 = dst->getOwner()->getProcessor();

    auto it = processorLinksCache_.find(ProcessorPair(p1, p2));
    if (it != processorLinksCache_.end()) {
        util::erase_remove(it->second, propertyLink);
        if (it->second.empty()) processorLinksCache_.erase(it);
    }

    // Update primary cache
    auto& cachelist = propertyLinkPrimaryCache_[src];
    util::erase_remove(cachelist, dst);

    if (cachelist.empty()) {
        propertyLinkPrimaryCache_.erase(src);
    }

    propertyLinkSecondaryCache_.clear();
}

std::vector<PropertyLink*> LinkEvaluator::getLinksBetweenProcessors(Processor* p1, Processor* p2) {
    auto it = processorLinksCache_.find(ProcessorPair(p1, p2));
    if (it != processorLinksCache_.end()) {
        return it->second;
    } else {
        return std::vector<PropertyLink*>();
    }
}

std::vector<LinkEvaluator::Link>& LinkEvaluator::getTriggerdLinksForProperty(Property* property) {
    if (propertyLinkSecondaryCache_.find(property) != propertyLinkSecondaryCache_.end()) {
        return propertyLinkSecondaryCache_[property];
    } else {
        return addToSecondaryCache(property);
    }
}

std::vector<Property*> LinkEvaluator::getPropertiesLinkedTo(Property* property) {
    // check if link connectivity has been computed and cached already
    auto& list = (propertyLinkSecondaryCache_.find(property) != propertyLinkSecondaryCache_.end())
                     ? propertyLinkSecondaryCache_[property]
                     : addToSecondaryCache(property);

    return util::transform(list, [](const Link& link) { return link.dst_; });
}

std::vector<LinkEvaluator::Link>& LinkEvaluator::addToSecondaryCache(Property* src) {
    for (auto& dst : propertyLinkPrimaryCache_[src]) {
        if (src != dst) secondaryCacheHelper(propertyLinkSecondaryCache_[src], src, dst);
    }
    return propertyLinkSecondaryCache_[src];
}

void LinkEvaluator::secondaryCacheHelper(std::vector<Link>& links, Property* src, Property* dst) {
    // Check that we don't use a previous source or destination as the new destination.
    if (!util::contains_if(
            links, [dst](const Link& link) { return link.src_ == dst || link.dst_ == dst; })) {
        auto manager = InviwoApplication::getPtr()->getPropertyConverterManager();
        if (auto converter = manager->getConverter(src, dst)) {
            links.emplace_back(src, dst, converter);
        }

        // Follow the links of destination all links of all owners (CompositeProperties).
        for (Property* newSrc = dst; newSrc != nullptr;
             newSrc = dynamic_cast<Property*>(newSrc->getOwner())) {
            // Recurse over outgoing links.
            for (auto& elem : propertyLinkPrimaryCache_[newSrc]) {
                if (newSrc != elem) secondaryCacheHelper(links, newSrc, elem);
            }
        }

        // If we link to a CompositeProperty, make sure to evaluate sub-links.
        if (auto cp = dynamic_cast<CompositeProperty*>(dst)) {
            for (auto& srcProp : cp->getProperties()) {
                // Recurse over outgoing links.
                for (auto& elem : propertyLinkPrimaryCache_[srcProp]) {
                    if (srcProp != elem) secondaryCacheHelper(links, srcProp, elem);
                }
            }
        }
    }
}

bool LinkEvaluator::isLinking() const { return !visited_.empty(); }

void LinkEvaluator::evaluateLinksFromProperty(Property* modifiedProperty) {
    if (util::contains(visited_, modifiedProperty)) return;

    NetworkLock lock(network_);

    auto& links = getTriggerdLinksForProperty(modifiedProperty);
    VisitedHelper helper(visited_, links);

    for (auto& link : links) {
        link.converter_->convert(link.src_, link.dst_);
    }
}

}  // namespace