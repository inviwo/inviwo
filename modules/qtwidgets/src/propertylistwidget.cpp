/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2025 Inviwo Foundation
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

#include <inviwo/core/common/inviwoapplication.h>           // for InviwoApplication
#include <inviwo/core/network/processornetwork.h>           // for ProcessorNetwork
#include <inviwo/core/network/processornetworkevaluator.h>  // for ProcessorNetworkEv...
#include <inviwo/core/processors/processor.h>               // for Processor
#include <inviwo/core/properties/property.h>                // for Property
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/propertyowner.h>                      // for PropertyOwner
#include <inviwo/core/properties/propertywidgetfactory.h>              // for PropertyWidgetFactory
#include <inviwo/core/util/raiiutils.h>                                // for OnScopeExit, OnSco...
#include <inviwo/core/util/rendercontext.h>                            // for RenderContext
#include <modules/qtwidgets/inviwodockwidget.h>                        // for InviwoDockWidget
#include <modules/qtwidgets/inviwoqtutils.h>                           // for emToPx, refSpacePx
#include <modules/qtwidgets/properties/collapsiblegroupboxwidgetqt.h>  // for CollapsibleGroupBo...
#include <modules/qtwidgets/properties/propertywidgetqt.h>             // for PropertyWidgetQt

#include <functional>  // for __base
#include <memory>      // for unique_ptr
#include <utility>     // for pair
#include <vector>      // for vector

#include <QFrame>        // for QFrame, QFrame::No...
#include <QLayout>       // for QLayout, QLayout::...
#include <QPainter>      // for QPainter
#include <QScrollArea>   // for QScrollArea
#include <QSizeF>        // for QSizeF
#include <QSizePolicy>   // for QSizePolicy, QSize...
#include <QString>       // for QString
#include <QStyle>        // for QStyle, QStyle::PE...
#include <QStyleOption>  // for QStyleOption
#include <QVBoxLayout>   // for QVBoxLayout
#include <Qt>            // for AlignTop, operator|

class QPaintEvent;

namespace inviwo {

PropertyListFrame::PropertyListFrame(QWidget* parent, PropertyWidgetFactory* factory)
    : QWidget(parent), listLayout_{new QVBoxLayout()}, factory_{factory} {

    QSizePolicy sp(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
    sp.setVerticalStretch(0);
    sp.setHorizontalStretch(1);
    QWidget::setSizePolicy(sp);

    setLayout(listLayout_);

    const auto space = utilqt::refSpacePx(this);
    listLayout_->setAlignment(Qt::AlignTop);
#ifdef __APPLE__
    // Add some space for the scrollbar on mac
    listLayout_->setContentsMargins(0, space, 10, space);
#else
    listLayout_->setContentsMargins(0, space, 0, space);
#endif
    listLayout_->setSpacing(space);
    listLayout_->setSizeConstraint(QLayout::SetMinAndMaxSize);
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
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void PropertyListFrame::add(Processor* processor) {
    setUpdatesEnabled(false);
    listLayout_->setEnabled(false);
    util::OnScopeExit enableUpdates([&]() {
        listLayout_->setEnabled(true);
        setUpdatesEnabled(true);
    });

    if (auto widget = get(processor)) {
        widget->show();
    }
}

void PropertyListFrame::hide(Processor* processor) {
    auto it = processorMap_.find(processor);
    if (it != processorMap_.end()) it->second->hide();
}

void PropertyListFrame::remove(Processor* processor) {
    auto it = processorMap_.find(processor);
    if (it != processorMap_.end()) {
        it->second->hide();
        listLayout_->removeWidget(it->second);
        delete it->second;
        processorMap_.erase(it);
    }
}

void PropertyListFrame::add(Property* property) {
    setUpdatesEnabled(false);
    listLayout_->setEnabled(false);
    util::OnScopeExit enableUpdates([&]() {
        listLayout_->setEnabled(true);
        setUpdatesEnabled(true);
    });

    if (auto widget = get(property)) {
        widget->show();
    }
}
void PropertyListFrame::hide(Property* property) {
    auto it = propertyMap_.find(property);
    if (it != propertyMap_.end()) it->second->hide();
}
void PropertyListFrame::remove(Property* property) {
    auto it = propertyMap_.find(property);
    if (it != propertyMap_.end()) {
        it->second->hide();
        listLayout_->removeWidget(it->second);
        delete it->second;
        propertyMap_.erase(it);
    }
}

void PropertyListFrame::clear() {
    for (auto&& [p, w] : processorMap_) {
        w->hide();
        listLayout_->removeWidget(w);
        delete w;
    }
    processorMap_.clear();

    for (auto&& [p, w] : propertyMap_) {
        w->hide();
        listLayout_->removeWidget(w);
        delete w;
    }
    propertyMap_.clear();
}

QWidget* PropertyListFrame::get(Processor* processor) {
    // check if processor widget has been already generated
    auto it = processorMap_.find(processor);
    if (it != processorMap_.end()) {
        return it->second;
    } else {
        return create(processor);
    }
}

QWidget* PropertyListFrame::create(Processor* processor) {
    // create property widget and store it in the map
    auto widget = new CollapsibleGroupBoxWidgetQt(processor);
    widget->hide();
    listLayout_->insertWidget(-1, widget, 0, Qt::AlignTop);
    for (auto prop : processor->getProperties()) {
        widget->addProperty(prop);
    }
    processorMap_[processor] = widget;
    processor->getNetwork()->addObserver(this);
    return widget;
}

QWidget* PropertyListFrame::get(Property* property) {
    // check if processor widget has been already generated
    auto it = propertyMap_.find(property);
    if (it != propertyMap_.end()) {
        return it->second;
    } else {
        return create(property);
    }
}
QWidget* PropertyListFrame::create(Property* property) {
    // create property widget and store it in the map
    if (auto widget = static_cast<PropertyWidgetQt*>(factory_->create(property).release())) {
        widget->hide();
        listLayout_->insertWidget(-1, widget, 0, Qt::AlignTop);
        widget->initState();
        propertyMap_[property] = widget;
        property->getOwner()->addObserver(this);
        RenderContext::getPtr()->activateDefaultRenderContext();
        return widget;
    }
    return nullptr;
}

void PropertyListFrame::onProcessorNetworkWillRemoveProcessor(Processor* processor) {
    remove(processor);
}
void PropertyListFrame::onWillRemoveProperty(Property* property, size_t) { remove(property); }

QWidget* PropertyListFrame::findWidgetFor(Property* property) {
    if (auto it = propertyMap_.find(property); it != propertyMap_.end()) {
        return it->second;
    }

    std::vector<PropertyOwner*> stack{};
    QWidget* widget = nullptr;
    auto owner = property->getOwner();
    while (owner) {
        if (auto* processor = dynamic_cast<Processor*>(owner)) {
            if (auto it = processorMap_.find(processor); it != processorMap_.end()) {
                widget = it->second;
                break;
            }
        } else if (auto* cp = dynamic_cast<CompositeProperty*>(owner)) {
            if (auto it = propertyMap_.find(cp); it != propertyMap_.end()) {
                widget = it->second;
                break;
            }
        }
        stack.push_back(owner);
        owner = owner->getOwner();
    }

    if (!widget) return nullptr;

    while (!stack.empty()) {
        owner = stack.back();
        stack.pop_back();
        if (auto* cp = dynamic_cast<CompositeProperty*>(owner)) {
            if (auto* cw = dynamic_cast<CollapsibleGroupBoxWidgetQt*>(widget)) {
                if (auto pw = cw->widgetForProperty(cp)) {
                    widget = pw;
                    continue;
                }
            }
        }
        return nullptr;
    }

    if (auto* cw = dynamic_cast<CollapsibleGroupBoxWidgetQt*>(widget)) {
        if (auto pw = cw->widgetForProperty(property)) {
            return pw;
        }
    }
    return nullptr;
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

    frame_ = new PropertyListFrame(this, app->getPropertyWidgetFactory());
    scrollArea_->setWidget(frame_);
    setWidget(scrollArea_);
}

PropertyListWidget::~PropertyListWidget() = default;

void PropertyListWidget::addProcessorProperties(Processor* processor) {
    frame_->add(processor);
    QWidget::raise();  // Put this tab in front
}

void PropertyListWidget::removeProcessorProperties(Processor* processor) {
    frame_->hide(processor);
}

void PropertyListWidget::removeAndDeleteProcessorProperties(Processor* processor) {
    frame_->remove(processor);
}

void PropertyListWidget::addPropertyWidgets(Property* property) {
    frame_->add(property);
    QWidget::raise();  // Put this tab in front
}
void PropertyListWidget::removePropertyWidgets(Property* property) { frame_->hide(property); }
void PropertyListWidget::removeAndDeletePropertyWidgets(Property* property) {
    frame_->remove(property);
}

bool PropertyListWidget::event(QEvent* e) {
    // The network editor will post these events.
    if (e->type() == PropertyListEvent::type()) {
        PropertyListEvent* ple = static_cast<PropertyListEvent*>(e);
        ple->accept();

        auto network = app_->getProcessorNetwork();

        switch (ple->action) {
            case PropertyListEvent::Action::Add: {
                if (auto processor = network->getProcessorByIdentifier(ple->identifier)) {
                    addProcessorProperties(processor);
                }
                break;
            }
            case PropertyListEvent::Action::Remove: {
                if (auto processor = network->getProcessorByIdentifier(ple->identifier)) {
                    removeProcessorProperties(processor);
                }
                break;
            }
            case PropertyListEvent::Action::FocusProperty: {
                if (auto property = network->getProperty(ple->identifier)) {
                    focusProperty(property);
                }
                break;
            }
        }
        return true;

    } else {
        return InviwoDockWidget::event(e);
    }
}

PropertyListEvent::PropertyListEvent(Action aAction, std::string aIdentifier)
    : QEvent(PropertyListEvent::type()), action(aAction), identifier(aIdentifier) {}

QEvent::Type PropertyListEvent::type() {
    if (PropertyListEventType == QEvent::None) {
        PropertyListEventType = static_cast<QEvent::Type>(QEvent::registerEventType());
    }
    return PropertyListEventType;
}

void PropertyListWidget::focusProperty(Property* property) {
    if (auto widget = frame_->findWidgetFor(property)) {
        widget->setFocus();
    }
}

QEvent::Type PropertyListEvent::PropertyListEventType = QEvent::None;

}  // namespace inviwo
