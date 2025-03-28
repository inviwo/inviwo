/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2025 Inviwo Foundation
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

#include <inviwo/core/metadata/metadata.h>       // for BoolMetaData, IntVec2MetaData, IntMetaData
#include <inviwo/core/properties/property.h>     // for Property
#include <inviwo/core/util/glmvec.h>             // for ivec2
#include <modules/qtwidgets/inviwodockwidget.h>  // for InviwoDockWidget
#include <modules/qtwidgets/inviwoqtutils.h>     // for getApplicationMainWindow, toGLM, toQPoint

#include <utility>  // for move
#include <algorithm>

#include <QDebug>        // for operator<<
#include <QMainWindow>   // for QMainWindow
#include <QMoveEvent>    // for QMoveEvent
#include <QObject>       // for QObject
#include <QPoint>        // for QPoint
#include <QResizeEvent>  // for QResizeEvent
#include <QSettings>     // for QSettings
#include <QSize>         // for QSize
#include <QVariant>      // for QVariant
#include <Qt>            // for DockWidgetArea, operator|, qt_getEnumMet...
#include <QGuiApplication>



namespace inviwo {

PropertyEditorWidgetQt::PropertyEditorWidgetQt(Property* property, const std::string& widgetName,
                                               const std::string& objName)
    : InviwoDockWidget(utilqt::toQString(widgetName), utilqt::getApplicationMainWindow(),
                       utilqt::toQString(objName)) {

    setAllowedAreas(Qt::AllDockWidgetAreas);

    QObject::connect(this, &InviwoDockWidget::dockLocationChanged, this,
                     [property](Qt::DockWidgetArea area) {
                         property->setMetaData<IntMetaData>(dockareaKey, static_cast<int>(area));
                     });
    QObject::connect(this, &InviwoDockWidget::topLevelChanged, this, [property](bool floating) {
        property->setMetaData<BoolMetaData>(floatingKey, floating);
    });
    QObject::connect(this, &InviwoDockWidget::stickyFlagChanged, this, [property](bool sticky) {
        property->setMetaData<BoolMetaData>(stickyKey, sticky);
    });
    property->addObserver(this);
    setFloating(true);
}

PropertyEditorWidgetQt::PropertyEditorWidgetQt(Property* property, const std::string& widgetName)
    : PropertyEditorWidgetQt(property, widgetName, widgetName) {}

PropertyEditorWidgetQt::~PropertyEditorWidgetQt() = default;

void PropertyEditorWidgetQt::setVisible(bool visible) {
    InviwoDockWidget::setVisible(visible);
    getProperty()->setMetaData<BoolMetaData>(visibleKey, visible);
}

void PropertyEditorWidgetQt::setDimensions(const ivec2& dimensions) {
    InviwoDockWidget::resize(utilqt::toQSize(dimensions));
    getProperty()->setMetaData<IntVec2MetaData>(sizeKey, dimensions);
}

void PropertyEditorWidgetQt::setPosition(const ivec2& pos) {
    InviwoDockWidget::move(utilqt::toQPoint(pos));
    getProperty()->setMetaData<IntVec2MetaData>(positionKey, pos);
}

bool PropertyEditorWidgetQt::isVisible() const { return InviwoDockWidget::isVisible(); }

void PropertyEditorWidgetQt::resizeEvent(QResizeEvent* event) {
    getProperty()->setMetaData<IntVec2MetaData>(sizeKey, utilqt::toGLM(event->size()));
    InviwoDockWidget::resizeEvent(event);
}

void PropertyEditorWidgetQt::showEvent(QShowEvent* e) {
    getProperty()->setMetaData<BoolMetaData>(visibleKey, true);
    InviwoDockWidget::showEvent(e);
}

void PropertyEditorWidgetQt::closeEvent(QCloseEvent* e) {
    hide();
    InviwoDockWidget::closeEvent(e);
}

void PropertyEditorWidgetQt::moveEvent(QMoveEvent* event) {
    getProperty()->setMetaData<IntVec2MetaData>(positionKey, utilqt::toGLM(event->pos()));
    InviwoDockWidget::moveEvent(event);
}

void PropertyEditorWidgetQt::saveState() {
    QSettings settings;
    settings.beginGroup(objectName());

    settings.setValue("sticky", isSticky());
    getProperty()->setMetaData<BoolMetaData>(stickyKey, isSticky());

    settings.setValue("floating", isFloating());
    getProperty()->setMetaData<BoolMetaData>(floatingKey, isFloating());

    if (auto* mainWindow = utilqt::getApplicationMainWindow()) {
        settings.setValue("dockarea", static_cast<int>(mainWindow->dockWidgetArea(this)));
        getProperty()->setMetaData<IntMetaData>(dockareaKey,
                                                static_cast<int>(mainWindow->dockWidgetArea(this)));
    }

    settings.setValue("size", size());
    getProperty()->setMetaData<IntVec2MetaData>(sizeKey, getDimensions());

    getProperty()->setMetaData<IntVec2MetaData>(positionKey, getPosition());
    getProperty()->setMetaData<BoolMetaData>(visibleKey, isVisible());

    settings.endGroup();
}

void PropertyEditorWidgetQt::loadState() {
    // restore editor state from metadata (position, docking state, sticky flag, etc.)

    QSettings settings;
    settings.beginGroup(objectName());

    if (getProperty()->hasMetaData<BoolMetaData>(stickyKey)) {
        setSticky(getProperty()->getMetaData<BoolMetaData>(stickyKey, false));
    } else if (settings.contains("sticky")) {
        setSticky(settings.value("sticky").toBool());
    }

    if (getProperty()->hasMetaData<BoolMetaData>(floatingKey)) {
        setFloating(getProperty()->getMetaData<BoolMetaData>(floatingKey, false));
    } else if (settings.contains("floating")) {
        setFloating(settings.value("floating").toBool());
    }

    if (auto* mainWindow = utilqt::getApplicationMainWindow()) {
        if (getProperty()->hasMetaData<IntMetaData>(dockareaKey)) {
            auto dockarea = static_cast<Qt::DockWidgetArea>(getProperty()->getMetaData<IntMetaData>(
                dockareaKey, static_cast<int>(Qt::NoDockWidgetArea)));
            mainWindow->addDockWidget(dockarea, this);
        } else if (settings.contains("dockarea")) {
            auto dockarea = static_cast<Qt::DockWidgetArea>(settings.value("dockarea").toInt());
            mainWindow->addDockWidget(dockarea, this);
        }
    }

    if (getProperty()->hasMetaData<IntVec2MetaData>(positionKey)) {
        auto pos =
            utilqt::toQPoint(getProperty()->getMetaData<IntVec2MetaData>(positionKey, ivec2{0}));
        auto newPos = utilqt::movePointOntoDesktop(pos, InviwoDockWidget::size(), false);
        move(newPos);
    } else if (auto* mainWindow = utilqt::getApplicationMainWindow()) {
        // We assume that this is a new widget and give it a new position
        auto newPos = mainWindow->pos();
        newPos += utilqt::offsetWidget();
        move(newPos);
    }

    if (getProperty()->hasMetaData<IntVec2MetaData>(sizeKey)) {
        resize(utilqt::toQSize(getProperty()->getMetaData<IntVec2MetaData>(sizeKey, ivec2{0})));
    } else if (settings.contains("size")) {
        resize(settings.value("size").toSize());
    }

    if (auto* screen = QGuiApplication::screenAt(pos())) {
        const auto w = std::clamp(width(), 0, screen->availableSize().width());
        const auto h = std::clamp(height(), 0, screen->availableSize().height());
        if (QSize(w, h) != size()) {
            resize(w, h);
        }
    }

    if (getProperty()->hasMetaData<BoolMetaData>(visibleKey)) {
        setVisible(getProperty()->getMetaData<BoolMetaData>(visibleKey, false));
    }
    settings.endGroup();

    setReadOnly(getProperty()->getReadOnly());
}

void PropertyEditorWidgetQt::onSetReadOnly(Property*, bool readonly) { setReadOnly(readonly); }

void PropertyEditorWidgetQt::setReadOnly(bool readonly) { setDisabled(readonly); }

ivec2 PropertyEditorWidgetQt::getPosition() const { return utilqt::toGLM(pos()); }

ivec2 PropertyEditorWidgetQt::getDimensions() const { return utilqt::toGLM(size()); }

}  // namespace inviwo
