/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2024 Inviwo Foundation
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

#include <modules/qtwidgets/qtwidgetsmoduledefine.h>  // for IVW_MODULE_QTWIDGETS_API

#include <modules/qtwidgets/lineeditqt.h>  // for LineEditQt
#include <inviwo/core/util/filedialogstate.h>

#include <filesystem>

#include <QObject>  // for Q_OBJECT

class QFocusEvent;
class QLabel;
class QMouseEvent;
class QResizeEvent;
class QWidget;

namespace inviwo {

/**
 * \class FilePathLineEditQt
 * \brief QLineEdit for file paths. When editing the path, i.e. the widget is focused, the full path
 * is shown. When not in focus, it shows only the file name with extension. A small warning icon is
 * shown to indicate non-existing files and paths.
 */
class IVW_MODULE_QTWIDGETS_API FilePathLineEditQt : public LineEditQt {
    Q_OBJECT
public:
    FilePathLineEditQt(QWidget* parent = nullptr);
    virtual ~FilePathLineEditQt() = default;

    void setPath(const std::filesystem::path& path);
    const std::filesystem::path& getPath() const;

    void setAcceptMode(AcceptMode mode);
    void setFileMode(FileMode mode);

    void setCursorToEnd();

signals:
    void pathChanged(const std::filesystem::path&);

protected:
    virtual void resizeEvent(QResizeEvent* event) override;
    /**
     * update the status icon based on the given \p path
     * @return true if the path is valid given the current accept mode and file mode, otherwise
     * false
     */
    bool updateIcon(const std::filesystem::path& path);

private:
    QLabel* warningLabel_;        //!< warning icon which is visible if the path is invalid
    std::filesystem::path path_;  //!< full path including file name
    AcceptMode acceptMode_;
    FileMode fileMode_;
};

}  // namespace inviwo
