/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2020 Inviwo Foundation
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

#include <modules/openglqt/openglqtmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <modules/qtwidgets/inviwodockwidget.h>
#include <modules/opengl/shader/shaderobject.h>

#include <algorithm>

#include <warn/push>
#include <warn/ignore/all>
#include <QWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QEvent>
#include <warn/pop>

namespace inviwo {

class CodeEdit;

class IVW_MODULE_OPENGLQT_API ShaderWidget : public InviwoDockWidget {
public:
    ShaderWidget(ShaderObject* obj, QWidget* parent = nullptr);
    virtual ~ShaderWidget();

protected:
    virtual void closeEvent(QCloseEvent* event) override;

    virtual bool eventFilter(QObject* obj, QEvent* event) override;

private:
    void save();

    void updateState();
    void queryReloadFile();

    void shaderObjectChanged();

    ShaderObject* obj_;
    std::shared_ptr<typename ShaderObject::Callback> shaderObjOnChange_;
    CodeEdit* shadercode_;
    std::vector<std::shared_ptr<std::function<void()>>> codeCallbacks_;
    QAction* preprocess_;
    QAction* save_;

    bool fileChangedInBackground_ = false;
    bool reloadQueryInProgress_ = false;
    bool ignoreNextUpdate_ = false;
};

}  // namespace inviwo
