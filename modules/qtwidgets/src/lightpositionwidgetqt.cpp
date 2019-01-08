/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2018 Inviwo Foundation
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

#include <modules/qtwidgets/lightpositionwidgetqt.h>
#include <warn/push>
#include <warn/ignore/all>
#include <QRadialGradient>
#include <QPixmap>
#include <QPainter>
#include <QBrush>
#include <QMouseEvent>
#include <QPoint>
#include <QtCore/qmath.h>
#include <warn/pop>

namespace inviwo {

LightPositionWidgetQt::LightPositionWidgetQt() : QLabel(), mouseDown_(false) {
    gradientPixmap_ = new QPixmap(100, 100);
    gradientPixmap_->fill(Qt::transparent);
    this->setFixedWidth(100);
    this->setFixedHeight(100);
    gradient_ = new QRadialGradient(50.0f, 50.0f, 50.0f, 50.0f, 50.0f);
    gradient_->setColorAt(0, QColor::fromRgbF(1.0f, 1.0f, 1.0f, 1.0f));
    gradient_->setColorAt(1, QColor::fromRgbF(0.0f, 0.0f, 0.0f, 0.0f));
    radius_ = static_cast<float>(gradient_->radius());
    painter_ = new QPainter(gradientPixmap_);
    painter_->fillRect(0, 0, 100, 100, *gradient_);
    this->setPixmap(*gradientPixmap_);
}

LightPositionWidgetQt::~LightPositionWidgetQt() {
    delete painter_;
    delete gradient_;
    delete gradientPixmap_;
}

void LightPositionWidgetQt::mousePressEvent(QMouseEvent*) { mouseDown_ = true; }

void LightPositionWidgetQt::mouseMoveEvent(QMouseEvent* e) {
    if (!mouseDown_) return;
    setNewPosition(e);
}

void LightPositionWidgetQt::mouseReleaseEvent(QMouseEvent* e) {
    setNewPosition(e);
    mouseDown_ = false;
}

void LightPositionWidgetQt::setNewPosition(QMouseEvent* e) {
    const QPointF p{e->pos()};
    QPointF center{gradientPixmap_->width() / 2.0f, gradientPixmap_->height() / 2.0f};
    float x = static_cast<float>(p.x() - center.x());
    float y = static_cast<float>(p.y() - center.y());
    float gradientSpaceRadius = sqrt(x * x + y * y);

    float gradientRadius = static_cast<float>(gradient_->radius());

    // Check if user clicked close to, or outside of radius
    if (gradientSpaceRadius + 3.f > gradientRadius) {
        // User clicked outside of radius so we need to normalize x,y coordinate
        // Add a small number to avoid gradient on the border
        float normFactor = gradientRadius / (gradientSpaceRadius + 3.f);
        x *= normFactor;
        y *= normFactor;
    }

    float z = sqrt(gradientRadius * gradientRadius - x * x - y * y);
    theta_ = acos(z / gradientRadius);
    phi_ = atan2(y, x);
    // Spherical to Cartesian coordinates
    float x1 = sin(theta_) * cos(phi_) * gradientRadius;
    float y1 = sin(theta_) * sin(phi_) * gradientRadius;
    QPointF newPoint(x1, y1);
    gradient_->setFocalPoint(newPoint + center);
    gradientPixmap_->fill(Qt::transparent);
    painter_->fillRect(0, 0, 100, 100, *gradient_);
    this->setPixmap(*gradientPixmap_);
    emit positionChanged();
}

void LightPositionWidgetQt::setPosition(const vec3& p) {
    radius_ = glm::length(p);
    radius_ *= p.z < 0.0f ? -1.0f : 1.0f;

    if (radius_ == 0) return;
    float gradientRadius = static_cast<float>(gradient_->radius());

    QPointF center(gradientPixmap_->width() / 2, gradientPixmap_->height() / 2);
    theta_ = acos(p.z / std::abs(radius_));
    phi_ = atan2(-p.y, p.x);
    float x1 = sin(theta_) * cos(phi_) * gradientRadius;
    float y1 = sin(theta_) * sin(phi_) * gradientRadius;
    float gradientSpaceRadius = sqrt(pow(x1, 2) + pow(y1, 2));

    // Check if user clicked close to, or outside of radius
    if (gradientSpaceRadius + 3.f > gradientRadius) {
        float normFactor = gradientRadius / (3.f + gradientSpaceRadius);
        x1 *= normFactor;
        y1 *= normFactor;
    }

    QPointF newPoint(x1, y1);
    gradient_->setFocalPoint(newPoint + center);
    gradientPixmap_->fill(Qt::transparent);
    painter_->fillRect(0, 0, 100, 100, *gradient_);
    this->setPixmap(*gradientPixmap_);
}

vec3 LightPositionWidgetQt::getPosition() const {
    return std::abs(radius_) * vec3(getX(), getY(), getZ());
}

void LightPositionWidgetQt::setRadius(float radius) {
    radius_ = radius;
    emit positionChanged();
}

float LightPositionWidgetQt::getRadius() const { return radius_; }

float LightPositionWidgetQt::getX() const { return sin(theta_) * cos(phi_); }

float LightPositionWidgetQt::getY() const { return -sin(theta_) * sin(phi_); }

float LightPositionWidgetQt::getZ() const { return (radius_ < 0.0f ? -1.0f : 1.0f) * cos(theta_); }

void LightPositionWidgetQt::mouseDoubleClickEvent(QMouseEvent*) {
    radius_ *= -1.0;
    emit positionChanged();
}

}  // namespace inviwo