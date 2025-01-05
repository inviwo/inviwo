/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022-2026 Inviwo Foundation
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

#include <inviwo/qt/applicationbase/qtapptools.h>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/filesystemobserver.h>
#include <inviwo/core/util/fileobserver.h>
#include <inviwo/core/util/assertion.h>
#include <inviwo/core/util/zip.h>
#include <inviwo/core/util/raiiutils.h>

#include <QApplication>
#include <QGuiApplication>
#include <QFileSystemWatcher>
#include <QFile>
#include <QIcon>
#include <QWidget>
#include <QMessageBox>
#include <QCursor>
#include <QFont>
#include <QSurfaceFormat>
#include <QThread>
#include <QTimer>

#include <string>
#include <string_view>
#include <vector>
#include <mutex>

#ifdef WIN32
#include <windows.h>
#endif  // WIN32

namespace inviwo {

namespace {

std::filesystem::path toPath(const QString& str) {
    auto buffer = str.toUtf8();
    std::u8string_view u8str{reinterpret_cast<const char8_t*>(buffer.constData()),
                             static_cast<size_t>(buffer.size())};
    return std::filesystem::path{u8str};
}
QString toQString(const std::filesystem::path& path) {
    auto str = path.generic_u8string();
    return QString::fromUtf8(reinterpret_cast<char*>(str.data()), str.size());
}

class FileSystemObserverQt : public QObject, public FileSystemObserver {
public:
    FileSystemObserverQt() : fileWatcher_{new QFileSystemWatcher(this)} {

        connect(fileWatcher_, &QFileSystemWatcher::fileChanged, this,
                [this](const QString& fileName) {
                    QTimer::singleShot(200, this, [this, fileName]() { fileChanged(fileName); });
                });

        connect(fileWatcher_, &QFileSystemWatcher::directoryChanged, this,
                [this](const QString& fileName) {
                    QTimer::singleShot(200, this, [this, fileName]() { fileChanged(fileName); });
                });
    }

    virtual void registerFileObserver(FileObserver* fileObserver) override;
    virtual void unRegisterFileObserver(FileObserver* fileObserver) override;

private:
    virtual void startFileObservation(const std::filesystem::path& fileName,
                                      FileObserver* source) override;
    virtual void stopFileObservation(const std::filesystem::path& fileName,
                                     FileObserver* source) override;

    void fileChanged(const QString& fileName);

    std::mutex mutex_;
    std::vector<FileObserver*> fileObservers_;
    QFileSystemWatcher* fileWatcher_;
};

void FileSystemObserverQt::registerFileObserver(FileObserver* fileObserver) {
    const std::scoped_lock lock{mutex_};
    IVW_ASSERT(std::find(fileObservers_.cbegin(), fileObservers_.cend(), fileObserver) ==
                   fileObservers_.cend(),
               "File observer already registered.");
    fileObservers_.push_back(fileObserver);
}

void FileSystemObserverQt::unRegisterFileObserver(FileObserver* fileObserver) {
    const std::scoped_lock lock{mutex_};
    std::erase(fileObservers_, fileObserver);
}

void FileSystemObserverQt::startFileObservation(const std::filesystem::path& fileName,
                                                FileObserver*) {
    const std::scoped_lock lock{mutex_};
    // Will add the path if file exists and is not already being watched.
    fileWatcher_->addPath(toQString(fileName));
}

void FileSystemObserverQt::stopFileObservation(const std::filesystem::path& fileName,
                                               FileObserver* source) {
    const std::scoped_lock lock{mutex_};
    auto it = std::ranges::find_if(fileObservers_, [&](const FileObserver* observer) {
        return observer != source && observer->isObserved(fileName);
    });
    // Make sure that no observer is observing the file
    if (it == std::end(fileObservers_)) {
        fileWatcher_->removePath(toQString(fileName));
    }
}

void FileSystemObserverQt::fileChanged(const QString& fileName) {
    if (!QFile::exists(fileName)) return;

    const auto fileNameStd = toPath(fileName);
    std::unique_lock lock{mutex_};
    // don't use iterators here, they might be invalidated.
    size_t size = fileObservers_.size();
    for (size_t i = 0; i < size && i < fileObservers_.size(); ++i) {
        if (fileObservers_[i]->isObserved(fileNameStd)) {
            auto* observer = fileObservers_[i];
            lock.unlock();
            const util::OnScopeExit relock{[&]() { lock.lock(); }};
            try {
                observer->fileChanged(fileNameStd);
            } catch (const Exception& e) {
                log::exception(e);
            }
        }
    }

    if (!fileWatcher_->files().contains(fileName)) {
        fileWatcher_->addPath(fileName);
    }
}

void showFatalMessage(const QString& msg) {
    if (QApplication::instance()->thread() == QThread::currentThread()) {
        QMessageBox::critical(nullptr, "Fatal Error", msg);
    } else {
        // If the assertion happened in a different thread, we can not show a dialog
        // directly, so we pass it on to the main thread. But we have to wait for it
        // to show, since we will probably exit after we leave this function.
        QMetaObject::invokeMethod(
            QApplication::instance(),
            [msg]() { QMessageBox::critical(nullptr, "Assertion Failed", msg); },
            Qt::BlockingQueuedConnection);
    }
}

}  // namespace

void utilqt::logQtMessages([[maybe_unused]] QtMsgType type,
                           [[maybe_unused]] const QMessageLogContext& context,
                           [[maybe_unused]] const QString& msg) {
#if defined(__APPLE__)
    // There is some weird bug on mac that sets complains about
    // QWidgetWindow(...) Attempt to set a screen on a child window
    // Does not seem to be a real problem lets, ignore it.
    // http://stackoverflow.com/questions/33545006/
    // qt5-attempt-to-set-a-screen-on-a-child-window-many-runtime-warning-messages
    if (msg.contains("Attempt to set a screen on a child window")) return;
#endif

    constexpr std::string_view missing = "-";

    const auto localMsg = std::string(msg.toUtf8().constData());
    const auto file = context.file ? std::string_view{context.file} : missing;
    const auto function = context.function ? std::string_view{context.function} : missing;
    const auto line = static_cast<std::uint32_t>(context.line);
    switch (type) {
        case QtInfoMsg:
            log::report(LogLevel::Info, SourceContext{"Qt Info"_sl, file, function, line},
                        localMsg);
            break;
        case QtDebugMsg:
            log::report(LogLevel::Info, SourceContext{"Qt Debug"_sl, file, function, line},
                        localMsg);
            break;
        case QtWarningMsg:
            log::report(LogLevel::Warn, SourceContext{"Qt Warning"_sl, file, function, line},
                        localMsg);
            break;
        case QtCriticalMsg:
            log::report(LogLevel::Error, SourceContext{"Qt Critical"_sl, file, function, line},
                        localMsg);
            break;
        case QtFatalMsg:
            log::report(LogLevel::Error, SourceContext{"Qt Fatal"_sl, file, function, line},
                        localMsg);
            util::debugBreak();
            showFatalMessage(msg);
            abort();
            break;
    }
}

void utilqt::configureInviwoQtApp() {
#ifdef __linux__
    /*
     * Suppress warning "QApplication: invalid style override passed, ignoring it." when starting
     * Inviwo on Linux. See
     * https://forum.qt.io/topic/75398/qt-5-8-0-qapplication-invalid-style-override-passed-ignoring-it/2
     */
    qputenv("QT_STYLE_OVERRIDE", "");
#endif

    QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
    QSurfaceFormat defaultFormat;
    defaultFormat.setMajorVersion(10);
    defaultFormat.setProfile(QSurfaceFormat::CoreProfile);
    defaultFormat.setSwapInterval(0);
    QSurfaceFormat::setDefaultFormat(defaultFormat);

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

    QApplication::setFont(
        QFont(QString::fromWCharArray(metrics.lfMessageFont.lfFaceName), pointSize));
#endif  // WIN32
}

void utilqt::configureInviwoDefaultNames() {
    QCoreApplication::setOrganizationName("Inviwo Foundation");
    QCoreApplication::setOrganizationDomain("inviwo.org");
    QCoreApplication::setApplicationName("Inviwo");
    QGuiApplication::setWindowIcon(QIcon(":/inviwo/inviwo_light.png"));
}

void utilqt::configureFileSystemObserver(InviwoApplication& app) {
    app.setFileSystemObserver(std::make_unique<FileSystemObserverQt>());
}

void utilqt::setStyleSheetFile(const std::filesystem::path& file) {
    QFile styleSheetFile{file};
    if (styleSheetFile.open(QFile::ReadOnly)) {
        const auto styleSheet = QString::fromUtf8(styleSheetFile.readAll());
        qApp->setStyleSheet(styleSheet);
    }
}

void utilqt::configurePalette() {
    QPalette palette(QGuiApplication::palette());
    palette.setColor(QPalette::Link, QColor("#268BD2"));
    palette.setColor(QPalette::LinkVisited, QColor("#268BD2"));
    QGuiApplication::setPalette(palette);
}

void utilqt::configurePostEnqueueFront(InviwoApplication& app) {
    auto qapp = QApplication::instance();
    auto helper = std::make_shared<detail::QtProcessFrontHelper>();
    app.setPostEnqueueFront([helper]() { emit helper->postEnqueue(); });
    QObject::connect(
        helper.get(), &detail::QtProcessFrontHelper::postEnqueue, qapp,
        [&app]() { app.processFront(); }, Qt::QueuedConnection);

    app.setProcessEventsCallback([]() { QApplication::instance()->processEvents(); });
}

void utilqt::configureAssertionHandler(InviwoApplication& app) {
    app.setAssertionHandler([](std::string_view message, SourceContext context) {
        auto error = QString{"<b>Assertion Failed</b><br>File: %1:%2<br>Function: %3<p>%4"}
                         .arg(toQString(context.file()))
                         .arg(context.line())
                         .arg(toQString(context.function()))
                         .arg(toQString(message));

        showFatalMessage(error);
    });
}

void utilqt::configurePoolResizeWait(InviwoApplication& app, QWidget* window) {
    app.setPoolResizeWaitCallback(
        [window, enabled = true](InviwoApplication::LongWait type) mutable {
            switch (type) {
                case InviwoApplication::LongWait::Start: {
                    enabled = window->isEnabled();
                    QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
                    break;
                }
                case InviwoApplication::LongWait::Update: {
                    QApplication::instance()->processEvents();
                    break;
                }
                case InviwoApplication::LongWait::End: {
                    window->setEnabled(enabled);
                    QApplication::restoreOverrideCursor();
                    break;
                }
            }
        });
}

}  // namespace inviwo
