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

#include <inviwo/qt/editor/settingswidget.h>
#include <inviwo/qt/widgets/properties/propertywidgetqt.h>
#include <inviwo/qt/widgets/propertylistwidget.h>
#include <inviwo/qt/widgets/properties/collapsiblegroupboxwidgetqt.h>
#include <inviwo/core/properties/propertywidgetfactory.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <QLayout>
#include <QFrame>

namespace inviwo {

SettingsWidget::SettingsWidget(QString title, QWidget* parent) : InviwoDockWidget(title, parent) {
    generateWidget();
}

SettingsWidget::SettingsWidget(QWidget* parent) : InviwoDockWidget(tr("Settings"), parent) {
    generateWidget();
}

void SettingsWidget::generateWidget() {
    setObjectName("SettingsWidget");
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    
    scrollArea_ = new QScrollArea();
    scrollArea_->setWidgetResizable(true);
    scrollArea_->setMinimumWidth(300);
    scrollArea_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea_->setFrameShape(QFrame::NoFrame);
    scrollArea_->setContentsMargins(0, 0, 0, 0);

    mainWidget_ = new QWidget();
    layout_ = new QVBoxLayout(mainWidget_);
    layout_->setAlignment(Qt::AlignTop);
    layout_->setContentsMargins(0, PropertyWidgetQt::SPACING, 0, PropertyWidgetQt::SPACING);
    layout_->setSpacing(7);
    scrollArea_->setWidget(mainWidget_);

    setWidget(scrollArea_);

}

SettingsWidget::~SettingsWidget() {}

void SettingsWidget::updateSettingsWidget() {
    std::vector<Settings*> settings = InviwoApplication::getPtr()->getModuleSettings();

    for (auto& setting : settings) {
        CollapsibleGroupBoxWidgetQt* settingsGroup = new CollapsibleGroupBoxWidgetQt(setting->getIdentifier());
        settingsGroup->setParentPropertyWidget(nullptr, this);
        layout_->addWidget(settingsGroup);
        settingsGroup->initState();

        std::vector<Property*> props = setting->getProperties();

        for (auto& prop : props) {
            settingsGroup->addProperty(prop);
            for (auto p : prop->getWidgets()){
                connect(static_cast<PropertyWidgetQt*>(p), SIGNAL(updateSemantics(PropertyWidgetQt*)), this,
                    SLOT(updatePropertyWidgetSemantics(PropertyWidgetQt*)));
            }
        }

        if (!settingsGroup->isCollapsed()){
            settingsGroup->toggleCollapsed();
        }
    }
    layout_->addStretch();
}

void SettingsWidget::saveSettings() {
    const std::vector<Settings*> settings = InviwoApplication::getPtr()->getModuleSettings();
    for (auto& setting : settings) {
        setting->saveToDisk();
    }
}

void SettingsWidget::updatePropertyWidgetSemantics(PropertyWidgetQt* widget) {
    Property* prop = widget->getProperty();

    QVBoxLayout* listLayout = static_cast<QVBoxLayout*>(widget->parentWidget()->layout());
    int layoutPosition = listLayout->indexOf(widget);
    PropertyWidgetQt* propertyWidget =
        static_cast<PropertyWidgetQt*>(PropertyWidgetFactory::getPtr()->create(prop));

    if (propertyWidget) {
        prop->deregisterWidget(widget);

        listLayout->removeWidget(widget);
        listLayout->insertWidget(layoutPosition, propertyWidget);
        prop->registerWidget(propertyWidget);

        connect(propertyWidget, SIGNAL(updateSemantics(PropertyWidgetQt*)), this,
                SLOT(updatePropertyWidgetSemantics(PropertyWidgetQt*)));

        propertyWidget->initState();

    } else {
        LogWarn("Could not change semantic for property: " << prop->getClassIdentifier());
    }
}

} // namespace