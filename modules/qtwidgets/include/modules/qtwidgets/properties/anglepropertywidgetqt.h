/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2024 Inviwo Foundation
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

#include <inviwo/core/properties/ordinalproperty.h>                 // for OrdinalProperty
#include <inviwo/core/properties/property.h>                        // for Property
#include <modules/qtwidgets/angleradiuswidget.h>                    // for AngleRadiusWidget
#include <modules/qtwidgets/editablelabelqt.h>                      // for EditableLabelQt
#include <modules/qtwidgets/properties/propertysettingswidgetqt.h>  // for OrdinalPropertySettin...
#include <modules/qtwidgets/properties/propertywidgetqt.h>          // for PropertyWidgetQt

#include <memory>  // for unique_ptr

#include <QAction>         // for QAction
#include <QHBoxLayout>     // for QHBoxLayout
#include <QSignalBlocker>  // for QSignalBlocker
#include <Qt>              // for AlignCenter
#include <QMenu>           // IWYU pragma: keep

class QHBoxLayout;

namespace inviwo {

/** \class AnglePropertyWidgetQt
 * Widget for Float and Double properties to edit an angle in [0 2pi).
 *
 * @see AngleWidget
 */
template <typename T>
class AnglePropertyWidgetQt : public PropertyWidgetQt {
public:
    AnglePropertyWidgetQt(OrdinalProperty<T>* property);
    virtual ~AnglePropertyWidgetQt() = default;

    virtual void updateFromProperty() override;

protected:
    virtual std::unique_ptr<QMenu> getContextMenu() override;
    void showSettings();
    // Convenience function
    virtual OrdinalProperty<T>* getProperty() override;

    OrdinalPropertySettingsWidgetQt<T>* settings_;
    EditableLabelQt* displayName_;
    AngleRadiusWidget* angleWidget_;
};

using FloatAnglePropertyWidgetQt = AnglePropertyWidgetQt<float>;
using DoubleAnglePropertyWidgetQt = AnglePropertyWidgetQt<double>;

template <typename T>
AnglePropertyWidgetQt<T>::AnglePropertyWidgetQt(OrdinalProperty<T>* property)
    : PropertyWidgetQt(property)
    , settings_(nullptr)
    , displayName_(new EditableLabelQt(this, property))
    , angleWidget_(new AngleRadiusWidget(this)) {
    QHBoxLayout* hLayout = new QHBoxLayout();
    // Label showing the display name of the property
    setSpacingAndMargins(hLayout);
    hLayout->addWidget(displayName_);

    setFocusPolicy(angleWidget_->focusPolicy());
    setFocusProxy(angleWidget_);

    // Do not allow the user to change the radius
    angleWidget_->setMinRadius(1.);
    connect(angleWidget_, &AngleRadiusWidget::angleChanged, this,
            [&]() { getProperty()->set(static_cast<T>(angleWidget_->getAngle())); });
    connect(angleWidget_, &AngleRadiusWidget::angleMinMaxChanged, this, [&]() {
        getProperty()->setMinValue(static_cast<T>(angleWidget_->getMinAngle()));
        getProperty()->setMaxValue(static_cast<T>(angleWidget_->getMaxAngle()));
    });
    hLayout->addWidget(angleWidget_, Qt::AlignCenter);

    setLayout(hLayout);
    updateFromProperty();
}

template <typename T>
void AnglePropertyWidgetQt<T>::updateFromProperty() {
    QSignalBlocker block{angleWidget_};
    angleWidget_->setMinMaxAngle(static_cast<double>(getProperty()->getMinValue()),
                                 static_cast<double>(getProperty()->getMaxValue()));
    angleWidget_->setAngle(static_cast<double>(getProperty()->get()));
}

template <typename T>
std::unique_ptr<QMenu> AnglePropertyWidgetQt<T>::getContextMenu() {
    auto menu = PropertyWidgetQt::getContextMenu();
    auto settingsAction = menu->addAction(tr("&Property settings..."));
    settingsAction->setToolTip(
        tr("Open the property settings dialog to adjust min bound, start, end, max bound, "
           "minSeparation and increment values"));

    connect(settingsAction, &QAction::triggered, this, &AnglePropertyWidgetQt::showSettings);
    settingsAction->setEnabled(!property_->getReadOnly());
    return menu;
}

template <typename T>
OrdinalProperty<T>* AnglePropertyWidgetQt<T>::getProperty() {
    return static_cast<OrdinalProperty<T>*>(property_);
}

template <typename T>
void AnglePropertyWidgetQt<T>::showSettings() {
    if (!settings_) {
        settings_ = new OrdinalPropertySettingsWidgetQt<T>(getProperty(), this);
    }
    settings_->showWidget();
}

}  // namespace inviwo
