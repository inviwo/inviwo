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

#include <inviwo/qt/widgets/inviwoapplicationqt.h>
#include <inviwo/core/util/settings/systemsettings.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/fileobserver.h>
#include <inviwo/qt/widgets/qtwidgetmodule.h>
#include <inviwo/qt/widgets/inviwoqtutils.h>

#include <thread>

#include <warn/push>
#include <warn/ignore/all>
#include <QApplication>
#include <QDesktopWidget>
#include <QFile>
#include <warn/pop>

namespace inviwo {

InviwoApplicationQt::InviwoApplicationQt(std::string displayName, int& argc,
                                         char** argv, bool movePointsOn)
    : QApplication(argc, argv)
    , InviwoApplication(argc, argv, displayName)
    , movePointsOn_(movePointsOn)
    , mainWindow_(nullptr)
    , windowDecorationOffset_(0, 0) {
    QCoreApplication::setOrganizationName("Inviwo Foundation");
    QCoreApplication::setOrganizationDomain("inviwo.org");
    QCoreApplication::setApplicationName(displayName.c_str());

    setPostEnqueueFront([this]() { postEvent(this, new InviwoQtEvent()); });

    fileWatcher_ = new QFileSystemWatcher(this);
    connect(fileWatcher_, SIGNAL(fileChanged(QString)), this, SLOT(fileChanged(QString)));

    // Make qt write errors in the console;
    qInstallMessageHandler(&InviwoApplicationQt::logQtMessages);
}

InviwoApplicationQt::~InviwoApplicationQt() {}

void InviwoApplicationQt::setMainWindow(QMainWindow* mainWindow) { mainWindow_ = mainWindow; }

void InviwoApplicationQt::registerFileObserver(FileObserver* fileObserver) {
    ivwAssert(std::find(fileObservers_.begin(), fileObservers_.end(), fileObserver) ==
                  fileObservers_.end(),
              "File observer already registered.");
    fileObservers_.push_back(fileObserver);
}

void InviwoApplicationQt::unRegisterFileObserver(FileObserver* fileObserver) {
    util::erase_remove(fileObservers_, fileObserver);
}

void InviwoApplicationQt::startFileObservation(std::string fileName) {
    QString qFileName = QString::fromStdString(fileName);

    if (!fileWatcher_->files().contains(qFileName)) fileWatcher_->addPath(qFileName);
}

void InviwoApplicationQt::stopFileObservation(std::string fileName) {
    QString qFileName = QString::fromStdString(fileName);

    if (fileWatcher_->files().contains(qFileName)) fileWatcher_->removePath(qFileName);
}

void InviwoApplicationQt::fileChanged(QString fileName) {
    wait(200);

    if (QFile::exists(fileName)) {
        std::string fileNameStd = fileName.toLocal8Bit().constData();

        // don't use iterators here, they might be invalidated.
        size_t size = fileObservers_.size();
        for (size_t i = 0; i < size && i < fileObservers_.size(); ++i) {
            if (fileObservers_[i]->isObserved(fileNameStd)) {
                fileObservers_[i]->fileChanged(fileNameStd);
            }
        }

        if (!fileWatcher_->files().contains(fileName)) {
            fileWatcher_->addPath(fileName);
        }
    }
}

void InviwoApplicationQt::closeInviwoApplication() { QCoreApplication::quit(); }

void InviwoApplicationQt::playSound(Message message) {}

std::locale InviwoApplicationQt::getUILocale() const { return utilqt::getCurrentStdLocale(); }

void InviwoApplicationQt::printApplicationInfo() {
    InviwoApplication::printApplicationInfo();
    LogInfoCustom("InviwoInfo", "Qt Version " << QT_VERSION_STR);
}

void InviwoApplicationQt::registerModules(RegisterModuleFunc regModuleFunc) {
    auto func = [&]() {
        auto modules = regModuleFunc();

        // Since QtWidgets are not a module we have to register it our self
        modules.emplace_back(new InviwoModuleFactoryObjectTemplate<QtWidgetModule>(
            "QtWidgetModule", "Module with Qt implementation of all propertywidgets etc.", {}));

        return modules;
    };

    InviwoApplication::registerModules(func);
}

void InviwoApplicationQt::wait(int ms) {
    if (ms <= 0) return;
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}


void InviwoApplicationQt::logQtMessages(QtMsgType type, const QMessageLogContext& context,
                                        const QString& msg) {
#ifdef IVW_DEBUG
    QByteArray localMsg = msg.toLocal8Bit();
    switch (type) {
        case QtDebugMsg:
            inviwo::LogCentral::getPtr()->log("Qt Debug", LogLevel::Info, LogAudience::Developer,
                                              context.file, context.function, context.line,
                                              msg.toUtf8().constData());

            fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file,
                    context.line, context.function);
            break;
        case QtWarningMsg:
            inviwo::LogCentral::getPtr()->log("Qt Warning", LogLevel::Info, LogAudience::Developer,
                                              context.file, context.function, context.line,
                                              msg.toUtf8().constData());

            fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file,
                    context.line, context.function);
            break;
        case QtCriticalMsg:
            inviwo::LogCentral::getPtr()->log("Qt Critical", LogLevel::Info, LogAudience::Developer,
                                              context.file, context.function, context.line,
                                              msg.toUtf8().constData());

            fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file,
                    context.line, context.function);
            break;
        case QtFatalMsg:
            inviwo::LogCentral::getPtr()->log("Qt Fatal", LogLevel::Info, LogAudience::Developer,
                                              context.file, context.function, context.line,
                                              msg.toUtf8().constData());

            fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file,
                    context.line, context.function);
            abort();
            break;

        #if (QT_VERSION >= QT_VERSION_CHECK(5, 5, 0))
        case QtInfoMsg:
            inviwo::LogCentral::getPtr()->log("Qt Info", LogLevel::Info, LogAudience::Developer,
                                              context.file, context.function, context.line,
                                              msg.toUtf8().constData());

            fprintf(stderr, "Info: %s (%s:%u, %s)\n", localMsg.constData(), context.file,
                    context.line, context.function);
            break;
        #endif
    }
#endif
}


QPoint InviwoApplicationQt::getWindowDecorationOffset() const { return windowDecorationOffset_; }

void InviwoApplicationQt::setWindowDecorationOffset(QPoint windowDecorationOffset) {
    windowDecorationOffset_ = windowDecorationOffset;
}

QPoint InviwoApplicationQt::movePointOntoDesktop(const QPoint& point, const QSize& size,
                                                 bool decorationOffset) {
    QPoint pos(point);
    if (movePointsOn_) {
        QDesktopWidget* desktop = QApplication::desktop();
        int primaryScreenIndex = desktop->primaryScreen();
        QRect wholeScreen = desktop->screenGeometry(primaryScreenIndex);

        for (int i = 0; i < desktop->screenCount(); i++) {
            if (i != primaryScreenIndex)
                wholeScreen = wholeScreen.united(desktop->screenGeometry(i));
        }

        wholeScreen.setRect(wholeScreen.x() - 10, wholeScreen.y() - 10, wholeScreen.width() + 20,
                            wholeScreen.height() + 20);
        QPoint bottomRight = QPoint(point.x() + size.width(), point.y() + size.height());
        QPoint appPos = getMainWindow()->pos();

        if (decorationOffset) {
            QPoint offset = getWindowDecorationOffset();
            pos -= offset;
        }

        if (!wholeScreen.contains(pos) || !wholeScreen.contains(bottomRight)) {
            // If the widget is outside visible screen
            pos = appPos;
            pos += offsetWidget();
        }
    }
    return pos;
}

QPoint InviwoApplicationQt::offsetWidget() {
    static int offsetCounter = 0;
    static ivec2 baseOffset(350, 100);

    ivec2 pos(0, 0);
    pos += baseOffset + ivec2(40 * offsetCounter++);

    if (offsetCounter == 10) {  // reset offset
        offsetCounter = 0;
        baseOffset.x += 200;
        if (baseOffset.x >= 800) {
            baseOffset.x = 350;
            baseOffset.y += 100;
            if (baseOffset.y >= 800) {
                baseOffset.y = 100;
            }
        }
    }
    return QPoint(pos.x, pos.y);
}

bool InviwoApplicationQt::event(QEvent* e) {
    if (e->type() == InviwoQtEvent::type()) {
        e->accept();
        processFront();
        return true;
    } else {
        return QApplication::event(e);
    }
}

QEvent::Type InviwoQtEvent::InviwoQtEventType = QEvent::None;

}  // namespace
