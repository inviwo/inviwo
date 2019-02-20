/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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

#include <modules/qtwidgets/qtwidgetsmoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <inviwo/core/util/fileobserver.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QObject>
#include <warn/pop>

#include <functional>

class QWidget;

namespace inviwo {

namespace utilqt {

class IVW_MODULE_QTWIDGETS_API EditorFileObserver : public QObject, public FileObserver {
public:
    explicit EditorFileObserver(QWidget *parent, const QString &title = "Editor",
                                const std::string filename = std::string{});
    virtual ~EditorFileObserver() = default;

    void setTitle(const QString &title);
    const QString &getTitle() const;

    void resumeObservingFile();
    void suspendObservingFile();

    void ignoreNextUpdate();

    void setFileName(const std::string &filename);
    const std::string &getFileName() const;

    void setModifiedCallback(std::function<void(bool)> cb);
    void setReloadFileCallback(std::function<void()> cb);

private:
    virtual void fileChanged(const std::string &fileName) override;
    virtual bool eventFilter(QObject *obj, QEvent *ev) override;
    void queryReloadFile();
    bool widgetIsFocused() const;

    std::function<void(bool)> modifiedCallback_;
    std::function<void()> reloadFileCallback_;

    QWidget *parent_;
    QString title_;
    std::string filename_;
    bool fileChangedInBackground_ = false;
    bool reloadQueryInProgress_ = false;
    bool ignoreNextUpdate_ = false;
};

}  // namespace utilqt

}  // namespace inviwo
