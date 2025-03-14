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
#include <QApplication>

class QHBoxLayout;

namespace inviwo {

EventPropertyWidgetQt::EventPropertyWidgetQt(EventProperty* eventproperty)
    : PropertyWidgetQt(eventproperty)
    , eventProperty_(eventproperty)
    , button_{new IvwPushButton(this)}
    , label_{new EditableLabelQt(this, eventProperty_)} {

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
    auto* em = eventProperty_->getEventMatcher();
    if (!em) return;

    matcher_ = std::unique_ptr<EventMatcher>(em->clone());
    keyMatcher_ = dynamic_cast<KeyboardEventMatcher*>(matcher_.get());
    mouseMatcher_ = dynamic_cast<MouseEventMatcher*>(matcher_.get());

    if (keyMatcher_) {
        keyMatcher_->setModifiers(KeyModifiers(flags::none));
        keyMatcher_->setKey(IvwKey::Unknown);
        grabKeyboard();
        grabMouse();
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
        keyMatcher_->setKey(utilqt::getKeyButton(event));
        keyMatcher_->setModifiers(utilqt::getModifiers(event));
        button_->setText(utilqt::toQString(keyMatcher_->displayString()));
    } else if (mouseMatcher_) {
        mouseMatcher_->setModifiers(utilqt::getModifiers(QApplication::queryKeyboardModifiers()));
        button_->setText(utilqt::toQString(mouseMatcher_->displayString()));
    } else {
        PropertyWidgetQt::keyPressEvent(event);
    }
}

void EventPropertyWidgetQt::keyReleaseEvent(QKeyEvent* event) {
    if (keyMatcher_ && (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)) {
        releaseKeyboard();
        releaseMouse();
        eventProperty_->getEventMatcher()->assign(keyMatcher_);
        setButtonText();
        button_->setEnabled(true);
        keyMatcher_ = nullptr;
        event->accept();
    } else if (keyMatcher_ && event->key() == Qt::Key_Escape) {
        releaseKeyboard();
        releaseMouse();
        setButtonText();
        button_->setEnabled(true);
        keyMatcher_ = nullptr;
        event->accept();
    } else if (mouseMatcher_) {
        mouseMatcher_->setModifiers(utilqt::getModifiers(QApplication::queryKeyboardModifiers()));
        button_->setText(utilqt::toQString(mouseMatcher_->displayString()));
    } else {
        PropertyWidgetQt::keyReleaseEvent(event);
    }
}

void EventPropertyWidgetQt::mousePressEvent(QMouseEvent* event) {
    if (keyMatcher_) {
        releaseKeyboard();
        releaseMouse();
        setButtonText();
        button_->setEnabled(true);
        keyMatcher_ = nullptr;
        PropertyWidgetQt::mousePressEvent(event);
    } else if (mouseMatcher_) {
        mouseMatcher_->setButtons(utilqt::getMouseButtons(event));
        mouseMatcher_->setModifiers(utilqt::getModifiers(event));
        button_->setText(utilqt::toQString(mouseMatcher_->displayString()));
        event->accept();
    } else {
        PropertyWidgetQt::mousePressEvent(event);
    }
}
void EventPropertyWidgetQt::mouseMoveEvent(QMouseEvent* event) {
    if (mouseMatcher_) {
        mouseMatcher_->setButtons(utilqt::getMouseButtons(event));
        mouseMatcher_->setModifiers(utilqt::getModifiers(event));
        button_->setText(utilqt::toQString(mouseMatcher_->displayString()));
    } else {
        PropertyWidgetQt::mouseMoveEvent(event);
    }
}
void EventPropertyWidgetQt::mouseReleaseEvent(QMouseEvent* event) {
    if (mouseMatcher_) {
        eventProperty_->getEventMatcher()->assign(mouseMatcher_);
        setButtonText();
        button_->setEnabled(true);
        mouseMatcher_ = nullptr;
        releaseMouse();
        event->accept();
    } else {
        PropertyWidgetQt::mouseReleaseEvent(event);
    }
}

void EventPropertyWidgetQt::focusOutEvent(QFocusEvent* event) {
    if (keyMatcher_) {
        releaseKeyboard();
        setButtonText();
        button_->setEnabled(true);
        keyMatcher_ = nullptr;
    }
    PropertyWidgetQt::focusOutEvent(event);
}

void EventPropertyWidgetQt::setButtonText() {
    if (auto* ev = eventProperty_->getEventMatcher()) {
        button_->setText(utilqt::toQString(ev->displayString()));
    } else {
        button_->setText("Unset");
    }
}

std::unique_ptr<QMenu> EventPropertyWidgetQt::getContextMenu() {
    auto menu = PropertyWidgetQt::getContextMenu();
    auto* unsetAction = menu->addAction(tr("&Unset shortcut"));
    unsetAction->setToolTip(tr("Remove the currently set shortcut."));

    QObject::connect(unsetAction, &QAction::triggered, this, [this]() {
        auto* em = eventProperty_->getEventMatcher();

        if (auto* keyMatcher = dynamic_cast<KeyboardEventMatcher*>(em)) {
            keyMatcher->setKey(IvwKey::Undefined);
            keyMatcher->setModifiers(KeyModifier::None);
        } else if (auto* mouseMatcher = dynamic_cast<MouseEventMatcher*>(em)) {
            mouseMatcher->setButtons(MouseButton::None);
            mouseMatcher->setStates(MouseState::Press);
            mouseMatcher->setModifiers(KeyModifier::None);
        }
        setButtonText();
    });
    return menu;
}

}  // namespace inviwo
