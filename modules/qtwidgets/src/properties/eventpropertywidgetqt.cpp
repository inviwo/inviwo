/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2025 Inviwo Foundation
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

#include <modules/qtwidgets/properties/eventpropertywidgetqt.h>

#include <inviwo/core/interaction/events/eventmatcher.h>    // for EventMatcher, KeyboardEventMa...
#include <inviwo/core/interaction/events/keyboardkeys.h>    // for operator<<, KeyModifiers, IvwKey
#include <inviwo/core/interaction/events/mousebuttons.h>    // for MouseButton, MouseButton::None
#include <inviwo/core/properties/eventproperty.h>           // for EventProperty
#include <modules/qtwidgets/editablelabelqt.h>              // for EditableLabelQt
#include <modules/qtwidgets/eventconverterqt.h>             // for getModifiers, getKeyButton
#include <modules/qtwidgets/inviwoqtutils.h>                // for toQString
#include <modules/qtwidgets/inviwowidgetsqt.h>              // for IvwPushButton
#include <modules/qtwidgets/properties/propertywidgetqt.h>  // for PropertyWidgetQt

#include <ostream>  // for stringstream, operator<<, ost...

#include <QGridLayout>    // for QGridLayout
#include <QHBoxLayout>    // for QHBoxLayout
#include <QKeyEvent>      // for QKeyEvent
#include <QMouseEvent>    // for QMouseEvent
#include <QSizePolicy>    // for QSizePolicy
#include <QString>        // for QString
#include <QWidget>        // for QWidget
#include <Qt>             // for Key_Enter, Key_Escape, Key_Re...
#include <flags/flags.h>  // for operator!=, none
#include <QMenu>

class QHBoxLayout;

namespace inviwo {

EventPropertyWidgetQt::EventPropertyWidgetQt(EventProperty* eventproperty)
    : PropertyWidgetQt(eventproperty)
    , eventproperty_(eventproperty)
    , button_{new IvwPushButton(this)}
    , label_{new EditableLabelQt(this, eventproperty_)} {

    setFocusPolicy(button_->focusPolicy());
    setFocusProxy(button_);

    QHBoxLayout* hLayout = new QHBoxLayout();
    setSpacingAndMargins(hLayout);

    connect(button_, &IvwPushButton::clicked, this, &EventPropertyWidgetQt::clickedSlot);
    hLayout->addWidget(label_);

    {
        QWidget* widget = new QWidget(this);
        QSizePolicy sliderPol = widget->sizePolicy();
        sliderPol.setHorizontalStretch(3);
        widget->setSizePolicy(sliderPol);
        QGridLayout* vLayout = new QGridLayout();
        widget->setLayout(vLayout);
        vLayout->setContentsMargins(0, 0, 0, 0);
        vLayout->setSpacing(0);

        vLayout->addWidget(button_);
        hLayout->addWidget(widget);
    }

    setLayout(hLayout);

    setButtonText();
}

EventPropertyWidgetQt::~EventPropertyWidgetQt() = default;

void EventPropertyWidgetQt::updateFromProperty() { setButtonText(); }

void EventPropertyWidgetQt::clickedSlot() {
    matcher_ = std::unique_ptr<EventMatcher>(eventproperty_->getEventMatcher()->clone());

    keyMatcher_ = dynamic_cast<KeyboardEventMatcher*>(matcher_.get());
    mouseMatcher_ = dynamic_cast<MouseEventMatcher*>(matcher_.get());

    if (keyMatcher_) {
        keyMatcher_->setModifiers(KeyModifiers(flags::none));
        keyMatcher_->setKey(IvwKey::Unknown);
        grabKeyboard();
    } else if (mouseMatcher_) {
        mouseMatcher_->setModifiers(KeyModifiers(flags::none));
        mouseMatcher_->setButtons(MouseButton::None);
        grabMouse();
    } else {
        return;
    }

    button_->setText("Press a button");
    button_->setEnabled(false);
    setFocus(Qt::MouseFocusReason);
}

void EventPropertyWidgetQt::keyPressEvent(QKeyEvent* event) {
    if (keyMatcher_ && event->key() != Qt::Key_Enter && event->key() != Qt::Key_Return &&
        event->key() != Qt::Key_Escape) {
        auto key = utilqt::getKeyButton(event);
        auto modifers = utilqt::getModifiers(event);

        keyMatcher_->setKey(key);
        keyMatcher_->setModifiers(modifers);

        std::stringstream ss;
        if (keyMatcher_->modifiers() != KeyModifier::None) {
            ss << keyMatcher_->modifiers() << "+";
        }
        ss << keyMatcher_->key();

        button_->setText(QString::fromStdString(ss.str()));
    }

    QWidget::keyPressEvent(event);
}

void EventPropertyWidgetQt::keyReleaseEvent(QKeyEvent* event) {
    if (keyMatcher_ && (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)) {
        releaseKeyboard();
        eventproperty_->setEventMatcher(std::unique_ptr<EventMatcher>(keyMatcher_->clone()));
        setButtonText();
        button_->setEnabled(true);
    } else if (keyMatcher_ && event->key() == Qt::Key_Escape) {
        releaseKeyboard();
        setButtonText();
        button_->setEnabled(true);
    } else {
        QWidget::keyReleaseEvent(event);
    }
}

void EventPropertyWidgetQt::setButtonText() {
    button_->setText(utilqt::toQString(eventproperty_->getEventMatcher()->displayString()));
}

void EventPropertyWidgetQt::focusOutEvent(QFocusEvent*) {
    releaseKeyboard();
    setButtonText();
    button_->setEnabled(true);
}

void EventPropertyWidgetQt::mousePressEvent(QMouseEvent* event) {
    if (mouseMatcher_) {
        auto modifers = utilqt::getModifiers(event);

        mouseMatcher_->setButtons(utilqt::getMouseButtons(event));
        mouseMatcher_->setModifiers(modifers);

        eventproperty_->setEventMatcher(std::unique_ptr<EventMatcher>(mouseMatcher_->clone()));
    }

    setButtonText();
    button_->setEnabled(true);
    releaseMouse();
}

std::unique_ptr<QMenu> EventPropertyWidgetQt::getContextMenu() {
    auto menu = PropertyWidgetQt::getContextMenu();
    auto* unsetAction = menu->addAction(tr("&Unset shortcut"));
    unsetAction->setToolTip(tr("Remove the currently set shortcut."));

    QObject::connect(unsetAction, &QAction::triggered, this, [this]() {
        if (!matcher_) {
            matcher_ = std::unique_ptr<EventMatcher>(eventproperty_->getEventMatcher()->clone());
        }
        keyMatcher_ = dynamic_cast<KeyboardEventMatcher*>(matcher_.get());
        mouseMatcher_ = dynamic_cast<MouseEventMatcher*>(matcher_.get());
        if (keyMatcher_) {
            keyMatcher_->setKey(IvwKey::Undefined);
            keyMatcher_->setModifiers(KeyModifier::None);
            eventproperty_->setEventMatcher(std::unique_ptr<EventMatcher>(keyMatcher_->clone()));
        } else if (mouseMatcher_) {
            mouseMatcher_->setButtons(MouseButton::None);
            mouseMatcher_->setModifiers(KeyModifier::None);
            eventproperty_->setEventMatcher(std::unique_ptr<EventMatcher>(mouseMatcher_->clone()));
        }
        setButtonText();
    });
    return menu;
}

}  // namespace inviwo
