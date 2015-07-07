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
#include <cmath>

namespace inviwo {

template <class T>
inline T clamp(T value, T lower, T higher) {
    return std::min(std::max(value, lower), higher);
}

ColorWheel::ColorWheel(QWidget* parent) :
    QWidget(parent),
    initSize_(200,200),
    mouseDown_(false),
    margin_(0),
    wheelWidth_(30),
    currentColor_(Qt::red),
    inWheel_(false),
    inSquare_(false) {
    
    currentColor_ = currentColor_.toHsv();
    updateSquareImage(currentColor_.hue());

    setMinimumSize(initSize_);
    setMaximumSize(initSize_);
    setCursor(Qt::CrossCursor);
}

QColor ColorWheel::color() {
    return currentColor_;
}

void ColorWheel::setColor(const QColor& color) {
    if (color == currentColor_) return;

    if (color.hue() != currentColor_.hue())
        hueChanged(color.hue());

    if (color.saturation() != currentColor_.saturation()
        || color.value() != currentColor_.value())
        svChanged(color);

    update();
    emit colorChange(color);
}


QColor ColorWheel::posColor(const QPoint& point) {
    if (! this->rect().contains(point)) return QColor();

    if (inWheel_) {
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
                               currentColor_.saturation(),
                               currentColor_.value());
    }

    if (inSquare_) {
        // region of the widget
        int w = qMin(width(), height());
        // radius of outer circle
        qreal r = w/2-margin_;
        // radius of inner circle
        qreal ir = r-wheelWidth_;
        // left corner of square
        qreal m = w/2.0 - ir/qSqrt(2);
        QPoint p = point - QPoint(m, m);
        qreal SquareWidth = (ir * qSqrt(2));
        return QColor::fromHsv(currentColor_.hueF(),
                               clamp(static_cast<int>(p.x() / SquareWidth * 255.0), 0, 255),
                               clamp(static_cast<int>(p.y() / SquareWidth * 255.0), 0, 255));
    }
    
    return QColor();
}

QSize ColorWheel::sizeHint() const {
    return QSize(height(),height());
}

QSize ColorWheel::minimumSizeHint() const {
    return initSize_;
}

void ColorWheel::mousePressEvent(QMouseEvent* e) {
    lastPos_ = e->pos();

    if (wheelRegion_.contains(lastPos_)) {
        inWheel_ = true;
        inSquare_ = false;
        QColor color = posColor(lastPos_);

        if (e->button()==Qt::RightButton) hueChanged(color.hue());
        else setColor(color);
    } else if (squareRegion_.contains(lastPos_)) {
        inWheel_ = false;
        inSquare_ = true;
        QColor color = posColor(lastPos_);
        svChanged(color);
    }

    mouseDown_ = true;
}

void ColorWheel::mouseMoveEvent(QMouseEvent* e) {
    lastPos_ = e->pos();

    if (!mouseDown_) return;

    if (wheelRegion_.contains(lastPos_) && inWheel_) {
        QColor color = posColor(lastPos_);
        hueChanged(color.hue());
    } else if (squareRegion_.contains(lastPos_) && inSquare_) {
        QColor color = posColor(lastPos_);
        svChanged(color);
    } else {
        int length = qMin(width(), height());
        QPoint center(length/2, length/2);

        if (inWheel_) {
            int r =  length / 2;
            double x0 = lastPos_.x()-center.x();
            double y0 = lastPos_.y()-center.y();
            double vNorm = qSqrt(qPow(x0,2)+qPow(y0,2));
            double x1 = r*(x0/vNorm);
            double y1 = r*(y0/vNorm);
            x1 += center.x();
            y1 += center.y();
            QColor color = posColor(QPoint(x1, y1));
            hueChanged(color.hue());
        } else if (inSquare_) {
            int w = qMin(width(), height());
            // radius of outer circle
            qreal r = w/2-margin_;
            // radius of inner circle
            qreal ir = r-wheelWidth_;
            // left corner of square
            qreal m = w/2.0-ir/qSqrt(2);
            float x0 = 0.0f;
            float y0 = 0.0f;
            float w0 = squareRegion_.boundingRect().width();

            if (lastPos_.x() > m + w0)
                x0 = m + w0;
            else if (lastPos_.x() < m)
                x0 = m;
            else
                x0 = lastPos_.x();

            if (lastPos_.y() > m + w0)
                y0 = m + w0;
            else if (lastPos_.y() < m)
                y0 = m;
            else
                y0 = lastPos_.y();

            QColor color = posColor(QPoint(x0, y0));
            svChanged(color);
        }
    }
}

void ColorWheel::mouseReleaseEvent(QMouseEvent*) {
    mouseDown_ = false;
    inWheel_ = false;
    inSquare_ = false;
}

void ColorWheel::resizeEvent(QResizeEvent* event) {
    // update wheel region
    QSize size(event->size());
    int diameter = qMin(size.width(), size.height());
    wheelRegion_ = QRegion(diameter/2, diameter/2, diameter-2*margin_, diameter-2*margin_, QRegion::Ellipse);
    wheelRegion_.translate(-(diameter-2*margin_)/2, -(diameter-2*margin_)/2);
    int tmp = 2*(margin_+wheelWidth_);
    QRegion subRe(diameter/2, diameter/2, diameter-tmp, diameter-tmp, QRegion::Ellipse);
    subRe.translate(-(diameter-tmp)/2, -(diameter-tmp)/2);
    wheelRegion_ -= subRe;
    
    // update square region
    int w = qMin(width(), height());
    qreal r = w/2.0 - margin_; // radius of outer circle
    qreal ir = r - wheelWidth_; // radius of inner circle
    qreal m = w/2.0 - ir/std::sqrt(2.0); // left corner of square

    QRectF rect;
    rect.setTopLeft(QPointF(m, m));
    rect.setWidth(2.0 * ir / std::sqrt(2.0));
    rect.setHeight(2.0 * ir / std::sqrt(2.0));
    squareRegion_ = QRegion(rect.toRect());
}

void ColorWheel::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QStyleOption opt;    
    opt.initFrom(this);
    // use the following lines to obtain the background color of the widget
    //QBrush background = opt.palette.window();
    //wheel.fill(background.color());

    drawOuterRing(painter);
    painter.drawImage(squareRegion_.boundingRect(), squareImage_);

    drawIndicator(painter);
    drawPicker(painter);

    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
}

void ColorWheel::drawOuterRing(QPainter &painter) {
    int r = qMin(this->width(), this->height());
    QPointF center(r/2.0, r/2.0);

    // paint hue wheel
    painter.save();
    QConicalGradient conicalGradient(center, 0);
    conicalGradient.setColorAt(0.0, Qt::red);
    conicalGradient.setColorAt(60.0/360.0, Qt::yellow);
    conicalGradient.setColorAt(120.0/360.0, Qt::green);
    conicalGradient.setColorAt(180.0/360.0, Qt::cyan);
    conicalGradient.setColorAt(240.0/360.0, Qt::blue);
    conicalGradient.setColorAt(300.0/360.0, Qt::magenta);
    conicalGradient.setColorAt(1.0, Qt::red);
    QBrush brush(conicalGradient);
    painter.setPen(Qt::NoPen);
    painter.setBrush(brush);

    // outer circle
    QPainterPath path;
    path.addEllipse(center, r/2.0 - margin_, r/2.0 - margin_);
    // inner circle
    path.addEllipse(center, r/2.0 - margin_ - wheelWidth_, r/2.0 - margin_ - wheelWidth_);
    painter.drawPath(path);
    painter.restore();
}

void ColorWheel::updateSquareImage(const int& hue) {
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
    squareImage_ = square;
}

void ColorWheel::drawIndicator(QPainter &painter) {
    const int hue = currentColor_.hue();

    if (hue > 20 && hue < 200)
        painter.setPen(Qt::black);
    else
        painter.setPen(Qt::white);

    painter.save();
    painter.setBrush(Qt::NoBrush);
    QPen pen = painter.pen();
    pen.setWidth(2);
    pen.setCosmetic(true);
    painter.setPen(pen);
    qreal r = qMin(height(), width()) / 2.0;
    painter.translate(r, r);
    painter.rotate(-hue);
    r = qMin(height(), width()) / 2.0 - margin_ - wheelWidth_/2;
    painter.drawEllipse(QPointF(r,0.0),5,5);
    painter.restore();
}

void ColorWheel::drawPicker(QPainter &painter) {
    painter.save();
    painter.setBrush(Qt::NoBrush);
    QPen pen;
    // region of the widget
    int w = qMin(width(), height());
    qreal r = w/2-margin_; // radius of outer circle
    qreal ir = r-wheelWidth_; // radius of inner circle
    qreal m = w/2.0-ir/qSqrt(2); // left corner of square
    painter.translate(m-5, m-5);
    qreal SquareWidth = 2*ir/qSqrt(2);
    qreal S = currentColor_.saturationF()*SquareWidth;
    qreal V = currentColor_.valueF()*SquareWidth;

    if (currentColor_.saturation() > 30 || currentColor_.value() < 50)
        pen.setColor(Qt::white);

    pen.setWidth(2);
    pen.setCosmetic(true);
    painter.setPen(pen);
    painter.drawEllipse(S,V,10,10);
    painter.restore();
}

void ColorWheel::hueChanged(const int& hue) {
    if (hue < 0 || hue > 359) return;

    int s = currentColor_.saturation();
    int v = currentColor_.value();
    currentColor_.setHsv(hue, s, v);
    updateSquareImage(hue);

    if (!isVisible()) return;

    repaint();
    emit colorChange(currentColor_);
}

void ColorWheel::svChanged(const QColor& newcolor) {
    int hue = currentColor_.hue();
    currentColor_.setHsv(hue, newcolor.saturation(),
                   newcolor.value());

    if (!isVisible()) return;

    repaint();
    emit colorChange(currentColor_);
}

} //namespace
