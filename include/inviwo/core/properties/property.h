/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2024 Inviwo Foundation
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
#include <inviwo/core/properties/valuewrapper.h>
#include <inviwo/core/properties/propertyobserver.h>
#include <inviwo/core/properties/propertywidget.h>
#include <inviwo/core/properties/propertysemantics.h>
#include <inviwo/core/properties/invalidationlevel.h>
#include <inviwo/core/processors/processortraits.h>
#include <inviwo/core/util/callback.h>
#include <inviwo/core/util/document.h>
#include <inviwo/core/metadata/metadataowner.h>
#include <inviwo/core/util/introspection.h>

#include <functional>
#include <type_traits>
#include <vector>
#include <string>
#include <string_view>

namespace inviwo {

class NetworkVisitor;
class Serializer;
class Deserializer;

/**
 * \class PropertyTraits
 * \brief A traits class for getting the class identifier from a Property.
 * This provides a customization point if one wants to generate the class identifier dynamically,
 * by specializing the traits for your kind of Property:
 * \code{.cpp}
 *     template <typename T>
 *     struct PropertyTraits<MyProperty<T>> {
 *        static constexpr std::string_view classIdentifier() {
 *           static constexpr auto cid = generateMyPropertyClassIdentifier<T>();
 *           return cid;
 *        }
 *     };
 * \endcode
 * The default behavior returns the static member "classIdentifier" or "CLASS_IDENTIFIER";
 */
template <typename T>
struct PropertyTraits {
    /**
     * The Class Identifier has to be globally unique. Use a reverse DNS naming scheme.
     * Example: "org.someorg.mypropertytype"
     * The default implementation will look for a static std::string_view member
     * "T::classIdentifier" or "T::CLASS_IDENTIFIER". In case it is not found an empty string will
     * be returned. An empty class identifier will be considered an error in various factories.
     */
    static constexpr std::string_view classIdentifier() { return util::classIdentifier<T>(); }
};

enum class ReadOnly { No, Yes };

/**
 *	\defgroup properties Properties
 *  \brief Properties represents a parameters to Processors.
 */

class PropertyOwner;
/**
 * \ingroup properties
 *
 * \brief A Property represents a parameter to a processor.
 *
 * Concepts:
 *  * __PropertyOwner__: A property can have a owner, usually a Processor or a CompositeProperty. If
 *    the property is modified, by calling Property::propertyModified then the property will set
 *    it's owner's invalidation level to the property's invalidation level, usually
 *    InvalidationLevel::InvalidOutput. This will in turn trigger a network evaluation that will
 *    update the processors to a valid state again.
 *
 *  * __Serializable__: A property is serializable to be able to store all the processor parameters
 *    for each Processor in the saved ProcessorNetwork
 *
 *  * __PropertySemantics__: A property can be set to one or several different semantics, which is
 *    often used to display different PropertyWidets. For example using a color picker for Color
 *    Semantic or a text field for Text Semantics.
 *
 *  * __Reset__: A property has a default state specified in the constructor, or optionally be
 *    calling Property::setCurrentStateAsDefault. The property can then also be reset to it's
 *    default state  by calling Property::resetToDefaultState. Both these functions are virtual and
 *    all property subclasses that introduce more state should make sure to implement these two
 *    function and also in their implementation make sure that to call the base class
 *    implementation.
 *
 *  * __PropertyWidget__: A property can have one or multiple PropertyWidgets. The widget are used
 *    in the user interface to implement interactivity
 */
class IVW_CORE_API Property : public PropertyObservable,
                              public virtual Serializable,
                              public MetaDataOwner {
public:
    virtual std::string_view getClassIdentifier() const = 0;

    explicit Property(std::string_view identifier = "", std::string_view displayName = "",
                      Document help = {},
                      InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                      PropertySemantics semantics = PropertySemantics::Default,
                      ReadOnly readOnly = ReadOnly::No);

    Property(std::string_view identifier, std::string_view displayName,
             InvalidationLevel invalidationLevel,
             PropertySemantics semantics = PropertySemantics::Default);

    /**
     * Creates a clone of this property. The clone will have the same identifier, hence the
     * identifier must be changed if the clone should be added to the same owner as this. The new
     * clone does not have any owner set.
     */
    virtual Property* clone() const = 0;
    /**
     * \brief Removes itself from its PropertyOwner.
     */
    virtual ~Property();

    /**
     * Property identifier has to be unique within the scope
     * of a PropertyOwner. Property identifiers should only contain alpha numeric
     * characters, "-" and "_".
     */
    virtual Property& setIdentifier(std::string_view identifier);
    virtual const std::string& getIdentifier() const;

    /**
     * @brief Get the property path as string
     * @return string of dot separated identifiers starting with a processor identifier followed
     * by property identifiers.
     */
    const std::string& getPath() const;
    void getPath(std::pmr::string& out) const;

    /**
     * \brief A property's name displayed to the user
     */
    virtual Property& setDisplayName(std::string_view displayName);
    virtual const std::string& getDisplayName() const;

    /**
     * \brief Returns which property's widget should be used
     * when the WidgetFactory tries to create a widget.
     * Defaults to getClassIdentifier(), should only be overridden
     * if a subclass want to reuse another property's widget.
     */
    virtual std::string_view getClassIdentifierForWidget() const;

    virtual Property& setSemantics(const PropertySemantics& semantics);
    virtual PropertySemantics getSemantics() const;

    /**
     * \brief Enable or disable editing of property
     */
    virtual Property& setReadOnly(bool value);
    virtual bool getReadOnly() const;

    virtual Property& setInvalidationLevel(InvalidationLevel invalidationLevel);
    virtual InvalidationLevel getInvalidationLevel() const;

    virtual void setOwner(PropertyOwner* owner);
    PropertyOwner* getOwner();
    const PropertyOwner* getOwner() const;

    /**
     * Register a widget for the property. Registered widgets will receive updateFromProperty calls
     * when the value state of the property changes. One Property can have multiple widgets.
     * The property does not take ownership of the widget.
     * @see deregisterProperty
     * @see PropertyWidget
     */
    void registerWidget(PropertyWidget* propertyWidget);

    /**
     * Deregister a widget, the widget will no longer receive updateFromProperty calls.
     * @see registerProperty
     * @see PropertyWidget
     */
    void deregisterWidget(PropertyWidget* propertyWidget);

    /**
     * Retrieve all registered widgets.
     * @see registerProperty
     */
    const std::vector<PropertyWidget*>& getWidgets() const;

    /**
     * This function should be called by property widgets before they initiate a property
     * change. This is needed because when the property is modified it needs to update all
     * of its widgets. And since it won't know if the change started in one of them we will
     * update the property widget that started the change
     * @see registerProperty
     * @see PropertyWidget
     */
    void setInitiatingWidget(PropertyWidget* propertyWidget);
    /**
     * Clear the initiatingWidget
     * @see setInitiatingWidget
     */
    void clearInitiatingWidget();
    /**
     * Update all widgets using the current property state except for the initiating widget.
     * @see setInitiatingWidget, clearInitiatingWidget
     */
    void updateWidgets();

    /**
     * Does the property have any registered widgets.
     * @see registerProperty
     */
    bool hasWidgets() const;

    virtual Property& setSerializationMode(PropertySerializationMode mode);
    virtual PropertySerializationMode getSerializationMode() const;

    /**
     * Save the current state of the property as the default. This state will then be used as a
     * reference when serializing, only state different from the default will be serialized.
     * This method should usually only be called once directly after construction in the processor
     * constructor after setting property specific state.
     * It is important that all overriding properties make sure to call the base class
     * implementation.
     */
    virtual Property& setCurrentStateAsDefault();

    /**
     * Reset the state of the property back to it's default value.
     * It is important that all overriding properties make sure to call the base class
     * implementation.
     */
    virtual Property& resetToDefaultState();

    /**
     * Check if the property is in it's default state, i.e. resetToDefaultState would do nothing
     * @see setCurrentStateAsDefault @see resetToDefaultState
     */
    virtual bool isDefaultState() const;

    /**
     * Determinate if the property should be included in the serialization
     * Depends on the PropertySerializationMode and if the property is in the default state.
     * If the mode is All it always return true, None always returns false, and default delegates to
     * isDefaultState()
     */
    virtual bool needsSerialization() const;

    virtual Property& propertyModified();
    virtual void setValid();
    virtual Property& setModified();
    virtual bool isModified() const;

    /**
     * Set the value of this to that of src. The "value" is in this case considered to be for
     * example the string in a StringProperty or the float value in a FloatProperty. But not things
     * like the identifier of display name.
     */
    virtual void set(const Property* src);

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

    /**
     * Add an on change callback to the property.
     * The callback is run when ever propertyModified is called. Usually when even the value of
     * property changes. The return value is a RAII guard for the callback and will remove the
     * callback on destruction. Hence one must keep the return value around as long as the
     * callback should be active. To remove the callback one only need to destruct or reset the
     * return value. Multiple callbacks can be registered at the same time.
     */
    std::shared_ptr<std::function<void()>> onChangeScoped(std::function<void()> callback);

    /**
     * Add an on change callback to the property.
     * The callback is run when ever propertyModified is called. Usually when even the value of
     * property changes. The return value can be passed to removeOnChange to remove the callback.
     * Prefer onChangeScoped when the callback need to be removed.
     * Multiple callbacks can be registered at the same time.
     */
    const BaseCallBack* onChange(std::function<void()> callback);

    /**
     * Remove an on change callback registered using onChange.
     */
    void removeOnChange(const BaseCallBack* callback);

    virtual Property& setVisible(bool val);
    virtual bool getVisible() const;

    /* \brief sets visibility depending another property `prop`, according to `callback`
     * @param prop is the property on which the visibility depends
     * @param callback is a function that outputs a visibility boolean value. The function gets
     * `prop` as parameter
     *
     * Checks the expression in `callback` every time `prop` is changed and sets the
     * visibility accordingly. Note that this registers an onChange callback on `prop`, which
     * might result in poor performance when `prop` is a very frequently changed property.
     */
    template <typename P, typename DecisionFunc>
    Property& visibilityDependsOn(P& prop, DecisionFunc callback) {
        static_assert(
            std::is_invocable_r_v<bool, DecisionFunc, P&>,
            "The visibility callback must return a boolean and accept a property as argument!");
        static_assert(std::is_base_of_v<Property, P>, "P must be a Property!");
        this->setVisible(std::invoke(callback, prop));
        prop.onChange([c = callback, &prop, this]() { this->setVisible(std::invoke(c, prop)); });
        return *this;
    }

    /* \brief sets readonly depending another property `prop`, according to `callback`
     * @param prop is the property on which the readonly state depends
     * @param callback is a function that outputs a readonly boolean value. The function gets `prop`
     * as parameter
     *
     * Checks the expression in `callback` every time `prop` is changed and sets the
     * readonly state accordingly. Note that this registers an onChange callback on `prop`, which
     * might result in poor performance when `prop` is a very frequently changed property.
     */
    template <typename P, typename DecisionFunc>
    Property& readonlyDependsOn(P& prop, DecisionFunc callback) {
        static_assert(
            std::is_invocable_r_v<bool, DecisionFunc, P&>,
            "The readonly callback must return a boolean and accept a property as argument!");
        static_assert(std::is_base_of_v<Property, P>, "P must be a Property!");
        this->setReadOnly(std::invoke(callback, prop));
        prop.onChange([c = callback, &prop, this]() { this->setReadOnly(std::invoke(c, prop)); });
        return *this;
    }

    /**
     * The help should describe what state the property represents and how it is used.
     * This will be shown in the Processor help, and as part of the property description in the
     * property tooltip in the GUI.
     */
    const Document& getHelp() const;
    Document& getHelp();
    Property& setHelp(Document help);

    /**
     * This function should describe the state of the property
     * By default this will return a document describing all the
     * state. i.e. Identifier, DisplayName, Help, InvalidationLevel,
     * PropertySemantics, etc.
     * Derived properties should extend this function and add their
     * state, usually values etc.
     * The description is usually shown as a tooltip in the GUI.
     */
    virtual Document getDescription() const;

    template <typename T, typename U>
    static void setStateAsDefault(T& property, const U& state);

    template <typename P>
    Property& autoLinkToProperty(std::string_view propertyPath);
    const std::vector<std::pair<std::string, std::string>>& getAutoLinkToProperty() const;

    /**
     * @brief Accept a NetworkVisitor, the visitor will visit this Property.
     */
    virtual void accept(NetworkVisitor& visitor);

    class IVW_CORE_API OnChangeBlocker {
    public:
        explicit OnChangeBlocker(Property& property);
        OnChangeBlocker() = delete;
        OnChangeBlocker(const OnChangeBlocker&) = delete;
        OnChangeBlocker(OnChangeBlocker&&) = delete;
        OnChangeBlocker& operator=(OnChangeBlocker) = delete;
        ~OnChangeBlocker();

    private:
        Property& property_;
    };

protected:
    /**
     * Since property is a polymorphic class the copy constructor should only be used internally to
     * implement the clone functionality
     * @see clone
     */
    Property(const Property& rhs);

    /**
     * Properties do not support copy assignment.
     * To assign the "value" of a property to an other property the Property::set(const Property*
     * src) function should be used
     * @see set
     */
    Property& operator=(const Property& that) = delete;

    void notifyAboutChange();

    CallBackList onChangeCallback_;
    PropertySerializationMode serializationMode_;

private:
    std::string identifier_;
    mutable std::string path_;  // To avoid having to create a string here all the time.

    ValueWrapper<std::string> displayName_;
    ValueWrapper<bool> readOnly_;
    ValueWrapper<PropertySemantics> semantics_;
    ValueWrapper<bool> visible_;

    bool propertyModified_;
    InvalidationLevel invalidationLevel_;

    PropertyOwner* owner_;
    std::vector<PropertyWidget*> propertyWidgets_;

    PropertyWidget* initiatingWidget_;

    std::vector<std::pair<std::string, std::string>> autoLinkTo_;
    Document help_;
};

namespace util {

enum class OverwriteState { Yes, No };

/**
 * Update the default state of \p property to \p state and set the current state to \p state if \p
 * property is in the default state or \p overwrite is OverwriteState::Yes
 */
template <typename T, typename U>
void updateDefaultState(T& property, const U& state, OverwriteState overwrite) {
    if (property.isDefaultState() || overwrite == OverwriteState::Yes) {
        property.setDefault(state);
        property.set(state);
    } else {
        property.setDefault(state);
    }
}

}  // namespace util

template <typename T, typename U>
void Property::setStateAsDefault(T& property, const U& state) {
    U tmp = property;
    property = state;
    property.setCurrentStateAsDefault();
    property = tmp;
}

template <typename P>
Property& Property::autoLinkToProperty(std::string_view propertyPath) {
    autoLinkTo_.emplace_back(ProcessorTraits<P>::getProcessorInfo().classIdentifier, propertyPath);
    return *this;
}

}  // namespace inviwo
