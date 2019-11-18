/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
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
#include <inviwo/core/properties/valuewrapper.h>

#include <flags/flags.h>

#include <set>

namespace inviwo {

enum class ListPropertyUIFlag {
    Static = 0x0,   //!< no list modification via UI
    Add = 0x01,     //!< items can be added to property widget
    Remove = 0x02,  //!< items can be removed from the property widget
};

ALLOW_FLAGS_FOR_ENUM(ListPropertyUIFlag)

using ListPropertyUIFlags = flags::flags<ListPropertyUIFlag>;

/**
 * \class ListProperty
 * \brief A property that has specified sub-properties which can be added using the graphical user
 * interface.
 *
 * Represents a list of properties. Properties can be added by using the prefab objects registered
 * with the list property. The prefab objects serve as templates for instantiating new list entries.
 * If the display name of the prefab object contains a trailing number, the number is incremented
 * for each instance and used as display name of the newly added properties.
 *
 * The UI flags (ListPropertyUIFlags) determine whether the widget will allow to add and/or remove
 * list entries. The number of list elements is limited by setting maxNumberOfElements. A value of 0
 * refers to no limit.
 *
 * Example:
 * \code{.cpp}
 * // using a single prefab object and at most 10 elements
 * ListProperty listProperty("myListProperty", "My ListProperty",
 *     std::make_unique<BoolProperty>("boolProp", "BoolProperty", true), 10);
 *
 * // multiple prefab objects
 * ListProperty listProperty("myListProperty", "My List Property",
 *     []() {
 *         std::vector<std::unique_ptr<Property>> v;
 *         v.emplace_back(std::make_unique<IntProperty>("template1", "Template 1", 5, 0, 10));
 *         v.emplace_back(std::make_unique<IntProperty>("template2", "Template 2", 2, 0, 99));
 *         return v;
 *     }());
 * \endcode
 *
 * This also works when using different types of properties as prefab objects:
 * \code{.cpp}
 * ListProperty listProperty("myListProperty", "My List Property",
 *     []() {
 *         std::vector<std::unique_ptr<Property>> v;
 *         v.emplace_back(std::make_unique<BoolProperty>("boolProperty1", "Boolean Flag", true));
 *         v.emplace_back(std::make_unique<TransferFunctionProperty>("tf1", "Transfer Function"));
 *         v.emplace_back(std::make_unique<IntProperty>("template1", "Template 1", 5, 0, 10));
 *         return v;
 *     }());
 * \endcode
 */
class IVW_CORE_API ListProperty : public CompositeProperty {
public:
    using iterator = std::vector<Property*>::iterator;
    using const_iterator = std::vector<std::unique_ptr<Property>>::const_iterator;

    virtual std::string getClassIdentifier() const override;
    static const std::string classIdentifier;

    ListProperty(std::string identifier, const std::string& displayName,
                 size_t maxNumberOfElements = 0,
                 ListPropertyUIFlags uiFlags = ListPropertyUIFlag::Add | ListPropertyUIFlag::Remove,
                 InvalidationLevel invalidationLevel = InvalidationLevel::InvalidResources,
                 PropertySemantics semantics = PropertySemantics::Default);
    ListProperty(std::string identifier, const std::string& displayName,
                 std::vector<std::unique_ptr<Property>> prefabs, size_t maxNumberOfElements = 0,
                 ListPropertyUIFlags uiFlags = ListPropertyUIFlag::Add | ListPropertyUIFlag::Remove,
                 InvalidationLevel invalidationLevel = InvalidationLevel::InvalidResources,
                 PropertySemantics semantics = PropertySemantics::Default);
    ListProperty(std::string identifier, const std::string& displayName,
                 std::unique_ptr<Property> prefab, size_t maxNumberOfElements = 0,
                 ListPropertyUIFlags uiFlags = ListPropertyUIFlag::Add | ListPropertyUIFlag::Remove,
                 InvalidationLevel invalidationLevel = InvalidationLevel::InvalidResources,
                 PropertySemantics semantics = PropertySemantics::Default);
    ListProperty(const ListProperty& rhs);

    virtual ListProperty* clone() const override;
    virtual ~ListProperty() = default;

    virtual std::string getClassIdentifierForWidget() const override;

    virtual void set(const Property* src) override;
    void set(const ListProperty* src);

    /**
     * \brief set the max number of list elements. This will remove additional properties if the
     * list property contains more than \p n items.
     *
     * @param n    maximum number of elements in this list property
     */
    void setMaxNumberOfElements(size_t n);
    size_t getMaxNumberOfElements() const;

    /**
     * \brief remove all list entries
     */
    void clear();

    /**
     * \brief construct a list entry which is created from the respective prefab object.
     * This function has no effect if the list size will exceed the maximum number of elements.
     *
     * @param prefabIndex   index of prefab object used for creating the new entry
     * @return constructed property, or nullptr if the new property cannot be added to the list
     * @throw RangeException  in case prefabIndex is invalid
     */
    Property* constructProperty(size_t prefabIndex);

    /**
     * \brief add \p property as new list entry. The type of the property must match one of the
     * prefab objects. This function has no effect if the list size will exceed the maximum number
     * of elements.
     *
     * @param property     property to be added
     * @param owner        if true, the list property takes ownership of the property
     * @throw Exception    if the type of \p property does not match any prefab object
     */
    virtual void addProperty(Property* property, bool owner = true) override;

    /**
     * \brief add \p property as new list entry. The type of the property must match one of the
     * prefab objects. This function has no effect if the list size will exceed the maximum number
     * of elements.
     *
     * @param property     property to be added
     * @throw Exception    if the type of \p property does not match any prefab object
     */
    virtual void addProperty(Property& property) override;

    /**
     * \brief insert \p property in the list at position \p index
     * If \p index is not valid, the property is appended. The type of the property must match one
     * of the prefab objects. This function has no effect if the list size will exceed the maximum
     * number of elements.
     *
     * @param index        insertion point for property
     * @param property     property to be inserted
     * @param owner        if true, the list property takes ownership of the property
     */
    virtual void insertProperty(size_t index, Property* property, bool owner = true) override;

    /**
     * \brief insert \p property in the list at position \p index
     * If \p index is not valid, the property is appended. The type of the property must match one
     * of the prefab objects. This function has no effect if the list size will exceed the maximum
     * number of elements.
     *
     * @param index        insertion point for property
     * @param property     property to be inserted
     */
    virtual void insertProperty(size_t index, Property& property) override;

    virtual Property* removeProperty(const std::string& identifier) override;
    virtual Property* removeProperty(Property* property) override;
    virtual Property* removeProperty(Property& property) override;
    virtual Property* removeProperty(size_t index) override;

    /**
     * \brief return number of prefab objects
     *
     * @return count of prefabs
     */
    size_t getPrefabCount() const;

    /**
     * \brief add a new prefab object \p p to be used as template when instantiating new list
     * elements
     *
     * @param p  prefab object
     */
    void addPrefab(std::unique_ptr<Property> p);

    const std::vector<std::unique_ptr<Property>>& getPrefabs() const;

    ListPropertyUIFlags getUIFlags() const;

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

private:
    std::set<std::string> getPrefabIDs() const;

    ListPropertyUIFlags uiFlags_;
    ValueWrapper<size_t> maxNumElements_;
    std::vector<std::unique_ptr<Property>> prefabs_;
};

}  // namespace inviwo

#endif  // IVW_LISTPROPERTY_H
