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

#include <modules/qtwidgets/properties/isotfpropertywidgetqt.h>

#include <inviwo/core/properties/isotfproperty.h>
#include <inviwo/core/util/filesystem.h>
#include <modules/qtwidgets/properties/tfpropertywidgetqt.h>
#include <modules/qtwidgets/editablelabelqt.h>
#include <modules/qtwidgets/inviwoqtutils.h>
#include <modules/qtwidgets/inviwowidgetsqt.h>
#include <modules/qtwidgets/tf/tfutils.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QHBoxLayout>
#include <warn/pop>

namespace inviwo {

IsoTFPropertyWidgetQt::IsoTFPropertyWidgetQt(IsoTFProperty* property)
    : PropertyWidgetQt(property)
    , label_{new EditableLabelQt(this, property_)}
    , btnOpenTF_{new TFPushButton(property, this)} {

    setFocusPolicy(btnOpenTF_->focusPolicy());
    setFocusProxy(btnOpenTF_);

    QHBoxLayout* hLayout = new QHBoxLayout;
    setSpacingAndMargins(hLayout);

    hLayout->addWidget(label_);
    hLayout->addWidget(btnOpenTF_);

    setLayout(hLayout);
    updateFromProperty();

    connect(btnOpenTF_, &IvwPushButton::clicked, [this, property]() {
        if (!tfDialog_) {
            tfDialog_ = std::make_unique<TFPropertyDialog>(property);
            tfDialog_->setVisible(true);
        } else {
            tfDialog_->setVisible(!tfDialog_->isVisible());
        }
    });

    QSizePolicy sp = sizePolicy();
    sp.setVerticalPolicy(QSizePolicy::Fixed);
    setSizePolicy(sp);
}

IsoTFPropertyWidgetQt::~IsoTFPropertyWidgetQt() {
    if (tfDialog_) tfDialog_->hide();
}

void IsoTFPropertyWidgetQt::updateFromProperty() { btnOpenTF_->updateFromProperty(); }

TFPropertyDialog* IsoTFPropertyWidgetQt::getEditorWidget() const { return tfDialog_.get(); }

bool IsoTFPropertyWidgetQt::hasEditorWidget() const { return tfDialog_ != nullptr; }

void IsoTFPropertyWidgetQt::setReadOnly(bool readonly) { label_->setDisabled(readonly); }

std::unique_ptr<QMenu> IsoTFPropertyWidgetQt::getContextMenu() {
    auto menu = PropertyWidgetQt::getContextMenu();

    menu->addSeparator();

    auto presets = menu->addMenu("&TF Presets");
    presets->setEnabled(!property_->getReadOnly());
    if (!property_->getReadOnly()) {
        const auto basePath = filesystem::getPath(PathType::TransferFunctions);
        auto files = filesystem::getDirectoryContentsRecursively(basePath);

        auto p = static_cast<TransferFunctionProperty*>(property_);

        for (auto file : files) {
            for (auto& ext : p->get().getSupportedExtensions()) {
                if (ext.matches(file)) {
                    // remove basepath and trailing directory separator
                    auto action =
                        presets->addAction(utilqt::toQString(file.substr(basePath.length() + 1)));
                    connect(action, &QAction::triggered, this, [this, p, file, ext]() {
                        NetworkLock lock(p);
                        p->get().load(file, ext);
                    });

                    break;
                }
            }
        }
        if (presets->actions().empty()) {
            auto action = presets->addAction("No Presets Available");
            action->setEnabled(false);
        }
    }

    auto clearTF = menu->addAction("&Clear TF & Isovalues");
    clearTF->setEnabled(!property_->getReadOnly());

    connect(clearTF, &QAction::triggered, this, [this]() {
        NetworkLock lock(property_);
        auto p = static_cast<IsoTFProperty*>(property_);
        p->tf_.get().clear();
        p->isovalues_.get().clear();
    });

    auto importMenu = menu->addMenu("&Import");
    auto exportMenu = menu->addMenu("&Export");
    importMenu->setEnabled(!property_->getReadOnly());

    auto importTF = importMenu->addAction("&TF...");
    auto importIso = importMenu->addAction("&Isovalues...");
    connect(importTF, &QAction::triggered, this, [this]() {
        util::importFromFile(static_cast<IsoTFProperty*>(property_)->tf_.get(), this);
    });
    connect(importIso, &QAction::triggered, this, [this]() {
        util::importFromFile(static_cast<IsoTFProperty*>(property_)->isovalues_.get(), this);
    });

    auto exportTF = exportMenu->addAction("&TF...");
    auto exportIso = exportMenu->addAction("&Isovalues...");
    connect(exportTF, &QAction::triggered, this, [this]() {
        util::exportToFile(static_cast<IsoTFProperty*>(property_)->tf_.get(), this);
    });
    connect(exportIso, &QAction::triggered, this, [this]() {
        util::exportToFile(static_cast<IsoTFProperty*>(property_)->isovalues_.get(), this);
    });

    return menu;
}

}  // namespace inviwo
