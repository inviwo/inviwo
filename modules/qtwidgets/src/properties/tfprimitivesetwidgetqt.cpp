/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2025 Inviwo Foundation
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

#include <inviwo/core/datastructures/datamapper.h>                         // for DataMapper
#include <inviwo/core/datastructures/tfprimitive.h>                        // for TFPrimitiveData
#include <inviwo/core/datastructures/tfprimitiveset.h>                     // for TFPrimitiveSet
#include <inviwo/core/network/networklock.h>                               // for NetworkLock
#include <inviwo/core/ports/volumeport.h>                                  // for VolumeInport
#include <inviwo/core/properties/isovalueproperty.h>                       // for IsoValueProperty
#include <inviwo/core/properties/property.h>                               // for Property
#include <inviwo/core/properties/propertysemantics.h>                      // for PropertySemantics
#include <inviwo/core/properties/transferfunctionproperty.h>               // for TransferFuncti...
#include <inviwo/core/util/colorconversion.h>                              // for hex2rgba, rgb2hex
#include <inviwo/core/util/exception.h>                                    // for Exception
#include <inviwo/core/util/glmvec.h>                                       // for dvec2, vec3, vec4
#include <inviwo/core/util/logcentral.h>                                   // for LogCentral
#include <inviwo/core/util/transformiterator.h>                            // for TransformIterator
#include <modules/qtwidgets/editablelabelqt.h>                             // for EditableLabelQt
#include <modules/qtwidgets/inviwoqtutils.h>                               // for fromQString
#include <modules/qtwidgets/properties/propertywidgetqt.h>                 // for PropertyWidgetQt
#include <modules/qtwidgets/properties/stringmultilinepropertywidgetqt.h>  // for MultilineTextEdit

#include <QHBoxLayout>   // for QHBoxLayout
#include <QSizePolicy>   // for QSizePolicy
#include <QString>       // for operator!=
#include <QTextCursor>   // for QTextCursor
#include <glm/vec2.hpp>  // for vec<>::(anonym...

#include <cstddef>      // for size_t
#include <sstream>      // for char_traits
#include <stdexcept>    // for invalid_argument
#include <type_traits>  // for remove_extent_t

class QHBoxLayout;

namespace inviwo {

TFPrimitiveSetWidgetQt::TFPrimitiveSetWidgetQt(IsoValueProperty* property)
    : PropertyWidgetQt(property)
    , propertyPtr_(std::make_unique<PropertyModel<IsoValueProperty*>>(property)) {

    initializeWidget();
}

TFPrimitiveSetWidgetQt::TFPrimitiveSetWidgetQt(TransferFunctionProperty* property)
    : PropertyWidgetQt(property)
    , propertyPtr_(std::make_unique<PropertyModel<TransferFunctionProperty*>>(property)) {

    initializeWidget();
}

void TFPrimitiveSetWidgetQt::setPropertyValue() {
    property_->setInitiatingWidget(this);

    auto primitives = extractPrimitiveData(utilqt::fromQString(textEdit_->toPlainText()));

    // need to undo value mapping in case of relative TF and PropertySemantics
    // being "Text (normalized)"
    const bool performMapping = (propertyPtr_->get().getType() == TFPrimitiveSetType::Relative) &&
                                (property_->getSemantics().getString() == "Text");

    if (const auto* map = propertyPtr_->data().getDataMap()) {
        if (performMapping) {
            const dvec2 range = map->valueRange;
            std::ranges::for_each(
                primitives, [&](double& x) { x = util::linearMapToNormalized(x, range); },
                &TFPrimitiveData::pos);
        }
    }
    {
        const NetworkLock lock(property_);
        propertyPtr_->get().clear();
        propertyPtr_->get().add(primitives);
    }

    property_->clearInitiatingWidget();
}

void TFPrimitiveSetWidgetQt::updateFromProperty() {
    bool performMapping = (propertyPtr_->get().getType() == TFPrimitiveSetType::Relative) &&
                          (property_->getSemantics().getString() == "Text");

    dvec2 range(0.0, 1.0);
    if (const auto* map = propertyPtr_->data().getDataMap()) {
        range = map->valueRange;
    } else {
        // no need to perform mapping without a proper value range
        performMapping = false;
    }

    // convert TF primitives to "position alpha #RRGGBB"
    std::ostringstream ss;
    std::ranges::for_each(propertyPtr_->get(), [&](const TFPrimitive& p) {
        const auto pos = performMapping ? util::linearMapFromNormalized(p.getPosition(), range)
                                        : p.getPosition();
        ss << pos << " " << p.getAlpha() << " " << color::rgb2hex(p.getColor()) << "\n";
    });

    const QString newContents(utilqt::toQString(ss.str()));
    if (textEdit_->toPlainText() != newContents) {
        textEdit_->setPlainText(newContents);
        textEdit_->moveCursor(QTextCursor::Start);
        textEdit_->adjustHeight();
    }
}

void TFPrimitiveSetWidgetQt::initializeWidget() {
    auto* hLayout = new QHBoxLayout;
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

std::vector<TFPrimitiveData> TFPrimitiveSetWidgetQt::extractPrimitiveData(
    const std::string& str) const {
    std::string errorMsg;

    auto convertToDouble = [&errorMsg](double& retVal, const std::string& str,
                                       const std::string& errSource, size_t line) {
        try {
            size_t idx = 0;
            retVal = std::stod(str, &idx);
            if (idx != str.size()) {
                // there was some excess data after the number
                throw std::invalid_argument("excess information");
            }
        } catch (std::invalid_argument&) {
            errorMsg += "\n(" + std::to_string(line) + ") Invalid " + errSource + ": '" + str +
                        "'. Expected a double value.";
            return false;
        }
        return true;
    };

    // tokenize line using space or tab (multiple spaces will be collapsed)
    auto tokenize = [](const std::string& str) {
        std::vector<std::string> tokens;
        size_t tokenStart = str.find_first_not_of(" \t");
        while (tokenStart != std::string::npos) {
            auto tokenEnd = str.find_first_of(" \t", tokenStart);
            tokens.push_back(str.substr(tokenStart, tokenEnd - tokenStart));
            tokenStart = str.find_first_not_of(" \t", tokenEnd);
        }
        return tokens;
    };

    // convert string back to position, alpha, and RGB color values
    std::vector<TFPrimitiveData> primitives;

    std::istringstream ss(str);
    // proceed line by line
    size_t lineCount = 0;
    std::string line;
    while (std::getline(ss, line)) {
        ++lineCount;
        if (line.empty()) continue;

        auto tokens = tokenize(line);
        if (tokens.empty()) continue;

        if (tokens.size() < 3) {
            errorMsg += "\n(" + std::to_string(lineCount) + ") Invalid TF primitive: '" + line +
                        "'. Expected 'double double #RRGGBB' (position alpha color).";
        } else {
            double pos = 0.0;
            double alpha = 0.0;
            if (!convertToDouble(pos, tokens[0], "position", lineCount)) continue;
            if (!convertToDouble(alpha, tokens[1], "alpha", lineCount)) continue;
            try {
                primitives.push_back(
                    {pos, vec4(vec3(color::hex2rgba(tokens[2])), static_cast<float>(alpha))});
            } catch (Exception& e) {
                errorMsg += "\n(" + std::to_string(lineCount) + ") " + e.getMessage();
            }
        }
    }
    if (!errorMsg.empty()) {
        log::error("Parse error(s): {}", errorMsg);
    }
    return primitives;
}

}  // namespace inviwo
