/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2022 Inviwo Foundation
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

#include <inviwo/qt/applicationbase/inviwoapplicationqt.h>
#include <inviwo/core/util/settings/systemsettings.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/fileobserver.h>
#include <inviwo/core/util/raiiutils.h>
#include <inviwo/core/util/filesystemobserver.h>
#include <inviwo/core/util/zip.h>

#include <thread>

#include <warn/push>
#include <warn/ignore/all>
#include <QApplication>
#include <QFile>
#include <QCursor>
#include <QMouseEvent>
#include <QTouchEvent>
#include <QEvent>
#include <QMessageBox>
#include <QFileSystemWatcher>
#include <warn/pop>

#ifdef WIN32
#include <windows.h>
#endif  // WIN32

namespace inviwo {

namespace {

// QApplication always needs some argc and argv
int dummyArgc = 1;
char** dummyArgs() {
    static std::string arg{"inviwo"};
    static std::array<char*, 1> array = {arg.data()};
    return array.data();
}

class FileSystemObserverQt : public QObject, public FileSystemObserver {
public:
    FileSystemObserverQt() : fileWatcher_{new QFileSystemWatcher(this)} {

        connect(fileWatcher_, &QFileSystemWatcher::fileChanged, this,
                &FileSystemObserverQt::fileChanged);
        connect(fileWatcher_, &QFileSystemWatcher::directoryChanged, this,
                &FileSystemObserverQt::fileChanged);
    }

    virtual void registerFileObserver(FileObserver* fileObserver) override;
    virtual void unRegisterFileObserver(FileObserver* fileObserver) override;

private:
    virtual void startFileObservation(const std::string& fileName) override;
    virtual void stopFileObservation(const std::string& fileName) override;

    void fileChanged(QString fileName);

    std::vector<FileObserver*> fileObservers_;
    QFileSystemWatcher* fileWatcher_;
};

void FileSystemObserverQt::registerFileObserver(FileObserver* fileObserver) {
    IVW_ASSERT(std::find(fileObservers_.cbegin(), fileObservers_.cend(), fileObserver) ==
                   fileObservers_.cend(),
               "File observer already registered.");
    fileObservers_.push_back(fileObserver);
}

void FileSystemObserverQt::unRegisterFileObserver(FileObserver* fileObserver) {
    util::erase_remove(fileObservers_, fileObserver);
}

void FileSystemObserverQt::startFileObservation(const std::string& fileName) {
    QString qFileName = QString::fromStdString(fileName);
    // Will add the path if file exists and is not already being watched.
    fileWatcher_->addPath(qFileName);
}

void FileSystemObserverQt::stopFileObservation(const std::string& fileName) {
    auto it =
        std::find_if(std::begin(fileObservers_), std::end(fileObservers_),
                     [fileName](const auto observer) { return observer->isObserved(fileName); });
    // Make sure that no observer is observing the file
    if (it == std::end(fileObservers_)) fileWatcher_->removePath(QString::fromStdString(fileName));
}

void FileSystemObserverQt::fileChanged(QString fileName) {
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

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

}  // namespace

InviwoApplicationQt::InviwoApplicationQt(int& argc, char** argv, const std::string& displayName,
                                         const std::string_view organizationName,
                                         const std::string_view organizationDomain,
                                         const std::string_view windowIconResource)
    : QApplication(argc, argv)
    , InviwoApplication(argc, argv, displayName)
    , mainWindow_(nullptr)
    , uiLocal_(getCurrentStdLocale())
    , undoTrigger_{[]() {}} {

    setAttribute(Qt::AA_NativeWindows);
    setWindowIcon(QIcon(windowIconResource.data()));

    QCoreApplication::setOrganizationName(organizationName.data());
    QCoreApplication::setOrganizationDomain(organizationDomain.data());
    QCoreApplication::setApplicationName(displayName.c_str());

    setFileSystemObserver(std::make_unique<FileSystemObserverQt>());

    setPostEnqueueFront([this]() { postEvent(this, new InviwoQtEvent(), Qt::LowEventPriority); });

#ifdef WIN32
    // set default font since the QApplication font is not properly initialized
    // (see https://bugreports.qt.io/browse/QTBUG-22572)
    //
    // query system font and font size, then set the QApplication font (Win7: Segoe UI, 9pt)
    //
    NONCLIENTMETRICS metrics = {sizeof(NONCLIENTMETRICS)};
    SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &metrics, 0);
    int pointSize = metrics.lfMessageFont.lfHeight;
    if (pointSize < 0) {
        // height is in pixels, convert to points
        HDC hDC = GetDC(NULL);
        pointSize = MulDiv(abs(pointSize), 72, GetDeviceCaps(hDC, LOGPIXELSY));
        ReleaseDC(NULL, hDC);
    }

    setFont(QFont(QString::fromWCharArray(metrics.lfMessageFont.lfFaceName), pointSize));
#endif  // WIN32

    // Make qt write errors in the console;
    qInstallMessageHandler(&InviwoApplicationQt::logQtMessages);
}

InviwoApplicationQt::InviwoApplicationQt(const std::string& displayName,
                                         const std::string_view organizationName,
                                         const std::string_view organizationDomain,
                                         const std::string_view windowIconResource)
    : InviwoApplicationQt(dummyArgc, dummyArgs(), displayName, organizationName, organizationDomain,
                          windowIconResource) {}

InviwoApplicationQt::~InviwoApplicationQt() = default;

void InviwoApplicationQt::setMainWindow(QMainWindow* mainWindow) {
    mainWindow_ = mainWindow;
    // Enable widgets to find the main window using the object name
    mainWindow_->setObjectName("InviwoMainWindow");
}

void InviwoApplicationQt::closeInviwoApplication() { QCoreApplication::quit(); }

std::locale InviwoApplicationQt::getUILocale() const { return uiLocal_; }

void InviwoApplicationQt::printApplicationInfo() {
    InviwoApplication::printApplicationInfo();
    LogInfoCustom("InviwoInfo", "Qt Version " << QT_VERSION_STR);
}

void InviwoApplicationQt::setStyleSheetFile(QString file) {
    QFile styleSheetFile(file);
    styleSheetFile.open(QFile::ReadOnly);
    QString styleSheet = QString::fromUtf8(styleSheetFile.readAll());
    setStyleSheet(styleSheet);
    styleSheetFile.close();
}

void InviwoApplicationQt::resizePool(size_t newSize) {
    if (pool_.getSize() == newSize) return;

    auto start = std::chrono::system_clock::now();
    std::chrono::milliseconds timelimit(250);
    auto timeout = [&timelimit, &start]() {
        return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() -
                                                                start) > timelimit;
    };

    // try resizing for timelimit time, run processFront to avoid potential deadlocks.
    size_t size = pool_.trySetSize(newSize);
    while (size != newSize && !timeout()) {
        processFront();
        size = pool_.trySetSize(newSize);
    }
    if (size == newSize) return;

    // if not done yet, continue trying, but block interaction and keep the GUI running by also
    // calling processEvents
    auto enabled = mainWindow_->isEnabled();
    util::OnScopeExit cleanup{[this, enabled]() {
        mainWindow_->setEnabled(enabled);
        QApplication::restoreOverrideCursor();
    }};
    mainWindow_->setEnabled(false);
    QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
    while (size != newSize) {
        if (timeout()) {
            auto left = size - newSize;
            LogInfo("Waiting for " << left << " background thread" << (left > 1 ? "s" : "")
                                   << " to finish");
            timelimit += std::chrono::milliseconds(1000);
        }

        size = pool_.trySetSize(newSize);
        processFront();
        processEvents();
    }
}

void InviwoApplicationQt::logQtMessages([[maybe_unused]] QtMsgType type,
                                        [[maybe_unused]] const QMessageLogContext& context,
                                        [[maybe_unused]] const QString& msg) {
#ifdef IVW_DEBUG

#if defined(__APPLE__)
    // There is some weird bug on mac that sets complains about
    // QWidgetWindow(...) Attempt to set a screen on a child window
    // Does not seem to be a real problem lets, ignore it.
    // http://stackoverflow.com/questions/33545006/qt5-attempt-to-set-a-screen-on-a-child-window-many-runtime-warning-messages
    if (msg.contains("Attempt to set a screen on a child window")) return;
#endif

    std::string localMsg = std::string(msg.toUtf8().constData());
    std::string_view file = context.file ? std::string_view{context.file} : std::string_view{};
    std::string_view function =
        context.function ? std::string_view{context.function} : std::string_view{};

    switch (type) {
        case QtDebugMsg:
            LogCentral::getPtr()->log("Qt Debug", LogLevel::Info, LogAudience::Developer, file,
                                      function, context.line, localMsg);

            fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.c_str(), context.file, context.line,
                    context.function);
            break;
        case QtWarningMsg:
            LogCentral::getPtr()->log("Qt Warning", LogLevel::Warn, LogAudience::Developer, file,
                                      function, context.line, localMsg);

            fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.c_str(), context.file,
                    context.line, context.function);
            break;
        case QtCriticalMsg:
            LogCentral::getPtr()->log("Qt Critical", LogLevel::Error, LogAudience::Developer, file,
                                      function, context.line, localMsg);

            fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.c_str(), context.file,
                    context.line, context.function);
            break;
        case QtFatalMsg:
            LogCentral::getPtr()->log("Qt Fatal", LogLevel::Error, LogAudience::Developer, file,
                                      function, context.line, localMsg);

            fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.c_str(), context.file, context.line,
                    context.function);
            QMessageBox::critical(nullptr, "Fatal Error", msg);
            util::debugBreak();
            abort();
            break;

#if (QT_VERSION >= QT_VERSION_CHECK(5, 5, 0))
        case QtInfoMsg:
            LogCentral::getPtr()->log("Qt Info", LogLevel::Info, LogAudience::Developer, file,
                                      function, context.line, localMsg);

            fprintf(stderr, "Info: %s (%s:%u, %s)\n", localMsg.c_str(), context.file, context.line,
                    context.function);
            break;
#endif
    }
#endif
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

#include <warn/push>
#include <warn/ignore/switch-enum>
bool InviwoApplicationQt::notify(QObject* receiver, QEvent* e) {
    auto res = QApplication::notify(receiver, e);

    switch (e->type()) {
        case QEvent::MouseButtonRelease: {
            undoTrigger_();
            break;
        }
        case QEvent::TouchEnd: {
            auto te = static_cast<QTouchEvent*>(e);
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
            if (util::all_of(te->points(), [](const QTouchEvent::TouchPoint& tp) {
                    return tp.state() == QEventPoint::Released;
                })) {
                undoTrigger_();
                break;
            }
#else
            if (util::all_of(te->touchPoints(), [](const QTouchEvent::TouchPoint& tp) {
                    return tp.state() == Qt::TouchPointReleased;
                })) {
                undoTrigger_();
                break;
            }
#endif

            break;
        }
        case QEvent::KeyRelease: {
            undoTrigger_();
            break;
        }
        case QEvent::Drop: {
            undoTrigger_();
            break;
        }
        default:
            break;
    }
    return res;
}
#include <warn/pop>

void InviwoApplicationQt::setUndoTrigger(std::function<void()> func) { undoTrigger_ = func; }

std::locale InviwoApplicationQt::getCurrentStdLocale() {
    static std::locale loc = []() -> std::locale {
#ifdef WIN32
        // need to change locale given by Qt from underscore to hyphenated ("sv_SE" to "sv-SE")
        // although std::locale should only accept locales with underscore, e.g. "sv_SE"
        std::string localeName(QLocale::system().name().replace('_', '-').toStdString());
#else
        std::string localeName(QLocale::system().name().toStdString());
#endif

        // try to use the system locale provided by Qt
        std::vector<std::string> localeNames = {localeName, "en_US.UTF-8", "en_US.UTF8",
                                                "en-US.UTF-8", "en-US.UTF8"};
        for (auto&& [i, name] : util::enumerate(localeNames)) {
            try {
                if (i != 0) {
                    LogWarnCustom("getStdLocale", fmt::format("Falling back to locale '{}'", name));
                }
                return std::locale(name);
            } catch (std::exception& e) {
                LogWarnCustom("getStdLocale",
                              fmt::format("Locale could not be set to '{}', {}", name, e.what()));
            }
        }
        LogWarnCustom("getStdLocale", "Using the default locale");
        return std::locale{};
    }();

    return loc;
}

QEvent::Type InviwoQtEvent::InviwoQtEventType = QEvent::None;

}  // namespace inviwo
