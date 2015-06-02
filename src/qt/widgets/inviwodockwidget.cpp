/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#include <inviwo/qt/widgets/inviwodockwidget.h>
#include <inviwo/qt/widgets/inviwodockwidgettitlebar.h>
#include <QKeyEvent>

namespace inviwo {

InviwoDockWidget::InviwoDockWidget(QString title, QWidget* parent)
    : QDockWidget(title, parent) {
#ifdef __APPLE__
    this->setStyleSheet("QDockWidget::title {padding-left: 45px; }");
#endif

    // adding custom title bar to dock widget
    this->setTitleBarWidget(new InviwoDockWidgetTitleBar(this));

    QObject::connect(this, SIGNAL(topLevelChanged(bool)),
        this->titleBarWidget(), SLOT(floating(bool)));
}

InviwoDockWidget::~InviwoDockWidget() {}

void InviwoDockWidget::show(){
    raise();
    QDockWidget::show();
}

void InviwoDockWidget::keyPressEvent(QKeyEvent* keyEvent) {
    if (keyEvent->key() == Qt::Key_Escape && isFloating()){
        hide();
    } else {
        keyEvent->ignore();
    }
    QDockWidget::keyPressEvent(keyEvent);
}

void InviwoDockWidget::setSticky(bool sticky) {
    auto titleBar = dynamic_cast<InviwoDockWidgetTitleBar *>(this->titleBarWidget());
    if (titleBar) {
        titleBar->setSticky(sticky);
    }
}

bool InviwoDockWidget::isSticky() const {
    return (this->allowedAreas() != Qt::NoDockWidgetArea);
}

void InviwoDockWidget::setContents(QWidget *widget) {
    QWidget *oldWidget = this->widget();
    if (oldWidget) {
        this->setWidget(nullptr);
        delete oldWidget;
    }

    this->setWidget(widget);
}

void InviwoDockWidget::setContents(QLayout *layout) {
    QWidget *oldWidget = this->widget();
    if (oldWidget) {
        this->setWidget(nullptr);
        delete oldWidget;
    }

    QWidget *centralWidget = new QWidget();
    centralWidget->setLayout(layout);
    this->setWidget(centralWidget);
}

} // namespace