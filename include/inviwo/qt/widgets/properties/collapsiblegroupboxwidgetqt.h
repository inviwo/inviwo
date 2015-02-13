/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
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

namespace inviwo {

class Property;
class PropertyOwner;
class EditableLabelQt;

class IVW_QTWIDGETS_API CollapsibleGroupBoxWidgetQt : public PropertyWidgetQt,
                                                      public PropertyOwnerObserver,
                                                      public ProcessorObserver {
    Q_OBJECT
public:
    CollapsibleGroupBoxWidgetQt(std::string displayName = "");
    virtual std::string getDisplayName() const;
    virtual void setDisplayName(const std::string& displayName);

    void addProperty(Property* tmpProperty);
    std::vector<Property*> getProperties();
    std::vector<PropertyWidgetQt*> getPropertyWidgets();

    void setPropertyOwner(PropertyOwner* propertyOwner);
    PropertyOwner* getPropertyOwner() const;

    void setShowIfEmpty(bool val);
    
    virtual bool isCollapsed() const;
    virtual void setCollapsed(bool value);

    // Overridden from PropertyWidget
    virtual void showWidget();
    virtual void hideWidget();
    bool getVisible() const;
    virtual UsageMode getUsageMode() const;
    virtual void updateFromProperty(){};

    // Overridden from PropertyOwnerObserver to add and remove properties dynamically
    virtual void onDidAddProperty(Property* property, size_t index);
    virtual void onWillRemoveProperty(Property* property, size_t index);

    // Override ProcessorObserver
    void onProcessorIdentifierChange(Processor*);

    // Overridden from QWidget
    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;

public slots:
    void toggleCollapsed();
    void updateVisibility();
    virtual void setDeveloperUsageMode(bool value);
    virtual void setApplicationUsageMode(bool value);
    virtual void labelDidChange();
    virtual void resetPropertyToDefaultState();
    void updatePropertyWidgetSemantics(PropertyWidgetQt*);

protected:
    void generateWidget();
    void updateWidgets();

    std::string displayName_;
    bool collapsed_;

    std::vector<Property*> properties_;
    std::vector<PropertyWidgetQt*> propertyWidgets_;

private:
    EditableLabelQt* label_;
    QToolButton* btnCollapse_;
    QWidget* propertyWidgetGroup_;
    QVBoxLayout* propertyWidgetGroupLayout_;
    QLabel* defaultLabel_;
    PropertyOwner* propertyOwner_;
    bool showIfEmpty_;
};


} // namespace

#endif //IVW_COLLAPSIVEGROUPBOXWIDGETQT_H