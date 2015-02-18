/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
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

#include <inviwo/qt/widgets/properties/syntaxhighlighter.h>
#include <inviwo/core/common/inviwomodule.h>
#include <inviwo/core/util/filesystem.h>

#include <QTextDocument>
#include <QTextBlock>



namespace inviwo {

SyntaxHighligther::SyntaxHighligther(QTextDocument* parent):QSyntaxHighlighter(parent) {
}

SyntaxHighligther::~SyntaxHighligther() {
    clearFormaters();
}

void SyntaxHighligther::clearFormaters() {
    while (!formaters_.empty()) {
        delete formaters_.back();
        formaters_.pop_back();
    }
}

void SyntaxHighligther::highlightBlock(const QString& text) {
    setFormat(0,text.size(),defaultFormat_);
    std::vector<SyntaxFormater*>::iterator it;

    for (it = formaters_.begin(); it != formaters_.end(); ++it) {
        SyntaxFormater::Result res = (*it)->eval(text,previousBlockState());

        for (size_t i = 0; i<res.start.size(); i++) {
            setFormat(res.start[i],res.length[i],*res.format);
            setCurrentBlockState(res.outgoingState);
        }
    }
}

template<>
void SyntaxHighligther::loadConfig<None>() {
    QColor textColor;
    QColor bgColor;
    textColor.setNamedColor("#aaaaaa");
    bgColor.setNamedColor("#4d4d4d");
    defaultFormat_.setBackground(bgColor);
    defaultFormat_.setForeground(textColor);
}



} // namespace


