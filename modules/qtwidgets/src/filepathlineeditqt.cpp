/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2023 Inviwo Foundation
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

#include <modules/qtwidgets/filepathlineeditqt.h>

#include <inviwo/core/util/logcentral.h>
#include <inviwo/core/util/filesystem.h>      // for fileExists, getFileNameWithExtension, getFi...
#include <modules/qtwidgets/inviwoqtutils.h>  // for toQString, fromQString
#include <modules/qtwidgets/lineeditqt.h>     // for LineEditQt

#include <QCursor>      // for QCursor
#include <QFocusEvent>  // for QFocusEvent
#include <QLabel>       // for QLabel
#include <QLineEdit>    // for QLineEdit
#include <QPixmap>      // for QPixmap
#include <QSize>        // for QSize
#include <QString>      // for QString, operator!=
#include <QStyle>       // for QStyle, QStyle::PM_DefaultFrameWidth
#include <Qt>           // for MouseFocusReason
#include <QCompleter>
#include <QFileSystemModel>
#include <QListView>

class QFocusEvent;
class QMouseEvent;
class QResizeEvent;
class QWidget;

namespace inviwo {

FilePathLineEditQt::FilePathLineEditQt(QWidget* parent)
    : LineEditQt{parent}
    , warningLabel_{new QLabel(this)}
    , path_{}
    , acceptMode_{AcceptMode::Open}
    , fileMode_{FileMode::AnyFile} {

    int width = this->sizeHint().height();
    QSize labelSize(width, width);
    warningLabel_->setScaledContents(true);
    warningLabel_->setPixmap(QPixmap(":/svgicons/file-warning.svg"));
    warningLabel_->setFixedSize(labelSize);
    warningLabel_->setToolTip("Could not locate file");
    warningLabel_->hide();

    auto fsModel = new QFileSystemModel(this);
    fsModel->setRootPath(QString());
    // Need to access the model once to have it cache the current path
    // Otherwise there will be no suggestions.
    connect(this, &QLineEdit::textChanged,
            [fsModel](const QString& path) { fsModel->index(path); });
    QCompleter* completer = new QCompleter(fsModel, this);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    setCompleter(completer);
    completer->popup()->setObjectName("FileCompleterDropdown");

    connect(this, &QLineEdit::textEdited,
            [this](const QString& str) { updateIcon(utilqt::toPath(str)); });

    connect(completer, QOverload<const QString&>::of(&QCompleter::highlighted),
            [=](const QString& str) { updateIcon(utilqt::toPath(str)); });

    connect(this, &QLineEdit::editingFinished, [this]() {
        setCursorToEnd();
        const auto path = utilqt::toPath(text().trimmed());
        updateIcon(path);
        if (path != path_) {
            path_ = path;
            emit pathChanged(path_);
        }
    });

    connect(this, &LineEditQt::editingCanceled, [this]() {
        setText(utilqt::toQString(path_));
        setCursorToEnd();
        updateIcon(path_);
    });
}

void FilePathLineEditQt::setAcceptMode(AcceptMode acceptMode) {
    if (acceptMode_ != acceptMode) {
        acceptMode_ = acceptMode;
        updateIcon(path_);
    }
}
void FilePathLineEditQt::setFileMode(FileMode fileMode) {
    if (fileMode_ != fileMode) {
        fileMode_ = fileMode;
        updateIcon(path_);
    }
}

void FilePathLineEditQt::setPath(const std::filesystem::path& path) {
    if (path != path_) {
        path_ = path;
        setText(utilqt::toQString(path_));
        setCursorToEnd();
        updateIcon(path_);
    }
}

const std::filesystem::path& FilePathLineEditQt::getPath() const { return path_; }

void FilePathLineEditQt::setCursorToEnd() { setCursorPosition(static_cast<int>(text().length())); }

void FilePathLineEditQt::resizeEvent(QResizeEvent*) {
    // adjust position of warning label to be on the right side
    int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    warningLabel_->move(width() - warningLabel_->width() - frameWidth, 0);
}

void FilePathLineEditQt::updateIcon(const std::filesystem::path& path) {
    // update visibility of warning icon

    bool visible = false;
    QString tooltip;

    bool hasWildcard = (path.string().find_first_of("*?#", 0) != std::string::npos);

    auto status = std::filesystem::status(path);
    const bool isFile = std::filesystem::is_regular_file(status);
    const bool isDir = std::filesystem::is_directory(status);

    const bool parentDir = std::filesystem::is_directory(path.parent_path());

    switch (acceptMode_) {
        case AcceptMode::Open: {
            switch (fileMode_) {
                case FileMode::AnyFile: {
                    break;
                }
                case FileMode::ExistingFile: {
                    if (!hasWildcard) {
                        visible = !isFile;
                        tooltip = "Could not locate file!";
                    } else {
                        visible = !parentDir;
                        tooltip = "Could not locate parent directory!";
                    }
                    break;
                }
                case FileMode::ExistingFiles: {
                    if (!hasWildcard) {
                        visible = !isFile;
                        tooltip = "Could not locate file!";
                    } else {
                        visible = !parentDir;
                        tooltip = "Could not locate parent directory!";
                    }
                    break;
                }
                case FileMode::Directory: {
                    visible = !isDir;
                    tooltip = "Could not locate directory!";
                    break;
                }
            }
            break;
        }
        case AcceptMode::Save: {
            switch (fileMode_) {
                case FileMode::AnyFile: {
                    break;
                }
                case FileMode::ExistingFile: {
                    break;
                }
                case FileMode::ExistingFiles: {
                    break;
                }
                case FileMode::Directory: {
                    visible = !isDir;
                    tooltip = "Could not locate directory!";
                    break;
                }
            }

            break;
        }
    }

    warningLabel_->setVisible(visible);
    warningLabel_->setToolTip(tooltip);
    // make sure there is no text flowing into the warning label
    this->setStyleSheet(QString("QLineEdit:enabled { padding-right: %1; }")
                            .arg(visible ? warningLabel_->width() + 2 : 0));
}

}  // namespace inviwo
