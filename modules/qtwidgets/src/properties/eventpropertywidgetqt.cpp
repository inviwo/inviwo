/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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
#include <inviwo/core/properties/eventproperty.h>
#include <modules/qtwidgets/editablelabelqt.h>
#include <modules/qtwidgets/eventconverterqt.h>
#include <modules/qtwidgets/inviwowidgetsqt.h>

#include <inviwo/core/interaction/events/interactionevent.h>
#include <inviwo/core/interaction/events/mouseevent.h>
#include <inviwo/core/interaction/events/keyboardevent.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QFocusEvent>
#include <warn/pop>

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
    std::stringstream ss;

    if (auto keyMatcher = dynamic_cast<KeyboardEventMatcher*>(eventproperty_->getEventMatcher())) {
        if (keyMatcher->modifiers() != KeyModifier::None) {
            ss << keyMatcher->modifiers() << "+";
        }
        ss << keyMatcher->key();
    } else if (auto mouseMatcher =
                   dynamic_cast<MouseEventMatcher*>(eventproperty_->getEventMatcher())) {
        if (mouseMatcher->modifiers() != KeyModifier::None) {
            ss << mouseMatcher->modifiers() << "+";
        }
        ss << mouseMatcher->buttons();
    }

    button_->setText(QString::fromStdString(ss.str()));
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

}  // namespace inviwo
