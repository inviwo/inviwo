/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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

#include <inviwo/core/properties/property.h>
#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/util/introspection.h>
#include <inviwo/core/util/assertion.h>
#include <inviwo/core/util/enumtraits.h>
#include <type_traits>
#include <iterator>

namespace inviwo {

/**
 *  Base class for the option properties
 *  @see TemplateOptionProperty
 */
class IVW_CORE_API BaseOptionProperty : public Property {
public:
    BaseOptionProperty(const std::string& identifier, const std::string& displayName,
                       InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                       PropertySemantics semantics = PropertySemantics::Default);

    BaseOptionProperty(const BaseOptionProperty& rhs);

    virtual ~BaseOptionProperty();

    virtual std::string getClassIdentifier() const override = 0;

    virtual BaseOptionProperty& clearOptions() = 0;

    virtual size_t size() const = 0;
    virtual size_t getSelectedIndex() const = 0;
    virtual const std::string& getSelectedIdentifier() const = 0;
    virtual const std::string& getSelectedDisplayName() const = 0;
    virtual const std::string& getOptionIdentifier(size_t index) const = 0;
    virtual const std::string& getOptionDisplayName(size_t index) const = 0;
    virtual std::vector<std::string> getIdentifiers() const = 0;
    virtual std::vector<std::string> getDisplayNames() const = 0;

    virtual bool setSelectedIndex(size_t index) = 0;
    virtual bool setSelectedIdentifier(const std::string& identifier) = 0;
    virtual bool setSelectedDisplayName(const std::string& name) = 0;

    virtual bool isSelectedIndex(size_t index) const = 0;
    virtual bool isSelectedIdentifier(const std::string& identifier) const = 0;
    virtual bool isSelectedDisplayName(const std::string& name) const = 0;

    virtual void set(const Property* srcProperty) override;
};

template <typename T>
class OptionPropertyOption : public Serializable {
public:
    OptionPropertyOption();
    OptionPropertyOption(const OptionPropertyOption& rhs);
    OptionPropertyOption(OptionPropertyOption&& rhs) noexcept;
    OptionPropertyOption& operator=(const OptionPropertyOption& that);
    OptionPropertyOption& operator=(OptionPropertyOption&& that) noexcept;

    OptionPropertyOption(const std::string& id, const std::string& name, const T& value);
    template <typename U = T,
              class = typename std::enable_if<std::is_same<U, std::string>::value, void>::type>
    OptionPropertyOption(const std::string& id, const std::string& name);

    template <typename U = T,
              class = typename std::enable_if<util::is_stream_insertable<U>::value, void>::type>
    OptionPropertyOption(const T& val);

    std::string id_;
    std::string name_;
    T value_ = T{};

    virtual void serialize(Serializer& s) const;
    virtual void deserialize(Deserializer& d);

    bool operator==(const OptionPropertyOption<T>& rhs) const;
    bool operator!=(const OptionPropertyOption<T>& rhs) const;
};

template <typename T>
class TemplateOptionProperty : public BaseOptionProperty {

public:
    using value_type = T;

    TemplateOptionProperty(const std::string& identifier, const std::string& displayName,
                           InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                           PropertySemantics semantics = PropertySemantics::Default);

    TemplateOptionProperty(const std::string& identifier, const std::string& displayName,
                           const std::vector<OptionPropertyOption<T>>& options,
                           size_t selectedIndex = 0,
                           InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                           PropertySemantics semantics = PropertySemantics::Default);

    template <typename U = T,
              class = typename std::enable_if<util::is_stream_insertable<U>::value, void>::type>
    TemplateOptionProperty(const std::string& identifier, const std::string& displayName,
                           const std::vector<T>& options, size_t selectedIndex = 0,
                           InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                           PropertySemantics semantics = PropertySemantics::Default);

    TemplateOptionProperty(const TemplateOptionProperty<T>& rhs);

    virtual TemplateOptionProperty<T>* clone() const override;
    virtual ~TemplateOptionProperty();

    virtual std::string getClassIdentifier() const override;

    /**
     * Implicit conversion operator. The OptionProperty will implicitly be converted to T when
     * possible.
     */
    operator const T&() const;

    /**
     * \brief Adds an option to the property
     *
     * Adds a option to the property and stores it as a struct in the options_
     * The option name is the name of the option that will be displayed in the widget.
     */
    TemplateOptionProperty& addOption(const std::string& identifier, const std::string& displayName,
                                      const T& value);
    template <typename U = T,
              class = typename std::enable_if<std::is_same<U, std::string>::value, void>::type>
    TemplateOptionProperty& addOption(const std::string& identifier,
                                      const std::string& displayName) {
        addOption(identifier, displayName, identifier);
        return *this;
    }

    virtual TemplateOptionProperty& removeOption(const std::string& identifier);
    virtual TemplateOptionProperty& removeOption(size_t index);
    virtual TemplateOptionProperty& clearOptions() override;

    virtual size_t size() const override;
    virtual size_t getSelectedIndex() const override;
    virtual const std::string& getSelectedIdentifier() const override;
    virtual const std::string& getSelectedDisplayName() const override;
    const T& getSelectedValue() const;
    virtual std::vector<std::string> getIdentifiers() const override;
    virtual std::vector<std::string> getDisplayNames() const override;
    std::vector<T> getValues() const;
    const std::vector<OptionPropertyOption<T>>& getOptions() const;

    virtual const std::string& getOptionIdentifier(size_t index) const override;
    virtual const std::string& getOptionDisplayName(size_t index) const override;
    const T& getOptionValue(size_t index) const;
    const OptionPropertyOption<T>& getOptions(size_t index) const;

    virtual bool setSelectedIndex(size_t index) override;
    virtual bool setSelectedIdentifier(const std::string& identifier) override;
    virtual bool setSelectedDisplayName(const std::string& name) override;
    bool setSelectedValue(const T& val);
    virtual TemplateOptionProperty& replaceOptions(const std::vector<std::string>& ids,
                                                   const std::vector<std::string>& displayNames,
                                                   const std::vector<T>& values);
    virtual TemplateOptionProperty& replaceOptions(std::vector<OptionPropertyOption<T>> options);

    template <typename U = T,
              class = typename std::enable_if<util::is_stream_insertable<U>::value, void>::type>
    TemplateOptionProperty& replaceOptions(const std::vector<T>& options);

    virtual bool isSelectedIndex(size_t index) const override;
    virtual bool isSelectedIdentifier(const std::string& identifier) const override;
    virtual bool isSelectedDisplayName(const std::string& name) const override;
    bool isSelectedValue(const T& val) const;

    const T& get() const;
    const T& operator*() const;
    const T* operator->() const;
    void set(const T& value);
    virtual void set(const Property* srcProperty) override;

    /**
     * Sets the default state, since the constructor can't add any default state, you must call this
     * function after adding all the default options, usually in the processor constructor.
     * @see Property::setCurrentStateAsDefault()
     */
    virtual TemplateOptionProperty& setCurrentStateAsDefault() override;
    virtual TemplateOptionProperty& resetToDefaultState() override;

    virtual std::string getClassIdentifierForWidget() const override;
    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

    virtual Document getDescription() const override;

protected:
    size_t selectedIndex_;
    std::vector<OptionPropertyOption<T>> options_;

private:
    size_t defaultSelectedIndex_;
    std::vector<OptionPropertyOption<T>> defaultOptions_;
};

template <typename T, typename U>
bool operator==(const TemplateOptionProperty<T>& lhs, const U& rhs) {
    return lhs.get() == rhs;
}
template <typename T, typename U>
bool operator==(const U& lhs, const TemplateOptionProperty<T>& rhs) {
    return lhs == rhs.get();
}

template <typename T, typename U>
bool operator!=(const TemplateOptionProperty<T>& lhs, const U& rhs) {
    return lhs.get() != rhs;
}
template <typename T, typename U>
bool operator!=(const U& lhs, const TemplateOptionProperty<T>& rhs) {
    return lhs != rhs.get();
}

template <typename T>
struct PropertyTraits<TemplateOptionProperty<T>> {
    static std::string classIdentifier() {
        if constexpr (std::is_enum_v<T>) {
            return "org.inviwo.OptionProperty" + util::enumName<T>();
        } else {
            return "org.inviwo.OptionProperty" + Defaultvalues<T>::getName();
        }
    }
};

template <typename T>
std::string TemplateOptionProperty<T>::getClassIdentifier() const {
    return PropertyTraits<TemplateOptionProperty<T>>::classIdentifier();
}

template <typename T>
std::string TemplateOptionProperty<T>::getClassIdentifierForWidget() const {
    if constexpr (std::is_enum_v<T>) {
        return "org.inviwo.OptionProperty" + Defaultvalues<std::underlying_type_t<T>>::getName();
    } else {
        return PropertyTraits<TemplateOptionProperty<T>>::classIdentifier();
    }
}

using OptionPropertyUIntOption = OptionPropertyOption<unsigned int>;
using OptionPropertyIntOption = OptionPropertyOption<int>;
using OptionPropertySize_tOption = OptionPropertyOption<size_t>;
using OptionPropertyFloatOption = OptionPropertyOption<float>;
using OptionPropertyDoubleOption = OptionPropertyOption<double>;
using OptionPropertyStringOption = OptionPropertyOption<std::string>;

using OptionPropertyUInt = TemplateOptionProperty<unsigned int>;
using OptionPropertyInt = TemplateOptionProperty<int>;
using OptionPropertySize_t = TemplateOptionProperty<size_t>;
using OptionPropertyFloat = TemplateOptionProperty<float>;
using OptionPropertyDouble = TemplateOptionProperty<double>;
using OptionPropertyString = TemplateOptionProperty<std::string>;

template <typename T>
OptionPropertyOption<T>::OptionPropertyOption() = default;

template <typename T>
OptionPropertyOption<T>::OptionPropertyOption(const OptionPropertyOption& rhs) = default;
template <typename T>
OptionPropertyOption<T>::OptionPropertyOption(OptionPropertyOption&& rhs) noexcept = default;
template <typename T>
OptionPropertyOption<T>& OptionPropertyOption<T>::operator=(const OptionPropertyOption& that) =
    default;
template <typename T>
OptionPropertyOption<T>& OptionPropertyOption<T>::operator=(OptionPropertyOption&& that) noexcept =
    default;

template <typename T>
OptionPropertyOption<T>::OptionPropertyOption(const std::string& id, const std::string& name,
                                              const T& value)
    : id_(id), name_(name), value_(value) {}

template <typename T>
template <typename U, class>
OptionPropertyOption<T>::OptionPropertyOption(const std::string& id, const std::string& name)
    : id_(id), name_(name), value_(id) {}

template <typename T>
template <typename U, class>
OptionPropertyOption<T>::OptionPropertyOption(const T& val)
    : id_(toString(val)), name_(camelCaseToHeader(toString(val))), value_(val) {}

template <typename T>
void OptionPropertyOption<T>::serialize(Serializer& s) const {
    s.serialize("id", id_);
    s.serialize("name", name_);
    s.serialize("value", value_);
}

template <typename T>
void OptionPropertyOption<T>::deserialize(Deserializer& d) {
    d.deserialize("id", id_);
    d.deserialize("name", name_);
    d.deserialize("value", value_);
}

template <typename T>
bool OptionPropertyOption<T>::operator==(const OptionPropertyOption<T>& rhs) const {
    return id_ == rhs.id_ && name_ == rhs.name_ && value_ == rhs.value_;
}

template <typename T>
bool OptionPropertyOption<T>::operator!=(const OptionPropertyOption<T>& rhs) const {
    return !operator==(rhs);
}

template <typename T>
TemplateOptionProperty<T>::TemplateOptionProperty(const std::string& identifier,
                                                  const std::string& displayName,
                                                  InvalidationLevel invalidationLevel,
                                                  PropertySemantics semantics)
    : BaseOptionProperty(identifier, displayName, invalidationLevel, semantics)
    , selectedIndex_(0)
    , defaultSelectedIndex_(0) {}

template <typename T>
TemplateOptionProperty<T>::TemplateOptionProperty(
    const std::string& identifier, const std::string& displayName,
    const std::vector<OptionPropertyOption<T>>& options, size_t selectedIndex,
    InvalidationLevel invalidationLevel, PropertySemantics semantics)
    : BaseOptionProperty(identifier, displayName, invalidationLevel, semantics)
    , selectedIndex_(std::min(selectedIndex, options.size() - 1))
    , options_(options)
    , defaultSelectedIndex_(selectedIndex_)
    , defaultOptions_(options_) {}

template <typename T>
template <typename U, class>
TemplateOptionProperty<T>::TemplateOptionProperty(
    const std::string& identifier, const std::string& displayName, const std::vector<T>& options,
    size_t selectedIndex, InvalidationLevel invalidationLevel, PropertySemantics semantics)
    : BaseOptionProperty(identifier, displayName, invalidationLevel, semantics)
    , selectedIndex_(std::min(selectedIndex, options.size() - 1))
    , options_()
    , defaultSelectedIndex_(selectedIndex_)
    , defaultOptions_() {

    for (const auto& option : options) {
        options_.emplace_back(option);
        defaultOptions_.emplace_back(option);
    }
}

template <typename T>
TemplateOptionProperty<T>::TemplateOptionProperty(const TemplateOptionProperty<T>& rhs) = default;

template <typename T>
TemplateOptionProperty<T>::~TemplateOptionProperty() = default;

template <typename T>
TemplateOptionProperty<T>& TemplateOptionProperty<T>::addOption(const std::string& identifier,
                                                                const std::string& displayName,
                                                                const T& value) {
    options_.push_back(OptionPropertyOption<T>(identifier, displayName, value));

    // in case we add the first option, we also select it
    if (options_.size() == 1) {
        selectedIndex_ = 0;
    }

    propertyModified();
    return *this;
}

template <typename T>
TemplateOptionProperty<T>& TemplateOptionProperty<T>::removeOption(size_t index) {
    if (options_.empty()) return *this;
    std::string id = getSelectedIdentifier();
    options_.erase(options_.begin() + index);
    if (!setSelectedIdentifier(id)) {
        selectedIndex_ = 0;
    }
    propertyModified();
    return *this;
}

template <typename T>
TemplateOptionProperty<T>& TemplateOptionProperty<T>::removeOption(const std::string& identifier) {
    if (options_.empty()) return *this;
    std::string id = getSelectedIdentifier();
    util::erase_remove_if(
        options_, [&](const OptionPropertyOption<T>& opt) { return opt.id_ == identifier; });
    if (!setSelectedIdentifier(id)) {
        selectedIndex_ = 0;
    }
    propertyModified();
    return *this;
}

template <typename T>
TemplateOptionProperty<T>& TemplateOptionProperty<T>::clearOptions() {
    options_.clear();
    selectedIndex_ = 0;
    return *this;
}

// Getters
template <typename T>
size_t TemplateOptionProperty<T>::size() const {
    return options_.size();
}

template <typename T>
size_t TemplateOptionProperty<T>::getSelectedIndex() const {
    return selectedIndex_;
}

template <typename T>
const std::string& TemplateOptionProperty<T>::getSelectedIdentifier() const {
    ivwAssert(selectedIndex_ < options_.size(),
              "Index out of range (number of options: " << options_.size()
                                                        << ", index: " << selectedIndex_ << ")");
    return options_[selectedIndex_].id_;
}

template <typename T>
const std::string& TemplateOptionProperty<T>::getSelectedDisplayName() const {
    ivwAssert(selectedIndex_ < options_.size(),
              "Index out of range (number of options: " << options_.size()
                                                        << ", index: " << selectedIndex_ << ")");
    return options_[selectedIndex_].name_;
}

template <typename T>
const T& TemplateOptionProperty<T>::getSelectedValue() const {
    ivwAssert(selectedIndex_ < options_.size(),
              "Index out of range (number of options: " << options_.size()
                                                        << ", index: " << selectedIndex_ << ")");
    return options_[selectedIndex_].value_;
}

template <typename T>
TemplateOptionProperty<T>::operator const T&() const {
    ivwAssert(selectedIndex_ < options_.size(),
              "Index out of range (number of options: " << options_.size()
                                                        << ", index: " << selectedIndex_ << ")");
    return options_[selectedIndex_].value_;
}

template <typename T>
std::vector<std::string> TemplateOptionProperty<T>::getIdentifiers() const {
    std::vector<std::string> result;
    for (size_t i = 0; i < options_.size(); i++) {
        result.push_back(options_[i].id_);
    }
    return result;
}

template <typename T>
std::vector<std::string> TemplateOptionProperty<T>::getDisplayNames() const {
    std::vector<std::string> result;
    for (size_t i = 0; i < options_.size(); i++) {
        result.push_back(options_[i].name_);
    }
    return result;
}

template <typename T>
std::vector<T> TemplateOptionProperty<T>::getValues() const {
    std::vector<T> result;
    for (size_t i = 0; i < options_.size(); i++) {
        result.push_back(options_[i].value_);
    }
    return result;
}

template <typename T>
const std::vector<OptionPropertyOption<T>>& TemplateOptionProperty<T>::getOptions() const {
    return options_;
}

template <typename T>
const std::string& TemplateOptionProperty<T>::getOptionIdentifier(size_t index) const {
    return options_[index].id_;
}
template <typename T>
const std::string& TemplateOptionProperty<T>::getOptionDisplayName(size_t index) const {
    return options_[index].name_;
}
template <typename T>
const T& TemplateOptionProperty<T>::getOptionValue(size_t index) const {
    return options_[index].value_;
}
template <typename T>
const OptionPropertyOption<T>& TemplateOptionProperty<T>::getOptions(size_t index) const {
    return options_[index];
}

// Setters
template <typename T>
bool TemplateOptionProperty<T>::setSelectedIndex(size_t option) {
    if (selectedIndex_ == option) {
        return true;
    } else if (option < options_.size()) {
        selectedIndex_ = option;
        propertyModified();
        return true;
    } else {
        return false;
    }
}

template <typename T>
bool TemplateOptionProperty<T>::setSelectedIdentifier(const std::string& identifier) {
    auto it = util::find_if(options_, [&](auto& opt) { return opt.id_ == identifier; });
    if (it != options_.end()) {
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

template <typename T>
bool TemplateOptionProperty<T>::setSelectedDisplayName(const std::string& name) {
    auto it = util::find_if(options_, [&](auto& opt) { return opt.name_ == name; });
    if (it != options_.end()) {
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

template <typename T>
bool TemplateOptionProperty<T>::setSelectedValue(const T& val) {
    auto it = util::find_if(options_, [&](auto& opt) { return opt.value_ == val; });
    if (it != options_.end()) {
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

template <typename T>
TemplateOptionProperty<T>& TemplateOptionProperty<T>::replaceOptions(
    const std::vector<std::string>& ids, const std::vector<std::string>& displayNames,
    const std::vector<T>& values) {
    std::string selectId{};
    if (!options_.empty()) selectId = getSelectedIdentifier();

    options_.clear();
    for (size_t i = 0; i < ids.size(); i++)
        options_.emplace_back(ids[i], displayNames[i], values[i]);

    auto it = util::find_if(options_, [&](auto& opt) { return opt.id_ == selectId; });
    if (it != options_.end()) {
        selectedIndex_ = std::distance(options_.begin(), it);
    } else {
        selectedIndex_ = 0;
    }
    propertyModified();
    return *this;
}

template <typename T>
TemplateOptionProperty<T>& TemplateOptionProperty<T>::replaceOptions(
    std::vector<OptionPropertyOption<T>> options) {
    std::string selectId{};
    if (!options_.empty()) selectId = getSelectedIdentifier();

    options_ = std::move(options);
    auto it = util::find_if(options_, [&](auto& opt) { return opt.id_ == selectId; });
    if (it != options_.end()) {
        selectedIndex_ = std::distance(options_.begin(), it);
    } else {
        selectedIndex_ = 0;
    }
    propertyModified();
    return *this;
}

template <typename T>
template <typename U, class>
TemplateOptionProperty<T>& TemplateOptionProperty<T>::replaceOptions(
    const std::vector<T>& options) {

    std::string selectId{};
    if (!options_.empty()) selectId = getSelectedIdentifier();

    options_.clear();
    for (size_t i = 0; i < options.size(); i++) options_.emplace_back(options[i]);

    auto it = util::find_if(options_, [&](auto& opt) { return opt.id_ == selectId; });
    if (it != options_.end()) {
        selectedIndex_ = std::distance(options_.begin(), it);
    } else {
        selectedIndex_ = 0;
    }
    propertyModified();
    return *this;
}

// Is...
template <typename T>
bool TemplateOptionProperty<T>::isSelectedIndex(size_t index) const {
    return index == selectedIndex_;
}

template <typename T>
bool TemplateOptionProperty<T>::isSelectedIdentifier(const std::string& identifier) const {
    return identifier == options_[selectedIndex_].id_;
}

template <typename T>
bool TemplateOptionProperty<T>::isSelectedDisplayName(const std::string& name) const {
    return name == options_[selectedIndex_].name_;
}

template <typename T>
bool TemplateOptionProperty<T>::isSelectedValue(const T& val) const {
    return val == options_[selectedIndex_].value_;
}

// Convenience
template <typename T>
const T& TemplateOptionProperty<T>::get() const {
    return options_[selectedIndex_].value_;
}

template <typename T>
const T& TemplateOptionProperty<T>::operator*() const {
    return options_[selectedIndex_].value_;
}

template <typename T>
const T* TemplateOptionProperty<T>::operator->() const {
    return &(options_[selectedIndex_].value_);
}

template <typename T>
void TemplateOptionProperty<T>::set(const T& value) {
    setSelectedValue(value);
}

template <typename T>
void TemplateOptionProperty<T>::set(const Property* srcProperty) {
    BaseOptionProperty::set(srcProperty);
}

template <typename T>
TemplateOptionProperty<T>& TemplateOptionProperty<T>::resetToDefaultState() {
    bool modified = false;
    if (options_ != defaultOptions_) {
        modified = true;
        options_ = defaultOptions_;
    }
    if (selectedIndex_ != defaultSelectedIndex_) {
        modified = true;
        selectedIndex_ = defaultSelectedIndex_;
    }

    if (defaultOptions_.empty()) {
        LogWarn("Resetting option property: " + this->getIdentifier() +
                " to an empty option list. Probably the default values have never been set, " +
                "Remember to call setCurrentStateAsDefault() after adding all the options.")
    }

    if (modified) this->propertyModified();
    return *this;
}

template <typename T>
TemplateOptionProperty<T>& TemplateOptionProperty<T>::setCurrentStateAsDefault() {
    Property::setCurrentStateAsDefault();
    defaultSelectedIndex_ = selectedIndex_;
    defaultOptions_ = options_;
    return *this;
}

template <typename T>
void TemplateOptionProperty<T>::serialize(Serializer& s) const {
    BaseOptionProperty::serialize(s);
    if (this->serializationMode_ == PropertySerializationMode::None) return;

    if ((this->serializationMode_ == PropertySerializationMode::All ||
         options_ != defaultOptions_) &&
        options_.size() > 0) {
        s.serialize("options", options_, "option");
    }
    if ((this->serializationMode_ == PropertySerializationMode::All ||
         selectedIndex_ != defaultSelectedIndex_) &&
        options_.size() > 0) {
        s.serialize("selectedIdentifier", getSelectedIdentifier());
    }
}

template <typename T>
void TemplateOptionProperty<T>::deserialize(Deserializer& d) {
    BaseOptionProperty::deserialize(d);
    if (this->serializationMode_ == PropertySerializationMode::None) return;

    auto oldIndex = selectedIndex_;
    auto oldOptions = options_;

    // We need to reset to default since that state was never serialized.
    if (this->serializationMode_ != PropertySerializationMode::All) {
        options_ = defaultOptions_;
        selectedIndex_ = defaultSelectedIndex_;
    }

    d.deserialize("options", options_, "option");

    if (!options_.empty()) {
        std::string identifier;
        d.deserialize("selectedIdentifier", identifier);
        auto it = util::find_if(options_, [&](auto& opt) { return opt.id_ == identifier; });
        if (it != options_.end()) {
            selectedIndex_ = std::distance(options_.begin(), it);
        }
    } else {
        selectedIndex_ = 0;
    }

    if (oldIndex != selectedIndex_ || oldOptions != options_) propertyModified();
}

template <typename T>
Document TemplateOptionProperty<T>::getDescription() const {
    using P = Document::PathComponent;
    using H = utildoc::TableBuilder::Header;

    Document doc = BaseOptionProperty::getDescription();

    if (options_.size() > 0) {
        auto table = doc.get({P("html"), P("body"), P("table", {{"identifier", "propertyInfo"}})});
        utildoc::TableBuilder tb(table);
        tb(H("Number of Options"), options_.size());
        tb(H("Selected Index"), selectedIndex_);
        tb(H("Selected Name"), options_[selectedIndex_].name_);
        tb(H("Selected Value"), options_[selectedIndex_].value_);
    }
    return doc;
}

template <typename T>
TemplateOptionProperty<T>* TemplateOptionProperty<T>::clone() const {
    return new TemplateOptionProperty<T>(*this);
}

extern template class IVW_CORE_TMPL_EXP OptionPropertyOption<unsigned int>;
extern template class IVW_CORE_TMPL_EXP OptionPropertyOption<int>;
extern template class IVW_CORE_TMPL_EXP OptionPropertyOption<size_t>;
extern template class IVW_CORE_TMPL_EXP OptionPropertyOption<float>;
extern template class IVW_CORE_TMPL_EXP OptionPropertyOption<double>;
extern template class IVW_CORE_TMPL_EXP OptionPropertyOption<std::string>;

extern template class IVW_CORE_TMPL_EXP TemplateOptionProperty<unsigned int>;
extern template class IVW_CORE_TMPL_EXP TemplateOptionProperty<int>;
extern template class IVW_CORE_TMPL_EXP TemplateOptionProperty<size_t>;
extern template class IVW_CORE_TMPL_EXP TemplateOptionProperty<float>;
extern template class IVW_CORE_TMPL_EXP TemplateOptionProperty<double>;
extern template class IVW_CORE_TMPL_EXP TemplateOptionProperty<std::string>;

}  // namespace inviwo

#endif  // IVW_BASEOPTIONPROPERTY_H
