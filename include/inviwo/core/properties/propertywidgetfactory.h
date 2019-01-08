/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2018 Inviwo Foundation
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

#ifndef IVW_PROPERTYWIDGETFACTORY_H
#define IVW_PROPERTYWIDGETFACTORY_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/properties/propertywidget.h>
#include <inviwo/core/properties/propertywidgetfactoryobject.h>
#include <inviwo/core/util/factory.h>

namespace inviwo {

class IVW_CORE_API PropertyWidgetFactory : public Factory<PropertyWidget, Property*> {
public:
    PropertyWidgetFactory();
    virtual ~PropertyWidgetFactory();

    virtual bool registerObject(PropertyWidgetFactoryObject* propertyWidget);
    virtual bool unRegisterObject(PropertyWidgetFactoryObject* propertyWidget);

    std::unique_ptr<PropertyWidget> create(Property* property) const override;
    virtual bool hasKey(Property* property) const override;

    std::vector<PropertySemantics> getSupportedSemanicsForProperty(Property* property);

    using WidgetMap = std::multimap<std::string, PropertyWidgetFactoryObject*>;

private:
    mutable WidgetMap widgetMap_;
};

}  // namespace inviwo

#endif  // IVW_PROPERTYWIDGETFACTORY_H
