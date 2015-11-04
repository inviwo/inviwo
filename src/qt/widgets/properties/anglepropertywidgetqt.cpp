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

#include <inviwo/qt/widgets/properties/anglepropertywidgetqt.h>
#include <warn/push>
#include <warn/ignore/all>
#include <QLabel>
#include <QPainter>
#include <warn/pop>

namespace inviwo {

BaseAnglePropertyWidgetQt::BaseAnglePropertyWidgetQt(Property* prop)
    : PropertyWidgetQt(prop)
    , settingsAction_(nullptr)
    , minAction_(nullptr)
    , maxAction_(nullptr)
    , settingsWidget_(nullptr)
    , displayName_(nullptr)
    , angleWidget_(nullptr) {
    generateWidget();
    generatesSettingsWidget();
}

void BaseAnglePropertyWidgetQt::generateWidget() {
    QHBoxLayout* hLayout = new QHBoxLayout();
    // Label showing the display name of the property
    setSpacingAndMargins(hLayout);
    displayName_ = new EditableLabelQt(this, property_);
    hLayout->addWidget(displayName_);

    // Angle widget
    angleWidget_ = new AngleRadiusWidget(this);
    // Do not allow the user to change the radius
    angleWidget_->setMinRadius(1.);
    connect(angleWidget_, SIGNAL(angleChanged()), this, SLOT(onAngleChanged()));
    connect(angleWidget_, SIGNAL(angleMinMaxChanged()), this, SLOT(onAngleMinMaxChanged()));
    hLayout->addWidget(angleWidget_, Qt::AlignCenter);

    setLayout(hLayout);
}

void BaseAnglePropertyWidgetQt::generatesSettingsWidget() {
    settingsAction_ = new QAction(tr("&Property settings..."), this);
    settingsAction_->setToolTip(tr("&Open the property settings dialog to adjust min, max, and increment values"));
    minAction_ = new QAction(tr("&Set as Min"), this);
    minAction_->setToolTip(tr("&Use the current value as the min value for the property"));
    maxAction_ = new QAction(tr("&Set as Max"), this);
    maxAction_->setToolTip(tr("&Use the current value as the max value for the property"));

    connect(settingsAction_,
        SIGNAL(triggered()),
        this,
        SLOT(showSettings()));

    connect(minAction_,
        SIGNAL(triggered()),
        this,
        SLOT(setCurrentAsMin()));

    connect(maxAction_,
        SIGNAL(triggered()),
        this,
        SLOT(setCurrentAsMax()));

    QMenu* contextMenu_ = getContextMenu();
    contextMenu_->addAction(settingsAction_);
    contextMenu_->addAction(minAction_);
    contextMenu_->addAction(maxAction_);
}


}  // namespace
