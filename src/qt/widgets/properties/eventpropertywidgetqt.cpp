/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#include <inviwo/qt/widgets/properties/eventpropertywidgetqt.h>
#include <inviwo/core/properties/eventproperty.h>
#include <inviwo/qt/widgets/editablelabelqt.h>
#include <inviwo/qt/widgets/eventconverterqt.h>

#include <inviwo/core/interaction/events/interactionevent.h>
#include <inviwo/core/interaction/events/mouseevent.h>
#include <inviwo/core/interaction/events/keyboardevent.h>

#include <QPushButton>

namespace inviwo {

EventPropertyWidgetQt::EventPropertyWidgetQt(EventProperty* eventproperty)
    : PropertyWidgetQt(eventproperty)
    , eventproperty_(eventproperty)
    , tmpEvent_(nullptr)
    , keyevent_(nullptr)
    , mouseEvent_(nullptr) {
    generateWidget();
}

void inviwo::EventPropertyWidgetQt::generateWidget() {
    QHBoxLayout* hLayout = new QHBoxLayout();
    setSpacingAndMargins(hLayout);
    
    label_ = new EditableLabelQt(this, eventproperty_);

    button_ = new IvwPushButton(this);
    connect(button_, SIGNAL(clicked()), this, SLOT(clickedSlot()));
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

void EventPropertyWidgetQt::updateFromProperty() { setButtonText(); }

void EventPropertyWidgetQt::clickedSlot() {
    
    if(tmpEvent_) delete tmpEvent_;
    tmpEvent_ = eventproperty_->getEvent()->clone();
    tmpEvent_->setModifiers(0);

    keyevent_ = dynamic_cast<KeyboardEvent*>(tmpEvent_);
    mouseEvent_ = dynamic_cast<MouseEvent*>(tmpEvent_);

    if (keyevent_) {
        keyevent_->setButton(0);
        grabKeyboard();
    } else if(mouseEvent_) { 
        mouseEvent_->setButton(0);
        grabMouse();
    } else {
        return;
    }

    button_->setText("Press a button");
    button_->setEnabled(false);
    setFocus(Qt::MouseFocusReason);
}

void EventPropertyWidgetQt::keyPressEvent(QKeyEvent* event) {
    if (keyevent_ && event->key() != Qt::Key_Enter && event->key() != Qt::Key_Return &&
        event->key() != Qt::Key_Escape) {
        int key = EventConverterQt::getKeyButton(event);
        int modifer = EventConverterQt::getModifier(event);

        keyevent_->setButton(key);
        keyevent_->setModifiers(keyevent_->modifiers() | modifer);

        std::string text = keyevent_->modifierNames();
        if (text != "") text.append("-");
        text += std::string(1, static_cast<char>(keyevent_->button()));

        button_->setText(QString::fromStdString(text));
    }

    QWidget::keyPressEvent(event);
}

void EventPropertyWidgetQt::keyReleaseEvent(QKeyEvent* event) {
    if (keyevent_ && (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)) {
        releaseKeyboard();
        eventproperty_->setEvent(keyevent_->clone());
        setButtonText();
        button_->setEnabled(true);
    } else if (keyevent_ && event->key() == Qt::Key_Escape) {
        releaseKeyboard();
        setButtonText();
        button_->setEnabled(true);  
    } else {
        QWidget::keyReleaseEvent(event);
    }
}

void EventPropertyWidgetQt::setButtonText() {
    std::string text = eventproperty_->getEvent()->modifierNames();

    if (text != "") text.append("+");

    KeyboardEvent* keyboardEvent = dynamic_cast<KeyboardEvent*>(eventproperty_->getEvent());
    if (keyboardEvent) {
        text += std::string(1, static_cast<char>(keyboardEvent->button()));
    }

    MouseEvent* mouseEvent = dynamic_cast<MouseEvent*>(eventproperty_->getEvent());
    if (mouseEvent) {
        text += mouseEvent->buttonName();
    }

    button_->setText(QString::fromStdString(text));
}

void EventPropertyWidgetQt::focusOutEvent(QFocusEvent* event) {
    releaseKeyboard();
    setButtonText();
    button_->setEnabled(true);
}

void EventPropertyWidgetQt::mousePressEvent(QMouseEvent* event) {  
    if(mouseEvent_) {
        int modifer = EventConverterQt::getModifier(event);
        mouseEvent_->setButton(EventConverterQt::getMouseButton(event));
        mouseEvent_->setModifiers(mouseEvent_->modifiers() | modifer);

        eventproperty_->setEvent(mouseEvent_->clone());
    }

    setButtonText();
    button_->setEnabled(true);
    releaseMouse();
}

}  // namespace
