/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#ifndef IVW_COMPOSITEPROPERTY_H
#define IVW_COMPOSITEPROPERTY_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/properties/property.h>
#include <inviwo/core/properties/propertyowner.h>
#include <inviwo/core/util/observer.h>
#include <inviwo/core/properties/compositepropertyobserver.h>

namespace inviwo {

class IVW_CORE_API CompositeProperty : public Property,
                                       public PropertyOwner,
                                       public CompositePropertyObservable {
public:
    InviwoPropertyInfo();

    CompositeProperty(std::string identifier, std::string displayName,
                      InvalidationLevel invalidationLevel = INVALID_RESOURCES,
                      PropertySemantics semantics = PropertySemantics::Default);

    CompositeProperty(const CompositeProperty& rhs);
    CompositeProperty& operator=(const CompositeProperty& that);
    virtual CompositeProperty* clone() const override;
    virtual ~CompositeProperty();
    virtual std::string getClassIdentifierForWidget() const override;

    virtual bool isCollapsed() const;
    virtual void setCollapsed(bool value);

    // Override original functions in Property
    virtual void setOwner(PropertyOwner* owner) override;

    virtual void set(const Property* src) override;
    void set(const CompositeProperty* src);
    virtual void setPropertyModified(bool modified) override;
    virtual bool isPropertyModified() const override;
    virtual InvalidationLevel getInvalidationLevel() const override;

    virtual void setCurrentStateAsDefault() override;
    virtual void resetToDefaultState() override;

    // Override from the PropertyOwner
    virtual void invalidate(InvalidationLevel invalidationLevel,
                            Property* modifiedProperty = 0) override;
    void setValid() override;
    virtual Processor* getProcessor() override;
    virtual const Processor* getProcessor() const override;
    virtual std::vector<std::string> getPath() const override;

    virtual void serialize(IvwSerializer& s) const override;
    virtual void deserialize(IvwDeserializer& d) override;

private:
    bool collapsed_;
    InvalidationLevel subPropertyInvalidationLevel_;
};

}  // namespace

#endif  // IVW_COMPOSITEPROPERTY_H