/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016 Inviwo Foundation
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

#include <inviwo/qt/editor/globaleventfilter.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QAction>
#include <QEvent>
#include <QApplication>
#include <QGuiApplication>
#include <QMouseEvent>
#include <QTouchEvent>
#include <warn/pop>

namespace inviwo {

GlobalEventFilter::GlobalEventFilter(InteractionStateManager &manager) : manager_(manager) {}

#include <warn/push>
#include <warn/ignore/switch-enum>

bool GlobalEventFilter::eventFilter(QObject *obj, QEvent *event) {
    switch (event->type()) {
        case QEvent::MouseButtonPress: {
            manager_.beginInteraction();
            break;
        }
        case QEvent::MouseButtonRelease: {
            auto me = static_cast<QMouseEvent *>(event);
            if (me->buttons() == Qt::NoButton) {
                manager_.endInteraction();
            }
            break;
        }
        case QEvent::TouchBegin: {
            manager_.beginInteraction();
            break;
        }
        case QEvent::TouchEnd: {
            auto te = static_cast<QTouchEvent *>(event);
            if (util::all_of(te->touchPoints(), [](const QTouchEvent::TouchPoint &tp) {
                    return tp.state() == Qt::TouchPointReleased;
                })) {
                manager_.endInteraction();
            }
            break;
        }
        default:
            break;
    }
    return QObject::eventFilter(obj, event);
}

#include <warn/pop>

} // namespace

