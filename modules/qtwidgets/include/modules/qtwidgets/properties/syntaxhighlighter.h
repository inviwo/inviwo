/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2020 Inviwo Foundation
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

#include <functional>
#include <memory>
#include <vector>

// QT includes
#include <warn/push>
#include <warn/ignore/all>
#include <QSyntaxHighlighter>
#include <warn/pop>

class QTextDocument;

namespace inviwo {

enum SyntaxType {
    None = 0,
    GLSL = 1,
    Python = 2,
};

class IVW_MODULE_QTWIDGETS_API SyntaxFormater {
public:
    struct Result {
        std::vector<unsigned int> start;
        std::vector<unsigned int> length;
        QTextCharFormat* format;
        int outgoingState;

        Result() : format(0), outgoingState(-1) {}
    };

    virtual Result eval(const QString& text, const int& previousBlockState) = 0;
    SyntaxFormater() = default;
    virtual ~SyntaxFormater() = default;
};

class IVW_MODULE_QTWIDGETS_API SyntaxHighligther : public QSyntaxHighlighter {
public:
    template <SyntaxType T>
    void setSyntax();
    template <SyntaxType T>
    static SyntaxHighligther* createSyntaxHighligther(QTextDocument* parent);
    virtual ~SyntaxHighligther();

    const QColor& getBackgroundColor() const;

protected:
    SyntaxHighligther(QTextDocument* parent);
    void highlightBlock(const QString& text);

private:
    template <SyntaxType T>
    void loadConfig();
    QTextCharFormat defaultFormat_;
    std::vector<std::unique_ptr<SyntaxFormater>> formaters_;
    std::shared_ptr<std::function<void()>> callback_;
    QColor backgroundColor_;
};

template <SyntaxType T>
void SyntaxHighligther::setSyntax() {
    formaters_.clear();
    loadConfig<T>();
}

template <SyntaxType T>
SyntaxHighligther* SyntaxHighligther::createSyntaxHighligther(QTextDocument* parent) {
    SyntaxHighligther* s = new SyntaxHighligther(parent);
    s->loadConfig<T>();
    return s;
}

template <>
IVW_MODULE_QTWIDGETS_API void SyntaxHighligther::loadConfig<None>();
template <>
IVW_MODULE_QTWIDGETS_API void SyntaxHighligther::loadConfig<GLSL>();
template <>
IVW_MODULE_QTWIDGETS_API void SyntaxHighligther::loadConfig<Python>();

}  // namespace inviwo
