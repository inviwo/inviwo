/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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
#include <inviwo/core/properties/property.h>
#include <modules/qtwidgets/inviwoqtutils.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QMoveEvent>
#include <QShowEvent>
#include <QResizeEvent>
#include <QSettings>
#include <warn/pop>

namespace inviwo {

const std::string PropertyEditorWidgetQt::visibleKey = "PropertyEditorWidgetVisible";
const std::string PropertyEditorWidgetQt::floatingKey = "PropertyEditorWidgetFloating";
const std::string PropertyEditorWidgetQt::stickyKey = "PropertyEditorWidgetSticky";
const std::string PropertyEditorWidgetQt::sizeKey = "PropertyEditorWidgetSize";
const std::string PropertyEditorWidgetQt::positionKey = "PropertyEditorWidgetPosition";
const std::string PropertyEditorWidgetQt::dockareaKey = "PropertyEditorWidgetDockStatus";

PropertyEditorWidgetQt::PropertyEditorWidgetQt(Property* property, const std::string& widgetName,
                                               const std::string& objName)
    : InviwoDockWidget(utilqt::toQString(widgetName), utilqt::getApplicationMainWindow(),
                       utilqt::toQString(objName))
    , property_{property} {

    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    QObject::connect(
        this, &InviwoDockWidget::dockLocationChanged, this, [this](Qt::DockWidgetArea dockArea) {
            property_->setMetaData<IntMetaData>(dockareaKey, static_cast<int>(dockArea));
        });
    QObject::connect(this, &InviwoDockWidget::topLevelChanged, this, [this](bool floatimg) {
        property_->setMetaData<BoolMetaData>(floatingKey, floatimg);
    });
    QObject::connect(this, &InviwoDockWidget::stickyFlagChanged, this, [this](bool sticky) {
        property_->setMetaData<BoolMetaData>(stickyKey, sticky);
    });
    property_->addObserver(this);
    setFloating(true);
}

PropertyEditorWidgetQt::PropertyEditorWidgetQt(Property* property, const std::string& widgetName)
    : PropertyEditorWidgetQt(property, widgetName, widgetName) {}

PropertyEditorWidgetQt::~PropertyEditorWidgetQt() = default;

void PropertyEditorWidgetQt::setVisible(bool visible) {
    InviwoDockWidget::setVisible(visible);
    property_->setMetaData<BoolMetaData>(visibleKey, visible);
}

void PropertyEditorWidgetQt::setDimensions(const ivec2& dimensions) {
    InviwoDockWidget::resize(utilqt::toQSize(dimensions));
    property_->setMetaData<IntVec2MetaData>(sizeKey, dimensions);
}

void PropertyEditorWidgetQt::setPosition(const ivec2& pos) {
    InviwoDockWidget::move(utilqt::toQPoint(pos));
    property_->setMetaData<IntVec2MetaData>(positionKey, pos);
}

bool PropertyEditorWidgetQt::isVisible() const { return InviwoDockWidget::isVisible(); }

void PropertyEditorWidgetQt::resizeEvent(QResizeEvent* event) {
    property_->setMetaData<IntVec2MetaData>(sizeKey, utilqt::toGLM(event->size()));
    InviwoDockWidget::resizeEvent(event);
}

void PropertyEditorWidgetQt::showEvent(QShowEvent* e) {
    property_->setMetaData<BoolMetaData>(visibleKey, true);
    InviwoDockWidget::showEvent(e);
}

void PropertyEditorWidgetQt::closeEvent(QCloseEvent* e) {
    hide();
    InviwoDockWidget::closeEvent(e);
}

void PropertyEditorWidgetQt::moveEvent(QMoveEvent* event) {
    property_->setMetaData<IntVec2MetaData>(positionKey, utilqt::toGLM(event->pos()));
    InviwoDockWidget::moveEvent(event);
}

void PropertyEditorWidgetQt::saveState() {
    QSettings settings;
    settings.beginGroup(objectName());

    settings.setValue("sticky", isSticky());
    property_->setMetaData<BoolMetaData>(stickyKey, isSticky());

    settings.setValue("floating", isFloating());
    property_->setMetaData<BoolMetaData>(floatingKey, isFloating());

    if (auto mainWindow = utilqt::getApplicationMainWindow()) {
        settings.setValue("dockarea", static_cast<int>(mainWindow->dockWidgetArea(this)));
        property_->setMetaData<IntMetaData>(dockareaKey,
                                            static_cast<int>(mainWindow->dockWidgetArea(this)));
    }

    settings.setValue("size", size());
    property_->setMetaData<IntVec2MetaData>(sizeKey, getDimensions());

    property_->setMetaData<IntVec2MetaData>(positionKey, getPosition());
    property_->setMetaData<BoolMetaData>(visibleKey, isVisible());

    settings.endGroup();
}

void PropertyEditorWidgetQt::loadState() {
    // restore editor state from metadata (position, docking state, sticky flag, etc.)

    QSettings settings;
    settings.beginGroup(objectName());

    if (property_->hasMetaData<BoolMetaData>(stickyKey)) {
        setSticky(property_->getMetaData<BoolMetaData>(stickyKey, false));
    } else if (settings.contains("sticky")) {
        setSticky(settings.value("sticky").toBool());
    }

    if (property_->hasMetaData<BoolMetaData>(floatingKey)) {
        setFloating(property_->getMetaData<BoolMetaData>(floatingKey, false));
    } else if (settings.contains("floating")) {
        setFloating(settings.value("floating").toBool());
    }

    if (auto mainWindow = utilqt::getApplicationMainWindow()) {
        if (property_->hasMetaData<IntMetaData>(dockareaKey)) {
            auto dockarea = static_cast<Qt::DockWidgetArea>(
                property_->getMetaData<IntMetaData>(dockareaKey), Qt::NoDockWidgetArea);
            mainWindow->addDockWidget(dockarea, this);
        } else if (settings.contains("dockarea")) {
            auto dockarea = static_cast<Qt::DockWidgetArea>(settings.value("dockarea").toInt());
            mainWindow->addDockWidget(dockarea, this);
        }
    }

    if (property_->hasMetaData<IntVec2MetaData>(sizeKey)) {
        resize(utilqt::toQSize(property_->getMetaData<IntVec2MetaData>(sizeKey, ivec2{0})));
    } else if (settings.contains("size")) {
        resize(settings.value("size").toSize());
    }

    if (property_->hasMetaData<IntVec2MetaData>(positionKey)) {
        auto pos = utilqt::toQPoint(property_->getMetaData<IntVec2MetaData>(positionKey, ivec2{0}));
        auto newPos = utilqt::movePointOntoDesktop(pos, InviwoDockWidget::size(), false);
        move(newPos);
    } else if (auto mainWindow = utilqt::getApplicationMainWindow()) {
        // We assume that this is a new widget and give it a new position
        auto newPos = mainWindow->pos();
        newPos += utilqt::offsetWidget();
        move(newPos);
    }

    if (property_->hasMetaData<BoolMetaData>(visibleKey)) {
        setVisible(property_->getMetaData<BoolMetaData>(visibleKey, false));
    }
    settings.endGroup();

    setReadOnly(property_->getReadOnly());
}

void PropertyEditorWidgetQt::onSetReadOnly(Property*, bool readonly) { setReadOnly(readonly); }

void PropertyEditorWidgetQt::setReadOnly(bool readonly) { setDisabled(readonly); }

Property* PropertyEditorWidgetQt::getProperty() const { return property_; }

ivec2 PropertyEditorWidgetQt::getPosition() const { return utilqt::toGLM(pos()); }

ivec2 PropertyEditorWidgetQt::getDimensions() const { return utilqt::toGLM(size()); }

}  // namespace inviwo
