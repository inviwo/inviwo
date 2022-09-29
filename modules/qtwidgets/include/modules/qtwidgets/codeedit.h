/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2022 Inviwo Foundation
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

#include <inviwo/core/util/glmvec.h>                  // for vec4

#include <functional>                                 // for function
#include <string>                                     // for string

#include <QPlainTextEdit>                             // for QPlainTextEdit
#include <QSize>                                      // for QSize
#include <QWidget>                                    // for QWidget

class QKeyEvent;
class QPaintEvent;
class QRect;
class QResizeEvent;

namespace inviwo {

class SyntaxHighlighter;

class IVW_MODULE_QTWIDGETS_API CodeEdit : public QPlainTextEdit {
public:
    CodeEdit(QWidget* parent = nullptr);
    virtual ~CodeEdit() = default;

    // QPlainTextEdit overrides
    virtual void keyPressEvent(QKeyEvent* keyEvent) override;

    void setLineAnnotation(std::function<std::string(int)>);
    void setLineAnnotationColor(std::function<vec4(int, vec4)>);
    void setAnnotationSpace(std::function<int(int)>);

    SyntaxHighlighter& syntaxHighlighter();

protected:
    void lineNumberAreaPaintEvent(QPaintEvent* event);
    int lineNumberAreaWidth();

    class LineNumberArea : public QWidget {
    public:
        LineNumberArea(CodeEdit* editor) : QWidget(editor) { codeEdit_ = editor; }
        QSize sizeHint() const override { return QSize(codeEdit_->lineNumberAreaWidth(), 0); }

    protected:
        void paintEvent(QPaintEvent* event) override { codeEdit_->lineNumberAreaPaintEvent(event); }
        CodeEdit* codeEdit_;
    };

    void resizeEvent(QResizeEvent* event) override;

    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect&, int);

    QWidget* lineNumberArea_;
    vec4 textColor_;
    vec4 highlightColor_;
    SyntaxHighlighter* sh_;
    std::function<std::string(int)> annotateLine_;
    std::function<vec4(int, vec4)> annotateColor_;
    std::function<int(int)> annotationSpace_;
};

}  // namespace inviwo
