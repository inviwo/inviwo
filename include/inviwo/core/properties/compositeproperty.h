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
#include <inviwo/core/properties/property.h>
#include <inviwo/core/properties/propertyowner.h>
#include <inviwo/core/util/observer.h>
#include <inviwo/core/properties/compositepropertyobserver.h>

#include <vector>

namespace inviwo {

class InviwoApplication;

/**
 * \ingroup properties
 * A grouping property for collecting properties in a hierarchy, CompositeProperties can be nested
 * arbitrarily deep.
 */
class IVW_CORE_API CompositeProperty : public Property,
                                       public PropertyOwner,
                                       public CompositePropertyObservable {
public:
    virtual std::string_view getClassIdentifier() const override;
    static const std::string classIdentifier;

    enum class CollapseAction { Collapse, Expand };
    enum class CollapseTarget { Current, Recursive, Children, Siblings };

    CompositeProperty(std::string_view identifier, std::string_view displayName, Document help,
                      InvalidationLevel invalidationLevel = InvalidationLevel::InvalidResources,
                      PropertySemantics semantics = PropertySemantics::Default);

    CompositeProperty(std::string_view identifier, std::string_view displayName,
                      InvalidationLevel invalidationLevel = InvalidationLevel::InvalidResources,
                      PropertySemantics semantics = PropertySemantics::Default);

    CompositeProperty(const CompositeProperty& rhs) = default;

    virtual CompositeProperty* clone() const override;
    virtual ~CompositeProperty() = default;

    virtual const std::string& getIdentifier() const override;

    virtual std::string_view getClassIdentifierForWidget() const override;

    virtual bool isCollapsed() const;
    virtual CompositeProperty& setCollapsed(bool value);
    CompositeProperty& setCollapsed(CollapseAction action, CollapseTarget target);

    // Override original functions in Property
    virtual void setOwner(PropertyOwner* owner) override;

    virtual void set(const Property* src) override;
    void set(const CompositeProperty* src);
    virtual void setValid() override;
    virtual InvalidationLevel getInvalidationLevel() const override;

    virtual CompositeProperty& setCurrentStateAsDefault() override;
    virtual CompositeProperty& resetToDefaultState() override;

    virtual bool isDefaultState() const override;

    virtual bool needsSerialization() const override;

    virtual CompositeProperty& setReadOnly(bool value) override;

    // Override from the PropertyOwner
    virtual void invalidate(InvalidationLevel invalidationLevel,
                            Property* modifiedProperty = 0) override;

    virtual Processor* getProcessor() override;
    virtual const Processor* getProcessor() const override;

    virtual const PropertyOwner* getOwner() const override;
    virtual PropertyOwner* getOwner() override;

    virtual InviwoApplication* getInviwoApplication() override;

    /**
     * @brief Accept a NetworkVisitor, the visitor will visit this and then each Property of the
     * CompositeProperty in an undefined order. The Visitor will then visit each Properties's
     * properties and so on.
     */
    virtual void accept(NetworkVisitor& visitor) override;

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

private:
    ValueWrapper<bool> collapsed_;
    InvalidationLevel subPropertyInvalidationLevel_;
};

}  // namespace inviwo
