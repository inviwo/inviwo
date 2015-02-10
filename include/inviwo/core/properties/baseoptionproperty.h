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

#ifndef IVW_BASEOPTIONPROPERTY_H
#define IVW_BASEOPTIONPROPERTY_H


#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>

namespace inviwo {

/** \class BaseOptionProperty
 *  Base class for the option properties 
 *  @see TemplateOptionProperty
 */
class IVW_CORE_API BaseOptionProperty : public Property {
public:

    BaseOptionProperty(std::string identifier,
                       std::string displayName,
                       InvalidationLevel invalidationLevel=INVALID_OUTPUT,
                       PropertySemantics semantics=PropertySemantics::Default);

    BaseOptionProperty(const BaseOptionProperty& rhs);
    virtual ~BaseOptionProperty();

    virtual std::string getClassIdentifier() const = 0;

    virtual void clearOptions() = 0;

    virtual size_t size() const = 0;
    virtual size_t getSelectedIndex() const = 0;
    virtual std::string getSelectedIdentifier() const = 0;
    virtual std::string getSelectedDisplayName() const = 0;
    virtual std::vector<std::string> getIdentifiers() const = 0;
    virtual std::vector<std::string> getDisplayNames() const = 0;

    virtual bool setSelectedIndex(size_t index) = 0;
    virtual bool setSelectedIdentifier(std::string identifier) = 0;
    virtual bool setSelectedDisplayName(std::string name) = 0;

    virtual bool isSelectedIndex(size_t index) const = 0;
    virtual bool isSelectedIdentifier(std::string identifier) const = 0;
    virtual bool isSelectedDisplayName(std::string name) const = 0;

    virtual void set(const Property* srcProperty) {
        const BaseOptionProperty* optionSrcProp = dynamic_cast<const BaseOptionProperty*>(srcProperty);

        if(optionSrcProp) {
            size_t option = optionSrcProp->getSelectedIndex();

            if(option < size()) {
                setSelectedIndex(option);
            } else {
                setSelectedIndex(size() - 1);
            }
        }
        propertyModified();
    }
};

/** \class TemplateOptionProperty
 *  Template class for option properties
 *  @see OptionProperties
 *  @see BaseOptionProperty
 */
template<typename T>
class BaseTemplateOptionProperty : public BaseOptionProperty {
    
    template <typename U>
    struct Option : public IvwSerializable {
        Option() {}
        Option(const std::string& id, const std::string& name, U value)
            : id_(id), name_(name), value_(value) {}

        std::string id_;
        std::string name_;
        U value_;

        virtual void serialize(IvwSerializer& s) const {
            s.serialize("id", id_);
            s.serialize("name", name_);
            s.serialize("value", value_);
        }
        virtual void deserialize(IvwDeserializer& d) {
            d.deserialize("id", id_);
            d.deserialize("name", name_);
            d.deserialize("value", value_);
        }

        bool operator==(const Option<U>& rhs) const {
            return id_ == rhs.id_ && name_ == rhs.name_ && value_ == rhs.value_;
        }
        bool operator!=(const Option<U>& rhs) const {
            return !operator==(rhs);
        }
    };


    template <typename U>
    struct MatchId {
        MatchId(const std::string& s) : s_(s) {}
        bool operator()(const Option<U>& obj) const {
            return obj.id_ == s_;
        }
    private:
        const std::string& s_;
    };
    template <typename U>
    struct MatchName {
        MatchName(const std::string& s) : s_(s) {}
        bool operator()(const Option<U>& obj) const {
            return obj.name_ == s_;
        }
    private:
        const std::string& s_;
    };
    template <typename U>
    struct MatchValue {
        MatchValue(const T& s) : s_(s) {}
        bool operator()(const Option<U>& obj) const {
            return obj.value_ == s_;
        }
    private:
        const U& s_;
    };

public:
    typedef T valueType;

    BaseTemplateOptionProperty(std::string identifier, 
                               std::string displayName,
                               InvalidationLevel invalidationLevel=INVALID_OUTPUT,
                               PropertySemantics semantics=PropertySemantics::Default);
    
    BaseTemplateOptionProperty(const BaseTemplateOptionProperty<T>& rhs);
    BaseTemplateOptionProperty<T>& operator=(const BaseTemplateOptionProperty<T>& that);
    //virtual BaseTemplateOptionProperty<T>* clone() const = 0;
    virtual ~BaseTemplateOptionProperty();

    /**
     * \brief Adds an option to the property
     *
     * Adds a option to the property and stores it as a struct in the options_
     * The option name is the name of the option that will be displayed in the widget.
     *
     * @param std::string identifier identifier name
     * @param T value the value of the option
     */
    virtual void addOption(std::string identifier, std::string displayName, T value);
    virtual void removeOption(size_t index);
    virtual void clearOptions();

    virtual size_t size() const;
    virtual size_t getSelectedIndex() const;
    virtual std::string getSelectedIdentifier() const;
    virtual std::string getSelectedDisplayName() const;
    virtual T getSelectedValue() const;
    virtual std::vector<std::string> getIdentifiers() const;
    virtual std::vector<std::string> getDisplayNames() const;
    virtual std::vector<T> getValues() const;

    virtual bool setSelectedIndex(size_t index);
    virtual bool setSelectedIdentifier(std::string identifier);
    virtual bool setSelectedDisplayName(std::string name);
    virtual bool setSelectedValue(T val);
    virtual void replaceOptions(std::vector<std::string> ids, std::vector<std::string> displayNames, std::vector<T> values);
    
    virtual bool isSelectedIndex(size_t index) const;
    virtual bool isSelectedIdentifier(std::string identifier) const;
    virtual bool isSelectedDisplayName(std::string name) const;
    virtual bool isSelectedValue(T val) const;

    virtual T get() const;
    virtual void set(const T& value);
    virtual void set(const Property* srcProperty);

    /**
     * Sets the default state, since the constructor can't add any default state, you must call this
     * function after adding all the default options, usually in the processor constructor.
     * @see Property::setCurrentStateAsDefault()
     */
    virtual void setCurrentStateAsDefault();
    virtual void resetToDefaultState();

    virtual void serialize(IvwSerializer& s) const;
    virtual void deserialize(IvwDeserializer& d);

protected:
    size_t selectedIndex_;
    std::vector<Option<T> > options_;

private: 
    size_t defaultSelectedIndex_;
    std::vector<Option<T> > defaultOptions_;
};


template<typename T>
class TemplateOptionProperty : public BaseTemplateOptionProperty<T> {

public:
    InviwoPropertyInfo();
    TemplateOptionProperty(std::string identifier,
                           std::string displayName,
                           InvalidationLevel invalidationLevel = INVALID_OUTPUT,
                           PropertySemantics semantics = PropertySemantics::Default);
    TemplateOptionProperty(const TemplateOptionProperty<T>& rhs);
    TemplateOptionProperty<T>& operator=(const TemplateOptionProperty<T>& that);
//    virtual TemplateOptionProperty<T>* clone() const;
    virtual ~TemplateOptionProperty();
};

template <typename T> PropertyClassIdentifier(TemplateOptionProperty<T>, "org.inviwo.OptionProperty" + Defaultvalues<T>::getName());

// Specialization for strings.
class IVW_CORE_API OptionPropertyString : public TemplateOptionProperty<std::string> {
public:
    InviwoPropertyInfo();
    OptionPropertyString(
        std::string identifier, std::string displayName,
        InvalidationLevel invalidationLevel = INVALID_OUTPUT,
        PropertySemantics semantics = PropertySemantics::Default);

    OptionPropertyString(const OptionPropertyString& rhs);
    OptionPropertyString& operator=(const OptionPropertyString& that);
    virtual OptionPropertyString* clone() const;
    virtual ~OptionPropertyString();

    virtual void addOption(std::string identifier, std::string displayName, std::string value);
    virtual void addOption(std::string identifier, std::string displayName);
};

typedef TemplateOptionProperty<int> OptionPropertyInt;
typedef TemplateOptionProperty<float> OptionPropertyFloat;
typedef TemplateOptionProperty<double> OptionPropertyDouble;


template <typename T>
BaseTemplateOptionProperty<T>::BaseTemplateOptionProperty(std::string identifier,
                                                  std::string displayName,
                                                  InvalidationLevel invalidationLevel,
                                                  PropertySemantics semantics)
    : BaseOptionProperty(identifier, displayName, invalidationLevel, semantics)
    , selectedIndex_(0)
    , defaultSelectedIndex_(0) {
}


template<typename T>
inviwo::BaseTemplateOptionProperty<T>::BaseTemplateOptionProperty(const BaseTemplateOptionProperty<T>& rhs) 
    : BaseOptionProperty(rhs)
    , selectedIndex_(rhs.selectedIndex_)
    , options_(rhs.options_)
    , defaultSelectedIndex_(rhs.defaultSelectedIndex_)
    , defaultOptions_(rhs.defaultOptions_) {
}

template<typename T>
BaseTemplateOptionProperty<T>& BaseTemplateOptionProperty<T>::operator=(const BaseTemplateOptionProperty<T>& that) {
    if (this != &that) {
        BaseOptionProperty::operator=(that);
        selectedIndex_ = that.selectedIndex_;
        defaultSelectedIndex_ = that.defaultSelectedIndex_;
        options_.clear();
        options_.insert(options_.end(), that.options_.begin(), that.options_.end());
        defaultOptions_.clear();
        defaultOptions_.insert(defaultOptions_.end(), that.defaultOptions_.begin(), that.defaultOptions_.end());
    }
    return *this;
}

template<typename T>
BaseTemplateOptionProperty<T>::~BaseTemplateOptionProperty() {}

template<typename T>
void BaseTemplateOptionProperty<T>::addOption(std::string identifier, std::string displayName, T value) {
    options_.push_back(Option<T>(identifier, displayName, value));

    // in case we add the first option, we also select it
    if(options_.size() == 1) {
        selectedIndex_ = 0;
    } 
    
    propertyModified();
}

template<typename T>
void inviwo::BaseTemplateOptionProperty<T>::removeOption(size_t index) {
    std::string id = getSelectedIdentifier();
    options_.erase(options_.begin() + index);
    if (!setSelectedIdentifier(id)) {
        selectedIndex_ = 0;
    }
    propertyModified();
}

template<typename T>
void BaseTemplateOptionProperty<T>::clearOptions() {
    options_.clear();
    selectedIndex_ = 0;
}


// Getters
template<typename T>
size_t BaseTemplateOptionProperty<T>::size() const {
    return options_.size();
}

template<typename T>
size_t BaseTemplateOptionProperty<T>::getSelectedIndex() const {
    return selectedIndex_;
}

template<typename T>
std::string inviwo::BaseTemplateOptionProperty<T>::getSelectedIdentifier() const {
    return options_[selectedIndex_].id_;
}

template<typename T>
std::string inviwo::BaseTemplateOptionProperty<T>::getSelectedDisplayName() const {
    return options_[selectedIndex_].name_;
}

template<typename T>
T inviwo::BaseTemplateOptionProperty<T>::getSelectedValue() const {
    return options_[selectedIndex_].value_;
}

template<typename T>
std::vector<std::string> BaseTemplateOptionProperty<T>::getIdentifiers() const {
    std::vector<std::string> result;
    for(size_t i = 0; i < options_.size(); i++) {
        result.push_back(options_[i].id_);
    }
    return result;
}

template<typename T>
std::vector<std::string> BaseTemplateOptionProperty<T>::getDisplayNames() const {
    std::vector<std::string> result;
    for(size_t i = 0; i < options_.size(); i++) {
        result.push_back(options_[i].name_);
    }
    return result;
}

template<typename T>
std::vector<T> BaseTemplateOptionProperty<T>::getValues() const {
    std::vector<T> result;
    for(size_t i = 0; i < options_.size(); i++) {
        result.push_back(options_[i].value_);
    }
    return result;
}


// Setters
template<typename T>
bool BaseTemplateOptionProperty<T>::setSelectedIndex(size_t option) {
    if(selectedIndex_ == option){
        return true;
    } else if(option < options_.size()) {
        selectedIndex_ = option;
        propertyModified();
        return true;
    } else {
        return false;
    }
}

template<typename T>
bool inviwo::BaseTemplateOptionProperty<T>::setSelectedIdentifier(std::string identifier) {
    typename std::vector<Option<T> >::iterator it = std::find_if(options_.begin(), options_.end(), MatchId<T>(identifier));
    if (it != options_.end()) {
        size_t dist = std::distance(options_.begin(), it);
        if(selectedIndex_ != dist){
            selectedIndex_ = dist;
            propertyModified();
        }
        return true;
    } else {
        return false;
    }
}

template<typename T>
bool inviwo::BaseTemplateOptionProperty<T>::setSelectedDisplayName(std::string name) {
    typename std::vector<Option<T> >::iterator it = std::find_if(options_.begin(), options_.end(), MatchName<T>(name));
    if(it != options_.end()) {
        size_t dist = std::distance(options_.begin(), it);
        if (selectedIndex_ != dist) {
            selectedIndex_ = dist;
            propertyModified();
        }
        return true;
    } else {
        return false;
    }
}

template<typename T>
bool inviwo::BaseTemplateOptionProperty<T>::setSelectedValue(T val) {
    typename std::vector<Option<T> >::iterator it = std::find_if(options_.begin(), options_.end(), MatchValue<T>(val));
    if(it != options_.end()) {
        size_t dist = std::distance(options_.begin(), it);
        if (selectedIndex_ != dist) {
            selectedIndex_ = dist;
            propertyModified();
        }
        return true;
    } else {
        return false;
    }
}

template<typename T>
void inviwo::BaseTemplateOptionProperty<T>::replaceOptions(std::vector<std::string> ids, std::vector<std::string> displayNames, std::vector<T> values) {
    std::string selectId = getSelectedIdentifier();

    options_.clear();
    for (size_t i=0; i<ids.size(); i++)
        options_.push_back(Option<T>(ids[i], displayNames[i], values[i]));
    
    typename std::vector<Option<T> >::iterator it = std::find_if(options_.begin(), options_.end(), MatchId<T>(selectId));
    if (it != options_.end())
        selectedIndex_ = std::distance(options_.begin(), it);
    else
        selectedIndex_ = 0;

    propertyModified();
}

// Is...
template<typename T>
bool inviwo::BaseTemplateOptionProperty<T>::isSelectedIndex(size_t index) const {
    return index == selectedIndex_;
}

template<typename T>
bool BaseTemplateOptionProperty<T>::isSelectedIdentifier(std::string identifier) const {
    return identifier == options_[selectedIndex_].id_;
}

template<typename T>
bool inviwo::BaseTemplateOptionProperty<T>::isSelectedDisplayName(std::string name) const {
    return name == options_[selectedIndex_].name_;
}

template<typename T>
bool inviwo::BaseTemplateOptionProperty<T>::isSelectedValue(T val) const {
    return val == options_[selectedIndex_].value_;
}

// Convenience 
template<typename T>
T BaseTemplateOptionProperty<T>::get() const {
    return options_[selectedIndex_].value_;
}

template<typename T>
void BaseTemplateOptionProperty<T>::set(const T& value) {
    setSelectedValue(value);
}

template<typename T>
void BaseTemplateOptionProperty<T>::set(const Property* srcProperty) {
    BaseOptionProperty::set(srcProperty);
}

template<typename T>
void inviwo::BaseTemplateOptionProperty<T>::resetToDefaultState() {
    options_ = defaultOptions_;
    selectedIndex_ = defaultSelectedIndex_;
    
    if(defaultOptions_.empty()){
        LogWarn("Resetting to an empty option list. \
                 Probably the default values have never been set, \
                 Remember to call setCurrentStateAsDefault() after adding all the options.")
    }
    
    Property::resetToDefaultState();
}

template<typename T>
void inviwo::BaseTemplateOptionProperty<T>::setCurrentStateAsDefault() {
    Property::setCurrentStateAsDefault();
    defaultSelectedIndex_ = selectedIndex_;
    defaultOptions_ = options_;
}

template<typename T>
void BaseTemplateOptionProperty<T>::serialize(IvwSerializer& s) const {
    BaseOptionProperty::serialize(s);
    if ((this->serializationMode_==ALL || options_ != defaultOptions_ )&& options_.size() > 0) {
        s.serialize("options", options_, "option");
    }
    if ((this->serializationMode_==ALL || selectedIndex_ != defaultSelectedIndex_) && options_.size() > 0) {
        s.serialize("selectedIdentifier", getSelectedIdentifier());
    }
}

template<typename T>
void BaseTemplateOptionProperty<T>::deserialize(IvwDeserializer& d) {
    BaseOptionProperty::deserialize(d) ;
    
    d.deserialize("options", options_, "option");

    if (!options_.empty()) {
        std::string id = getSelectedIdentifier();
        d.deserialize("selectedIdentifier", id);
        setSelectedIdentifier(id);

        T value = getSelectedValue();
        d.deserialize("value", value);
        setSelectedValue(value);
    }
}

template <typename T>
TemplateOptionProperty<T>::TemplateOptionProperty(
    std::string identifier, std::string displayName,
    InvalidationLevel invalidationLevel /*= INVALID_OUTPUT*/,
    PropertySemantics semantics /*= PropertySemantics::Default*/)
    : BaseTemplateOptionProperty<T>(identifier, displayName, invalidationLevel, semantics) {}

template <typename T>
TemplateOptionProperty<T>::TemplateOptionProperty(const TemplateOptionProperty<T>& rhs) 
    : BaseTemplateOptionProperty<T>(rhs) {
}

template <typename T>
TemplateOptionProperty<T>& TemplateOptionProperty<T>::operator=(
    const TemplateOptionProperty<T>& that) {
    if (this != &that) {
        BaseTemplateOptionProperty<T>::operator=(that);
    }
    return *this;
}

// template <typename T>
// TemplateOptionProperty<T>* TemplateOptionProperty<T>::clone() const {
//     return new TemplateOptionProperty<T>(*this);
// }



template <typename T>
TemplateOptionProperty<T>::~TemplateOptionProperty() {}

} // namespace inviwo

#endif // IVW_BASEOPTIONPROPERTY_H