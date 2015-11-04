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

#ifndef IVW_INVIWOAPPLICATIONQT_H
#define IVW_INVIWOAPPLICATIONQT_H

#include <inviwo/qt/widgets/inviwoqtwidgetsdefine.h>
#include <warn/push>
#include <warn/ignore/all>
#include <QApplication>
#include <QMainWindow>
#include <QFileSystemWatcher>
#include <QEvent>
#include <warn/pop>

#include <inviwo/core/common/inviwoapplication.h>

namespace inviwo {

class IVW_QTWIDGETS_API InviwoQtEvent : public QEvent {
    Q_GADGET
public:
    InviwoQtEvent() : QEvent(INVIWO_QT_EVENT) {}
    static QEvent::Type type() {
        if (INVIWO_QT_EVENT == QEvent::None) {
            INVIWO_QT_EVENT = static_cast<QEvent::Type>(QEvent::registerEventType());
        }
        return INVIWO_QT_EVENT;
    }

private:
    static QEvent::Type INVIWO_QT_EVENT;
};

class IVW_QTWIDGETS_API InviwoApplicationQt : public QApplication, public InviwoApplication {
    #include <warn/push>
    #include <warn/ignore/all>
    Q_OBJECT
    #include <warn/pop>

public:
    InviwoApplicationQt(std::string displayName_, std::string basePath_, int& argc, char** argv, bool movePointsOn = true);
    virtual ~InviwoApplicationQt();

    virtual void initialize(registerModuleFuncPtr) override;
    
    virtual void registerFileObserver(FileObserver* fileObserver) override;
    virtual void startFileObservation(std::string fileName) override;
    virtual void stopFileObservation(std::string fileName) override;
    virtual void closeInviwoApplication() override;
    virtual void playSound(Message soundID) override;
    virtual std::locale getUILocale() const override;

    void setMainWindow(QMainWindow* mainWindow);
    QMainWindow* getMainWindow() { return mainWindow_; }

    QPoint getWindowDecorationOffset() const;
    void setWindowDecorationOffset(QPoint windowDecorationOffset);

    QPoint movePointOntoDesktop(const QPoint& point, const QSize& size, bool decorationOffset=true);
    QPoint offsetWidget();

    virtual bool event(QEvent* e) override;

public slots:
    void fileChanged(QString fileName);

protected:
    void wait(int);

private:
    #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    static void logQtMessages(QtMsgType type, const QMessageLogContext& context,
                              const QString& msg);
    #endif

    bool movePointsOn_;
    QMainWindow* mainWindow_;
    std::vector<FileObserver*> fileObservers_;
    QFileSystemWatcher* fileWatcher_;

    // Only non zero on windows, due to a QT bug in window decoration handling.
    QPoint windowDecorationOffset_;
};

}  // namespace

#endif // IVW_INVIWOAPPLICATIONQT_H
