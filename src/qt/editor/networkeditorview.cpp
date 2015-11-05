/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#include <warn/push>
#include <warn/ignore/all>
#include <QMatrix>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QScrollBar>
#include <qmath.h>
#include <warn/pop>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/settings/linksettings.h>
#include <inviwo/core/util/settings/systemsettings.h>
#include <inviwo/qt/editor/networkeditorview.h>
#include <inviwo/core/network/processornetwork.h>

namespace inviwo {

NetworkEditorView::NetworkEditorView(NetworkEditor* networkEditor, QWidget* parent)
    : QGraphicsView(parent)
    , NetworkEditorObserver()
    , networkEditor_(networkEditor) {

    NetworkEditorObserver::addObservation(networkEditor_);
    QGraphicsView::setScene(networkEditor_);
    
    setRenderHint(QPainter::Antialiasing, true);
    setMouseTracking(true);
    setDragMode(QGraphicsView::RubberBandDrag);
    setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    setCacheMode(QGraphicsView::CacheBackground);

    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
}

NetworkEditorView::~NetworkEditorView() { QGraphicsView::setScene(nullptr); }

void NetworkEditorView::hideNetwork(bool hide) {
    if (hide) {
        if (scene()) {
            scrollPos_.x = horizontalScrollBar()->value();
            scrollPos_.y = verticalScrollBar()->value();
            QGraphicsView::setScene(nullptr);
        }
    } else {
        if (scene() != networkEditor_) {
            QGraphicsView::setScene(networkEditor_);
            horizontalScrollBar()->setValue(scrollPos_.x);
            verticalScrollBar()->setValue(scrollPos_.y);
        }
    }
}

void NetworkEditorView::mouseDoubleClickEvent(QMouseEvent* e) {
    QGraphicsView::mouseDoubleClickEvent(e);

    if (!e->isAccepted()) {
        fitNetwork();
        e->accept();
    }
}

void NetworkEditorView::resizeEvent(QResizeEvent* e) {
    QGraphicsView::resizeEvent(e);
}

void NetworkEditorView::fitNetwork() {
    const ProcessorNetwork* network = InviwoApplication::getPtr()->getProcessorNetwork();
    if (network) {
        if (network->getProcessors().size() > 0) {
            QRectF br = networkEditor_->itemsBoundingRect().adjusted(-50, -50, 50, 50);
            QSizeF viewsize = size();
            QSizeF brsize = br.size();

            if (brsize.width() < viewsize.width()) {
                br.setLeft(br.left() - 0.5*(viewsize.width() - brsize.width()));
                br.setRight(br.right() + 0.5*(viewsize.width() - brsize.width()));
            }
            if (brsize.height() < viewsize.height()) {
                br.setTop(br.top() - 0.5*(viewsize.height() - brsize.height()));
                br.setBottom(br.bottom() + 0.5*(viewsize.height() - brsize.height()));
            }

            setSceneRect(br);
            fitInView(br, Qt::KeepAspectRatio);
        }
    }
}

void NetworkEditorView::focusOutEvent(QFocusEvent *) {
    setDragMode(QGraphicsView::RubberBandDrag);
}

void NetworkEditorView::wheelEvent(QWheelEvent* e) {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    QPointF numPixels = e->pixelDelta() / 5.0;
    QPointF numDegrees = e->angleDelta() / 8.0 / 15;
#else
    QPointF numPixels;
    QPointF numDegrees = QPointF(0.0, e->delta() / 8.0 / 15.0);
#endif
    if (e->modifiers() == Qt::ControlModifier) {
        if (!numPixels.isNull()) {
            zoom(qPow(1.05,  numPixels.y()));

        } else if (!numDegrees.isNull()) {
            zoom(qPow(1.05,  numDegrees.y()));
        }
    } else {
        QGraphicsView::wheelEvent(e);
    }
    e->accept();
}

void NetworkEditorView::keyPressEvent(QKeyEvent* keyEvent) {
     if (keyEvent->modifiers() & Qt::ControlModifier) {
        setDragMode(QGraphicsView::ScrollHandDrag);
     }
}

void NetworkEditorView::keyReleaseEvent(QKeyEvent* keyEvent) {
    setDragMode(QGraphicsView::RubberBandDrag);
}

void NetworkEditorView::zoom(double dz) {
    if (matrix().m11() > 2 || matrix().m11() < 0.25) return;
    scale(dz, dz);
}


void NetworkEditorView::onNetworkEditorFileChanged(const std::string& newFilename) {
    fitNetwork();
}

}  // namespace