/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2019 Inviwo Foundation
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

#include <inviwo/qt/editor/processorgraphicsitem.h>
#include <inviwo/qt/editor/processorportgraphicsitem.h>
#include <inviwo/qt/editor/processorstatusgraphicsitem.h>
#include <inviwo/qt/editor/networkeditor.h>
#include <inviwo/core/ports/port.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/settings/systemsettings.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/qt/editor/connectiongraphicsitem.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QPen>
#include <QPainter>
#include <QPainterPath>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsDropShadowEffect>
#include <warn/pop>

namespace inviwo {

ProcessorPortGraphicsItem::ProcessorPortGraphicsItem(ProcessorGraphicsItem* parent,
                                                     const QPointF& pos, bool up, Port* port)
    : EditorGraphicsItem(parent), processor_(parent), size_(9.0f), lineWidth_(1.0f) {

    setRect(-(0.5f * size_ + lineWidth_), -(0.5f * size_ + lineWidth_), size_ + 2.0 * lineWidth_,
            size_ + 2.0 * lineWidth_);
    setPos(pos);
    setFlags(ItemSendsScenePositionChanges);

    auto color = port->getColorCode();
    connectionIndicator_ =
        new ProcessorPortConnectionIndicator(this, up, QColor(color.r, color.g, color.b));
    connectionIndicator_->setVisible(false);
}

void ProcessorPortGraphicsItem::addConnection(ConnectionGraphicsItem* connection) {
    connections_.push_back(connection);
    connectionIndicator_->setVisible(true);
    updateConnectionPositions();
    update();  // we need to repaint the connection
}
void ProcessorPortGraphicsItem::removeConnection(ConnectionGraphicsItem* connection) {
    connections_.erase(std::find(connections_.begin(), connections_.end(), connection));
    connectionIndicator_->setVisible(!connections_.empty());
    update();  // we need to repaint the connection
}

QVariant ProcessorPortGraphicsItem::itemChange(GraphicsItemChange change, const QVariant& value) {
    if (change == QGraphicsItem::ItemScenePositionHasChanged) {
        updateConnectionPositions();
    }
    return EditorGraphicsItem::itemChange(change, value);
}

std::vector<ConnectionGraphicsItem*>& ProcessorPortGraphicsItem::getConnections() {
    return connections_;
}

ProcessorGraphicsItem* ProcessorPortGraphicsItem::getProcessor() { return processor_; }

ProcessorPortGraphicsItem::~ProcessorPortGraphicsItem() = default;

ProcessorInportGraphicsItem::ProcessorInportGraphicsItem(ProcessorGraphicsItem* parent,
                                                         const QPointF& pos, Inport* inport)
    : ProcessorPortGraphicsItem(parent, pos, true, inport), inport_(inport) {}

Inport* ProcessorInportGraphicsItem::getPort() { return inport_; }

void ProcessorInportGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent* e) {
    if (e->buttons() == Qt::LeftButton && inport_->isConnected()) {
        getNetworkEditor()->releaseConnection(this);
    }
    e->accept();
}

void ProcessorInportGraphicsItem::updateConnectionPositions() {
    for (auto& elem : connections_) {
        elem->updateShape();
    }
}

void ProcessorInportGraphicsItem::showToolTip(QGraphicsSceneHelpEvent* e) {
    showPortInfo(e, inport_);
}

void ProcessorInportGraphicsItem::paint(QPainter* p, const QStyleOptionGraphicsItem*, QWidget*) {
    p->save();
    p->setRenderHint(QPainter::Antialiasing, true);
    p->setRenderHint(QPainter::SmoothPixmapTransform, true);

    QColor borderColor(40, 40, 40);

    uvec3 color = inport_->getColorCode();

    QRectF portRect(QPointF(-size_, size_) / 2.0f, QPointF(size_, -size_) / 2.0f);
    p->setBrush(QColor(color.r, color.g, color.b));
    p->setPen(QPen(borderColor, lineWidth_));

    if (inport_->isOptional()) {
        // Use a different shape for optional ports (rounded at the bottom)
        QPainterPath path;
        auto start = (portRect.topRight() + portRect.bottomRight()) * 0.5;
        path.moveTo(start);
        path.lineTo(portRect.bottomRight());
        path.lineTo(portRect.bottomLeft());
        path.lineTo((portRect.topLeft() + portRect.bottomLeft()) * 0.5);
        path.arcTo(portRect, 180, -180);
        p->drawPath(path);

        // Draw a dot in the middle
        const qreal radius = 1.5;
        QRectF dotRect(QPointF(-radius, radius), QPointF(radius, -radius));
        p->setPen(borderColor.lighter(180));
        p->setBrush(borderColor.lighter(180));
        p->drawEllipse(dotRect);

    } else {
        p->drawRect(portRect);
    }

    p->restore();
}

ProcessorOutportGraphicsItem::ProcessorOutportGraphicsItem(ProcessorGraphicsItem* parent,
                                                           const QPointF& pos, Outport* outport)
    : ProcessorPortGraphicsItem(parent, pos, false, outport), outport_(outport) {}

Outport* ProcessorOutportGraphicsItem::getPort() { return outport_; }

void ProcessorOutportGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent* e) {
    if (e->buttons() == Qt::LeftButton) {
        getNetworkEditor()->initiateConnection(this);
    }
    e->accept();
}

void ProcessorOutportGraphicsItem::updateConnectionPositions() {
    for (auto& elem : connections_) {
        elem->updateShape();
    }
}

void ProcessorOutportGraphicsItem::showToolTip(QGraphicsSceneHelpEvent* e) {
    showPortInfo(e, outport_);
}

void ProcessorOutportGraphicsItem::paint(QPainter* p, const QStyleOptionGraphicsItem*, QWidget*) {
    p->save();
    p->setRenderHint(QPainter::Antialiasing, true);
    p->setRenderHint(QPainter::SmoothPixmapTransform, true);

    QColor borderColor(40, 40, 40);
    uvec3 color = outport_->getColorCode();

    QRectF portRect(QPointF(-size_, size_) / 2.0f, QPointF(size_, -size_) / 2.0f);
    p->setBrush(QColor(color.r, color.g, color.b));
    p->setPen(QPen(borderColor, lineWidth_));
    p->drawRect(portRect);
    p->restore();
}

ProcessorPortConnectionIndicator::ProcessorPortConnectionIndicator(
    ProcessorPortGraphicsItem* parent, bool up, QColor color)
    : EditorGraphicsItem(parent), portConnectionItem_(parent), up_(up), color_(color) {

    setRect(-2.0f, -8.0f, 4.0f, 16.0f);
    setPos(0.0f, 0.0f);
}

void ProcessorPortConnectionIndicator::paint(QPainter* p, const QStyleOptionGraphicsItem*,
                                             QWidget*) {
    p->save();
    p->setRenderHint(QPainter::Antialiasing, true);

    const bool selected = util::any_of(portConnectionItem_->getConnections(),
                                       [](auto& connection) { return connection->isSelected(); });

    const float width{selected ? (4.0f - 1.0f) / 2.0f : (3.0f - 0.5f) / 2.0f};
    const float length = 7.0f;

    QPainterPath path;
    if (up_) {
        path.moveTo(-width, -length);
        path.lineTo(-width, 0.0f);
        path.arcTo(QRectF(-width, -width, 2 * width, 2 * width), 180.0f, 180.0f);
        path.lineTo(width, -length);
    } else {
        path.moveTo(-width, length);
        path.lineTo(-width, 0.0f);
        path.arcTo(QRectF(-width, -width, 2 * width, 2 * width), 180.0f, -180.0f);
        path.lineTo(width, length);
    }
    QPainterPath closedPath(path);
    closedPath.closeSubpath();

    QLinearGradient gradBrush(QPointF(0.0f, 0.0f), QPointF(0.0, up_ ? -length : length));
    gradBrush.setColorAt(0.0f, color_);
    gradBrush.setColorAt(0.75f, color_);
    gradBrush.setColorAt(1.0f, QColor(color_.red(), color_.green(), color_.blue(), 0));

    QLinearGradient gradPen(QPointF(0.0f, 0.0f), QPointF(0.0, up_ ? -length : length));
    QColor lineColor = selected ? Qt::darkRed : Qt::black;
    gradPen.setColorAt(0.0f, lineColor);
    gradPen.setColorAt(0.75f, lineColor);
    gradPen.setColorAt(1.0f, QColor(lineColor.red(), lineColor.green(), lineColor.blue(), 0));

    p->setPen(Qt::NoPen);
    p->setBrush(color_);
    p->drawPath(closedPath);

    p->setBrush(Qt::NoBrush);
    p->setPen(QPen(lineColor, selected ? 1.0f : 0.5f));
    p->drawPath(path);

    p->restore();
}

}  // namespace inviwo
