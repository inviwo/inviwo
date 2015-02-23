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

#include <inviwo/qt/editor/processorgraphicsitem.h>
#include <inviwo/qt/editor/processorportgraphicsitem.h>
#include <inviwo/qt/editor/processorstatusgraphicsitem.h>
#include <inviwo/qt/editor/networkeditor.h>
#include <inviwo/core/ports/port.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/settings/systemsettings.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/qt/editor/connectiongraphicsitem.h>

#include <QPen>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>

namespace inviwo {

ProcessorPortGraphicsItem::ProcessorPortGraphicsItem(ProcessorGraphicsItem* parent,
                                                     const QPointF& pos, bool up, Port* port)
    : EditorGraphicsItem(parent)
    , processor_(parent)
    , port_(port)
    , size_(9.0f)
    , lineWidth_(1.0f)
    , up_(up) {
    

    setCacheMode(QGraphicsItem::DeviceCoordinateCache);
    setRect(-0.5f * size_ - lineWidth_, -0.5f * size_ - lineWidth_, size_ + 2.0 * lineWidth_,
            size_ + 2.0 * lineWidth_);
    setPos(pos);
    setFlags(ItemSendsScenePositionChanges);
}

void ProcessorPortGraphicsItem::paint(QPainter* p, const QStyleOptionGraphicsItem* options,
                                      QWidget* widget) {
    p->save();
    p->setBrush(Qt::NoBrush);
    p->setRenderHint(QPainter::Antialiasing, true);

    QColor bottomColor(40, 40, 40);
    p->setPen(QPen(bottomColor, lineWidth_));

    uvec3 color = port_->getColorCode();

    QRectF portRect(QPointF(-size_, size_) / 2, QPointF(size_, -size_) / 2);
    QLinearGradient portGrad(portRect.topLeft(), portRect.bottomLeft());
    portGrad.setColorAt(0.0f, QColor(color.r * 0.6, color.g * 0.6, color.b * 0.6));
    portGrad.setColorAt(0.3f, QColor(color.r, color.g, color.b));
    portGrad.setColorAt(1.0f, QColor(color.r, color.g, color.b));
    p->setBrush(portGrad);
    p->drawRect(portRect);

    if (port_->isConnected()) {
        if (up_) {
            p->drawRect(portRect.adjusted(3, -3, -3, -0));
        } else {
            p->drawRect(portRect.adjusted(3, 0, -3, 3));
        }
    }

    p->restore();
}

void ProcessorPortGraphicsItem::addConnection(ConnectionGraphicsItem* connection) {
    connections_.push_back(connection);
    updateConnectionPositions();
    update(); // we need to repaint the connection
}
void ProcessorPortGraphicsItem::removeConnection(ConnectionGraphicsItem* connection) {
    connections_.erase(std::find(connections_.begin(), connections_.end(), connection));
    update(); // we need to repaint the connection
}

QVariant ProcessorPortGraphicsItem::itemChange(GraphicsItemChange change, const QVariant& value) {
    if (change == QGraphicsItem::ItemScenePositionHasChanged) {
        updateConnectionPositions();
    }
    return EditorGraphicsItem::itemChange(change, value);
}

std::vector<ConnectionGraphicsItem*> ProcessorPortGraphicsItem::getConnections() {
    return connections_;
}

ProcessorGraphicsItem* ProcessorPortGraphicsItem::getProcessor() { return processor_; }

ProcessorPortGraphicsItem::~ProcessorPortGraphicsItem() {}

void ProcessorPortGraphicsItem::showToolTip(QGraphicsSceneHelpEvent* e) {
    showPortInfo(e, port_);
}

ProcessorInportGraphicsItem::ProcessorInportGraphicsItem(ProcessorGraphicsItem* parent,
                                                         const QPointF& pos, Inport* port)
    : ProcessorPortGraphicsItem(parent, pos, true, port), port_(port) {}

Inport* ProcessorInportGraphicsItem::getPort() { return port_; }

void ProcessorInportGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent* e) {
    if(e->buttons() == Qt::LeftButton && port_->isConnected()) {
        NetworkEditor::getPtr()->releaseConnection(this);
    }
    e->accept();
}

void ProcessorInportGraphicsItem::updateConnectionPositions() {
    for (auto& elem : connections_) {
        elem->updateShape();
    }
}

void ProcessorInportGraphicsItem::showToolTip(QGraphicsSceneHelpEvent* e) {
    if (port_->isConnected()) {
        showPortInfo(e, port_->getConnectedOutport());
    } else {
        showPortInfo(e, port_);
    }
}

ProcessorOutportGraphicsItem::ProcessorOutportGraphicsItem(ProcessorGraphicsItem* parent,
                                                           const QPointF& pos, Outport* port)
    : ProcessorPortGraphicsItem(parent, pos, false, port), port_(port) {}

Outport* ProcessorOutportGraphicsItem::getPort() { return port_; }

void ProcessorOutportGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent* e) {
    if(e->buttons() == Qt::LeftButton) {
        NetworkEditor::getPtr()->initiateConnection(this);
    }
    e->accept();
}

void ProcessorOutportGraphicsItem::updateConnectionPositions() {
    for (auto& elem : connections_) {
        elem->updateShape();
    }
}


}  // namespace
