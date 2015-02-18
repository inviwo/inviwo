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

namespace inviwo {

class IVW_CORE_API CompositeProperty : public Property, public PropertyOwner {
public:
    InviwoPropertyInfo();

    CompositeProperty(std::string identifier, std::string displayName,
                      InvalidationLevel invalidationLevel=INVALID_OUTPUT,
                      PropertySemantics semantics = PropertySemantics::Default);
    
    CompositeProperty(const CompositeProperty& rhs);
    CompositeProperty& operator=(const CompositeProperty& that);
    virtual CompositeProperty* clone() const;
    virtual ~CompositeProperty();

    virtual void setOwner(PropertyOwner* owner);

    virtual bool getReadOnly() const; // returns true if all sub properties are read only
    virtual void setReadOnly(const bool& value);

    //override original function in property
    virtual void set(const Property* src);
    virtual void setPropertyModified(bool modified);
    virtual bool isPropertyModified() const;

    virtual void invalidate(InvalidationLevel invalidationLevel,
                            Property* modifiedProperty = 0);

    virtual void setCurrentStateAsDefault();
    virtual void resetToDefaultState();
    
    virtual UsageMode getUsageMode() const; // returns the lowest mode of all sub properties
    virtual void setUsageMode(UsageMode visibilityMode);
    virtual bool getVisible(); // returns true if any sub property is visible 
    virtual void setVisible(bool val);

    virtual void updateVisibility();
    
    virtual bool isCollapsed() const;
    virtual void setCollapsed(bool value);
    

    // Override from the property owner
    virtual Processor* getProcessor();
    virtual const Processor* getProcessor() const;
    virtual std::vector<std::string> getPath() const;

    virtual void serialize(IvwSerializer& s) const;
    virtual void deserialize(IvwDeserializer& d);
    
private:
    bool collapsed_;
};

} // namespace

#endif // IVW_COMPOSITEPROPERTY_H