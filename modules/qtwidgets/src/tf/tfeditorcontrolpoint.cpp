/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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

#include <modules/qtwidgets/tf/tfeditorcontrolpoint.h>
#include <modules/qtwidgets/tf/tfcontrolpointconnection.h>
#include <modules/qtwidgets/tf/tfeditor.h>
#include <modules/qtwidgets/tf/tfeditorview.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QGraphicsScene>
#include <QPainter>
#include <warn/pop>

namespace inviwo {

TFEditorControlPoint::TFEditorControlPoint(TFPrimitive& primitive, QGraphicsScene* scene,
                                           double size)
    : TFEditorPrimitive(primitive, scene, vec2(primitive.getPosition(), primitive.getAlpha()),
                        size) {
    data_.addObserver(this);
}

void TFEditorControlPoint::onTFPrimitiveChange(const TFPrimitive& p) {
    setTFPosition(vec2(p.getPosition(), p.getAlpha()));
}

QRectF TFEditorControlPoint::boundingRect() const {
    double bBoxSize = getSize() + 5.0;  //<! consider size of pen
    auto bRect = QRectF(-bBoxSize / 2.0, -bBoxSize / 2.0, bBoxSize, bBoxSize);

    return bRect;
}

QPainterPath TFEditorControlPoint::shape() const {
    QPainterPath path;
    const auto radius = getSize() * 0.5 + 1.5;  //<! consider size of pen
    path.addEllipse(QPointF(0.0, 0.0), radius, radius);
    return path;
}

void TFEditorControlPoint::paintPrimitive(QPainter* painter) {
    const auto radius = getSize() * 0.5;
    painter->drawEllipse(QPointF(0.0, 0.0), radius, radius);
}

QPointF TFEditorControlPoint::prepareItemPositionChange(const QPointF& pos) {
    QPointF adjustedPos(pos);

    if (auto tfe = qobject_cast<TFEditor*>(scene())) {
        QRectF rect = scene()->sceneRect();
        const double d = 2.0 * rect.width() * glm::epsilon<float>();

        const int moveMode = tfe->getMoveMode();

        // need to update position prior to updating connections
        currentPos_ = adjustedPos;

        if (left_) {
            if (left_->left_ && *(left_->left_) > *this) {
                switch (moveMode) {
                    case 0:  // Free
                        break;
                    case 1:  // Restrict
                        adjustedPos.setX(left_->left_->getCurrentPos().x() + d);
                        // need to update position prior to updating connections
                        currentPos_ = adjustedPos;
                        break;
                    case 2:  // Push
                        left_->left_->setPos(
                            QPointF(adjustedPos.x() - d, left_->left_->getCurrentPos().y()));
                        break;
                }

                tfe->updateConnections();
            } else {
                currentPos_ = adjustedPos;
                left_->updateShape();
            }
        }
        if (right_) {
            if (right_->right_ && *(right_->right_) < *this) {
                switch (moveMode) {
                    case 0:  // Free
                        break;
                    case 1:  // Restrict
                        adjustedPos.setX(right_->right_->getCurrentPos().x() - d);
                        // need to update position prior to updating connections
                        currentPos_ = adjustedPos;
                        break;
                    case 2:  // Push
                        right_->right_->setPos(
                            QPointF(adjustedPos.x() + d, right_->right_->getCurrentPos().y()));
                        break;
                }

                tfe->updateConnections();
            } else {
                right_->updateShape();
            }
        }
    }

    return adjustedPos;
}

void TFEditorControlPoint::onItemPositionChange(const vec2& newPos) {
    data_.setPositionAlpha(newPos);
}

void TFEditorControlPoint::onItemSceneHasChanged() { onTFPrimitiveChange(data_); }

bool operator==(const TFEditorControlPoint& lhs, const TFEditorControlPoint& rhs) {
    return lhs.data_ == rhs.data_;
}

bool operator!=(const TFEditorControlPoint& lhs, const TFEditorControlPoint& rhs) {
    return !operator==(lhs, rhs);
}

bool operator<(const TFEditorControlPoint& lhs, const TFEditorControlPoint& rhs) {
    return lhs.currentPos_.x() < rhs.currentPos_.x();
}

bool operator>(const TFEditorControlPoint& lhs, const TFEditorControlPoint& rhs) {
    return rhs < lhs;
}

bool operator<=(const TFEditorControlPoint& lhs, const TFEditorControlPoint& rhs) {
    return !(rhs < lhs);
}

bool operator>=(const TFEditorControlPoint& lhs, const TFEditorControlPoint& rhs) {
    return !(lhs < rhs);
}

}  // namespace inviwo
