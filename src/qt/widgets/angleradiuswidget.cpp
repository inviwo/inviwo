/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#include <inviwo/core/common/inviwo.h>
#include <inviwo/qt/widgets/angleradiuswidget.h>
#include <cmath>
#include <iostream>
#include <sstream>
#include <warn/push>
#include <warn/ignore/all>
#include <QStylePainter>
#include <QPointF>
#include <QMouseEvent>
#include <warn/pop>

namespace inviwo {


AngleRadiusWidget::AngleRadiusWidget(QWidget* parent)
    : QWidget(parent), angle_(0.0), radius_(0.8), minRadius_(0.), maxRadius_(1.)  {
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
    resize(200, 200);
}
QSize AngleRadiusWidget::sizeHint() const {
    return QSize(120, 100);
}

QSize AngleRadiusWidget::minimumSizeHint() const {
    return QSize(50, 50);
}

void AngleRadiusWidget::paintEvent(QPaintEvent *) {
    const int referenceRadius = getMaxPixelSpaceRadius();
    const double pixelSpaceRadius =  referenceRadius*(getRadius()/getMaxRadius());
    const qreal angleIndicatorCircleDiameter = 6.;

    QPen angleIndicatorPen(palette().midlight(), 2, Qt::SolidLine,  Qt::SquareCap, Qt::MiterJoin);
    QPen coordinateSystemPen(palette().alternateBase(), 1, Qt::SolidLine,  Qt::SquareCap, Qt::MiterJoin);
    QPen circleBoundsPen(palette().alternateBase(), 1, Qt::DashLine,  Qt::SquareCap, Qt::MiterJoin);

    // x and y in pixel coordinates
    auto x = pixelSpaceRadius*std::cos(getAngle());
    auto y = -pixelSpaceRadius*std::sin(getAngle());

    int side = qMin(width(), height());

    QStylePainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.translate(width() / 2, height() / 2);
    painter.scale(side / (qreal)100., side / (qreal)100.);

    
    
    painter.setPen(coordinateSystemPen);
    // Draw axes and bounds circle(s)
    for (int i = 0; i < 4; ++i) {
        painter.drawLine(0, 0, referenceRadius+5, 0);
        painter.rotate(90.0);
    }
    painter.setPen(circleBoundsPen);
    // Display angle bounds by drawing a pie (pacman) if min/max is not 0/2pi 
    int innerBoundsCircleRadius = static_cast<int>(static_cast<double>(referenceRadius)*(getMinRadius()/getMaxRadius()));
    if (getMinAngle() > 0. || getMaxAngle() < 2.*M_PI) {
        // drawPie wants 16*degrees
        int pieStart = static_cast<int>( 16*glm::degrees(getMinAngle()) );
        int pieEnd = static_cast<int>( 16*(glm::degrees(getMaxAngle())-glm::degrees(getMinAngle())) );
        painter.drawPie(-referenceRadius, -referenceRadius, 2*referenceRadius, 2*referenceRadius, pieStart, pieEnd);
        if (minRadius_ > 0.) {
            painter.drawPie(-innerBoundsCircleRadius, -innerBoundsCircleRadius, 2*innerBoundsCircleRadius, 2*innerBoundsCircleRadius, pieStart, pieEnd);
        }
    } else {
        painter.drawEllipse(-referenceRadius, -referenceRadius, 2*referenceRadius, 2*referenceRadius);
        if (minRadius_ > 0.) {
            painter.drawEllipse(-innerBoundsCircleRadius, -innerBoundsCircleRadius, 2*innerBoundsCircleRadius, 2*innerBoundsCircleRadius);
        }
    }



    // Draw angle and radius 
    painter.setPen(QPen(palette().text().color()));
    QPainterPath anglePath;
    // Make sure that angle goes from 0 to 2*pi
    double sweepAngle = getAngle() < 0. ? 2.*M_PI+getAngle() : getAngle();
    // Draw angle arc and text
    anglePath.arcTo(-10., -10., 20., 20., 0., glm::degrees(sweepAngle));
    painter.drawPath(anglePath);
    std::stringstream angleStream; angleStream.precision(1); angleStream << std::fixed << glm::degrees(sweepAngle);
    int anglePosX = 5;
    int anglePosY = -10;
    QFontMetrics fm(painter.font());
    QString angleText = QString::fromStdString(angleStream.str())+ QChar(0260);
    int angleTextWidth = fm.width(angleText);
    if (anglePosX+angleTextWidth > referenceRadius) {
        anglePosX = referenceRadius+2;
        anglePosY = -2;
    }

    // 0260 is the degree symbol
    painter.drawText(anglePosX, anglePosY, angleText);
    // Draw radius text
    std::stringstream radiusStream; radiusStream.precision(2); radiusStream << getRadius();
    painter.drawText(static_cast<int>(x+angleIndicatorCircleDiameter+angleIndicatorPen.width()), 
                     static_cast<int>(y), QString::fromStdString(radiusStream.str()));
    // Rotated line and circle
    painter.setPen(angleIndicatorPen);   
    painter.drawLine(QLineF(QPointF(0., 0.), QPointF(x, y)));
 
    painter.setBrush(QBrush(palette().shadow().color(), Qt::SolidPattern));
    painter.drawEllipse(QPointF(x, y), angleIndicatorCircleDiameter, angleIndicatorCircleDiameter);
}

void AngleRadiusWidget::mouseMoveEvent(QMouseEvent* e) {
    setAngleRadiusAtPosition(e->pos());
}

void AngleRadiusWidget::mousePressEvent(QMouseEvent* e) {
    if (e->button() == Qt::LeftButton)
        setAngleRadiusAtPosition(e->pos());
}

void AngleRadiusWidget::setAngle(double angle) {
    if (angle_ != angle) {
        angle_ = glm::clamp(angle, minAngle_, maxAngle_);;
        update();
        emit angleChanged();
    } 
}

void AngleRadiusWidget::setRadius(double radius) {
    if (radius_ != radius) {
        radius_ = glm::clamp(radius, minRadius_, maxRadius_);
        update();
        emit radiusChanged();
    } 
}


void AngleRadiusWidget::setMinMaxAngle(double minAngle, double maxAngle) {
    if (minAngle_ != minAngle || maxAngle_ != maxAngle) {
        minAngle_ = minAngle;
        maxAngle_ = maxAngle;
        // Update radius to be within bounds
        setAngle(glm::clamp(getAngle(), getMinAngle(), getMaxAngle()));
        update();
        emit angleMinMaxChanged();
    } 
}

void AngleRadiusWidget::setMinMaxRadius(double minRadius, double maxRadius) {
    if (minRadius_ != minRadius || maxRadius_ != maxRadius) {
        minRadius_ = minRadius;
        maxRadius_ = maxRadius;
        // Update radius to be within bounds
        setRadius(glm::clamp(getRadius(), getMinRadius(), getMaxRadius()));
        update();
        emit radiusMinMaxChanged();
    } 
}

void AngleRadiusWidget::setMaxRadius(double radius) {
    if (maxRadius_ != radius) {
        maxRadius_ = radius;
        // Update radius to be within bounds
        setRadius(std::min(getRadius(), getMaxRadius()));
        update();
        emit radiusMinMaxChanged();
    } 
}

void AngleRadiusWidget::setMinRadius(double radius) {
    if (minRadius_ != radius) {
        minRadius_ = radius;
        // Update radius to be within bounds
        setRadius(std::max(getRadius(), getMinRadius()));
        update();
        emit radiusMinMaxChanged();
    } 
}

int AngleRadiusWidget::getMaxPixelSpaceRadius() const {
    return sizeHint().height()/2-10;
}

void AngleRadiusWidget::setMinAngle(double angle) {
    if (minAngle_ != angle) {
        minAngle_ = angle;
        // Update angle to be within bounds
        setAngle(std::max(getAngle(), getMinAngle()));
        update();
        emit angleMinMaxChanged();
    } 
}

void AngleRadiusWidget::setMaxAngle(double angle) {
    if (maxAngle_ != angle) {
        maxAngle_ = angle;
        // Update angle to be within bounds
        setAngle(std::min(getAngle(), getMaxAngle()));
        update();
        emit angleMinMaxChanged();
    } 
}

void AngleRadiusWidget::setAngleRadiusAtPosition(const QPoint& pos) {
    QPoint center(width()/2, height()/2);
    QPoint pixelSpacePos = pos - center;
    // Scale point from pixel coordinates to lie in [-maxRadius maxRadius]
    // Flip y-coordinate to get it facing upward instead of downward
    double x =  getMaxRadius()*pixelSpacePos.x() / ((double)getMaxPixelSpaceRadius());
    double y = -getMaxRadius()*pixelSpacePos.y() / ((double)getMaxPixelSpaceRadius());

    double radius = std::sqrt(x*x+y*y);
    double theta = std::atan2(y, x);
    // Convert angle to lie within [0 2pi), http://en.wikipedia.org/wiki/Atan2
    if (theta < 0.) theta += 2.*M_PI;
    setAngle(theta);
    setRadius(radius);
}


}  // namespace
