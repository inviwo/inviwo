/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016 Inviwo Foundation
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

#include <modules/qtwidgets/properties/propertyeditorwidgetqt.h>
#include <modules/qtwidgets/inviwoqtutils.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QMoveEvent>
#include <QShowEvent>
#include <QResizeEvent>
#include <warn/pop>

namespace inviwo {

PropertyEditorWidgetQt::PropertyEditorWidgetQt(Property* property, std::string widgetName,
                                               QWidget* parent)
    : InviwoDockWidget(QString(widgetName.c_str()), parent), PropertyEditorWidget(property) {

    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    auto dockingChanged = [this](Qt::DockWidgetArea dockArea) {
        if (dockArea == Qt::LeftDockWidgetArea) {
            setDockStatus(PropertyEditorWidgetDockStatus::DockedLeft);
        }
        else if (dockArea == Qt::RightDockWidgetArea) {
            setDockStatus(PropertyEditorWidgetDockStatus::DockedRight);
        }
        else {
            setDockStatus(PropertyEditorWidgetDockStatus::Floating);
        }
    };

    QObject::connect(this, &InviwoDockWidget::dockLocationChanged, dockingChanged);
    QObject::connect(this, &InviwoDockWidget::stickyFlagChanged,
                     [this, widgetName](bool sticky) { setEditorStickyFlag(sticky); });

    // restore editor state from metadata (position, docking state, sticky flag, etc.)
    auto mainWindow = utilqt::getApplicationMainWindow();

    // adjust docking status (floating or docked)
    if (mainWindow) {
        auto docStatus = this->getEditorDockStatus();
        switch (this->getEditorDockStatus()) {
            case PropertyEditorWidgetDockStatus::DockedLeft:
                mainWindow->addDockWidget(Qt::LeftDockWidgetArea, this);
                this->setFloating(false);
                break;
            case PropertyEditorWidgetDockStatus::DockedRight:
                mainWindow->addDockWidget(Qt::RightDockWidgetArea, this);
                this->setFloating(false);
                break;
            default:
                mainWindow->addDockWidget(Qt::RightDockWidgetArea, this);
                this->setFloating(true);
                break;
        }
    }

    // adjust window size
    const ivec2 widgetDimension = getEditorDimensionMetaData();
    resize(QSize(widgetDimension.x, widgetDimension.y));

    // adjust window position
    if (mainWindow) {
        const ivec2 pos = getEditorPositionMetaData();
        // move widget relative to main window to make sure that it is visible on screen.
        QPoint newPos = utilqt::movePointOntoDesktop(
            QPoint(pos.x, pos.y), QSize(widgetDimension.x, widgetDimension.y), false);

        if (!(newPos.x() == 0 && newPos.y() == 0)) {
            move(newPos);
        } else {  // We assume that this is a new widget and give it a new position
            newPos = mainWindow->pos();
            newPos += utilqt::offsetWidget();
            move(newPos);
        }
    }

    // set sticky flag
    bool sticky = getEditorStickyFlag();
    setSticky(sticky);

    // adjust visibility
    setVisible(getEditorVisibilityMetaData());
}

PropertyEditorWidgetQt::~PropertyEditorWidgetQt() = default;

void PropertyEditorWidgetQt::resizeEvent(QResizeEvent* event) {
    setEditorDimensions(ivec2(event->size().width(), event->size().height()));
    InviwoDockWidget::resizeEvent(event);
}

void PropertyEditorWidgetQt::showEvent(QShowEvent*) { showEditor(); }

void PropertyEditorWidgetQt::closeEvent(QCloseEvent*) { hideEditor(); }

void PropertyEditorWidgetQt::moveEvent(QMoveEvent* event) {
    ivec2 pos = ivec2(event->pos().x(), event->pos().y());
    moveEditor(pos);

    if (isFloating() && !(getEditorDockStatus() == PropertyEditorWidgetDockStatus::Floating)) {
        setDockStatus(PropertyEditorWidgetDockStatus::Floating);
    }

    InviwoDockWidget::moveEvent(event);
}

} // namespace inviwo
