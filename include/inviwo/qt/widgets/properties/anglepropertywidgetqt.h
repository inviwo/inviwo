/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#ifndef IVW_ANGLEPROPERTYWIDGETQT_H
#define IVW_ANGLEPROPERTYWIDGETQT_H

#include <inviwo/qt/widgets/inviwoqtwidgetsdefine.h>
#include <inviwo/qt/widgets/angleradiuswidget.h>
#include <inviwo/qt/widgets/editablelabelqt.h>
#include <inviwo/qt/widgets/properties/propertysettingswidgetqt.h>
#include <inviwo/qt/widgets/properties/propertywidgetqt.h>

#include <inviwo/core/properties/ordinalproperty.h>


namespace inviwo {


/** \class BaseAnglePropertyWidgetQt
 * Widget for Float and Double properties to edit an angle in [0 2pi).
 * 
 * @see AngleWidget
 */
class IVW_QTWIDGETS_API BaseAnglePropertyWidgetQt : public PropertyWidgetQt {
    Q_OBJECT
public:
    BaseAnglePropertyWidgetQt(Property* prop);
    virtual ~BaseAnglePropertyWidgetQt() {};

    virtual void updateFromProperty() = 0;

public slots:
    virtual void onAngleChanged() = 0;
    virtual void onAngleMinMaxChanged() = 0;
    /** 
     * Set current value as minimum value.
     */
    virtual void setCurrentAsMin() = 0;
    /** 
     * Set current value as maximum value.
     */
    virtual void setCurrentAsMax() = 0;
    virtual void showSettings() = 0;

protected:

    void generateWidget();
    void generatesSettingsWidget();
    // Actions for the context menu
    QAction* settingsAction_; 
    QAction* minAction_;
    QAction* maxAction_;

    PropertySettingsWidgetQt* settingsWidget_;
    EditableLabelQt* displayName_;
    AngleRadiusWidget* angleWidget_; 
};

// Qt does not allow us to template class with Q_OBJECT so we inherit from it instead
template <typename T>
class AnglePropertyWidgetQt : public BaseAnglePropertyWidgetQt {
public:
    AnglePropertyWidgetQt(OrdinalProperty<T>* property) : BaseAnglePropertyWidgetQt(property) {
        // Set values
        updateFromProperty();
    }

    virtual ~AnglePropertyWidgetQt(){};

    void updateFromProperty() {
        angleWidget_->blockSignals(true);
        angleWidget_->setMinMaxAngle(static_cast<double>(getProperty()->getMinValue()),
                                     static_cast<double>(getProperty()->getMaxValue()));
        angleWidget_->setAngle(static_cast<double>(getProperty()->get()));
        angleWidget_->blockSignals(false);
    }
    void onAngleChanged() { getProperty()->set(static_cast<T>(angleWidget_->getAngle())); }
    void onAngleMinMaxChanged() {
        getProperty()->setMinValue(static_cast<T>(angleWidget_->getMinAngle()));
        getProperty()->setMaxValue(static_cast<T>(angleWidget_->getMaxAngle()));
    }
    void setCurrentAsMin() { getProperty()->setMinValue(static_cast<T>(getProperty()->get())); }
    void setCurrentAsMax() { getProperty()->setMaxValue(static_cast<T>(getProperty()->get())); }

    void showSettings() {
        if (!this->settingsWidget_) {
            this->settingsWidget_ = new TemplatePropertySettingsWidgetQt<T, T>(getProperty(), this);
        }
        this->settingsWidget_->reload();
        this->settingsWidget_->show();
    }
    // Convenience function
    OrdinalProperty<T>* getProperty() { return static_cast<OrdinalProperty<T>*>(property_); }
};

typedef AnglePropertyWidgetQt<float> FloatAnglePropertyWidgetQt;
typedef AnglePropertyWidgetQt<double> DoubleAnglePropertyWidgetQt;
}  // namespace




#endif  // IVW_ANGLEPROPERTYWIDGETQT_H