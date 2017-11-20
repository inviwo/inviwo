/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2017 Inviwo Foundation
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

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/settings/linksettings.h>
#include <inviwo/core/util/settings/systemsettings.h>
#include <inviwo/qt/editor/networkeditorview.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/qt/editor/inviwomainwindow.h>
#include <inviwo/qt/editor/inviwoeditmenu.h>
#include <inviwo/core/util/filesystem.h>
#include <modules/qtwidgets/inviwoqtutils.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QMatrix>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QScrollBar>
#include <QApplication>
#include <QClipboard>
#include <QPdfWriter>
#include <QImage>
#include <qmath.h>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QUrl>
#include <warn/pop>

namespace inviwo {

NetworkEditorView::NetworkEditorView(NetworkEditor* networkEditor, InviwoMainWindow* parent)
    : QGraphicsView(parent)
    , NetworkEditorObserver()
    , mainwindow_(parent)
    , networkEditor_(networkEditor) {

    NetworkEditorObserver::addObservation(networkEditor_);
    QGraphicsView::setScene(networkEditor_);

    setRenderHint(QPainter::Antialiasing, true);
    setMouseTracking(true);
    setDragMode(QGraphicsView::RubberBandDrag);
    setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    setCacheMode(QGraphicsView::CacheBackground);

    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    setAcceptDrops(true);

    loadHandle_ = mainwindow_->getInviwoApplication()->getWorkspaceManager()->onLoad(
        [this](Deserializer&) { fitNetwork(); });

    auto editmenu = mainwindow_->getInviwoEditMenu();

    editActionsHandle_ = editmenu->registerItem(std::make_shared<MenuItem>(
        this,
        [this](MenuItemType t) -> bool {
            switch (t) {
                case MenuItemType::cut:
                    return networkEditor_->selectedItems().size() > 0;
                case MenuItemType::copy:
                    return networkEditor_->selectedItems().size() > 0;
                case MenuItemType::paste: {
                    auto clipboard = QApplication::clipboard();
                    auto mimeData = clipboard->mimeData();
                    if (mimeData->formats().contains(
                            utilqt::toQString(NetworkEditor::getMimeTag()))) {
                        return true;
                    } else if (mimeData->formats().contains(QString("text/plain"))) {
                        return true;
                    } else {
                        return false;
                    }
                }
                case MenuItemType::del:
                    return networkEditor_->selectedItems().size() > 0;
                case MenuItemType::select:
                    return true;
                default:
                    return false;
            }

        },
        [this](MenuItemType t) -> void {
            switch (t) {
                case MenuItemType::cut: {
                    if (networkEditor_->selectedItems().empty()) return;
                    auto data = networkEditor_->cut();
                    auto mimedata = util::make_unique<QMimeData>();
                    mimedata->setData(utilqt::toQString(NetworkEditor::getMimeTag()), data);
                    mimedata->setData(QString("text/plain"), data);
                    QApplication::clipboard()->setMimeData(mimedata.release());
                    return;
                }
                case MenuItemType::copy: {
                    if (networkEditor_->selectedItems().empty()) return;
                    auto data = networkEditor_->copy();
                    auto mimedata = util::make_unique<QMimeData>();
                    mimedata->setData(utilqt::toQString(NetworkEditor::getMimeTag()), data);
                    mimedata->setData(QString("text/plain"), data);
                    QApplication::clipboard()->setMimeData(mimedata.release());
                    return;
                }
                case MenuItemType::paste: {
                    auto clipboard = QApplication::clipboard();
                    auto mimeData = clipboard->mimeData();
                    if (mimeData->formats().contains(
                            utilqt::toQString(NetworkEditor::getMimeTag()))) {
                        networkEditor_->paste(
                            mimeData->data(utilqt::toQString(NetworkEditor::getMimeTag())));
                    } else if (mimeData->formats().contains(QString("text/plain"))) {
                        networkEditor_->paste(mimeData->data(QString("text/plain")));
                    }
                    return;
                }
                case MenuItemType::del:
                    if (networkEditor_->selectedItems().empty()) return;
                    networkEditor_->deleteSelection();
                    return;
                case MenuItemType::select:
                    networkEditor_->selectAll();
                    return;
                default:
                    return;
            }

        }));
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

void NetworkEditorView::resizeEvent(QResizeEvent* e) { QGraphicsView::resizeEvent(e); }

void NetworkEditorView::fitNetwork() {
    const ProcessorNetwork* network = InviwoApplication::getPtr()->getProcessorNetwork();
    if (network) {
        if (network->getProcessors().size() > 0) {
            QRectF br = networkEditor_->itemsBoundingRect().adjusted(-50, -50, 50, 50);
            QSizeF viewsize = size();
            QSizeF brsize = br.size();

            if (brsize.width() < viewsize.width()) {
                br.setLeft(br.left() - 0.5 * (viewsize.width() - brsize.width()));
                br.setRight(br.right() + 0.5 * (viewsize.width() - brsize.width()));
            }
            if (brsize.height() < viewsize.height()) {
                br.setTop(br.top() - 0.5 * (viewsize.height() - brsize.height()));
                br.setBottom(br.bottom() + 0.5 * (viewsize.height() - brsize.height()));
            }

            setSceneRect(br);
            fitInView(br, Qt::KeepAspectRatio);
        }
    }
}

void NetworkEditorView::focusOutEvent(QFocusEvent* e) {
    setDragMode(QGraphicsView::RubberBandDrag);
    QGraphicsView::focusOutEvent(e);
}

void NetworkEditorView::wheelEvent(QWheelEvent* e) {
    QPointF numPixels = e->pixelDelta() / 5.0;
    QPointF numDegrees = e->angleDelta() / 8.0 / 15;

    if (e->modifiers() == Qt::ControlModifier) {
        if (!numPixels.isNull()) {
            zoom(qPow(1.025, std::max(-15.0, std::min(15.0, numPixels.y()))));
        } else if (!numDegrees.isNull()) {
            zoom(qPow(1.025, std::max(-15.0, std::min(15.0, numDegrees.y()))));
        }
    } else if (e->modifiers() & Qt::ShiftModifier) {
        // horizontal scrolling
        auto modifiers = e->modifiers();
        // remove the shift key temporarily from the event
        e->setModifiers(e->modifiers() ^ Qt::ShiftModifier);
        horizontalScrollBar()->event(e);
        // restore previous modifiers
        e->setModifiers(modifiers);
    } else {
        QGraphicsView::wheelEvent(e);
    }
    e->accept();
}

void NetworkEditorView::keyPressEvent(QKeyEvent* keyEvent) {
    if (keyEvent->modifiers() & Qt::ControlModifier) {
        setDragMode(QGraphicsView::ScrollHandDrag);
    }
    QGraphicsView::keyPressEvent(keyEvent);
}

void NetworkEditorView::keyReleaseEvent(QKeyEvent* keyEvent) {
    setDragMode(QGraphicsView::RubberBandDrag);
    QGraphicsView::keyReleaseEvent(keyEvent);
}

void NetworkEditorView::zoom(double dz) {
    if ((dz > 1.0 && matrix().m11() > 8.0) || (dz < 1.0 && matrix().m11() < 0.125)) return;

    setTransform(QTransform::fromScale(dz, dz), true);
}

void NetworkEditorView::onNetworkEditorFileChanged(const std::string& /*newFilename*/) {
    fitNetwork();
}

void NetworkEditorView::exportViewToFile(const QString& filename, bool entireScene,
                                         bool backgroundVisible) {
    QRectF rect;
    if (entireScene) {
        rect = scene()->itemsBoundingRect() + QMargins(10, 10, 10, 10);

    } else {
        rect = viewport()->rect();
    }
    const QRect destRect(QPoint(0, 0), rect.size().toSize());

    networkEditor_->setBackgroundVisible(backgroundVisible);

    auto renderCall = [&, entireScene](QPainter& painter) {
        if (entireScene) {
            scene()->render(&painter, destRect, rect.toRect());
        } else {
            render(&painter, destRect, rect.toRect());
        }
    };

    if (toLower(filesystem::getFileExtension(utilqt::fromQString(filename))) == "pdf") {
        QPdfWriter pdfwriter(filename);
        pdfwriter.setPageSize(QPageSize(destRect.size(), QPageSize::Point));
        pdfwriter.setPageMargins(QMarginsF(), QPageLayout::Point);
        pdfwriter.setResolution(72);
        QPainter painter(&pdfwriter);
        renderCall(painter);
        painter.end();
    } else {
        QImage image(destRect.size(), QImage::Format_ARGB32);
        QPainter painter(&image);
        painter.setRenderHint(QPainter::Antialiasing);
        renderCall(painter);
        painter.end();
        image.save(filename);
    }

    networkEditor_->setBackgroundVisible(true);
}

void NetworkEditorView::dropEvent(QDropEvent* event) {
    const QMimeData* mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        QStringList pathList;
        QList<QUrl> urlList = mimeData->urls();

        // pick first url
        auto filename = urlList.front().toLocalFile();
        mainwindow_->openWorkspace(filename);
        event->acceptProposedAction();
    }
}

void NetworkEditorView::dragEnterEvent(QDragEnterEvent* event) { event->acceptProposedAction(); }

}  // namespace inviwo
