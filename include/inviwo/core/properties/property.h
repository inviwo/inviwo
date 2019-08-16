/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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

#ifndef IVW_PROPERTY_H
#define IVW_PROPERTY_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/properties/valuewrapper.h>
#include <inviwo/core/properties/propertyobserver.h>
#include <inviwo/core/properties/propertywidget.h>
#include <inviwo/core/properties/propertysemantics.h>
#include <inviwo/core/properties/propertyvisibility.h>
#include <inviwo/core/properties/invalidationlevel.h>
#include <inviwo/core/processors/processortraits.h>
#include <inviwo/core/util/callback.h>
#include <inviwo/core/util/document.h>
#include <inviwo/core/metadata/metadataowner.h>
#include <inviwo/core/util/introspection.h>

#include <functional>
#include <type_traits>

namespace inviwo {

/**
 * \class PropertyTraits
 * \brief A traits class for getting the class identifier from a Property.
 * This provides a customization point if one wants to generate the class identifier dynamically,
 * by specializing the traits for your kind of Property:
 * \code{.cpp}
 *     template <typename T>
 *     struct PropertyTraits<MyProperty<T>> {
 *        static std::string classIdentifier() {
 *           return generateMyPropertyClassIdentifier<T>();
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
     * The default implementation will look for a static std::string member "T::classIdentifier" or
     * "T::CLASS_IDENTIFIER". In case it is not found an empty string will be returned. An empty
     * class identifier will be considered an error in various factories.
     */
    static std::string classIdentifier() { return util::classIdentifier<T>(); }
};

// Deprecated
#define InviwoPropertyInfo()                                                             \
    virtual std::string getClassIdentifier() const override { return CLASS_IDENTIFIER; } \
    static const std::string CLASS_IDENTIFIER

// Deprecated
#define PropertyClassIdentifier(T, classIdentifier) \
    const std::string T::CLASS_IDENTIFIER = classIdentifier

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
 *  * __UsageMode__: A property can have different usage modes. Either Development or Application.
 *    Only properties with Application mode will show up when running Inviwo in Application mode.
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
    virtual std::string getClassIdentifier() const = 0;

    Property(const std::string& identifier = "", const std::string& displayName = "",
             InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
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
    virtual Property& setIdentifier(const std::string& identifier);
    virtual std::string getIdentifier() const;
    virtual std::vector<std::string> getPath() const;

    /**
     * \brief A property's name displayed to the user
     */
    virtual Property& setDisplayName(const std::string& displayName);
    virtual std::string getDisplayName() const;

    /**
     * \brief Returns which property's widget should be used
     * when the WidgetFactory tries to create a widget.
     * Defaults to getClassIdentifier(), should only be overridden
     * if a subclass want to reuse another property's widget.
     */
    virtual std::string getClassIdentifierForWidget() const;

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
     * Does the property have any registered widgets.
     * @see registerProperty
     */
    bool hasWidgets() const;

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

    // clang-format off
    template <typename T>
    [[deprecated("was declared deprecated. Use `onChange(std::function<void()>)` instead")]]
    const BaseCallBack* onChange(T* object, void (T::*method)());
    template <typename T>
    [[deprecated("was declared deprecated. Use `removeOnChange(const BaseCallBack*)` instead")]]
    void removeOnChange(T* object);
    // clang-format on

    virtual Property& setUsageMode(UsageMode usageMode);
    virtual UsageMode getUsageMode() const;

    virtual void setSerializationMode(PropertySerializationMode mode);
    virtual PropertySerializationMode getSerializationMode() const;

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
        typename std::result_of<DecisionFunc(P&)>::type b = true;
        static_assert(std::is_same<decltype(b), bool>::value,
                      "The visibility callback must return a boolean!");
        static_assert(std::is_base_of<Property, P>::value, "P must be a Property!");
        this->setVisible(callback(prop));
        prop.onChange([callback, &prop, this]() {
            bool visible = callback(prop);
            this->setVisible(visible);
        });
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
        typename std::result_of<DecisionFunc(P&)>::type b = true;
        static_assert(std::is_same<decltype(b), bool>::value,
                      "The readonly callback must return a boolean!");
        static_assert(std::is_base_of<Property, P>::value, "P must be a Property!");
        this->setReadOnly(callback(prop));
        prop.onChange([callback, &prop, this]() {
            bool readonly = callback(prop);
            this->setReadOnly(readonly);
        });
        return *this;
    }

    virtual Document getDescription() const;

    template <typename T, typename U>
    static void setStateAsDefault(T& property, const U& state);

    template <typename P>
    Property& autoLinkToProperty(const std::string& propertyPath);
    const std::vector<std::pair<std::string, std::string>>& getAutoLinkToProperty() const;

    class IVW_CORE_API OnChangeBlocker {
    public:
        OnChangeBlocker(Property& property);
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

    void updateWidgets();
    void notifyAboutChange();

    CallBackList onChangeCallback_;
    PropertySerializationMode serializationMode_;

private:
    std::string identifier_;

    ValueWrapper<std::string> displayName_;
    ValueWrapper<bool> readOnly_;
    ValueWrapper<PropertySemantics> semantics_;
    ValueWrapper<UsageMode> usageMode_;
    ValueWrapper<bool> visible_;

    bool propertyModified_;
    InvalidationLevel invalidationLevel_;

    PropertyOwner* owner_;
    std::vector<PropertyWidget*> propertyWidgets_;

    PropertyWidget* initiatingWidget_;

    std::vector<std::pair<std::string, std::string>> autoLinkTo_;
};

// clang-format off
template <typename T>
[[deprecated("was declared deprecated. Use `onChange(std::function<void()>)` instead")]]
const BaseCallBack* Property::onChange(T* o, void (T::*m)()) {
    return onChangeCallback_.addLambdaCallback([o, m]() {
        if (m) (*o.*m)();
    });
}

template <typename T>
[[deprecated("was declared deprecated. Use `removeOnChange(const BaseCallBack*)` instead")]]
void Property::removeOnChange(T* o) {
    onChangeCallback_.removeMemberFunction(o);
}
// clang-format on

template <typename T, typename U>
void Property::setStateAsDefault(T& property, const U& state) {
    U tmp = property;
    property = state;
    property.setCurrentStateAsDefault();
    property = tmp;
}

template <typename P>
Property& Property::autoLinkToProperty(const std::string& propertyPath) {
    autoLinkTo_.push_back(
        std::make_pair(ProcessorTraits<P>::getProcessorInfo().classIdentifier, propertyPath));
    return *this;
}

}  // namespace inviwo

#endif  // IVW_PROPERTY_H
