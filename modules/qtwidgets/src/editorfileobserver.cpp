/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2025 Inviwo Foundation
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

#include <modules/qtwidgets/editorfileobserver.h>

#include <inviwo/core/common/inviwoapplicationutil.h>  // for getInviwoApplication
#include <inviwo/core/util/fileobserver.h>             // for FileObserver
#include <inviwo/core/util/filesystem.h>               // for getFileNameWithExtension
#include <modules/qtwidgets/inviwoqtutils.h>           // for toQString

#include <algorithm>  // for any_of

#include <fmt/std.h>

#include <QEvent>       // for QEvent, QEvent::FocusIn
#include <QList>        // for QList
#include <QMessageBox>  // for QMessageBox, operator|, QMessageBo...
#include <QWidget>      // for QWidget
#include <Qt>           // for WindowModal

namespace inviwo {

namespace utilqt {

EditorFileObserver::EditorFileObserver(QWidget* parent, const QString& title,
                                       const std::filesystem::path& filename)
    : QObject(parent), FileObserver(util::getInviwoApplication()), parent_(parent), title_(title) {
    if (parent_) {
        parent_->installEventFilter(this);
    }
    setFileName(filename);
}

void EditorFileObserver::setTitle(const QString& title) { title_ = title; }

const QString& EditorFileObserver::getTitle() const { return title_; }

void EditorFileObserver::resumeObservingFile() { startFileObservation(filename_); }

void EditorFileObserver::suspendObservingFile() { stopAllObservation(); }

void EditorFileObserver::ignoreNextUpdate() { ignoreNextUpdate_ = true; }

void EditorFileObserver::setFileName(const std::filesystem::path& filename) {
    if (filename_ == filename) {
        return;
    }
    stopAllObservation();
    filename_ = filename;
    startFileObservation(filename_);
}

const std::filesystem::path& EditorFileObserver::getFileName() const { return filename_; }

void EditorFileObserver::setModifiedCallback(std::function<void(bool)> cb) {
    modifiedCallback_ = cb;
}
void EditorFileObserver::setReloadFileCallback(std::function<void()> cb) {
    reloadFileCallback_ = cb;
}

void EditorFileObserver::fileChanged(const std::filesystem::path&) {
    if (ignoreNextUpdate_) {
        ignoreNextUpdate_ = false;
        return;
    }
    if (!fileChangedInBackground_) {
        fileChangedInBackground_ = true;
        queryReloadFile();
    }
}

bool EditorFileObserver::eventFilter(QObject* obj, QEvent* event) {
    if (event->type() == QEvent::FocusIn) {
        if (fileChangedInBackground_) {
            queryReloadFile();
        }
        return false;
    } else {
        // standard event processing
        return QObject::eventFilter(obj, event);
    }
}

void EditorFileObserver::queryReloadFile() {
    if (widgetIsFocused() && fileChangedInBackground_ && !reloadQueryInProgress_) {
        reloadQueryInProgress_ = true;
        std::string msg = fmt::format(
            "The file {} has been modified outside of Inviwo, do you want to reload its "
            "contents?",
            filename_.stem());

        QMessageBox msgBox(QMessageBox::Question, title_, utilqt::toQString(msg),
                           QMessageBox::Yes | QMessageBox::No, parent_);
        msgBox.setWindowModality(Qt::WindowModal);

        if (msgBox.exec() == QMessageBox::Yes) {
            reloadFileCallback_();
        } else {
            modifiedCallback_(true);
        }
        fileChangedInBackground_ = false;
        reloadQueryInProgress_ = false;
    }
}

bool EditorFileObserver::widgetIsFocused() const {
    auto children = parent_->findChildren<QWidget*>();
    return std::any_of(children.begin(), children.end(), [](auto w) { return w->hasFocus(); });
}

}  // namespace utilqt

}  // namespace inviwo
