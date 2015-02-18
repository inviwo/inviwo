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

/*********************************************************************************
* Parts of code is courtesy of (viewed latest at 2014-02-18):
* https://github.com/liuyanghejerry/Qt-Plus
* README text:
* Welcome to the small world of QtPlus.info.
* All the code here is free for you.
 *********************************************************************************/

#include <inviwo/qt/widgets/colorwheel.h>
#include <QPainter>
#include <QResizeEvent>
#include <QStyleOption>
#include <QtCore/qmath.h> 

#include <algorithm>

namespace inviwo {

template <class T>
inline T clamp(T value, T lower, T higher) {
    return std::min(std::max(value, lower), higher);
}

ColorWheel::ColorWheel(QWidget* parent) :
    QWidget(parent),
    initSize(200,200),
    wheel(initSize),
    mouseDown(false),
    margin(0),
    wheelWidth(30),
    current(Qt::red),
    inWheel(false),
    inSquare(false) {
    
    current = current.toHsv();
    setMinimumSize(initSize);
    setMaximumSize(initSize);
    setCursor(Qt::CrossCursor);
}

QColor ColorWheel::color() {
    return current;
}

void ColorWheel::setColor(const QColor& color) {
    if (color == current) return;

    if (color.hue() != current.hue())
        hueChanged(color.hue());

    if (color.saturation() != current.saturation()
        || color.value() != current.value())
        svChanged(color);

    update();
    emit colorChange(color);
}


QColor ColorWheel::posColor(const QPoint& point) {
    if (! wheel.rect().contains(point)) return QColor();

    if (inWheel) {
        qreal hue = 0;
        int r = qMin(width(), height()) / 2;

        if (point.x() > r) {
            if (point.y() < r) {
                //1
                hue = 90 - (qAtan2((point.x() - r), (r - point.y()))  / 3.14 / 2 * 360);
            } else {
                //4
                hue = 270 + (qAtan2((point.x() - r), (point.y() - r))  / 3.14 / 2 * 360);
            }
        } else {
            if (point.y() < r) {
                //2
                hue =  90 + (qAtan2((r - point.x()), (r - point.y()))  / 3.14 / 2 * 360);
            } else {
                //3
                hue =  270 - (qAtan2((r - point.x()), (point.y() - r))  / 3.14 / 2 * 360);
            }
        }

        int hueI = clamp(static_cast<int>(hue), 0, 359);
        return QColor::fromHsv(hueI,
                               current.saturation(),
                               current.value());
    }

    if (inSquare) {
        // region of the widget
        int w = qMin(width(), height());
        // radius of outer circle
        qreal r = w/2-margin;
        // radius of inner circle
        qreal ir = r-wheelWidth;
        // left corner of square
        qreal m = w/2.0 - ir/qSqrt(2);
        QPoint p = point - QPoint(m, m);
        qreal SquareWidth = (ir * qSqrt(2));
        return QColor::fromHsv(current.hueF(),
                               clamp(static_cast<int>(p.x() / SquareWidth * 255.0), 0, 255),
                               clamp(static_cast<int>(p.y() / SquareWidth * 255.0), 0, 255));
    }
    
    return QColor();
}

QSize ColorWheel::sizeHint() const {
    return QSize(height(),height());
}

QSize ColorWheel::minimumSizeHint() const {
    return initSize;
}

void ColorWheel::mousePressEvent(QMouseEvent* e) {
    lastPos = e->pos();

    if (wheelRegion.contains(lastPos)) {
        inWheel = true;
        inSquare = false;
        QColor color = posColor(lastPos);

        if (e->button()==Qt::RightButton) hueChanged(color.hue());
        else setColor(color);
    } else if (squareRegion.contains(lastPos)) {
        inWheel = false;
        inSquare = true;
        QColor color = posColor(lastPos);
        svChanged(color);
    }

    mouseDown = true;
}

void ColorWheel::mouseMoveEvent(QMouseEvent* e) {
    lastPos = e->pos();

    if (!mouseDown) return;

    if (wheelRegion.contains(lastPos) && inWheel) {
        QColor color = posColor(lastPos);
        hueChanged(color.hue());
    } else if (squareRegion.contains(lastPos) && inSquare) {
        QColor color = posColor(lastPos);
        svChanged(color);
    } else {
        int length = qMin(width(), height());
        QPoint center(length/2, length/2);

        if (inWheel) {
            int r =  length / 2;
            double x0 = lastPos.x()-center.x();
            double y0 = lastPos.y()-center.y();
            double vNorm = qSqrt(qPow(x0,2)+qPow(y0,2));
            double x1 = r*(x0/vNorm);
            double y1 = r*(y0/vNorm);
            x1 += center.x();
            y1 += center.y();
            QColor color = posColor(QPoint(x1, y1));
            hueChanged(color.hue());
        } else if (inSquare) {
            int w = qMin(width(), height());
            // radius of outer circle
            qreal r = w/2-margin;
            // radius of inner circle
            qreal ir = r-wheelWidth;
            // left corner of square
            qreal m = w/2.0-ir/qSqrt(2);
            float x0 = 0.0f;
            float y0 = 0.0f;
            float w0 = squareRegion.boundingRect().width();

            if (lastPos.x() > m + w0)
                x0 = m + w0;
            else if (lastPos.x() < m)
                x0 = m;
            else
                x0 = lastPos.x();

            if (lastPos.y() > m + w0)
                y0 = m + w0;
            else if (lastPos.y() < m)
                y0 = m;
            else
                y0 = lastPos.y();

            QColor color = posColor(QPoint(x0, y0));
            svChanged(color);
        }
    }
}

void ColorWheel::mouseReleaseEvent(QMouseEvent*) {
    mouseDown = false;
    inWheel = false;
    inSquare = false;
}

void ColorWheel::resizeEvent(QResizeEvent* event) {
    wheel = QPixmap(event->size());
    wheel.fill(palette().background().color());
    drawWheelImage(event->size());
    drawSquareImage(current.hue());
    update();
}

void ColorWheel::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    QStyleOption opt;
    composeWheel();
    painter.drawPixmap(0,0,wheel);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
}

void ColorWheel::drawWheelImage(const QSize& newSize) {
    int r = qMin(newSize.width(), newSize.height());
    QStyleOption option;
    option.initFrom(this);
    QBrush background = option.palette.window();
    wheelImage = QImage(newSize, QImage::Format_ARGB32_Premultiplied);
    wheelImage.fill(background.color());
    QPointF center(r/2.0, r/2.0);
    QPainter painter(&wheelImage);
    painter.setRenderHint(QPainter::Antialiasing);
    QConicalGradient conicalGradient(center, 0);
    conicalGradient.setColorAt(0.0, Qt::red);
    conicalGradient.setColorAt(60.0/360.0, Qt::yellow);
    conicalGradient.setColorAt(120.0/360.0, Qt::green);
    conicalGradient.setColorAt(180.0/360.0, Qt::cyan);
    conicalGradient.setColorAt(240.0/360.0, Qt::blue);
    conicalGradient.setColorAt(300.0/360.0, Qt::magenta);
    conicalGradient.setColorAt(1.0, Qt::red);
    // outer circle
    QBrush brush(conicalGradient);
    painter.setPen(Qt::NoPen);
    painter.setBrush(brush);
    painter.drawEllipse(center, r/2.0 - margin, r/2.0 - margin);
    // inner circle
    painter.setBrush(background);
    painter.drawEllipse(center, r/2.0 - margin - wheelWidth, r/2.0 - margin - wheelWidth);
    
    // debug layout: draw diagonal cross
    //painter.setPen(QColor("gray"));
    //painter.drawLine(QPointF(0.0, 0.0), QPointF(r, r));
    //painter.drawLine(QPointF(0.0, r), QPointF(r, 0.0));

    // calculate wheel region
    wheelRegion = QRegion(r/2, r/2, r-2*margin, r-2*margin, QRegion::Ellipse);
    wheelRegion.translate(-(r-2*margin)/2, -(r-2*margin)/2);
    int tmp = 2*(margin+wheelWidth);
    QRegion subRe(r/2, r/2, r-tmp, r-tmp, QRegion::Ellipse);
    subRe.translate(-(r-tmp)/2, -(r-tmp)/2);
    wheelRegion -= subRe;
}

void ColorWheel::drawSquareImage(const int& hue) {
    QImage square(255,255, QImage::Format_ARGB32_Premultiplied);
    QColor color;
    QRgb vv;

    for (int i=0; i<255; ++i) {
        for (int j=0; j<255; ++j) {
            color = QColor::fromHsv(hue,i,j);
            vv = qRgb(color.red(),color.green(),color.blue());
            square.setPixel(i,j,vv);
        }
    }

    // region of the widget
    int w = qMin(width(), height());
    // radius of outer circle
    qreal r = w/2.0 - margin;
    // radius of inner circle
    qreal ir = r - wheelWidth;
    // left corner of square
    qreal m = w/2.0 - ir/qSqrt(2.0);

    QRectF rect;
    rect.setTopLeft(QPointF(m, m));
    rect.setWidth(2.0 * ir / qSqrt(2.0));
    rect.setHeight(2.0 * ir / qSqrt(2.0));
    squareRegion = QRegion(rect.toRect());
    squareImage = square;
}

void ColorWheel::drawIndicator(const int& hue) {
    QPainter painter(&wheel);
    painter.setRenderHint(QPainter::Antialiasing);

    if (hue > 20 && hue < 200)
        painter.setPen(Qt::black);
    else
        painter.setPen(Qt::white);

    painter.setBrush(Qt::NoBrush);
    QPen pen = painter.pen();
    pen.setWidth(2);
    pen.setCosmetic(true);
    painter.setPen(pen);
    qreal r = qMin(height(), width()) / 2.0;
    painter.translate(r, r);
    painter.rotate(-hue);
    r = qMin(height(), width()) / 2.0 - margin - wheelWidth/2;
    painter.drawEllipse(QPointF(r,0.0),5,5);
}

void ColorWheel::drawPicker(const QColor& color) {
    QPainter painter(&wheel);
    painter.setRenderHint(QPainter::Antialiasing);
    QPen pen;
    // region of the widget
    int w = qMin(width(), height());
    // radius of outer circle
    qreal r = w/2-margin;
    // radius of inner circle
    qreal ir = r-wheelWidth;
    // left corner of square
    qreal m = w/2.0-ir/qSqrt(2);
    painter.translate(m-5, m-5);
    qreal SquareWidth = 2*ir/qSqrt(2);
    qreal S = color.saturationF()*SquareWidth;
    qreal V = color.valueF()*SquareWidth;

    if (color.saturation() > 30 ||color.value() < 50)
        pen.setColor(Qt::white);

    pen.setWidth(2);
    pen.setCosmetic(true);
    painter.setPen(pen);
    painter.drawEllipse(S,V,10,10);
}

void ColorWheel::composeWheel() {
    QPainter composePainter(&wheel);
    composePainter.drawImage(0, 0, wheelImage);
    composePainter.drawImage(squareRegion.boundingRect(), squareImage);
    composePainter.end();
    drawIndicator(current.hue());
    drawPicker(current);
}

void ColorWheel::hueChanged(const int& hue) {
    if (hue < 0 || hue > 359) return;

    int s = current.saturation();
    int v = current.value();
    current.setHsv(hue, s, v);

    if (!isVisible()) return;

    drawSquareImage(hue);
    repaint();
    emit colorChange(current);
}

void ColorWheel::svChanged(const QColor& newcolor) {
    int hue = current.hue();
    current.setHsv(hue, newcolor.saturation(),
                   newcolor.value());

    if (!isVisible()) return;

    repaint();
    emit colorChange(current);
}
} //namespace