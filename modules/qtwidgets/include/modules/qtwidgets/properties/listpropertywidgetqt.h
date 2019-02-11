/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#ifndef IVW_LISTPROPERTYWIDGETQT_H
#define IVW_LISTPROPERTYWIDGETQT_H

#include <modules/qtwidgets/qtwidgetsmoduledefine.h>
#include <modules/qtwidgets/properties/compositepropertywidgetqt.h>

#include <inviwo/core/properties/propertyownerobserver.h>

class QMenu;
class QToolButton;

namespace inviwo {

class ListProperty;
class IvwPushButton;

/**
 * \class ListPropertyWidgetQt
 * \brief PropertyWidget for a ListProperty.
 *
 * This widget considers the UI flags of the property. If the property supports adding list
 * elements, a tool button is added next to the property name (indicated by a plus). In case
 * multiple prefab objects are registered with the property, a menu is shown. Alternatively, new
 * entries can be added using the context menu.
 * List entries can be removed via a small "x" tool button next to them, if enabled.
 */
class IVW_MODULE_QTWIDGETS_API ListPropertyWidgetQt : public CompositePropertyWidgetQt {
public:
    ListPropertyWidgetQt(ListProperty* property);
    virtual ~ListPropertyWidgetQt() = default;

    virtual void updateFromProperty() override;

    virtual bool isChildRemovable() const override;

    virtual std::unique_ptr<QMenu> getContextMenu() override;

    // virtual void onDidAddProperty(Property* property, size_t index) override;
    // virtual void onDidRemoveProperty(Property* property, size_t index) override;

protected:
    bool canAddElements() const;
    void addNewItem(size_t index);

    ListProperty* listProperty_;
    QToolButton* addItemButton_;
};

}  // namespace inviwo

#endif  // IVW_LISTPROPERTYWIDGETQT_H
