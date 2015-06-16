/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
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

#include <inviwo/core/links/linkconditions.h>
#include <inviwo/core/properties/propertyconvertermanager.h>

namespace inviwo {

bool SimpleCondition::canLink(const Property* src,const Property* dst) {
    return PropertyConverterManager::getPtr()->canConvert(src,dst);
}

bool PartiallyMatchingIdCondition::canLink(const Property* src, const Property* dst) {
    bool canLink = false;
    //conversion to lower case
    std::string srcIdentifier = src->getIdentifier();
    std::transform(srcIdentifier.begin(), srcIdentifier.end(), srcIdentifier.begin(), tolower);
    std::string dstIdentifier = dst->getIdentifier();
    std::transform(dstIdentifier.begin(), dstIdentifier.end(), dstIdentifier.begin(), tolower);
    std::string srcClassName = src->getClassIdentifier();
    std::transform(srcClassName.begin(), srcClassName.end(), srcClassName.begin(), tolower);
    std::string dstClassName = dst->getClassIdentifier();
    std::transform(dstClassName.begin(), dstClassName.end(), dstClassName.begin(), tolower);

    //does class name occurs in identifiers
    if (srcIdentifier.find(dstClassName)!=std::string::npos &&
        dstIdentifier.find(srcClassName)!=std::string::npos)
        canLink = true;

    //does identifier occur in other identifier
    if (srcIdentifier.find(dstIdentifier)!=std::string::npos ||
        dstIdentifier.find(srcIdentifier)!=std::string::npos)
        canLink = true;

    return canLink;
}

bool AutoLinker::canLink(const Property* src,const Property* dst, LinkingConditions givenConditions) {
    int satisfiedConditions = 0;



    //does properties have same class names
    if ((givenConditions&LinkMatchingTypes) && dst->CLASS_IDENTIFIER == src->CLASS_IDENTIFIER)
        satisfiedConditions |= ((int) LinkMatchingTypes);

    //does partially matching identifier strings
    if ((givenConditions&LinkMatchingId) && PartiallyMatchingIdCondition::canLink(src, dst))
        satisfiedConditions |= ((int) LinkMatchingId);

    if (givenConditions == satisfiedConditions && givenConditions)
        return true;

    return false;
}

}