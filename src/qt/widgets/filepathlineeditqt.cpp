/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016 Inviwo Foundation
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

#include <inviwo/qt/widgets/filepathlineeditqt.h>
#include <inviwo/core/util/filesystem.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QLabel>
#include <QCursor>
#include <QFocusEvent>
#include <QMouseEvent>
#include <warn/pop>

namespace inviwo {

FilePathLineEditQt::FilePathLineEditQt(QWidget* parent) 
    : LineEditQt(parent) 
    , editingEnabled_(false)
    , cursorPos_(-1)
    , cursorPosDirty_(false)
{
    // warning icon at the right side of the line edit for indication of "file not found"
    warningLabel_ = new QLabel(this);
    int width = this->sizeHint().height();
    QSize labelSize(width, width);
    warningLabel_->setScaledContents(true);
    warningLabel_->setPixmap(QPixmap(":/icons/filewarning.png"));
    warningLabel_->setFixedSize(labelSize);
    warningLabel_->setToolTip("Invalid File: Could not locate file");
    //warningLabel_->setStyleSheet("QLabel { border : none; padding: 2px }");
    warningLabel_->hide();

    QObject::connect(this, &QLineEdit::returnPressed, [this]() {
        if (editingEnabled_) {
            cursorPos_ = -1;
            path_ = this->text().toStdString();
            setEditing(false);
        }
    });
    QObject::connect(this, &QLineEdit::editingFinished, [this]() {
        if (editingEnabled_) {
            LogInfo("editing finished, cursorpos (old) " << cursorPos_);
            cursorPos_ = this->cursorPosition();
            LogInfo(" cursor (new) " << cursorPos_);
            path_ = this->text().toStdString();
            setEditing(false);
        }
    });
    QObject::connect(this, &LineEditQt::editingCanceled, [this]() {
        // revert changes
        if (editingEnabled_) {
            cursorPos_ = -1;
            updateContents();
            setEditing(false);
        }
    });
}

void FilePathLineEditQt::setPath(const std::string &path) {
    if (path_ != path) {
        path_ = path;
        cursorPos_ = -1;
        updateContents();
    }
}

const std::string& FilePathLineEditQt::getPath() const {
    return path_;
}

void FilePathLineEditQt::setEditing(bool editing) {
    if (editing != editingEnabled_) {
        editingEnabled_ = editing;
        updateContents();
    }
}

bool FilePathLineEditQt::isEditingEnabled() const {
    return editingEnabled_;
}

void FilePathLineEditQt::resizeEvent(QResizeEvent *event) {
    // adjust position of warning label to be on the right side
    int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    warningLabel_->move(width() - warningLabel_->width() - frameWidth, 0);
}

void FilePathLineEditQt::focusInEvent(QFocusEvent *event) {
    if (event->reason() == Qt::MouseFocusReason) {
        // user has used the mouse to click into this widget
        auto cursor = QCursor::pos();
        // get current cursor position in line edit
        int pos = this->cursorPositionAt(this->mapFromGlobal(cursor));
        // transform position into position within entire path
        auto lenFilename = filesystem::getFileNameWithExtension(path_).size();
        cursorPos_ = static_cast<int>(path_.size() - lenFilename) + pos;
        cursorPosDirty_ = true;
    }
    setEditing(true);
    QLineEdit::focusInEvent(event);
    if (cursorPos_ >= 0) {
        this->setCursorPosition(cursorPos_);
    }
}

void FilePathLineEditQt::mousePressEvent(QMouseEvent *event) {
    LineEditQt::mousePressEvent(event);
    if (cursorPosDirty_) {
        // adjust cursor position since the text has changed
        this->setCursorPosition(cursorPos_);
        cursorPosDirty_ = false;
    }
}

void FilePathLineEditQt::updateContents() {
    if (editingEnabled_) {
        // show entire path
        this->setText(QString::fromStdString(path_));
    }
    else {
        // abbreviate file path and show only the file name        
        this->setText(QString::fromStdString(filesystem::getFileNameWithExtension(path_)));
    }
    updateIcon();
}

void FilePathLineEditQt::updateIcon() {
    // update visibility of warning icon
    bool visible = !filesystem::fileExists(path_);
    warningLabel_->setVisible(visible);
    // make sure there is no text flowing into the warning label
    this->setStyleSheet(QString("QLineEdit:enabled { padding-right: %1; }").arg(visible ? warningLabel_->width() + 2 : 0));
}

}  // namespace inviwo
