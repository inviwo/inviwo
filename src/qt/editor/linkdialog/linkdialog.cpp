/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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

#include <inviwo/qt/editor/linkdialog/linkdialog.h>
#include <inviwo/qt/editor/linkdialog/linkdialoggraphicsitems.h>
#include <inviwo/qt/editor/linkdialog/linkdialogscene.h>
#include <inviwo/qt/editor/linkdialog/linkdialogview.h>

#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/propertyowner.h>
#include <inviwo/core/properties/compositeproperty.h>

#include <modules/qtwidgets/inviwoqtutils.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QSettings>
#include <QPushButton>
#include <QVBoxLayout>
#include <QStandardItemModel>
#include <QLineEdit>
#include <QToolButton>
#include <QMenu>
#include <QAction>
#include <QComboBox>
#include <QEventLoop>
#include <QCheckBox>
#include <warn/pop>

namespace inviwo {

LinkDialog::LinkDialog(Processor* srcProcessor, Processor* dstProcessor, QWidget* parent)
    : InviwoDockWidget("Edit Property Links", parent, "LinkDialogWidget") {

    setFloating(true);
    setAttribute(Qt::WA_DeleteOnClose);
    setAllowedAreas(Qt::NoDockWidgetArea);
    setFixedWidth(
        utilqt::emToPx(this, linkdialog::dialogWidth / static_cast<double>(utilqt::refEm())));
    setMinimumHeight(
        utilqt::emToPx(this, linkdialog::dialogHeight / static_cast<double>(utilqt::refEm())));

    auto scene =
        new LinkDialogGraphicsScene(this, srcProcessor->getNetwork(), srcProcessor, dstProcessor);
    scene->setSceneRect(0.0, 0.0, linkdialog::dialogWidth, size().height());
    connect(scene, &LinkDialogGraphicsScene::closeDialog, this, &LinkDialog::close);

    auto view = new LinkDialogGraphicsView(this);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setScene(scene);
    const auto scale = utilqt::emToPx(this, 1.0) / static_cast<double>(utilqt::refEm());
    view->setTransform(QTransform::fromScale(scale, scale), false);

    auto mainLayout = new QVBoxLayout();
    setContents(mainLayout);
    mainLayout->addWidget(view);

    auto smartLinkPushButtonLayout = new QHBoxLayout{};
    smartLinkPushButtonLayout->setAlignment(Qt::AlignLeft);

    // hidden check box
    auto showHidden = new QCheckBox("Show Hidden", this);
    connect(showHidden, &QCheckBox::stateChanged,
            [scene](int state) { scene->showHidden(state == Qt::Checked); });
    smartLinkPushButtonLayout->addWidget(showHidden, 10);

    // smart link button
    auto smartLink = new QToolButton(this);
    smartLink->setObjectName("smartLinkButton");
    smartLink->setText("SmartLink");
    smartLink->setPopupMode(QToolButton::MenuButtonPopup);
    smartLink->setToolButtonStyle(Qt::ToolButtonTextOnly);

    auto menu = new QMenu(this);
    auto matchType = menu->addAction("Match Type");
    matchType->setCheckable(true);
    matchType->setChecked(true);
    auto matchId = menu->addAction("Match Identifier");
    matchId->setCheckable(true);
    matchId->setChecked(true);

    auto leftLink = menu->addAction("Link Left to Right");
    leftLink->setCheckable(true);
    leftLink->setChecked(true);

    auto rightLink = menu->addAction("Link Right to Left");
    rightLink->setCheckable(true);
    rightLink->setChecked(true);

    smartLink->setMenu(menu);

    auto shouldLink = [net = srcProcessor->getNetwork(), matchType, matchId](Property* src,
                                                                             Property* dst) {
        if (!net->canLink(src, dst)) {
            return false;
        }
        if (matchType->isChecked() && dst->getClassIdentifier() != src->getClassIdentifier()) {
            return false;
        }
        if (matchId->isChecked() && dst->getIdentifier() != src->getIdentifier()) {
            return false;
        }

        return true;
    };

    connect(smartLink, &QToolButton::clicked, this,
            [scene, srcProcessor, dstProcessor, shouldLink, leftLink, rightLink]() {
                std::function<void(Property * src, Property * dst)> link = [&](Property* src,
                                                                               Property* dst) {
                    if (scene->isPropertyExpanded(src) || scene->isPropertyExpanded(dst)) {

                        if (shouldLink(src, dst)) {
                            auto compSrc = dynamic_cast<CompositeProperty*>(src);
                            auto compDst = dynamic_cast<CompositeProperty*>(dst);
                            if (compSrc && compDst) {
                                // If composite property then try to link sub-properties only
                                for (auto& i : compSrc->getProperties()) {
                                    for (auto& j : compDst->getProperties()) {
                                        link(i, j);
                                    }
                                }
                            }
                        }
                    } else if (leftLink->isChecked() && shouldLink(src, dst)) {
                        scene->addPropertyLink(src, dst, false);
                        if (rightLink->isChecked()) {
                            scene->addPropertyLink(dst, src, false);
                        }
                    }
                };

                for (auto& src : srcProcessor->getProperties()) {
                    for (auto& dst : dstProcessor->getProperties()) {
                        link(src, dst);
                    }
                }
            });
    smartLinkPushButtonLayout->addWidget(smartLink, 30);

    // delete button
    auto deleteAllLinkPushButton = new QPushButton("Delete All", this);
    connect(deleteAllLinkPushButton, &QPushButton::clicked, this,
            [scene]() { scene->removeAllPropertyLinks(); });
    smartLinkPushButtonLayout->addWidget(deleteAllLinkPushButton, 10);

    // expand composite
    auto expandCompositeButton = new QPushButton("Expand/Collapse", this);
    expandCompositeButton->setChecked(false);
    smartLinkPushButtonLayout->addWidget(expandCompositeButton, 10);
    connect(expandCompositeButton, &QPushButton::clicked, [scene]() { scene->toggleExpand(); });

    mainLayout->addLayout(smartLinkPushButtonLayout);

    loadState();
}

QSize LinkDialog::sizeHint() const {
    QSize size = layout()->sizeHint();
    size.setHeight(
        utilqt::emToPx(this, linkdialog::dialogHeight / static_cast<double>(utilqt::refEm())));
    return size;
}

}  // namespace inviwo
