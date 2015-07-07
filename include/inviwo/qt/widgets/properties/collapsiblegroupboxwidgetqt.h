/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#ifndef IVW_COLLAPSIVEGROUPBOXWIDGETQT_H
#define IVW_COLLAPSIVEGROUPBOXWIDGETQT_H

#include <inviwo/qt/widgets/inviwoqtwidgetsdefine.h>
#include <inviwo/qt/widgets/properties/propertywidgetqt.h>
#include <inviwo/core/properties/propertyownerobserver.h>
#include <inviwo/core/processors/processorobserver.h>

class QLineEdit;
class QToolButton;
class QGroupBox;
class QPushButton;
class QLabel;
class QGridLayout;
class QCheckBox;

namespace inviwo {

class Property;
class PropertyOwner;
class EditableLabelQt;

class IVW_QTWIDGETS_API CollapsibleGroupBoxWidgetQt : public PropertyWidgetQt,
                                                      public PropertyOwnerObserver,
                                                      public ProcessorObserver {
    Q_OBJECT
public:
    CollapsibleGroupBoxWidgetQt(Property* property, bool isCheckable=false);
    CollapsibleGroupBoxWidgetQt(std::string displayName = "", bool isCheckable=false);
    virtual std::string getDisplayName() const;
    virtual void setDisplayName(const std::string& displayName);

    void addProperty(Property* tmpProperty);
    std::vector<Property*> getProperties();
    std::vector<PropertyWidgetQt*> getPropertyWidgets();

    void setPropertyOwner(PropertyOwner* propertyOwner);
    PropertyOwner* getPropertyOwner() const;

    void setShowIfEmpty(bool val);
    
    virtual bool isCollapsed() const;
    virtual bool isChecked() const;

    bool isCheckable() const;
    void setCheckable(bool checkable);

    // Overridden from PropertyWidget
    virtual void updateFromProperty() override {} 

    // Overridden from PropertyOwnerObserver to add and remove properties dynamically
    virtual void onDidAddProperty(Property* property, size_t index) override;
    virtual void onWillRemoveProperty(Property* property, size_t index) override;

    // Override ProcessorObserver
    void onProcessorIdentifierChange(Processor*) override;

    // Overridden from PropertyWidgetQt/QWidget
    virtual QSize sizeHint() const override;
    virtual QSize minimumSizeHint() const override;

public slots:
    void toggleCollapsed();

    void checkedStateChanged();
    virtual void labelDidChange();
    virtual void resetPropertyToDefaultState();
    void updatePropertyWidgetSemantics(PropertyWidgetQt*);

protected:
    virtual void setVisible(bool visible);
    virtual void setCollapsed(bool value);
    virtual void setChecked(bool checked);

    void generateWidget();
    void updateWidgets();

    std::string displayName_;
    bool collapsed_;
    bool checked_;
    EditableLabelQt* label_;

    std::vector<Property*> properties_;
    std::vector<PropertyWidgetQt*> propertyWidgets_;

private:
    QToolButton* btnCollapse_;
    QWidget* propertyWidgetGroup_;
    QGridLayout* propertyWidgetGroupLayout_;
    QLabel* defaultLabel_;
    QCheckBox *checkBox_;
    PropertyOwner* propertyOwner_;
    bool showIfEmpty_;
    bool checkable_;
};


} // namespace

#endif //IVW_COLLAPSIVEGROUPBOXWIDGETQT_H