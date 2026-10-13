/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2026 Inviwo Foundation
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

#include <inviwo/core/metadata/metadata.h>
#include <inviwo/core/properties/property.h>
#include <inviwo/core/util/glmvec.h>
#include <modules/qtwidgets/inviwodockwidget.h>
#include <modules/qtwidgets/inviwoqtutils.h>

#include <utility>
#include <algorithm>

#include <QDebug>
#include <QMainWindow>
#include <QMoveEvent>
#include <QObject>
#include <QPoint>
#include <QResizeEvent>
#include <QSettings>
#include <QSize>
#include <QVariant>
#include <Qt>
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

#if (QT_VERSION >= QT_VERSION_CHECK(6, 9, 0))
    setDockLocation(Qt::RightDockWidgetArea);
#endif
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

void PropertyEditorWidgetQt::getState(UnorderedStringMap<QVariant>& state) const {
    InviwoDockWidget::getState(state);

    if (auto* mainWindow = utilqt::getApplicationMainWindow()) {
        state.emplace("dockarea", static_cast<int>(mainWindow->dockWidgetArea(
                                      const_cast<PropertyEditorWidgetQt*>(this))));
    }
}
void PropertyEditorWidgetQt::setState(const UnorderedStringMap<QVariant>& state) {
    InviwoDockWidget::setState(state);

    if (auto* mainWindow = utilqt::getApplicationMainWindow()) {
        const auto dockArea = [&]() {
            if (state.contains("dockarea")) {
                return static_cast<Qt::DockWidgetArea>(state.at("dockarea").toInt());
            } else {
                return Qt::NoDockWidgetArea;
            }
        }();
        if (dockArea != Qt::NoDockWidgetArea) {
            mainWindow->addDockWidget(dockArea, this);
        }
    }

    setReadOnly(getProperty()->getReadOnly());
}

void PropertyEditorWidgetQt::saveState() {
    UnorderedStringMap<QVariant> state;
    getState(state);
    saveStateDefault(state);

    if (state.contains("sticky")) {
        getProperty()->setMetaData<BoolMetaData>(stickyKey, state.at("sticky").toBool());
    }
    if (state.contains("floating")) {
        getProperty()->setMetaData<BoolMetaData>(floatingKey, state.at("floating").toBool());
    }
    if (state.contains("size")) {
        getProperty()->setMetaData<IntVec2MetaData>(sizeKey,
                                                    utilqt::toGLM(state.at("size").toSize()));
    }
    if (state.contains("pos")) {
        getProperty()->setMetaData<IntVec2MetaData>(positionKey,
                                                    utilqt::toGLM(state.at("pos").toPoint()));
    }
    if (state.contains("visible")) {
        getProperty()->setMetaData<BoolMetaData>(visibleKey, state.at("visible").toBool());
    }
    if (state.contains("dockarea")) {
        getProperty()->setMetaData<IntMetaData>(dockareaKey, state.at("dockarea").toInt());
    }
}

void PropertyEditorWidgetQt::loadState() {
    UnorderedStringMap<QVariant> state;
    loadStateDefault(state);

    if (auto* m = getProperty()->getMetaData<BoolMetaData>(stickyKey)) {
        state["sticky"] = m->get();
    }
    if (auto* m = getProperty()->getMetaData<BoolMetaData>(floatingKey)) {
        state["floating"] = m->get();
    }
    if (auto* m = getProperty()->getMetaData<IntVec2MetaData>(sizeKey)) {
        state["size"] = utilqt::toQSize(m->get());
    }
    if (auto* m = getProperty()->getMetaData<IntVec2MetaData>(positionKey)) {
        state["pos"] = utilqt::toQPoint(m->get());
    }
    if (auto* m = getProperty()->getMetaData<BoolMetaData>(visibleKey)) {
        state["visible"] = m->get();
    }
    if (auto* m = getProperty()->getMetaData<IntMetaData>(dockareaKey)) {
        state["dockarea"] = m->get();
    }

    setState(state);
}

void PropertyEditorWidgetQt::onSetReadOnly(Property*, bool readonly) { setReadOnly(readonly); }

void PropertyEditorWidgetQt::setReadOnly(bool readonly) { setDisabled(readonly); }

ivec2 PropertyEditorWidgetQt::getPosition() const { return utilqt::toGLM(pos()); }

ivec2 PropertyEditorWidgetQt::getDimensions() const { return utilqt::toGLM(size()); }

}  // namespace inviwo
