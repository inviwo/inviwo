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

#include <modules/qtwidgets/propertylistwidget.h>
#include <inviwo/core/properties/propertywidgetfactory.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/network/processornetwork.h>
#include <modules/qtwidgets/properties/collapsiblegroupboxwidgetqt.h>
#include <modules/qtwidgets/properties/propertywidgetqt.h>
#include <inviwo/core/processors/processor.h>
#include <modules/qtwidgets/inviwoqtutils.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QSignalMapper>
#include <QSettings>
#include <QStyle>
#include <QStyleOption>
#include <QPainter>
#include <warn/pop>

namespace inviwo {

PropertyListFrame::PropertyListFrame(QWidget* parent) : QWidget(parent) {
    QSizePolicy sp(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
    sp.setVerticalStretch(0);
    sp.setHorizontalStretch(1);
    QWidget::setSizePolicy(sp);
}

QSize PropertyListFrame::sizeHint() const {
    QSize size = layout()->minimumSize();
    size.setHeight(parentWidget()->width());
    return size;
}

QSize PropertyListFrame::minimumSizeHint() const {
    QSize size = layout()->minimumSize();
    size.setWidth(parentWidget()->width());
    return size;
}

void PropertyListFrame::paintEvent(QPaintEvent*) {
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

PropertyListWidget::PropertyListWidget(QWidget* parent, InviwoApplication* app)
    : InviwoDockWidget(tr("Properties"), parent, "PropertyListWidget"), app_{app} {

    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    resize(utilqt::emToPx(this, QSizeF(45, 80)));  // default size

    QSizePolicy sp(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
    sp.setVerticalStretch(1);
    sp.setHorizontalStretch(1);
    setSizePolicy(sp);

    const auto space = utilqt::refSpacePx(this);

    scrollArea_ = new QScrollArea(this);
    scrollArea_->setWidgetResizable(true);
    scrollArea_->setMinimumWidth(utilqt::emToPx(this, 30));
    scrollArea_->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
#ifdef __APPLE__
    // Scrollbars are overlayed in different way on mac...
    scrollArea_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
#else
    scrollArea_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
#endif
    scrollArea_->setFrameShape(QFrame::NoFrame);
    scrollArea_->setContentsMargins(0, space, 0, space);

    listWidget_ = new PropertyListFrame(this);
    listLayout_ = new QVBoxLayout();
    listWidget_->setLayout(listLayout_);
    listLayout_->setAlignment(Qt::AlignTop);
#ifdef __APPLE__
    // Add some space for the scrollbar on mac
    listLayout_->setContentsMargins(0, space, 10, space);
#else
    listLayout_->setContentsMargins(0, space, 0, space);
#endif
    listLayout_->setSpacing(space);
    listLayout_->setSizeConstraint(QLayout::SetMinAndMaxSize);

    scrollArea_->setWidget(listWidget_);
    setWidget(scrollArea_);
}

PropertyListWidget::~PropertyListWidget() = default;

void PropertyListWidget::addProcessorProperties(Processor* processor) {
    setUpdatesEnabled(false);
    if (auto widget = getPropertiesForProcessor(processor)) {
        widget->show();
    }
    QWidget::raise();  // Put this tab in front
    setUpdatesEnabled(true);
}

void PropertyListWidget::removeProcessorProperties(Processor* processor) {
    auto it = widgetMap_.find(processor);
    if (it != widgetMap_.end()) it->second->hide();
}

void PropertyListWidget::removeAndDeleteProcessorProperties(Processor* processor) {
    auto it = widgetMap_.find(processor);
    if (it != widgetMap_.end()) {
        it->second->hide();
        listLayout_->removeWidget(it->second);
        delete it->second;
        widgetMap_.erase(it);
    }
}

CollapsibleGroupBoxWidgetQt* PropertyListWidget::getPropertiesForProcessor(Processor* processor) {
    // check if processor widget has been already generated
    auto it = widgetMap_.find(processor);
    if (it != widgetMap_.end()) {
        return it->second;
    } else {
        return createPropertiesForProcessor(processor);
    }
}

CollapsibleGroupBoxWidgetQt* PropertyListWidget::createPropertiesForProcessor(
    Processor* processor) {
    // create property widget and store it in the map
    auto widget = new CollapsibleGroupBoxWidgetQt(processor);
    widget->hide();
    listLayout_->insertWidget(0, widget, 0, Qt::AlignTop);
    for (auto prop : processor->getProperties()) {
        widget->addProperty(prop);
    }
    widgetMap_[processor] = widget;
    return widget;
}

bool PropertyListWidget::event(QEvent* e) {
    // The network editor will post these events.
    if (e->type() == PropertyListEvent::type()) {
        PropertyListEvent* ple = static_cast<PropertyListEvent*>(e);
        ple->accept();

        auto nw = app_->getProcessorNetwork();
        Processor* p(nw->getProcessorByIdentifier(ple->processorId_));
        if (!p) return true;

        switch (ple->action_) {
            case PropertyListEvent::Action::Add: {
                addProcessorProperties(p);
                break;
            }
            case PropertyListEvent::Action::Remove: {
                removeProcessorProperties(p);
                break;
            }
        }
        return true;

    } else {
        return InviwoDockWidget::event(e);
    }
}

PropertyListEvent::PropertyListEvent(Action action, std::string processorId)
    : QEvent(PropertyListEvent::type()), action_(action), processorId_(processorId) {}

QEvent::Type PropertyListEvent::type() {
    if (PropertyListEventType == QEvent::None) {
        PropertyListEventType = static_cast<QEvent::Type>(QEvent::registerEventType());
    }
    return PropertyListEventType;
}

QEvent::Type PropertyListEvent::PropertyListEventType = QEvent::None;

}  // namespace inviwo