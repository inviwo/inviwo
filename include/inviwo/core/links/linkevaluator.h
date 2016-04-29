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

#ifndef IVW_LIKEVALUATOR_H
#define IVW_LIKEVALUATOR_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processorpair.h>
#include <inviwo/core/links/propertylink.h>

#include <unordered_map>

namespace inviwo {

class Property;
class PropertyConverter;
class Processor;
class PropertyLink;
class ProcessorNetwork;

class IVW_CORE_API LinkEvaluator {
public:
    using ProcessorLinkMap = std::unordered_map<ProcessorPair, std::vector<PropertyLink>>;

    LinkEvaluator(ProcessorNetwork* network);

    void evaluateLinksFromProperty(Property*);

    /**
      * Properties that are linked to the given property where the given property is a source
      * property
      *
      * @param property given property
      * @return std::vector<Property*> List of all properties that are affected by given property
      */
    std::vector<Property*> getPropertiesLinkedTo(Property* property);
    std::vector<PropertyLink> getLinksBetweenProcessors(Processor* p1, Processor* p2);

    void addLink(const PropertyLink& propertyLink);
    void removeLink(const PropertyLink& propertyLink);
    bool isLinking() const;

private:
    struct Link {
        Link(Property* src, Property* dst, const PropertyConverter* converter)
            : src_(src), dst_(dst), converter_(converter) {}
        Property* src_;
        Property* dst_;
        const PropertyConverter* converter_;
    };

    // Cache helpers
    std::vector<Link>& addToSecondaryCache(Property* property);
    void secondaryCacheHelper(std::vector<Link>& links, Property* src, Property* dst);
    std::vector<Link>& getTriggerdLinksForProperty(Property* property);

    ProcessorNetwork* network_;

    // The primary link cache is a map with all source properties and a vector of properties that
    // they link directly to
    std::unordered_map<Property*, std::vector<Property*>> propertyLinkPrimaryCache_;
    // The secondary link cache is a map with all source properties and a vector of ALL the
    // properties that they link to. Directly or indirectly.
    std::unordered_map<Property*, std::vector<Link>> propertyLinkSecondaryCache_;
    // A cache of all links between two processors.
    ProcessorLinkMap processorLinksCache_;

    // Used to make sure we don't end up in circular links
    std::vector<Property*> visited_;
    struct VisitedHelper {
        VisitedHelper(std::vector<Property*>& visited, std::vector<Link>& toVisit)
            : visited_(visited), toVisit_(toVisit) {
            for (auto& link : toVisit_) {
                util::push_back_unique(visited_, link.src_);
                util::push_back_unique(visited_, link.dst_);
            }
        }
        ~VisitedHelper() {
            for (auto& link : toVisit_) {
                util::erase_remove(visited_, link.src_);
                util::erase_remove(visited_, link.dst_);
            }
        }

    private:
        std::vector<Property*>& visited_;
        std::vector<Link>& toVisit_;
    };
};

}  // namespace

#endif  // IVW_LIKEVALUATOR_H