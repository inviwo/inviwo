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

#include <inviwo/qt/widgets/lightpositionwidgetqt.h>

namespace inviwo {


LightPositionWidgetQt::LightPositionWidgetQt() : QLabel(), mouseDown_(false) {
    generateWidget();
}

LightPositionWidgetQt::~LightPositionWidgetQt() {
    delete painter_;
    delete gradient_;
    delete gradientPixmap_;
}

void inviwo::LightPositionWidgetQt::generateWidget() {
    gradientPixmap_ = new QPixmap(100,100);
    gradientPixmap_->fill(Qt::transparent);
    this->setFixedWidth(100);
    this->setFixedHeight(100);
    gradient_ = new QRadialGradient(50, 50, 50, 50, 50);
    gradient_->setColorAt(0, QColor::fromRgbF(1, 1, 1, 1));
    gradient_->setColorAt(1, QColor::fromRgbF(0, 0, 0, 0));
    radius_ = gradient_->radius();
    painter_ = new QPainter(gradientPixmap_);
    painter_->fillRect(0, 0, 100, 100, *gradient_);
    this->setPixmap(*gradientPixmap_);    
}


void LightPositionWidgetQt::mousePressEvent(QMouseEvent* e) {
    mouseDown_ = true;
}

void LightPositionWidgetQt::mouseMoveEvent(QMouseEvent* e) {
    if (!mouseDown_) return;
    setNewPosition(e);
}

void LightPositionWidgetQt::mouseReleaseEvent(QMouseEvent* e) {
    setNewPosition(e);
    mouseDown_ = false;
}

void LightPositionWidgetQt::setNewPosition(QMouseEvent* e) {
    const QPoint p = e->pos();
    QPoint center(gradientPixmap_->width()/2, gradientPixmap_->height()/2);
    float x = p.x()-center.x();
    float y = p.y()-center.y();
    float gradientSpaceRadius = sqrt(x*x+y*y);

    // Check if user clicked close to, or outside of radius
    if (gradientSpaceRadius + 3.f > gradient_->radius()) {
        // User clicked outside of radius so we need to normalize x,y coordinate
        // Add a small number to avoid gradient on the border
        float normFactor = gradient_->radius()/(gradientSpaceRadius + 3.f);
        x *= normFactor;
        y *= normFactor;
    }

    float z = sqrt(gradient_->radius()*gradient_->radius() - x*x - y*y);
    theta_ = acos(z/gradient_->radius());
    phi_ = atan2(y, x);
    // Spherical to cartesian coordinates
    float x1=sin(theta_)*cos(phi_)*gradient_->radius();
    float y1=sin(theta_)*sin(phi_)*gradient_->radius();
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

    if (radius_==0)
        return;

    QPointF center(gradientPixmap_->width()/2, gradientPixmap_->height()/2);
    theta_ = acos(p.z/std::abs(radius_));
    phi_ = atan2(-p.y, p.x);
    float x1=sin(theta_)*cos(phi_)*gradient_->radius();
    float y1=sin(theta_)*sin(phi_)*gradient_->radius();
    float gradientSpaceRadius = sqrt(pow(x1,2)+pow(y1,2));

    // Check if user clicked close to, or outside of radius
    if (gradientSpaceRadius+3.f > gradient_->radius()) {
        float normFactor = gradient_->radius()/(3.f+gradientSpaceRadius);
        x1 *= normFactor;
        y1 *= normFactor;
    }

    QPointF newPoint(x1,y1);
    gradient_->setFocalPoint(newPoint + center);
    gradientPixmap_->fill(Qt::transparent);
    painter_->fillRect(0, 0, 100, 100, *gradient_);
    this->setPixmap(*gradientPixmap_);
}

vec3 LightPositionWidgetQt::getPosition() const {
    return std::abs(radius_ ) * vec3(getX(), getY(), getZ());
}

void LightPositionWidgetQt::setRadius(float radius) {
    radius_ = radius; 
    emit positionChanged();
}

float LightPositionWidgetQt::getRadius() const {
    return radius_;
}

float LightPositionWidgetQt::getX() const {
    return sin(theta_)*cos(phi_);
}

float LightPositionWidgetQt::getY() const {
    return -sin(theta_)*sin(phi_);
}

float LightPositionWidgetQt::getZ() const {
    return (radius_ < 0.0f ? -1.0f : 1.0f) * cos(theta_);
}

void LightPositionWidgetQt::mouseDoubleClickEvent(QMouseEvent* e) {
    radius_ *= -1.0;
    emit positionChanged();
}

}//namespace