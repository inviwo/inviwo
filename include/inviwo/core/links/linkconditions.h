/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#ifndef IVW_LINKCONDITIONS_H
#define IVW_LINKCONDITIONS_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/properties/property.h>

namespace inviwo {

enum LinkingConditions {
    NoLinkCondition = 0,
    LinkMatchingTypes = 1 << 1,
    LinkMatchingId = 1 << 2,
    LinkMatchingTypeAndId = 1 << 1 | 1 << 2,
    ApplyAllConditions = ~0,
    DefaultLinkingCondition = LinkMatchingTypes
};


class IVW_CORE_API SimpleCondition {
public:
    SimpleCondition() {}
    static bool canLink(const Property* src, const Property* dst);
    static LinkingConditions conditionType() { return LinkMatchingTypes;}
    static std::string conditionName() { return "Matching Type";}
};

class IVW_CORE_API PartiallyMatchingIdCondition {
public:
    PartiallyMatchingIdCondition() {}
    static bool canLink(const Property* src,const Property* dst);
    static LinkingConditions conditionType() { return LinkMatchingId;}
    static std::string conditionName() { return "Matching Id";}
};

class IVW_CORE_API AutoLinker {
public:
    AutoLinker() {}
    static bool canLink(const Property* src,const Property* dst, LinkingConditions);

};

} // namespace

#endif // IVW_LINKCONDITIONS_H