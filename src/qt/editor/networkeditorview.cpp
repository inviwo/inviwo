/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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
#include <modules/qtwidgets/textlabeloverlay.h>
#include <inviwo/qt/editor/networksearch.h>

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
#include <QLabel>
#include <QGridLayout>
#include <warn/pop>

namespace inviwo {

NetworkEditorView::NetworkEditorView(NetworkEditor* networkEditor, InviwoMainWindow* parent)
    : QGraphicsView(parent)
    , NetworkEditorObserver()
    , mainwindow_(parent)
    , editor_(networkEditor)
    , search_{new NetworkSearch(mainwindow_)}
    , overlay_{new TextLabelOverlay(viewport())}
    , scrollPos_{0, 0}
    , loadHandle_{mainwindow_->getInviwoApplication()->getWorkspaceManager()->onLoad(
          [this](Deserializer&) { fitNetwork(); })}
    , clearHandle_{mainwindow_->getInviwoApplication()->getWorkspaceManager()->onClear(
          [this]() { fitNetwork(); })} {

    NetworkEditorObserver::addObservation(editor_);
    QGraphicsView::setScene(editor_);

    auto grid = new QGridLayout(viewport());
    const auto space = utilqt::refSpacePx(this);
    grid->setContentsMargins(space, space, space, space);

    {
        grid->addWidget(overlay_, 0, 0, Qt::AlignTop | Qt::AlignLeft);
        auto sp = overlay_->sizePolicy();
        sp.setHorizontalStretch(10);
        sp.setHorizontalPolicy(QSizePolicy::Expanding);
        overlay_->setSizePolicy(sp);
    }
    {
        auto sp = search_->sizePolicy();
        sp.setHorizontalStretch(1);
        search_->setSizePolicy(sp);
        grid->addWidget(search_, 0, 1, Qt::AlignTop | Qt::AlignRight);
    }

    setRenderHint(QPainter::Antialiasing, true);
    setMouseTracking(true);
    setDragMode(QGraphicsView::RubberBandDrag);
    setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    setCacheMode(QGraphicsView::CacheBackground);

    const auto scale = utilqt::emToPx(this, 1.0) / static_cast<double>(utilqt::refEm());
    setTransform(QTransform::fromScale(scale, scale), false);

    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    auto editmenu = mainwindow_->getInviwoEditMenu();

    editActionsHandle_ = editmenu->registerItem(std::make_shared<MenuItem>(
        this,
        [this](MenuItemType t) -> bool {
            switch (t) {
                case MenuItemType::cut:
                    return editor_->selectedItems().size() > 0;
                case MenuItemType::copy:
                    return editor_->selectedItems().size() > 0;
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
                    return editor_->selectedItems().size() > 0;
                case MenuItemType::select:
                    return true;
                default:
                    return false;
            }
        },
        [this](MenuItemType t) -> void {
            switch (t) {
                case MenuItemType::cut: {
                    if (editor_->selectedItems().empty()) return;
                    auto data = editor_->cut();
                    auto mimedata = std::make_unique<QMimeData>();
                    mimedata->setData(utilqt::toQString(NetworkEditor::getMimeTag()), data);
                    mimedata->setData(QString("text/plain"), data);
                    QApplication::clipboard()->setMimeData(mimedata.release());
                    return;
                }
                case MenuItemType::copy: {
                    if (editor_->selectedItems().empty()) return;
                    auto data = editor_->copy();
                    auto mimedata = std::make_unique<QMimeData>();
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
                        editor_->paste(
                            mimeData->data(utilqt::toQString(NetworkEditor::getMimeTag())));
                    } else if (mimeData->formats().contains(QString("text/plain"))) {
                        editor_->paste(mimeData->data(QString("text/plain")));
                    }
                    return;
                }
                case MenuItemType::del:
                    if (editor_->selectedItems().empty()) return;
                    editor_->deleteSelection();
                    return;
                case MenuItemType::select:
                    editor_->selectAll();
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
        if (scene() != editor_) {
            QGraphicsView::setScene(editor_);
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

TextLabelOverlay& NetworkEditorView::getOverlay() const { return *overlay_; }

NetworkSearch& NetworkEditorView::getNetworkSearch() const { return *search_; }

void NetworkEditorView::resizeEvent(QResizeEvent* e) { QGraphicsView::resizeEvent(e); }

void NetworkEditorView::fitNetwork() {
    const auto scale = utilqt::emToPx(this, 1.0) / static_cast<double>(utilqt::refEm());
    setTransform(QTransform::fromScale(scale, scale), false);

    if (const auto network = mainwindow_->getInviwoApplication()->getProcessorNetwork()) {
        if (network->getProcessors().size() > 0) {
            const auto br = editor_->getProcessorsBoundingRect().adjusted(-50, -50, 50, 50);
            setSceneRect(br);
            fitInView(br, Qt::KeepAspectRatio);
        } else {
            QRectF r{rect()};
            r.moveCenter(QPointF(0, 0));
            setSceneRect(rect());
            fitInView(rect(), Qt::KeepAspectRatio);
        }
        if (matrix().m11() > scale) {
            setTransform(QTransform::fromScale(scale, scale), false);
        }
    }
}

void NetworkEditorView::onSceneSizeChanged() {
    auto br = editor_->getProcessorsBoundingRect();
    if (sceneRect().contains(br)) return;

    QSizeF viewsize = viewport()->size();
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

    editor_->setBackgroundVisible(backgroundVisible);

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

    editor_->setBackgroundVisible(true);
}

QImage NetworkEditorView::exportViewToImage(bool entireScene, bool backgroundVisible, QSize size) {
    QRectF rect;
    if (entireScene) {
        rect = scene()->itemsBoundingRect() + QMarginsF(10, 10, 10, 10);
    } else {
        rect = viewport()->rect();
    }
    const QRectF destRect(QPointF(0, 0), size.isNull() ? rect.size() : size);

    const auto srcAspectRatio = rect.width() / rect.height();
    const auto destAspectRatio = destRect.width() / destRect.height();

    // padding of source/scene rect if output aspect ratio does not match
    if (srcAspectRatio > destAspectRatio) {
        auto padding = (rect.width() / destAspectRatio - rect.height()) * 0.5;
        rect += QMarginsF(0.0, padding, 0.0, padding);
    } else if (srcAspectRatio < destAspectRatio) {
        auto padding = (rect.height() * destAspectRatio - rect.width()) * 0.5;
        rect += QMarginsF(padding, 0.0, padding, 0.0);
    }

    editor_->setBackgroundVisible(backgroundVisible);

    auto renderCall = [&, entireScene](QPainter& painter) {
        if (entireScene) {
            scene()->render(&painter, destRect, rect.toRect());
        } else {
            render(&painter, destRect, rect.toRect());
        }
    };

    QImage image(destRect.size().toSize(), QImage::Format_ARGB32);
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    renderCall(painter);
    painter.end();

    editor_->setBackgroundVisible(true);
    return image;
}

}  // namespace inviwo
