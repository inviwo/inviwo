/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022-2023 Inviwo Foundation
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
#pragma once

#include <inviwo/qt/applicationbase/qtapplicationbasemoduledefine.h>
#include <inviwo/qt/applicationbase/qtlocale.h>

#include <locale>
#include <QString>
#include <QObject>

class QApplication;
class QWidget;

namespace inviwo {

class InviwoApplication;

namespace utilqt {

/**
 * Configure Qt OpenGL settings for inviwo
 * Should be called before constructing a QApplication
 */
IVW_QTAPPLICATIONBASE_API void configureInviwoQtApp();

/**
 * Sets the standard inviwo names in the QApplication
 * Sets the Organization Name and Domain, and application name and icon
 * The names are used when saving/loading qt settings.
 * Should be called after constructing QApplication
 */
IVW_QTAPPLICATIONBASE_API void configureInviwoDefaultNames();

/**
 * A logging function that redirects qt log messages to the inviwo LogCentral.
 * Can be installed by calling qInstallMessageHandler(&logQtMessages)
 */
IVW_QTAPPLICATIONBASE_API void logQtMessages(QtMsgType type, const QMessageLogContext& context,
                                             const QString& msg);

/**
 * Installs a fileSystem observer service based on Qt file observers in the inviwo app
 * Should be called after constructing QApplication
 */
IVW_QTAPPLICATIONBASE_API void configureFileSystemObserver(InviwoApplication& app);

/**
 * Make the inviwo app post enqueue work with the qt event system
 * Should be called after constructing QApplication
 */
IVW_QTAPPLICATIONBASE_API void configurePostEnqueueFront(InviwoApplication& app);

/**
 * Configure the inviwo pool resize to work with the qt event system
 */
IVW_QTAPPLICATIONBASE_API void configurePoolResizeWait(InviwoApplication& app, QWidget* window);

/**
 * Sets a new style sheet file in the QApplication
 */
IVW_QTAPPLICATIONBASE_API void setStyleSheetFile(std::string_view file);

namespace detail {
class IVW_QTAPPLICATIONBASE_API QtProcessFrontHelper : public QObject {
    Q_OBJECT
public:
    using QObject::QObject;

signals:
    void postEnqueue();
};

}  // namespace detail

}  // namespace utilqt

}  // namespace inviwo
