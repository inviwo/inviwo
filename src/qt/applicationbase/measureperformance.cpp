/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2025-2026 Inviwo Foundation
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

#include <inviwo/qt/applicationbase/measureperformance.h>

#include <inviwo/core/util/clock.h>

#include <QCoreApplication>
#include <QWindow>
#include <QMouseEvent>

namespace inviwo::utilqt {

void simulateMouseDrag(QWidget& widget, size_t count, QPoint length) {

    const auto pos1 = widget.rect().center();
    const auto gpos1 = widget.mapToGlobal(pos1);

    const auto pos2 = pos1 + length;
    const auto gpos2 = widget.mapToGlobal(pos2);

    for (size_t i = 0; i < count; ++i) {
        QMouseEvent me1(QEvent::MouseButtonPress, pos1, gpos1, Qt::LeftButton, Qt::LeftButton,
                        Qt::NoModifier);
        QMouseEvent me2(QEvent::MouseMove, pos1, gpos1, Qt::NoButton, Qt::LeftButton,
                        Qt::NoModifier);

        QCoreApplication::sendEvent(widget.window()->windowHandle(), &me1);
        QCoreApplication::sendEvent(widget.window()->windowHandle(), &me2);

        QMouseEvent me3(QEvent::MouseMove, pos2, gpos2, Qt::NoButton, Qt::LeftButton,
                        Qt::NoModifier);
        QMouseEvent me4(QEvent::MouseButtonRelease, pos2, gpos2, Qt::LeftButton, Qt::NoButton,
                        Qt::NoModifier);

        QCoreApplication::sendEvent(widget.window()->windowHandle(), &me3);
        QCoreApplication::sendEvent(widget.window()->windowHandle(), &me4);
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    }
}

std::chrono::high_resolution_clock::duration measureSimulateMouseDrag(QWidget& widget, size_t count,
                                                                      QPoint length) {
    const Clock clock{};
    utilqt::simulateMouseDrag(widget, count, length);
    return clock.getElapsedTime();
}

}  // namespace inviwo::utilqt
