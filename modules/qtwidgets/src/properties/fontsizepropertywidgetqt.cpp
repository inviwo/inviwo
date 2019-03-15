/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
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

#include <modules/qtwidgets/properties/fontsizepropertywidgetqt.h>
#include <modules/qtwidgets/inviwowidgetsqt.h>
#include <modules/qtwidgets/editablelabelqt.h>

#include <typeinfo>
#include <algorithm>

#include <warn/push>
#include <warn/ignore/all>
#include <QHBoxLayout>
#include <QSignalBlocker>
#include <QMenu>
#include <QIntValidator>
#include <warn/pop>

namespace inviwo {

FontSizePropertyWidgetQt::FontSizePropertyWidgetQt(IntProperty* property)
    : PropertyWidgetQt(property)
    , property_(property)
    , comboBox_{new IvwComboBox(this)}
    , label_{new EditableLabelQt(this, property_)} {

    setFocusPolicy(comboBox_->focusPolicy());
    setFocusProxy(comboBox_);

    QHBoxLayout* hLayout = new QHBoxLayout();
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->setSpacing(7);
    setLayout(hLayout);

    comboBox_->setEnabled(!property_->getReadOnly());
    comboBox_->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(comboBox_, &IvwComboBox::customContextMenuRequested, this, [this](const QPoint& pos) {
        if (auto menu = getContextMenu()) {
            menu->exec(comboBox_->mapToGlobal(pos));
        }
    });

    QSizePolicy slidersPol = comboBox_->sizePolicy();
    slidersPol.setHorizontalStretch(3);
    comboBox_->setSizePolicy(slidersPol);

    hLayout->addWidget(label_);

    {
        QWidget* widget = new QWidget(this);
        QSizePolicy sliderPol = widget->sizePolicy();
        sliderPol.setHorizontalStretch(3);
        widget->setSizePolicy(sliderPol);
        QGridLayout* vLayout = new QGridLayout();
        widget->setLayout(vLayout);
        vLayout->setContentsMargins(0, 0, 0, 0);
        vLayout->setSpacing(0);

        vLayout->addWidget(comboBox_);
        hLayout->addWidget(widget);
    }

    comboBox_->setEditable(true);
    comboBox_->setValidator(new QIntValidator(0, 1000));
    comboBox_->setInsertPolicy(QComboBox::NoInsert);

    connect(comboBox_->lineEdit(), &QLineEdit::editingFinished, [&]() {
        int fontSize = comboBox_->currentText().toInt();
        // check whether it is a custom font size
        if (std::find(fontSizes_.begin(), fontSizes_.end(), fontSize) == fontSizes_.end()) {
            // custom font size
            if (fontSize != property_->get()) {
                property_->setInitiatingWidget(this);
                property_->set(fontSize);
                property_->clearInitiatingWidget();
            }
        }
    });

    connect(comboBox_, static_cast<void (QComboBox::*)(int)>(&IvwComboBox::currentIndexChanged),
            [&](int option) {
                if (option >= 0) {
                    int fontSize = comboBox_->currentData().toInt();

                    if (fontSize != property_->get()) {
                        property_->setInitiatingWidget(this);
                        property_->set(fontSize);
                        property_->clearInitiatingWidget();
                    }
                }
            });

    updateFromProperty();
}

void FontSizePropertyWidgetQt::updateFromProperty() {
    QSignalBlocker block{comboBox_};

    comboBox_->clear();
    for (auto size : fontSizes_) {
        comboBox_->addItem(QString::number(size), size);
    }

    auto it = std::find(fontSizes_.begin(), fontSizes_.end(), property_->get());
    const bool customFontSize = (it == fontSizes_.end());
    if (customFontSize) {
        comboBox_->setCurrentIndex(-1);
        comboBox_->setEditText(QString::number(property_->get()));
    } else {
        comboBox_->setCurrentIndex(static_cast<int>(std::distance(fontSizes_.begin(), it)));
    }
}

}  // namespace inviwo
