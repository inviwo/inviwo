/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#ifndef IVW_SYNTAXHIGHLIGTHER_H
#define IVW_SYNTAXHIGHLIGTHER_H


#include <inviwo/qt/widgets/inviwoqtwidgetsdefine.h>

//QT includes
#include <QSyntaxHighlighter>


class QTextDocument;

namespace inviwo {

enum SyntaxType {
    None = 0,
    GLSL = 1,
    Python = 2,
};


class IVW_QTWIDGETS_API SyntaxFormater {
public:
    struct Result {
        std::vector<unsigned int> start;
        std::vector<unsigned int> length;
        QTextCharFormat* format;
        int outgoingState;

        Result():format(0),outgoingState(-1) {}
    };

    virtual Result eval(const QString& text,const int& previousBlockState) = 0;
    SyntaxFormater() {}
    virtual ~SyntaxFormater() {}
};

class IVW_QTWIDGETS_API SyntaxHighligther : public QSyntaxHighlighter {
    Q_OBJECT
public:
    template<SyntaxType T> void setSyntax();
    template<SyntaxType T> static SyntaxHighligther* createSyntaxHighligther(QTextDocument* parent);
    virtual ~SyntaxHighligther();
protected:
    void clearFormaters();
    SyntaxHighligther(QTextDocument* parent);
    void highlightBlock(const QString& text);

private:
    template<SyntaxType T> void loadConfig();
    QTextCharFormat defaultFormat_;
    std::vector<SyntaxFormater*> formaters_;
};

template<SyntaxType T> void SyntaxHighligther::setSyntax() {
    clearFormaters();
    loadConfig<T>();
}


template<SyntaxType T> SyntaxHighligther* SyntaxHighligther::createSyntaxHighligther(QTextDocument* parent) {
    SyntaxHighligther* s = new SyntaxHighligther(parent);
    s->loadConfig<T>();
    return s;
}

template<> IVW_QTWIDGETS_API void SyntaxHighligther::loadConfig<None>();
template<> IVW_QTWIDGETS_API void SyntaxHighligther::loadConfig<GLSL>();
template<> IVW_QTWIDGETS_API void SyntaxHighligther::loadConfig<Python>();



}//namespace

#endif //IVW_SYNTAXHIGHLIGTHER_H