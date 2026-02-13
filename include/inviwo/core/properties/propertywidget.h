/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2026 Inviwo Foundation
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

namespace inviwo {

class Property;
class PropertyEditorWidget;

/**
 * A PropertyWidget is a graphical representation of a Property.
 * A widget will get updateFromProperty call to update its representation when ever the property
 * changes and should update the property when ever the user modifies the widget.
 */
class IVW_CORE_API PropertyWidget {
public:
    PropertyWidget();

    /**
     * The PropertyWidget will register it self with the property.
     */
    explicit PropertyWidget(Property* property);
    PropertyWidget(const PropertyWidget&);
    PropertyWidget(PropertyWidget&&);
    PropertyWidget& operator=(const PropertyWidget&);
    PropertyWidget& operator=(PropertyWidget&&);

    /**
     * The PropertyWidget will deregister it self with the property.
     */
    virtual ~PropertyWidget();

    /**
     * Implement this function to update the widget after the property has been modified.
     */
    virtual void updateFromProperty() = 0;

    /**
     * Override this function returning true if there can be a PropertyEditorWidget for this
     * PropertyWidget.
     */
    virtual bool hasEditorWidget() const { return false; }
    /**
     * This function should return the editor widget if hasEditorWidget() returns true.
     */
    virtual PropertyEditorWidget* getEditorWidget() { return nullptr; }
    virtual Property* getProperty();

protected:
    Property* property_ = nullptr;  //< Non owning reference, can be null
};

}  // namespace inviwo
