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

#include <inviwo/core/properties/property.h>
#include <inviwo/core/common/inviwocoredefine.h>
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
    BaseOptionProperty(std::string_view identifier, std::string_view displayName,
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
    virtual bool setSelectedIdentifier(std::string_view identifier) = 0;
    virtual bool setSelectedDisplayName(std::string_view name) = 0;

    virtual bool isSelectedIndex(size_t index) const = 0;
    virtual bool isSelectedIdentifier(std::string_view identifier) const = 0;
    virtual bool isSelectedDisplayName(std::string_view name) const = 0;

    void set(const BaseOptionProperty* srcProperty);
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

    OptionPropertyOption(std::string_view id, std::string_view name, T value);
    template <typename U = T,
              class = typename std::enable_if<std::is_same<U, std::string>::value, void>::type>
    OptionPropertyOption(std::string_view id, std::string_view name);

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

/**
 * OptionProperty with a custom type @p T.
 *
 * For dynamic template option properties, @p T needs to have some traits defining the class
 * identifier. Default identifiers are provided for the standard types defined in defaultvalues.h.
 * If the template parameter @p T is a custom enum class, there needs to be a <tt>EnumTraits<T></tt>
 * struct defining the name of the enum. This enum name then becomes part of the property
 * identifier.
 *
 * @code
 * #include <inviwo/core/util/enumtraits.h>
 *
 * enum class MyEnum { Value1, Value2, Value3 };
 *
 * template <>
 * struct EnumTraits<MyEnum> {
 *     static std::string name() { return "MyEnum"; }
 * };
 *
 * registerProperty<TemplateOptionProperty<MyEnum>>();
 * @endcode
 *
 * For other data types, the class identifier must be provided using @c PropertyTraits.
 * @code
 * template <>
 * struct PropertyTraits<TemplateOptionProperty<MyType>>
 *     static const std::string& classIdentifier() {
 *         static const std::string identifier = "org.inviwo.OptionPropertyMyType";
 *         return identifier;
 *     }
 * };
 * @endcode
 *
 * @tparam T  internal value type of the option property
 * @see defaultvalues.h enumtraits.h propertytraits.h
 */
template <typename T>
class TemplateOptionProperty : public BaseOptionProperty {

public:
    using value_type = T;

    TemplateOptionProperty(std::string_view identifier, std::string_view displayName,
                           InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                           PropertySemantics semantics = PropertySemantics::Default);

    TemplateOptionProperty(std::string_view identifier, std::string_view displayName,
                           const std::vector<OptionPropertyOption<T>>& options,
                           size_t selectedIndex = 0,
                           InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                           PropertySemantics semantics = PropertySemantics::Default);

    template <typename U = T,
              class = typename std::enable_if<util::is_stream_insertable<U>::value, void>::type>
    TemplateOptionProperty(std::string_view identifier, std::string_view displayName,
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

    template <typename U = T,
              class = typename std::enable_if<std::is_same_v<U, std::string>, void>::type>
    operator std::string_view() const;

    /**
     * \brief Adds an option to the property
     *
     * Adds a option to the property and stores it as a struct in the options_
     * The option name is the name of the option that will be displayed in the widget.
     */
    TemplateOptionProperty& addOption(std::string_view identifier, std::string_view displayName,
                                      const T& value);

    template <typename U = T,
              class = typename std::enable_if<std::is_same_v<U, std::string>, void>::type>
    TemplateOptionProperty& addOption(std::string_view identifier, std::string_view displayName);

    template <typename U = T,
              class = typename std::enable_if<std::is_same_v<U, std::string>, void>::type>
    TemplateOptionProperty& addOption(std::string_view identifier, std::string_view displayName,
                                      std::string_view value);

    virtual TemplateOptionProperty& removeOption(std::string_view identifier);
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
    virtual bool setSelectedIdentifier(std::string_view identifier) override;
    virtual bool setSelectedDisplayName(std::string_view name) override;

    /**
     * @brief Set the selected index to that of the value provided if found in the list of options
     * @param val to set
     * @return True if the value was found in the list of options else false.
     */
    bool setSelectedValue(const T& val);

    /**
     * @brief Set the selected index to that of the value provided if found in the list of options
     * @param value to set
     * @return True if the value was found in the list of options else false.
     */
    template <typename U = T,
              class = typename std::enable_if<std::is_same_v<U, std::string>, void>::type>
    bool setSelectedValue(std::string_view value);

    virtual TemplateOptionProperty& replaceOptions(const std::vector<std::string>& ids,
                                                   const std::vector<std::string>& displayNames,
                                                   const std::vector<T>& values);
    virtual TemplateOptionProperty& replaceOptions(std::vector<OptionPropertyOption<T>> options);

    template <typename U = T,
              class = typename std::enable_if<util::is_stream_insertable<U>::value, void>::type>
    TemplateOptionProperty& replaceOptions(const std::vector<T>& options);

    virtual bool isSelectedIndex(size_t index) const override;
    virtual bool isSelectedIdentifier(std::string_view identifier) const override;
    virtual bool isSelectedDisplayName(std::string_view name) const override;
    bool isSelectedValue(const T& val) const;

    const T& get() const;
    const T& operator*() const;
    const T* operator->() const;

    /**
     * @brief Set the selected index to that of the value provided if found in the list of options
     * @param value to set
     * @return True if the value was found in the list of options else false.
     */
    bool set(const T& value);

    /**
     * @brief Set the selected index to that of the value provided if found in the list of options
     * @param value to set
     * @return True if the value was found in the list of options else false.
     */
    template <typename U = T,
              class = typename std::enable_if<std::is_same_v<U, std::string>, void>::type>
    bool set(std::string_view value);

    void set(const TemplateOptionProperty* srcProperty);
    virtual void set(const Property* srcProperty) override;

    /**
     * Sets the default state, since the constructor can't add any default state, you must call this
     * function after adding all the default options, usually in the processor constructor.
     * @see Property::setCurrentStateAsDefault()
     */
    virtual TemplateOptionProperty& setCurrentStateAsDefault() override;
    TemplateOptionProperty<T>& setDefault(const T& value);
    TemplateOptionProperty<T>& setDefaultSelectedIndex(size_t index);
    virtual TemplateOptionProperty& resetToDefaultState() override;
    virtual bool isDefaultState() const override;

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
    static const std::string& classIdentifier() {
        if constexpr (std::is_enum_v<T>) {
            static const std::string identifier = "org.inviwo.OptionProperty" + util::enumName<T>();
            return identifier;
        } else {
            static const std::string identifier =
                "org.inviwo.OptionProperty" + Defaultvalues<T>::getName();
            return identifier;
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
OptionPropertyOption<T>::OptionPropertyOption(std::string_view id, std::string_view name, T value)
    : id_(id), name_(name), value_(std::move(value)) {}

template <typename T>
template <typename U, class>
OptionPropertyOption<T>::OptionPropertyOption(std::string_view id, std::string_view name)
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
TemplateOptionProperty<T>::TemplateOptionProperty(std::string_view identifier,
                                                  std::string_view displayName,
                                                  InvalidationLevel invalidationLevel,
                                                  PropertySemantics semantics)
    : BaseOptionProperty(identifier, displayName, invalidationLevel, semantics)
    , selectedIndex_(0)
    , defaultSelectedIndex_(0) {}

template <typename T>
TemplateOptionProperty<T>::TemplateOptionProperty(
    std::string_view identifier, std::string_view displayName,
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
    std::string_view identifier, std::string_view displayName, const std::vector<T>& options,
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
TemplateOptionProperty<T>& TemplateOptionProperty<T>::addOption(std::string_view identifier,
                                                                std::string_view displayName,
                                                                const T& value) {
    options_.emplace_back(identifier, displayName, value);

    // in case we add the first option, we also select it
    if (options_.size() == 1) {
        selectedIndex_ = 0;
    }

    propertyModified();
    return *this;
}

template <typename T>
template <typename U, class>
TemplateOptionProperty<T>& TemplateOptionProperty<T>::addOption(std::string_view identifier,
                                                                std::string_view displayName) {
    addOption(identifier, displayName, identifier);
    return *this;
}

template <typename T>
template <typename U, class>
TemplateOptionProperty<T>& TemplateOptionProperty<T>::addOption(std::string_view identifier,
                                                                std::string_view displayName,
                                                                std::string_view value) {
    options_.emplace_back(identifier, displayName, std::string{value});

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
TemplateOptionProperty<T>& TemplateOptionProperty<T>::removeOption(std::string_view identifier) {
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
    IVW_ASSERT(selectedIndex_ < options_.size(),
               "Index out of range (number of options: " << options_.size()
                                                         << ", index: " << selectedIndex_ << ")");
    return options_[selectedIndex_].id_;
}

template <typename T>
const std::string& TemplateOptionProperty<T>::getSelectedDisplayName() const {
    IVW_ASSERT(selectedIndex_ < options_.size(),
               "Index out of range (number of options: " << options_.size()
                                                         << ", index: " << selectedIndex_ << ")");
    return options_[selectedIndex_].name_;
}

template <typename T>
const T& TemplateOptionProperty<T>::getSelectedValue() const {
    IVW_ASSERT(selectedIndex_ < options_.size(),
               "Index out of range (number of options: " << options_.size()
                                                         << ", index: " << selectedIndex_ << ")");
    return options_[selectedIndex_].value_;
}

template <typename T>
TemplateOptionProperty<T>::operator const T&() const {
    IVW_ASSERT(selectedIndex_ < options_.size(),
               "Index out of range (number of options: " << options_.size()
                                                         << ", index: " << selectedIndex_ << ")");
    return options_[selectedIndex_].value_;
}

template <typename T>
template <typename U, class>
TemplateOptionProperty<T>::operator std::string_view() const {
    IVW_ASSERT(selectedIndex_ < options_.size(),
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
bool TemplateOptionProperty<T>::setSelectedIdentifier(std::string_view identifier) {
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
bool TemplateOptionProperty<T>::setSelectedDisplayName(std::string_view name) {
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
template <typename U, class>
bool TemplateOptionProperty<T>::setSelectedValue(std::string_view val) {
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
    std::vector<OptionPropertyOption<T>> options) {
    if (options != options_) {

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
    }
    return *this;
}

template <typename T>
TemplateOptionProperty<T>& TemplateOptionProperty<T>::replaceOptions(
    const std::vector<std::string>& ids, const std::vector<std::string>& displayNames,
    const std::vector<T>& values) {

    IVW_ASSERT(ids.size() == displayNames.size(), "Arguments has to have the same size");
    IVW_ASSERT(ids.size() == values.size(), "Arguments has to have the same size");

    std::vector<OptionPropertyOption<T>> options;
    for (size_t i = 0; i < ids.size(); i++) {
        options.emplace_back(ids[i], displayNames[i], values[i]);
    }
    return replaceOptions(std::move(options));
}

template <typename T>
template <typename U, class>
TemplateOptionProperty<T>& TemplateOptionProperty<T>::replaceOptions(const std::vector<T>& values) {

    std::vector<OptionPropertyOption<T>> options;
    for (size_t i = 0; i < values.size(); i++) {
        options.emplace_back(values[i]);
    }
    return replaceOptions(std::move(options));
}

// Is...
template <typename T>
bool TemplateOptionProperty<T>::isSelectedIndex(size_t index) const {
    return index == selectedIndex_;
}

template <typename T>
bool TemplateOptionProperty<T>::isSelectedIdentifier(std::string_view identifier) const {
    return identifier == options_[selectedIndex_].id_;
}

template <typename T>
bool TemplateOptionProperty<T>::isSelectedDisplayName(std::string_view name) const {
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
bool TemplateOptionProperty<T>::set(const T& value) {
    return setSelectedValue(value);
}

template <typename T>
template <typename U, class>
bool TemplateOptionProperty<T>::set(std::string_view value) {
    return setSelectedValue(value);
}

template <typename T>
void TemplateOptionProperty<T>::set(const TemplateOptionProperty* srcProperty) {
    BaseOptionProperty::set(srcProperty);
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
bool TemplateOptionProperty<T>::isDefaultState() const {
    return selectedIndex_ == defaultSelectedIndex_ && options_ == defaultOptions_;
}

template <typename T>
TemplateOptionProperty<T>& TemplateOptionProperty<T>::setCurrentStateAsDefault() {
    Property::setCurrentStateAsDefault();
    defaultSelectedIndex_ = selectedIndex_;
    defaultOptions_ = options_;
    return *this;
}

template <typename T>
TemplateOptionProperty<T>& TemplateOptionProperty<T>::setDefault(const T& value) {
    auto it = util::find_if(options_, [&](auto& opt) { return opt.value_ == value; });
    if (it != options_.end()) {
        defaultSelectedIndex_ = std::distance(options_.begin(), it);
    }
    return *this;
}

template <typename T>
TemplateOptionProperty<T>& TemplateOptionProperty<T>::setDefaultSelectedIndex(size_t index) {
    defaultSelectedIndex_ = index;
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

/// @cond
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
/// @endcond

}  // namespace inviwo
