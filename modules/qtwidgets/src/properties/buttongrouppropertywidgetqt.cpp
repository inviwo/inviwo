/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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

#include <modules/qtwidgets/properties/buttongrouppropertywidgetqt.h>

#include <inviwo/core/properties/buttongroupproperty.h>

#include <modules/qtwidgets/inviwoqtutils.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QHBoxLayout>
#include <QLayoutItem>
#include <QLabel>
#include <QPushButton>
#include <QInputDialog>
#include <QMenu>
#include <QAction>
#include <QIcon>
#include <warn/pop>

namespace inviwo {

ButtonGroupPropertyWidgetQt::ButtonGroupPropertyWidgetQt(ButtonGroupProperty* property)
    : PropertyWidgetQt(property), property_(property) {

    setObjectName("ButtonGroupPropertyWidgetQt");

    QHBoxLayout* hLayout = new QHBoxLayout();
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->setSpacing(0);

    for (size_t i = 0; i < property->size(); ++i) {
        const auto& button = property->getButton(i);

        auto buttonWidget = new QPushButton(this);
        if (button.icon) {
            buttonWidget->setIcon(QIcon(utilqt::toQString(*button.icon)));
        }
        if (button.name) {
            buttonWidget->setText(utilqt::toQString(*button.name));
        }
        if (button.tooltip) {
            buttonWidget->setToolTip(utilqt::toQString(*button.tooltip));
        }

        connect(buttonWidget, &QPushButton::released, this, [this, i]() {
            if (!property_->getReadOnly()) property_->pressButton(i);
        });
        hLayout->addWidget(buttonWidget);
    }
    if (property->size() > 0) {
        setFocusPolicy(hLayout->itemAt(0)->widget()->focusPolicy());
        setFocusProxy(hLayout->itemAt(0)->widget());
    }

    setLayout(hLayout);
}

void ButtonGroupPropertyWidgetQt::updateFromProperty() {}

}  // namespace inviwo
