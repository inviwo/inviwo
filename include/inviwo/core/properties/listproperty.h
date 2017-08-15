/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017 Inviwo Foundation
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

#ifndef IVW_LISTPROPERTY_H
#define IVW_LISTPROPERTY_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/optionproperty.h>

namespace inviwo {

/**
 * \class ListProperty
 * \brief A property that has specified sub-properties that can be added
 * Represents a List of properties that allows to add and delete items of this list
 */
template <typename T>
class IVW_CORE_API ListProperty : public CompositeProperty {
    static_assert(std::is_base_of<Property, T>::value, "T must be a property.");

private:
    std::string elementName_;
    std::unique_ptr<T> prefab_;

public:
    size_t numElements_;
    size_t maxNumElements_;

    ListProperty(std::string identifier, std::string displayName, std::string elementName = "Element",
                 T* prefab = nullptr, size_t maxNumberOfElements = 0,
                 InvalidationLevel = InvalidationLevel::InvalidResources,
                 PropertySemantics semantics = PropertySemantics::Default);
    ListProperty(const ListProperty& rhs);
    ListProperty& operator=(const ListProperty& that);
    virtual ListProperty* clone() const override;
    virtual ~ListProperty() = default;

    void addElement();
    void deleteElement();

    OptionPropertyString elementSelection_;
    ButtonProperty addElementButton_;
    ButtonProperty deleteElementButton_;
    CompositeProperty elements_;
};

}  // namespace inviwo

#endif  // IVW_LISTPROPERTY_H
