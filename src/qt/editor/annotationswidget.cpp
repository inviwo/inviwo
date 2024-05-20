/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2024 Inviwo Foundation
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

#include <inviwo/qt/editor/annotationswidget.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/qt/editor/inviwomainwindow.h>
#include <inviwo/qt/editor/networkeditor.h>
#include <inviwo/qt/editor/networkeditorview.h>

#include <modules/qtwidgets/properties/collapsiblegroupboxwidgetqt.h>
#include <modules/qtwidgets/inviwoqtutils.h>
#include <modules/qtwidgets/editorsettings.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QVBoxLayout>
#include <QString>
#include <QScrollArea>
#include <QLayout>
#include <QFrame>
#include <warn/pop>

namespace inviwo {

AnnotationsWidget::AnnotationsWidget(InviwoApplication* app, NetworkEditorView* networkEditorView,
                                     QWidget* parent)
    : InviwoDockWidget{"Annotations", parent, "AnnotationsWidget"} {

    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    resize(utilqt::emToPx(this, QSizeF(60, 60)));  // default size

    scrollArea_ = new QScrollArea();
    scrollArea_->setWidgetResizable(true);
    scrollArea_->setMinimumWidth(300);
    scrollArea_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea_->setFrameShape(QFrame::NoFrame);
    scrollArea_->setContentsMargins(0, 0, 0, 0);

    mainWidget_ = new QWidget();
    layout_ = new QVBoxLayout(mainWidget_);
    layout_->setAlignment(Qt::AlignTop);

    const auto space = utilqt::refSpacePx(this);
    layout_->setContentsMargins(0, space, 0, space);
    layout_->setSpacing(space);
    scrollArea_->setWidget(mainWidget_);

    setWidget(scrollArea_);

    // Need to delay this until the qtwidgets module is loaded to get access to the qt widgets
    onModulesDidRegister_ = app->getModuleManager().onModulesDidRegister([&]() { updateWidget(); });
    onModulesWillUnregister_ = app->getModuleManager().onModulesWillUnregister([&]() {
        while (auto item = layout_->takeAt(0)) {
            delete item;
        }
    });

    // register workspace annotation serialization and deserialization as well as clear callback
    annotationSerializationHandle_ = app->getWorkspaceManager()->onSave(
        [this, app, networkEditorView](Serializer& s) {
            const int fixedHeight = 256;
            try {
                auto canvases = utilqt::getCanvasImages(app->getProcessorNetwork(), false);
                for (auto& img : canvases) {
                    img.second = img.second.scaledToHeight(fixedHeight);
                }
                annotations_.setCanvasImages(canvases);

                annotations_.setNetworkImage(networkEditorView->exportViewToImage(
                    true, true, QSize(fixedHeight, fixedHeight)));
            } catch (...) {
                // something went wrong fetching the canvas images,
                // continue saving workspace file without any images
            }

            s.serialize("WorkspaceAnnotations", annotations_);
        },
        WorkspaceSaveMode::Disk);

    annotationDeserializationHandle_ =
        app->getWorkspaceManager()->onLoad([this, app](Deserializer& d) {
            annotationModifiedHandle_.reset();

            d.deserialize("WorkspaceAnnotations", annotations_);

            annotationModifiedHandle_ =
                annotations_.onModified([app]() { app->getWorkspaceManager()->setModified(); });
        });

    annotationClearHandle_ = app->getWorkspaceManager()->onClear([this, app]() {
        annotationModifiedHandle_.reset();

        annotations_.resetAllProperties();
        annotations_.setAuthor(app->getSettingsByType<EditorSettings>()->workspaceAuthor);

        annotationModifiedHandle_ =
            annotations_.onModified([app]() { app->getWorkspaceManager()->setModified(); });
    });
    annotationModifiedHandle_ =
        annotations_.onModified([app]() { app->getWorkspaceManager()->setModified(); });
}

AnnotationsWidget::~AnnotationsWidget() {
    // manually free scroll area and nested group box widget
    // The base class InviwoDockWidget is responsible for cleaning up the Qt layout but the child
    // property widgets refer to the annotations owned by this class instance.
    delete scrollArea_;
    setWidget(nullptr);
}

WorkspaceAnnotationsQt& AnnotationsWidget::getAnnotations() { return annotations_; }

const WorkspaceAnnotationsQt& AnnotationsWidget::getAnnotations() const { return annotations_; }

void AnnotationsWidget::updateWidget() {
    auto groupBox =
        new CollapsibleGroupBoxWidgetQt(nullptr, &annotations_, "Workspace Annotations");
    layout_->addWidget(groupBox);
    groupBox->initState();

    for (auto p : annotations_.getProperties()) {
        groupBox->addProperty(p);
    }

    if (groupBox->isCollapsed()) {
        groupBox->toggleCollapsed();
    }

    layout_->addStretch();
}

}  // namespace inviwo
