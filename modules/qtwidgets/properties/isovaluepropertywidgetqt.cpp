/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018 Inviwo Foundation
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

#include <modules/qtwidgets/properties/isovaluepropertywidgetqt.h>

#include <inviwo/core/properties/isovalueproperty.h>
#include <inviwo/core/util/colorconversion.h>
#include <modules/qtwidgets/editablelabelqt.h>
#include <modules/qtwidgets/inviwoqtutils.h>
#include <modules/qtwidgets/properties/stringmultilinepropertywidgetqt.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QHBoxLayout>
#include <warn/pop>

#include <sstream>

namespace inviwo {

IsoValuePropertyWidgetQt::IsoValuePropertyWidgetQt(IsoValueProperty* property)
    : PropertyWidgetQt(property), property_(property) {

    QHBoxLayout* hLayout = new QHBoxLayout;
    setSpacingAndMargins(hLayout);

    label_ = new EditableLabelQt(this, property_);
    hLayout->addWidget(label_);

    textEdit_ = new MultilineTextEdit;

    QSizePolicy sp = textEdit_->sizePolicy();
    sp.setHorizontalStretch(3);
    sp.setVerticalPolicy(QSizePolicy::Preferred);
    textEdit_->setSizePolicy(sp);

    hLayout->addWidget(textEdit_);

    setLayout(hLayout);
    connect(textEdit_, &MultilineTextEdit::editingFinished, [this]() { setPropertyValue(); });

    updateFromProperty();
}

void IsoValuePropertyWidgetQt::setPropertyValue() {
    std::string valueStr = utilqt::fromQString(textEdit_->toPlainText());
    property_->setInitiatingWidget(this);

    // convert string back to isovalue/color values
    std::vector<TFPrimitiveData> isovalues;

    std::istringstream ss(valueStr);
    std::string str;
    while (ss.good()) {
        float value = 0.0f;
        ss >> value >> str;
        if (ss.fail()) {
            break;
        }
        isovalues.push_back({value, color::hex2rgba(str)});
    }

    property_->set(isovalues);

    property_->clearInitiatingWidget();
}

void IsoValuePropertyWidgetQt::updateFromProperty() {

    // convert isovalue/color values to string
    std::ostringstream ss;
    for (size_t i = 0; i < property_->get().size(); ++i) {
        const auto& isoValue = property_->get().get(i);
        // write color as HTML color code
        ss << isoValue->getPosition() << " " << color::rgba2hex(isoValue->getColor()) << "\n";
    }

    QString newContents(utilqt::toQString(ss.str()));
    if (textEdit_->toPlainText() != newContents) {
        textEdit_->setPlainText(newContents);
        textEdit_->moveCursor(QTextCursor::Start);

        textEdit_->adjustHeight();
    }
}

}  // namespace inviwo
