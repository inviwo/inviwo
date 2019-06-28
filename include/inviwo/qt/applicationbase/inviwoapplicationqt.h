/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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

#include <inviwo/qt/applicationbase/qtapplicationbasemoduledefine.h>
#include <warn/push>
#include <warn/ignore/all>
#include <QApplication>
#include <QMainWindow>
#include <QFileSystemWatcher>
#include <QEvent>
#include <warn/pop>

#include <inviwo/core/common/inviwoapplication.h>

namespace inviwo {

class IVW_QTAPPLICATIONBASE_API InviwoQtEvent : public QEvent {
#include <warn/push>
#include <warn/ignore/all>
    Q_GADGET
#include <warn/pop>

public:
    InviwoQtEvent() : QEvent(InviwoQtEvent::type()) {}
    static QEvent::Type type() {
        if (InviwoQtEventType == QEvent::None) {
            InviwoQtEventType = static_cast<QEvent::Type>(QEvent::registerEventType());
        }
        return InviwoQtEventType;
    }

private:
    static QEvent::Type InviwoQtEventType;
};

class IVW_QTAPPLICATIONBASE_API InviwoApplicationQt : public QApplication,
                                                      public InviwoApplication {
public:
    InviwoApplicationQt(const std::string& displayName = "Inviwo");
    InviwoApplicationQt(int& argc, char** argv, const std::string& displayName);
    virtual ~InviwoApplicationQt() = default;

    virtual void registerFileObserver(FileObserver* fileObserver) override;
    virtual void unRegisterFileObserver(FileObserver* fileObserver) override;
    virtual void startFileObservation(std::string fileName) override;
    virtual void stopFileObservation(std::string fileName) override;
    virtual void closeInviwoApplication() override;
    virtual void playSound(Message soundID) override;
    /**
     * \brief Get locale object for determining parsing and formatting of data.
     *
     * @return std::locale acquired during construction.
     */
    virtual std::locale getUILocale() const override;

    /**
     * \brief Set the main window used by the application.
     * The widget object name will be set to "InviwoMainWindow".
     * Other widgets can thereby find the main window in QApplication.
     * @see utilqt::getApplicationMainWindow()
     * @param mainWindow The main window of the application.
     */
    void setMainWindow(QMainWindow* mainWindow);
    QMainWindow* getMainWindow() { return mainWindow_; }

    virtual bool event(QEvent* e) override;

    virtual bool notify(QObject* receiver, QEvent* e) override;
    void setUndoTrigger(std::function<void()> func);
    virtual void resizePool(size_t newSize) override;
    virtual void printApplicationInfo() override;
    void setStyleSheetFile(QString file);

private:
    void fileChanged(QString fileName);

    static void logQtMessages(QtMsgType type, const QMessageLogContext& context,
                              const QString& msg);
    /**
     * \brief getCurrentStdLocale
     * This function returns the current system locale provided by Qt.
     * If the Qt application has not been initialized, the returned
     * value is the environment's default locale.
     * @note This is a duplicate of utilqt::getCurrentStdLocale
     * in the qtwidgets module. We do not want to depend on modules in external.
     * @return std::locale   Qt locale converted to std::locale
     */
    static std::locale getCurrentStdLocale();

    QMainWindow* mainWindow_;
    std::vector<FileObserver*> fileObservers_;
    QFileSystemWatcher* fileWatcher_;

    std::locale uiLocal_;
    std::function<void()> undoTrigger_ = []() {};
};

}  // namespace inviwo

#endif  // IVW_INVIWOAPPLICATIONQT_H
