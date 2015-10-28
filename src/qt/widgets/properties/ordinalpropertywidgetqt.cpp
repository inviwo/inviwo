/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#include <inviwo/qt/widgets/properties/ordinalpropertywidgetqt.h>

namespace inviwo {

BaseOrdinalPropertyWidgetQt::BaseOrdinalPropertyWidgetQt(Property* property)
    : PropertyWidgetQt(property), settingsWidget_(nullptr), contextMenu_(nullptr) {}

BaseOrdinalPropertyWidgetQt::~BaseOrdinalPropertyWidgetQt() {
    if (settingsWidget_) {
        settingsWidget_->hide();
        property_->deregisterWidget(settingsWidget_);
        delete settingsWidget_;
    }
}

void BaseOrdinalPropertyWidgetQt::generateWidget() {
    signalMapperSetPropertyValue_ = new QSignalMapper(this);
    signalMapperContextMenu_ = new QSignalMapper(this);
    
    QHBoxLayout* hLayout = new QHBoxLayout(); 
    hLayout->setContentsMargins(0,0,0,0);
    hLayout->setSpacing(7);

    label_ = new EditableLabelQt(this, property_);
    hLayout->addWidget(label_);
 
    QWidget* sliderWidget = new QWidget();

    sliderWidgets_ = makeSliders(sliderWidget);

    for(size_t i = 0; i < sliderWidgets_.size(); i++) {
        sliderWidgets_[i]->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(sliderWidgets_[i],
                SIGNAL(customContextMenuRequested(const QPoint&)),
                signalMapperContextMenu_,
                SLOT(map()));

        connect(sliderWidgets_[i],
                SIGNAL(valueChanged()),
                signalMapperSetPropertyValue_,
                SLOT(map()));

        signalMapperContextMenu_->setMapping(sliderWidgets_[i], static_cast<int>(i));
        signalMapperSetPropertyValue_->setMapping(sliderWidgets_[i], static_cast<int>(i));
    }

    hLayout->addWidget(sliderWidget);

    sliderWidget->setMinimumHeight(sliderWidget->sizeHint().height());
    QSizePolicy sp = sliderWidget->sizePolicy();
    sp.setVerticalPolicy(QSizePolicy::Fixed);
    sliderWidget->setSizePolicy(sp);

    connect(signalMapperContextMenu_,
            SIGNAL(mapped(int)),
            this,
            SLOT(showContextMenuSlider(int)));

    connect(signalMapperSetPropertyValue_,
            SIGNAL(mapped(int)),
            this,
            SLOT(setPropertyValue(int)));

    setLayout(hLayout);

    setFixedHeight(sizeHint().height());
    sp = sizePolicy();
    sp.setVerticalPolicy(QSizePolicy::Fixed);
    setSizePolicy(sp);
}


void BaseOrdinalPropertyWidgetQt::generatesSettingsWidget() {

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
            SLOT(setAsMin()));
    
    connect(maxAction_,
            SIGNAL(triggered()),
            this,
            SLOT(setAsMax()));
               
    contextMenu_ = new QMenu(this);
    contextMenu_->addActions(PropertyWidgetQt::getContextMenu()->actions());
    contextMenu_->addAction(settingsAction_);
    contextMenu_->addAction(minAction_);
    contextMenu_->addAction(maxAction_);
    minAction_->setVisible(false);
    maxAction_->setVisible(false);
}



QMenu* BaseOrdinalPropertyWidgetQt::getContextMenu() {
    if (!contextMenu_) {
        generatesSettingsWidget();
    }
    return contextMenu_;
}


/****************************************************************************/
// Slots:

void BaseOrdinalPropertyWidgetQt::showContextMenu(const QPoint& pos) {
    showContextMenuSlider(-1);
}

// connected to sliderWidget_ customContextMenuRequested
void BaseOrdinalPropertyWidgetQt::showContextMenuSlider(int sliderId) {
    sliderId_ = sliderId;

    if (!contextMenu_) {
        generatesSettingsWidget();
    }

    PropertyWidgetQt::updateContextMenu();
    UsageMode appVisibilityMode = getApplicationUsageMode();

    if (appVisibilityMode == UsageMode::Development) {
        settingsAction_->setVisible(true);
    } else {
        settingsAction_->setVisible(false);
    }

    if (appVisibilityMode == UsageMode::Development && sliderId_ >= 0) {
        minAction_->setVisible(true);
        maxAction_->setVisible(true);
    } else {
        minAction_->setVisible(false);
        maxAction_->setVisible(false);
    }

    if (property_->getReadOnly()) {
        settingsAction_->setEnabled(false);
        minAction_->setEnabled(false);
        maxAction_->setEnabled(false);
    } else {
        settingsAction_->setEnabled(true);
        minAction_->setEnabled(true);
        maxAction_->setEnabled(true);
    }
    contextMenu_->exec(QCursor::pos());

    minAction_->setVisible(false);
    maxAction_->setVisible(false);
}

} // namespace
