/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2023 Inviwo Foundation
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

#include <modules/qtwidgets/properties/boolpropertywidgetqt.h>

#include <inviwo/core/properties/boolproperty.h>            // for BoolProperty
#include <inviwo/core/properties/propertysemantics.h>       // for operator==, PropertySemantics
#include <inviwo/core/util/rendercontext.h>                 // for RenderContext
#include <modules/qtwidgets/editablelabelqt.h>              // for EditableLabelQt
#include <modules/qtwidgets/properties/propertywidgetqt.h>  // for PropertyWidgetQt

#include <QCheckBox>                    // for QCheckBox
#include <QHBoxLayout>                  // for QHBoxLayout
#include <QLineEdit>                    // for QLineEdit
#include <QRegularExpression>           // for QRegularExpression
#include <QRegularExpressionValidator>  // for QRegularExpressionValidator
#include <QSize>                        // for QSize
#include <QSizePolicy>                  // for QSizePolicy, QSizePolicy::Fixed
#include <QString>                      // for QString

class QHBoxLayout;

namespace inviwo {

BoolPropertyWidgetQt::BoolPropertyWidgetQt(BoolProperty* property)
    : PropertyWidgetQt(property)
    , property_(property)
    , checkBox_{nullptr}
    , lineEdit_{nullptr}
    , label_{new EditableLabelQt(this, property_, false)} {

    // create widget layout
    QHBoxLayout* hLayout = new QHBoxLayout();
    setSpacingAndMargins(hLayout);
    hLayout->addWidget(label_);

    if (property_->getSemantics() == PropertySemantics::Text) {
        lineEdit_ = new QLineEdit();
        // adjust size policy of line edit
        QSizePolicy sizePolicy(lineEdit_->sizePolicy());
        sizePolicy.setHorizontalStretch(3);
        sizePolicy.setVerticalPolicy(QSizePolicy::Fixed);
        lineEdit_->setSizePolicy(sizePolicy);

        // set up a validator accepting "true"/1 and "false"/0
        lineEdit_->setValidator(
            new QRegularExpressionValidator(QRegularExpression("true|false|1|0"), lineEdit_));

        setFocusPolicy(lineEdit_->focusPolicy());
        setFocusProxy(lineEdit_);

        auto setPropertyValueFromString = [this]() {
            QString str(lineEdit_->text());
            RenderContext::getPtr()->activateDefaultRenderContext();
            property_->set(str == "true" || str == "1");
        };
        connect(lineEdit_, &QLineEdit::editingFinished, setPropertyValueFromString);
        hLayout->addWidget(lineEdit_);
    } else {
        checkBox_ = new QCheckBox();
        checkBox_->setFixedSize(QSize(15, 15));

        setFocusPolicy(checkBox_->focusPolicy());
        setFocusProxy(checkBox_);

        auto setPropertyValueFromCheckbox = [this](bool checked) {
            RenderContext::getPtr()->activateDefaultRenderContext();
            property_->set(checked);
        };
        connect(checkBox_, &QCheckBox::toggled, setPropertyValueFromCheckbox);
        hLayout->addWidget(checkBox_);
    }
    setDisabled(property_->getReadOnly());

    setLayout(hLayout);

    updateFromProperty();
}

void BoolPropertyWidgetQt::updateFromProperty() {
    if (checkBox_) checkBox_->setChecked(property_->get());
    if (lineEdit_) lineEdit_->setText(property_->get() ? "true" : "false");
}

}  // namespace inviwo
