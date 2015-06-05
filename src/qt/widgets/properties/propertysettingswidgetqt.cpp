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

#include <inviwo/qt/widgets/properties/propertysettingswidgetqt.h>
#include <inviwo/qt/widgets/inviwoapplicationqt.h>

namespace inviwo {

PropertySettingsWidgetQt::PropertySettingsWidgetQt(Property* property, QWidget* parent)
    : QDialog(parent)
    , PropertyWidget(property)
    , gridLayout_(new QGridLayout())
    , btnApply_("Apply", this)
    , btnOk_("Ok", this)
    , btnCancel_("Cancel", this) {
    //this->setWindowFlags(Qt::WindowStaysOnTopHint);
    this->setModal(false);
    // remove help button from title bar
    Qt::WindowFlags flags = this->windowFlags() ^ Qt::WindowContextHelpButtonHint;
    // make it a tool window
    flags |= Qt::Popup;
    this->setWindowFlags(flags);
}

PropertySettingsWidgetQt::~PropertySettingsWidgetQt() {
    if (gridLayout_ && (gridLayout_->parent() == nullptr)) {
        delete gridLayout_;
    }
}

void PropertySettingsWidgetQt::keyPressEvent(QKeyEvent * event) {
    if (event->key() == Qt::Key_Escape) {
        cancel();
    } else if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        apply();
    }
    QWidget::keyPressEvent(event);
}



} //namespace