/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2017 Inviwo Foundation
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

#ifndef IVW_PROPERTYWIDGET_H
#define IVW_PROPERTYWIDGET_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/metadata/propertyeditorwidgetmetadata.h>
#include <inviwo/core/properties/propertyvisibility.h>

namespace inviwo {

class Property;
class PropertyEditorWidget;

class IVW_CORE_API PropertyWidget {
public:
    PropertyWidget();
    PropertyWidget(Property* property);

    virtual ~PropertyWidget() = default;

    // Overload this function to update the widget after property modified has been called.
    virtual void updateFromProperty() = 0;

    virtual PropertyEditorWidget* getEditorWidget() const;
    virtual bool hasEditorWidget() const;
    virtual Property* getProperty();

protected:
    Property* property_ = nullptr;  //< Non owning reference, can be null
};

// Additional widget owned by property widget

class IVW_CORE_API PropertyEditorWidget {
public:
    PropertyEditorWidget(Property* property);
    virtual ~PropertyEditorWidget();
    // set functions
    virtual void setVisibility(bool visible);
    virtual void setDimensions(const ivec2& dimensions);
    virtual void setPosition(const ivec2& pos);
    virtual void setDockStatus(PropertyEditorWidgetDockStatus dockStatus);
    virtual void setSticky(bool sticky);
    // get functions
    virtual bool isVisible() const;
    virtual ivec2 getPosition() const;
    virtual ivec2 getDimensions() const;
    virtual PropertyEditorWidgetDockStatus getDockStatus() const;
    virtual bool isSticky() const;

protected:
    void updateVisibility(bool visible);
    void updateDimensions(const ivec2& dimensions);
    void updatePosition(const ivec2& pos);
    void updateDockStatus(PropertyEditorWidgetDockStatus dockStatus);
    void updateSticky(bool sticky);

    // Non owning reference to a metadata that belongs to property.
    Property* property_;
    PropertyEditorWidgetMetaData* metaData_;  
};

}  // namespace

#endif  // IVW_PROPERTYWIDGET_H
