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

#include <modules/qtwidgets/properties/indicatorwidget.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QPaintEvent>
#include <QPainter>
#include <warn/pop>

namespace inviwo {

IndicatorWidget::IndicatorWidget(QWidget *parent)
    : QWidget(parent, Qt::Window | Qt::FramelessWindowHint) {
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setWindowOpacity(0.7);
}

void IndicatorWidget::setBasePoint(QPoint p) {
    basePoint_ = p;
    set(basePoint_, 0);
}

void IndicatorWidget::setLength(int len) {
    length_ = len;
    set(basePoint_, len);
}

void IndicatorWidget::set(QPoint base, int len) {
    QRect r(base, QSize(0, -len));
    r = r.normalized();
    auto l = handleWidth_ / 2 + 1;
    r.adjust(-l, -l, l, l);
    handleRect_ = QRect(QPoint(-handleWidth_ / 2, -handleWidth_ / 2 + std::abs(len)),
                        QSize(handleWidth_, handleWidth_));
    int sign = (len > 0) ? 1 : -1;

    if (std::abs(len) > grooveWidth_) {
        grooveRect_ =
            QRect(QPoint(-grooveWidth_ / 2, -grooveWidth_ / 2), QSize(grooveWidth_, std::abs(len)));
    } else {
        grooveRect_ = QRect();
    }

    painterTransform_ =
        QTransform::fromScale(1.0, -sign).translate(l, -sign * l - std::max(len, 0));

    setGeometry(r);
}

void IndicatorWidget::paintEvent(QPaintEvent *e) {
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing);

    p.setClipRegion(e->region());

    QPen pen(QColor("#1e70a8").darker(150), 1.0);
    p.setPen(pen);

    p.setTransform(painterTransform_);

    if (grooveRect_.isValid()) {
        p.setBrush(QColor("#1e70a8"));

        auto radius = grooveWidth_ * 0.5;
        p.drawRoundedRect(grooveRect_, radius, radius, Qt::AbsoluteSize);
    }

    // draw circle on top
    p.setPen(QPen(QColor("#c8ccd0").darker(200), 1.0));
    p.setBrush(QColor("#c8ccd0").lighter(130));
    p.drawEllipse(handleRect_);
}

}  // namespace inviwo
