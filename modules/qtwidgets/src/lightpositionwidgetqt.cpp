/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2021 Inviwo Foundation
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
#include <modules/qtwidgets/inviwoqtutils.h>
#include <inviwo/core/util/glm.h>
#include <inviwo/core/util/logcentral.h>

#include <cmath>

#include <warn/push>
#include <warn/ignore/all>
#include <QRadialGradient>
#include <QPixmap>
#include <QPainter>
#include <QBrush>
#include <QMouseEvent>
#include <QPoint>
#include <warn/pop>

namespace inviwo {

LightPositionWidgetQt::LightPositionWidgetQt()
    : QLabel()
    , size_{utilqt::emToPx(this, 7)}
    , pixmap_{size_, size_}
    , gradient_{size_ / 2.0, size_ / 2.0, size_ / 2.0, size_ / 2.0, size_ / 2.0}
    , pos_{0.0f}
    , radius_{0.0} {

    setFixedWidth(size_);
    setFixedHeight(size_);

    gradient_.setColorAt(0, QColor::fromRgbF(1.0f, 1.0f, 1.0f, 1.0f));
    gradient_.setColorAt(1, QColor::fromRgbF(0.0f, 0.0f, 0.0f, 0.0f));

    pixmap_.fill(Qt::transparent);
    QPainter painter(&pixmap_);
    painter.fillRect(pixmap_.rect(), gradient_);
    setPixmap(pixmap_);
}

LightPositionWidgetQt::~LightPositionWidgetQt() = default;

void LightPositionWidgetQt::mousePressEvent(QMouseEvent* e) {
    if (e->button() == Qt::MouseButton::LeftButton) {
        setNewPosition(e);
    }
}
void LightPositionWidgetQt::mouseMoveEvent(QMouseEvent* e) {
    if (e->buttons() & Qt::MouseButton::LeftButton) {
        setNewPosition(e);
    }
}
void LightPositionWidgetQt::mouseReleaseEvent(QMouseEvent* e) {
    if (e->button() == Qt::MouseButton::LeftButton) {
        setNewPosition(e);
    }
}

void LightPositionWidgetQt::setNewPosition(QMouseEvent* e) {
    const auto rect = QRectF{pixmap_.rect()};
    auto p = utilqt::toGLM(e->localPos() - rect.center()) / gradient_.radius();

    const auto radius = glm::length(p);
    if (radius > 1.0) {
        p /= radius;
    }
    pos_ = p;

    const auto focal = rect.center() + 0.999 * gradient_.radius() * QPointF(pos_.x, pos_.y);
    gradient_.setFocalPoint(focal);
    pixmap_.fill(Qt::transparent);
    QPainter painter(&pixmap_);
    painter.fillRect(pixmap_.rect(), gradient_);
    setPixmap(pixmap_);

    emit positionChanged();
}

void LightPositionWidgetQt::setPosition(const dvec3& p) {
    const auto rect = QRectF{pixmap_.rect()};
    radius_ = glm::length(p);
    pos_ = dvec2{p.x, -p.y} / (radius_ == 0.0 ? 1.0 : radius_);
    radius_ *= p.z < 0.0f ? -1.0f : 1.0f;

    const auto focal = rect.center() + 0.999 * gradient_.radius() * QPointF(pos_.x, pos_.y);
    gradient_.setFocalPoint(focal);
    pixmap_.fill(Qt::transparent);
    QPainter painter(&pixmap_);
    painter.fillRect(pixmap_.rect(), gradient_);
    setPixmap(pixmap_);
}

dvec3 LightPositionWidgetQt::getPosition() const {
    return std::abs(radius_) * dvec3{getX(), getY(), getZ()};
}

void LightPositionWidgetQt::setRadius(double radius) {
    radius_ = radius;
    emit positionChanged();
}

double LightPositionWidgetQt::getRadius() const { return radius_; }

double LightPositionWidgetQt::getX() const { return pos_.x; }

double LightPositionWidgetQt::getY() const { return -pos_.y; }

double LightPositionWidgetQt::getZ() const {
    return (radius_ < 0.0 ? -1.0 : 1.0) * std::sqrt(1.0 - glm::length(pos_));
}

void LightPositionWidgetQt::mouseDoubleClickEvent(QMouseEvent*) {
    radius_ *= -1.0;
    emit positionChanged();
}

}  // namespace inviwo