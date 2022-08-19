/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2021 Inviwo Foundation
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

#pragma once

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/properties/templateproperty.h>

#include <string>
#include <string_view>

namespace inviwo {

/**
 * \ingroup properties
 * \brief  The StringProperty holds a string as value.
 *
 * The string property has 2 different widgets, one that just displays
 * the value of the string. The other one makes it possible to edit the string.
 * To use the editor widget you must specify it in the constructor and
 * set the property semantics to Editor.
 *
 * @see StringPropertyWidgetQt
 * @see TextEditorWidgetQt
 */

class IVW_CORE_API StringProperty : public TemplateProperty<std::string> {
public:
    virtual std::string getClassIdentifier() const override;
    static const std::string classIdentifier;

    /**
     *  \brief Constructor for the StringProperty
     *  The PropertySemantics can be set to Editor.
     *  Then a TextEditorWidget will be used instead of a StringPropertyWidget
     */
    StringProperty(std::string_view identifier, std::string_view displayName, Document help,
                   std::string_view value = "",
                   InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                   PropertySemantics semantics = PropertySemantics::Default);

    /**
     *  \brief Constructor for the StringProperty
     *  The PropertySemantics can be set to Editor.
     *  Then a TextEditorWidget will be used instead of a StringPropertyWidget
     */
    StringProperty(std::string_view identifier, std::string_view displayName,
                   std::string_view value = "",
                   InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                   PropertySemantics semantics = PropertySemantics::Default);

    StringProperty(const StringProperty& rhs);
    StringProperty& operator=(const std::string& value);
    virtual StringProperty* clone() const override;
    virtual ~StringProperty() = default;

    StringProperty& set(std::string_view value);
    StringProperty& set(const char* value);
    using TemplateProperty<std::string>::set;

    StringProperty& setDefault(std::string_view value);
    using TemplateProperty<std::string>::setDefault;

    operator std::string_view() const;

    virtual Document getDescription() const override;
};

}  // namespace inviwo
