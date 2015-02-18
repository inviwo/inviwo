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
    tabWidget_ = new QTabWidget(this);
    setWidget(tabWidget_);
}

SettingsWidget::~SettingsWidget() {}

void SettingsWidget::updateSettingsWidget() {
    std::vector<Settings*> settings = InviwoApplication::getPtr()->getModuleSettings();

    for (size_t i = 0; i < settings.size(); i++) {
        // Scroll widget
        QScrollArea* scrollAreaTab = new QScrollArea(tabWidget_);
        scrollAreaTab->setWidgetResizable(true);
        scrollAreaTab->setMinimumWidth(320);
        scrollAreaTab->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        scrollAreaTab->setFrameShape(QFrame::NoFrame);

        // Holder widget
        QWidget* listWidget = new PropertyListFrame(tabWidget_);
        QVBoxLayout* listLayout = new QVBoxLayout();
        listLayout->setSpacing(7);
        listLayout->setContentsMargins(7, 7, 7, 7);
        listLayout->setAlignment(Qt::AlignTop);
        listLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
        listWidget->setLayout(listLayout);
        scrollAreaTab->setWidget(listWidget);

        std::vector<Property*> props = settings[i]->getProperties();
 
        for (size_t j = 0; j < props.size(); j++) {

            PropertyWidgetQt* propertyWidget = static_cast<PropertyWidgetQt*>(
                PropertyWidgetFactory::getPtr()->create(props[j]));

            if (propertyWidget) {
                listLayout->addWidget(propertyWidget);
                props[j]->registerWidget(propertyWidget);
                propertyWidget->showWidget();
                connect(propertyWidget, SIGNAL(updateSemantics(PropertyWidgetQt*)), this,
                        SLOT(updatePropertyWidgetSemantics(PropertyWidgetQt*)));
            } else {
                LogWarn("Could not find a widget for property: " << props[j]->getClassIdentifier());
            }
        }

        tabWidget_->addTab(scrollAreaTab, tr(settings[i]->getIdentifier().c_str()));
    }
}

void SettingsWidget::saveSettings() {
    const std::vector<Settings*> settings = InviwoApplication::getPtr()->getModuleSettings();
    for (size_t i = 0; i < settings.size(); i++) {
        settings[i]->saveToDisk();
    }
}

void SettingsWidget::updatePropertyWidgetSemantics(PropertyWidgetQt* widget) {
    Property* prop = widget->getProperty();

    bool visible = widget->isVisible();
    QVBoxLayout* listLayout = static_cast<QVBoxLayout*>(widget->parentWidget()->layout());
    int layoutPosition = listLayout->indexOf(widget);
    PropertyWidgetQt* propertyWidget =
        static_cast<PropertyWidgetQt*>(PropertyWidgetFactory::getPtr()->create(prop));

    if (propertyWidget) {
        prop->deregisterWidget(widget);
        widget->hideWidget();
        listLayout->removeWidget(widget);
        listLayout->insertWidget(layoutPosition, propertyWidget);
        prop->registerWidget(propertyWidget);

        connect(propertyWidget, SIGNAL(updateSemantics(PropertyWidgetQt*)), this,
                SLOT(updatePropertyWidgetSemantics(PropertyWidgetQt*)));

        if (visible) {
            propertyWidget->showWidget();
        } else {
            propertyWidget->hideWidget();
        }

    } else {
        LogWarn("Could not change semantic for property: " << prop->getClassIdentifier());
    }
}

} // namespace