/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
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

#ifndef IVW_TIMERQT_H
#define IVW_TIMERQT_H

#include <inviwo/qt/widgets/inviwoqtwidgetsdefine.h>
#include <inviwo/core/util/timer.h>
#include <QBasicTimer>
#include <QObject>

namespace inviwo {

//class IVW_QTWIDGETS_API TimerQt: public QTimer, public Timer {
//    Q_OBJECT
//public:
//    TimerQt(): QTimer(), Timer() {};
//    virtual ~TimerQt() { stop();}
//
//    virtual void start(unsigned int intervalInMilliseconds, bool once = false) {
//        if(isActive())
//            return;
//
//        setSingleShot(once);
//
//        connect(this, SIGNAL(timeout()), this, SLOT(onIntervalEventSlot()));
//
//        QTimer::start(intervalInMilliseconds);
//    };
//
//    virtual void stop() {
//        QTimer::stop();
//    };
//
//public slots:
//    void onIntervalEventSlot() const {
//        Timer::onIntervalEvent();
//    }
//protected:
//
//
//};

class IVW_QTWIDGETS_API TimerQt: public QObject, public Timer {
    Q_OBJECT
public:
    TimerQt(): QObject(), Timer() {};
    virtual ~TimerQt() { stop();}

    virtual void start(unsigned int intervalInMilliseconds, bool once = false) {
        if (timer_.isActive())
            return;

        once_ = once;
        timer_.start(intervalInMilliseconds, this);
    };

    virtual void stop() {
        timer_.stop();
    };

protected:
    void timerEvent(QTimerEvent* event) {
        onIntervalEvent();

        if (once_)
            stop();
    }
    QBasicTimer timer_;
    bool once_;


};

} // namespace inviwo

#endif // IVW_TIMERQT_H