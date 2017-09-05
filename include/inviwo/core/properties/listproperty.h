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
class ListProperty : public CompositeProperty {
    static_assert(std::is_base_of<Property, T>::value, "T must be a property.");
public:  
    ListProperty(std::string identifier, std::string displayName, std::string elementName,
                 const T& prefab, size_t maxNumberOfElements = 0,
                 InvalidationLevel = InvalidationLevel::InvalidResources,
                 PropertySemantics semantics = PropertySemantics::Default);
    ListProperty(const ListProperty& rhs);
    ListProperty& operator=(const ListProperty& that);
    virtual ListProperty* clone() const override;
    virtual ~ListProperty() = default;

    void addElement();
    void deleteElement();
    size_t size() const {
        return elements_.size();
    }

    T& operator[](size_t);
    const T& operator[](size_t) const;

    size_t maxNumElements_;
    OptionPropertyString elementSelection_;
    CompositeProperty elements_;

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

private:
    std::string elementName_;
    const T prefab_;
    ButtonProperty addElementButton_;
    ButtonProperty deleteElementButton_;
};

template <typename T>
const T& ListProperty<T>::operator[](size_t i) const {
    return *static_cast<const T*>(elements_[i]);
}

template <typename T>
T& ListProperty<T>::operator[](size_t i) {
    return *static_cast<T*>(elements_[i]);
}

template <typename T>
ListProperty<T>::ListProperty(std::string identifier, std::string displayName,
                              std::string elementName, const T& prefab,
                              size_t maxNumberOfElements, InvalidationLevel invalidationLevel,
                              PropertySemantics semantics)
    : CompositeProperty(identifier, displayName, invalidationLevel, semantics)
    , maxNumElements_(maxNumberOfElements)
    , elementSelection_("elementSelection", "Element Selection")
    , elements_("lightsContainer", "Lights") 
    , elementName_(elementName)
    , prefab_(prefab)
    , addElementButton_("addElement", "Add Element", InvalidationLevel::InvalidResources)
    , deleteElementButton_("deleteElement", "Delete Element", InvalidationLevel::InvalidResources) {

    addElementButton_.onChange(this, &ListProperty<T>::addElement);
    deleteElementButton_.onChange(this, &ListProperty<T>::deleteElement);

    addProperty(elementSelection_);
    addProperty(deleteElementButton_);
    addProperty(addElementButton_);
    addProperty(elements_);
}

template <typename T>
ListProperty<T>::ListProperty(const ListProperty<T>& rhs)
    : CompositeProperty(rhs)
    , maxNumElements_(rhs.maxNumElements_)
    , elementSelection_(rhs.elementSelection_)
    , elements_(rhs.elements_) 
    , elementName_(rhs.elementName_)
    , prefab_(rhs.prefab_)
    , addElementButton_(rhs.addElementButton_)
    , deleteElementButton_(rhs.deleteElementButton_) {

    addElementButton_.onChange(this, &ListProperty<T>::addElement);
    deleteElementButton_.onChange(this, &ListProperty<T>::deleteElement);

    addProperty(elementSelection_);
    addProperty(deleteElementButton_);
    addProperty(addElementButton_);
    addProperty(elements_);
}

template <typename T>
ListProperty<T>& ListProperty<T>::operator=(const ListProperty<T>& that) {
    if (this != &that) {
        CompositeProperty::operator=(that);
        maxNumElements_ = that.maxNumElements_;
        elementSelection_ = that.elementSelection_;
        elements_ = that.elements_;
        elementName_ = that.elementName_;
        addElementButton_ = that.addElementButton_;
        deleteElementButton_ = that.deleteElementButton_;
    }
    return *this;
}

template <typename T>
ListProperty<T>* ListProperty<T>::clone() const {
    return new ListProperty<T>(*this);
}

template <typename T>
void ListProperty<T>::addElement() {
    if (size() < maxNumElements_ || maxNumElements_ == 0) {

        std::string num = std::to_string(size() + 1);

        elementSelection_.addOption("elementOption_" + num, elementName_ + " " + num);

        T* property = prefab_.clone();
        property->setSerializationMode(PropertySerializationMode::All);
        property->setIdentifier("element_" + num);
        property->setDisplayName(elementName_ + " " + num);
        elements_.addProperty(property, true);
    } else {
        LogInfo("The maximum number of elements is reached.");
    }
}

template <typename T>
void ListProperty<T>::deleteElement() {
    if (size() <= 0) return;

    size_t selectedElement = elementSelection_.getSelectedIndex();

    std::string identifier = elements_.getProperties().at(selectedElement)->getIdentifier();
    elements_.removeProperty(identifier);
    elementSelection_.removeOption(selectedElement);

    auto afterDeletion = elements_.getProperties();

    size_t loopCount = 1;
    for (Property* prop : afterDeletion) {
        T* casted = static_cast<T*>(prop);
        casted->setDisplayName(elementName_ + " " + std::to_string(loopCount));
        casted->setIdentifier("element_" + std::to_string(loopCount));
        loopCount++;
    }

    elementSelection_.clearOptions();

    for (size_t i = 1; i < afterDeletion.size() + 1; i++) {
        std::string str_i = std::to_string(i);
        elementSelection_.addOption("elementOption_" + str_i, elementName_ + " " + str_i);
    }
}

template <typename T>
void ListProperty<T>::serialize(Serializer& s) const {
    CompositeProperty::serialize(s);
    s.serialize("maxNumElements", maxNumElements_);
}

template <typename T>
void ListProperty<T>::deserialize(Deserializer& d) {
    CompositeProperty::deserialize(d);
    d.deserialize("maxNumElements", maxNumElements_);
}

}  // namespace inviwo

#endif  // IVW_LISTPROPERTY_H
