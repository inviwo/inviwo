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

#ifndef IVW_STRINGPROPERTY_H
#define IVW_STRINGPROPERTY_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/properties/templateproperty.h>

namespace inviwo {

/** class StringProperty
 *  \brief  The StringProperty holds a string as value.
 *
 *  The string property has 2 different widgets, one that just displays
 *  the value of the string. The other one makes it possible to edit the string.
 *  To use the editor widget you must specify it in the constructor and
 *  set the property semantics to Editor.
 *
 *  @see StringPropertyWidgetQt
 *  @see TextEditorWidgetQt
 */

class IVW_CORE_API StringProperty : public TemplateProperty<std::string> {
public:
    InviwoPropertyInfo();
    /**
     *  \brief Constructor for the StringProperty
     *
     *  The PropertySemantics can be set to Editor.
     *  Then a TextEditorWidget will be used instead of a StringPropertyWidget
     *
     */
    StringProperty(
        std::string identifier, std::string displayName, std::string value = "",
        InvalidationLevel invalidationLevel = INVALID_OUTPUT,
        PropertySemantics semantics = PropertySemantics::Default);

    StringProperty(const StringProperty& rhs);
    StringProperty& operator=(const StringProperty& that);
    StringProperty& operator=(const std::string& value);
    virtual StringProperty* clone() const override;
    virtual ~StringProperty();
};

}  // namespace

#endif // IVW_STRINGPROPERTY_H