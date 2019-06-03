/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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
#include <modules/qtwidgets/properties/propertywidgetqt.h>
#include <modules/qtwidgets/propertylistwidget.h>
#include <modules/qtwidgets/properties/collapsiblegroupboxwidgetqt.h>
#include <inviwo/core/properties/propertywidgetfactory.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/settings/settings.h>
#include <inviwo/qt/editor/inviwomainwindow.h>
#include <modules/qtwidgets/inviwoqtutils.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QVBoxLayout>
#include <QString>
#include <QScrollArea>
#include <QLayout>
#include <QFrame>
#include <warn/pop>

namespace inviwo {

SettingsWidget::SettingsWidget(const QString& title, InviwoMainWindow* mainwindow)
    : InviwoDockWidget(title, mainwindow, "SettingsWidget"), mainwindow_(mainwindow) {

    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    resize(utilqt::emToPx(this, QSizeF(60, 60)));  // default size

    scrollArea_ = new QScrollArea();
    scrollArea_->setWidgetResizable(true);
    scrollArea_->setMinimumWidth(utilqt::emToPx(this, 30));
    scrollArea_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea_->setFrameShape(QFrame::NoFrame);
    scrollArea_->setContentsMargins(0, 0, 0, 0);

    mainWidget_ = new QWidget();
    layout_ = new QVBoxLayout(mainWidget_);
    layout_->setAlignment(Qt::AlignTop);
    const auto space = utilqt::refSpacePx(this);
    layout_->setContentsMargins(0, space, 0, space);
    layout_->setSpacing(space);
    scrollArea_->setWidget(mainWidget_);

    setWidget(scrollArea_);

    onModulesDidRegister_ =
        mainwindow->getInviwoApplication()->getModuleManager().onModulesDidRegister(
            [&]() { updateSettingsWidget(); });
    onModulesWillUnregister_ =
        mainwindow->getInviwoApplication()->getModuleManager().onModulesWillUnregister([&]() {
            while (auto item = layout_->takeAt(0)) {
                delete item;
            }
        });
}

SettingsWidget::SettingsWidget(InviwoMainWindow* mainwindow)
    : SettingsWidget(tr("Settings"), mainwindow) {}

SettingsWidget::~SettingsWidget() = default;

void SettingsWidget::updateSettingsWidget() {
    auto settings = mainwindow_->getInviwoApplication()->getModuleSettings();

    for (auto setting : settings) {
        auto settingsGroup = new CollapsibleGroupBoxWidgetQt(setting);
        settingsGroup->setShowIfEmpty(true);
        layout_->addWidget(settingsGroup);
        settingsGroup->initState();

        for (auto prop : setting->getProperties()) {
            settingsGroup->addProperty(prop);
        }

        if (!settingsGroup->isCollapsed()) {
            settingsGroup->toggleCollapsed();
        }
    }
    layout_->addStretch();
}

}  // namespace inviwo