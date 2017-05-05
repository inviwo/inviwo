/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2017 Inviwo Foundation
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

#include <modules/qtwidgets/properties/optionpropertywidgetqt.h>
#include <modules/qtwidgets/inviwowidgetsqt.h>
#include <modules/qtwidgets/editablelabelqt.h>

#include <typeinfo>

#include <warn/push>
#include <warn/ignore/all>
#include <QHBoxLayout>
#include <QSignalBlocker>
#include <QMenu>
#include <warn/pop>

namespace inviwo {

OptionPropertyWidgetQt::OptionPropertyWidgetQt(BaseOptionProperty* property)
    : PropertyWidgetQt(property)
    , property_(property)
    , comboBox_{new IvwComboBox(this)}
    , label_{new EditableLabelQt(this, property_)} {

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

    connect(comboBox_, static_cast<void (QComboBox::*)(int)>(&IvwComboBox::currentIndexChanged),
            this, &OptionPropertyWidgetQt::optionChanged);

    updateFromProperty();
}

void OptionPropertyWidgetQt::optionChanged(int) {
    if (comboBox_->count()
        && comboBox_->currentIndex() >= 0
        && static_cast<size_t>(comboBox_->currentIndex()) != property_->getSelectedIndex()) {
        property_->setInitiatingWidget(this);
        property_->setSelectedIndex(comboBox_->currentIndex());
        property_->clearInitiatingWidget();
    }
}

void OptionPropertyWidgetQt::updateFromProperty() {
    QSignalBlocker block{comboBox_};

    comboBox_->clear();
    std::vector<std::string> names = property_->getDisplayNames();

    for (auto& name : names) {
        QString option = QString::fromStdString(name);
        comboBox_->addItem(option);
    }
    comboBox_->setCurrentIndex(static_cast<int>(property_->getSelectedIndex()));
}

} // namespace
