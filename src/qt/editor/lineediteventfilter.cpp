/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2024 Inviwo Foundation
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

#include <inviwo/qt/editor/lineediteventfilter.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QKeyEvent>
#include <QCoreApplication>
#include <QLineEdit>
#include <warn/pop>

namespace inviwo {

LineEditEventFilter::LineEditEventFilter(QWidget* w, QLineEdit* parent, bool focusAndDown)
    : QObject(parent), widget_(w), focusAndDown_(focusAndDown) {}

bool LineEditEventFilter::eventFilter(QObject* obj, QEvent* e) {
    if (widget_->isHidden()) QObject::eventFilter(obj, e);

    switch (e->type()) {
        case QEvent::KeyPress: {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(e);
            switch (keyEvent->key()) {
                case Qt::Key_Down: {
                    // set focus to the tree widget
                    widget_->setFocus(Qt::ShortcutFocusReason);
                    if (focusAndDown_) {
                        // move cursor down one row
                        QKeyEvent* keydown =
                            new QKeyEvent(QKeyEvent::KeyPress, Qt::Key_Down, Qt::NoModifier);
                        QCoreApplication::postEvent(widget_, keydown);
                    }
                    break;
                }
                case Qt::Key_Escape:
                    if (auto w = dynamic_cast<QLineEdit*>(parent())) {
                        w->clear();
                    }
                    break;
                default:
                    break;
            }
            break;
        }
        default:
            break;
    }
    return QObject::eventFilter(obj, e);
}

}  // namespace inviwo
