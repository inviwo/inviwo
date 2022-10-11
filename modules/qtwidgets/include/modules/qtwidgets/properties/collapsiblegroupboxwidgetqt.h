/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2022 Inviwo Foundation
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

#include <modules/qtwidgets/qtwidgetsmoduledefine.h>  // for IVW_MODULE_QTWIDGETS_API

#include <inviwo/core/processors/processor.h>               // for Processor, Processor::NameDis...
#include <inviwo/core/properties/propertyownerobserver.h>   // for PropertyOwnerObserver
#include <modules/qtwidgets/properties/propertywidgetqt.h>  // for PropertyWidgetQt

#include <cstddef>      // for size_t
#include <memory>       // for unique_ptr
#include <string>       // for string
#include <string_view>  // for string_view
#include <vector>       // for vector

#include <QSize>  // for QSize

class QCheckBox;
class QGridLayout;
class QLabel;
class QMenu;
class QMimeData;
class QToolButton;
class QWidget;

namespace inviwo {

class CompositeProperty;
class EditableLabelQt;
class Property;
class PropertyOwner;
class PropertySemantics;
class Settings;

class IVW_MODULE_QTWIDGETS_API CollapsibleGroupBoxWidgetQt : public PropertyWidgetQt,
                                                             public PropertyOwnerObserver {

public:
    CollapsibleGroupBoxWidgetQt(CompositeProperty* property, bool isCheckable = false);
    CollapsibleGroupBoxWidgetQt(Processor* property, bool isCheckable = false);
    CollapsibleGroupBoxWidgetQt(Settings* property, bool isCheckable = false);
    CollapsibleGroupBoxWidgetQt(Property* property, PropertyOwner* owner,
                                const std::string& displayName = "", bool isCheckable = false);

    virtual std::string getDisplayName() const;
    virtual void setDisplayName(const std::string& displayName);

    void addProperty(Property* tmpProperty);
    const std::vector<Property*>& getProperties();
    const std::vector<PropertyWidgetQt*>& getPropertyWidgets();

    void setPropertyOwner(PropertyOwner* propertyOwner);
    PropertyOwner* getPropertyOwner() const;

    void setShowIfEmpty(bool val);

    virtual bool isCollapsed() const;
    virtual bool isChecked() const;

    bool isCheckable() const;
    void setCheckable(bool checkable);

    void setCheckBoxText(std::string_view text);
    void setCheckBoxVisible(bool visible);
    void setCheckBoxReadonly(bool readonly);

    virtual bool isChildRemovable() const;

    // Overridden from PropertyWidgetQt/QWidget
    virtual QSize sizeHint() const override;
    virtual QSize minimumSizeHint() const override;
    virtual void setReadOnly(bool readonly) override;

    void toggleCollapsed();

    virtual std::unique_ptr<QMenu> getContextMenu() override;
    std::unique_ptr<QMimeData> getPropertyOwnerMimeData() const;

protected:
    // Overridden from PropertyWidget
    virtual void updateFromProperty() override;

    // Overridden from PropertyOwnerObserver to add and remove properties dynamically
    virtual void onDidAddProperty(Property* property, size_t index) override;
    virtual void onWillRemoveProperty(Property* property, size_t index) override;

    // PropertyObservable overrides
    virtual void onSetSemantics(Property* property, const PropertySemantics& semantics) override;
    virtual void onSetReadOnly(Property* property, bool readonly) override;
    virtual void onSetVisible(Property* property, bool visible) override;

    virtual void setVisible(bool visible) override;
    virtual void setCollapsed(bool value);
    virtual void setChecked(bool checked);

    /**
     * \brief set the text which is shown if there are no sub-properties and showIfEmpty is true
     *
     * @param str    text to indicate that there are no sub-properties
     *
     * @see setShowIfEmpty
     */
    void setEmptyLabelString(const std::string& str);

    static std::unique_ptr<QWidget> createPropertyLayoutWidget(QLabel* defaultLabel);
    void addButtonLayout(QGridLayout* layout, int row, Property* prop);
    void insertProperty(Property* prop, size_t index);
    void insertPropertyWidget(PropertyWidgetQt* propertyWidget, bool insertAtEnd);

    virtual void updateFocusPolicy();

    std::string displayName_;
    bool checked_;

    std::vector<Property*> properties_;
    std::vector<PropertyWidgetQt*> propertyWidgets_;
    std::vector<std::unique_ptr<PropertyWidgetQt>> oldWidgets_;

private:
    PropertyOwner* propertyOwner_;
    bool showIfEmpty_;
    bool checkable_;
    QLabel* defaultLabel_;
    QWidget* propertyWidgetGroup_;
    QGridLayout* propertyWidgetGroupLayout_;
    QToolButton* btnCollapse_;

protected:
    EditableLabelQt* label_;
    QToolButton* resetButton_;

private:
    QCheckBox* checkBox_;
    Processor::NameDispatcherHandle nameChange_;
};

}  // namespace inviwo
