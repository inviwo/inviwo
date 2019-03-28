/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#include <modules/qtwidgets/textlabeloverlay.h>
#include <modules/qtwidgets/inviwoqtutils.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QObject>
#include <QEvent>
#include <QMouseEvent>
#include <QTimer>
#include <warn/pop>

namespace inviwo {

TextLabelOverlay::TextLabelOverlay(QWidget* parent)
    : QLabel(parent), timer_{std::make_unique<QTimer>()} {
    setObjectName("TextLabelOverlay");

    setAttribute(Qt::WA_NoSystemBackground, true);
    setAttribute(Qt::WA_TransparentForMouseEvents, true);
    hide();
    timer_->setSingleShot(true);
    setWordWrap(true);
    setAlignment(Qt::AlignLeft | Qt::AlignTop);

    connect(timer_.get(), &QTimer::timeout, this, [&]() { clear(); });
}

TextLabelOverlay::~TextLabelOverlay() = default;

void TextLabelOverlay::setText(const std::string& text, std::chrono::milliseconds fade) {
    QLabel::setText(utilqt::toQString(text));
    if (fade != std::chrono::milliseconds{0}) {
        timer_->start(static_cast<int>(fade.count()));
    }
    show();
}

QSize TextLabelOverlay::sizeHint() const { return parentWidget()->size(); }

void TextLabelOverlay::clear() {
    hide();
    QLabel::setText("");
}

}  // namespace inviwo
