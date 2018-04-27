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

#include <modules/qtwidgets/properties/tfprimitivesetwidgetqt.h>

#include <inviwo/core/properties/isovalueproperty.h>
#include <inviwo/core/properties/transferfunctionproperty.h>
#include <inviwo/core/datastructures/tfprimitiveset.h>
#include <inviwo/core/util/colorconversion.h>
#include <inviwo/core/network/networklock.h>
#include <modules/qtwidgets/editablelabelqt.h>
#include <modules/qtwidgets/inviwoqtutils.h>
#include <modules/qtwidgets/properties/stringmultilinepropertywidgetqt.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QHBoxLayout>
#include <warn/pop>

#include <sstream>

namespace inviwo {

TFPrimitiveSetWidgetQt::TFPrimitiveSetWidgetQt(IsoValueProperty* property)
    : PropertyWidgetQt(property)
    , propertyPtr_(std::make_unique<Model<IsoValueProperty*>>(property)) {

    initializeWidget();
}

TFPrimitiveSetWidgetQt::TFPrimitiveSetWidgetQt(TransferFunctionProperty* property)
    : PropertyWidgetQt(property)
    , propertyPtr_(std::make_unique<Model<TransferFunctionProperty*>>(property)) {

    initializeWidget();
}

void TFPrimitiveSetWidgetQt::setPropertyValue() {
    property_->setInitiatingWidget(this);

    // convert string back to position, alpha, and RGB color values
    std::vector<TFPrimitiveData> primitives;

    std::istringstream ss(utilqt::fromQString(textEdit_->toPlainText()));
    std::string str;
    while (ss.good()) {
        double pos = 0.0;
        double alpha = 0.0;

        // TODO: trim white space, separate by line, separate by white space
        ss >> pos >> alpha >> str;
        if (ss.fail()) {
            LogError("Could not extract TF primitive ('position alpha #RRGGBB') from: \""
                     << ss.str() << "\"");
            break;
        }
        primitives.push_back({pos, vec4(vec3(color::hex2rgba(str)), static_cast<float>(alpha))});
    }

    // need to undo value mapping in case of relative TF and PropertySemantics
    // being "Text (normalized)"
    bool performMapping = (propertyPtr_->get().getType() == TFPrimitiveSetType::Relative) &&
                          (property_->getSemantics().getString() == "Text");

    auto port = propertyPtr_->getVolumePort();
    if (performMapping && port && port->hasData()) {
        const dvec2 range = port->getData()->dataMap_.valueRange;

        auto renormalizePos = [range](double pos) { return (pos - range.x) / (range.y - range.x); };
        for (auto &elem : primitives) {
            elem.pos = renormalizePos(elem.pos);
        }
    }

    {
        NetworkLock lock(property_);
        propertyPtr_->get().clear();
        propertyPtr_->get().add(primitives);
    }

    property_->clearInitiatingWidget();
}

void TFPrimitiveSetWidgetQt::updateFromProperty() {
    bool performMapping = (propertyPtr_->get().getType() == TFPrimitiveSetType::Relative) &&
                          (property_->getSemantics().getString() == "Text");

    dvec2 range(0.0, 1.0);
    auto port = propertyPtr_->getVolumePort();
    if (port && port->hasData()) {
        range = port->getData()->dataMap_.valueRange;
    } else {
        // no need to perform mapping without a proper value range
        performMapping = false;
    }

    auto mapPos = [range](double pos) { return pos * (range.y - range.x) + range.x; };

    // convert TF primitives to "position alpha #RRGGBB"
    std::ostringstream ss;
    for (auto elem : propertyPtr_->get()) {
        // write color as HTML color code
        auto pos = elem->getPosition();
        if (performMapping) {
            pos = mapPos(pos);
        }
        ss << pos << " " << elem->getAlpha() << " " << color::rgb2hex(elem->getColor()) << "\n";
    }

    QString newContents(utilqt::toQString(ss.str()));
    if (textEdit_->toPlainText() != newContents) {
        textEdit_->setPlainText(newContents);
        textEdit_->moveCursor(QTextCursor::Start);

        textEdit_->adjustHeight();
    }
}

void TFPrimitiveSetWidgetQt::initializeWidget() {
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

}  // namespace inviwo
