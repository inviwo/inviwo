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

#ifndef IVW_BOOLPROPERTY_H
#define IVW_BOOLPROPERTY_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/properties/templateproperty.h>

namespace inviwo {
/** class BoolProperty
 *\brief The BoolProperty class provides a property holding boolean values
 *
 * A bool property are normally used by a processor.
 * The bool property is created and added in the constructor of the processor.
 * The propertys widget consist of a checkbox that sets the value of the property.
 * @see TemplateProperty
 * @see BoolPropertyWidgetQt
 */

class IVW_CORE_API BoolProperty : public TemplateProperty<bool> {
public:
    InviwoPropertyInfo();

    BoolProperty(std::string identifier, std::string displayName, bool value = false,
                 InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                 PropertySemantics semantics = PropertySemantics::Default);

    BoolProperty(const BoolProperty& rhs);
    BoolProperty& operator=(const BoolProperty& that);
    BoolProperty& operator=(const bool& value);
    virtual BoolProperty* clone() const override;
    virtual ~BoolProperty();
};

}  // namespace

#endif // IVW_BOOLPROPERTY_H