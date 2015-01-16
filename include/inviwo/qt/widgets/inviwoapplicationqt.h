/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
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
#include <inviwo/qt/widgets/timerqt.h>
#include <QApplication>
#include <QMainWindow>
#include <QFileSystemWatcher>

#include <inviwo/core/common/inviwoapplication.h>

namespace inviwo {

class IVW_QTWIDGETS_API InviwoApplicationQt : public QApplication, public InviwoApplication {
    Q_OBJECT

public:
    InviwoApplicationQt(std::string displayName_, std::string basePath_, int& argc, char** argv);

    virtual ~InviwoApplicationQt();

    void setMainWindow(QMainWindow* mainWindow);
    QMainWindow* getMainWindow() { return mainWindow_; }

    virtual void registerFileObserver(FileObserver* fileObserver);
    virtual void startFileObservation(std::string fileName);
    virtual void stopFileObservation(std::string fileName);

    virtual void closeInviwoApplication();

    virtual void playSound(unsigned int soundID);

    virtual void initialize(registerModuleFuncPtr);

    virtual Timer* createTimer() const;

    QPoint getWindowDecorationOffset() const;
    void setWindowDecorationOffset(QPoint windowDecorationOffset);

    QPoint movePointOntoDesktop(const QPoint& point, const QSize& size);
    QPoint offsetWidget();

public slots:
    void fileChanged(QString fileName);

protected:
    void wait(int);

private:
    #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    static void logQtMessages(QtMsgType type, const QMessageLogContext& context,
                              const QString& msg);
    #endif

    QMainWindow* mainWindow_;
    std::vector<FileObserver*> fileObservers_;
    QFileSystemWatcher* fileWatcher_;

    // Only non zero on windows, due to a QT bug in window decoration handling.
    QPoint windowDecorationOffset_;
};

}  // namespace

#endif // IVW_INVIWOAPPLICATIONQT_H
