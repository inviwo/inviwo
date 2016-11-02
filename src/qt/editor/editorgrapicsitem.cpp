/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2016 Inviwo Foundation
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
#include <inviwo/core/datastructures/image/image.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/ports/port.h>
#include <inviwo/core/util/document.h>
#include <inviwo/core/util/settings/systemsettings.h>
#include <inviwo/qt/editor/editorgrapicsitem.h>
#include <inviwo/qt/editor/networkeditor.h>
#include <inviwo/qt/widgets/inviwoqtutils.h>

#include <inviwo/qt/widgets/inviwoapplicationqt.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QToolTip>
#include <QBuffer>
#include <warn/pop>

namespace inviwo {

EditorGraphicsItem::EditorGraphicsItem() : QGraphicsRectItem() {}

EditorGraphicsItem::EditorGraphicsItem(QGraphicsItem* parent) : QGraphicsRectItem(parent) {}

EditorGraphicsItem::~EditorGraphicsItem() = default;

QPoint EditorGraphicsItem::mapPosToSceen(QPointF inPos) const {
    if (scene() != nullptr                                  // the focus item belongs to a scene
        && !scene()->views().isEmpty()                      // that scene is displayed in a view...
        && scene()->views().first() != nullptr              // ... which is not null...
        && scene()->views().first()->viewport() != nullptr  // ... and has a viewport
        ) {
        QPointF sceneP = mapToScene(inPos);
        QGraphicsView* v = scene()->views().first();
        QPoint viewP = v->mapFromScene(sceneP);
        return v->viewport()->mapToGlobal(viewP);
    } else {
        return QPoint(0,0);
    }

}

const QPainterPath EditorGraphicsItem::makeRoundedBox(QRectF rect, float radius) {
    QPainterPath roundRectPath;
    roundRectPath.moveTo(rect.left(), rect.top() + radius);
    roundRectPath.lineTo(rect.left(), rect.bottom() - radius);
    roundRectPath.arcTo(rect.left(), rect.bottom() - (2 * radius), (2 * radius),
                        (2 * radius), 180.0, 90.0);
    roundRectPath.lineTo(rect.right() - radius, rect.bottom());
    roundRectPath.arcTo(rect.right() - (2 * radius), rect.bottom() - (2 * radius),
                        (2 * radius), (2 * radius), 270.0, 90.0);
    roundRectPath.lineTo(rect.right(), rect.top() + radius);
    roundRectPath.arcTo(rect.right() - (2 * radius), rect.top(), (2 * radius),
                        (2 * radius), 0.0, 90.0);
    roundRectPath.lineTo(rect.left() + radius, rect.top());
    roundRectPath.arcTo(rect.left(), rect.top(), (2 * radius), (2 * radius), 90.0,
                        90.0);

    return roundRectPath;
}

void EditorGraphicsItem::showToolTip(QGraphicsSceneHelpEvent* e) {
    showToolTipHelper(e, QString("Test"));
}

void EditorGraphicsItem::showToolTipHelper(QGraphicsSceneHelpEvent* e, QString string) const {
    QGraphicsView* v = scene()->views().first();
    QRectF rect = this->mapRectToScene(this->rect());
    QRect viewRect = v->mapFromScene(rect).boundingRect();
    QToolTip::showText(e->screenPos(), string, v, viewRect);
}

NetworkEditor* EditorGraphicsItem::getNetworkEditor() const {
    return qobject_cast<NetworkEditor*>(scene());
}

void EditorGraphicsItem::showPortInfo(QGraphicsSceneHelpEvent* e, Port* port) const {
    SystemSettings* settings = InviwoApplication::getPtr()->getSettingsByType<SystemSettings>();
    bool portinfo = settings->enablePortInformation_.get();
    bool inspector = settings->enablePortInspectors_.get();

    if (!inspector && !portinfo) return;

    size_t portInspectorSize = static_cast<size_t>(settings->portInspectorSize_.get());

    Document doc;
    using P = Document::PathComponent;
    using H = utildoc::TableBuilder::Header;
    auto t = doc.append("html").append("body").append("table");
    

    if (portinfo) {
        auto pi = t.append("tr").append("td");
        pi.append("b", port->getClassIdentifier(), {{"style", "color:white;"}});
        utildoc::TableBuilder tb(pi, P::end());
        if (auto inport = dynamic_cast<const Inport*>(port)) {
            if (inport->isOptional()) {
                tb(H("Optional Port"));
            }
        }
        tb(H("Identifier"), port->getIdentifier());
        
    }

    if (inspector) {
        const std::string imageType = "raw";

        if (auto outport = dynamic_cast<Outport*>(port)) {
            if (auto image = getNetworkEditor()->renderPortInspectorImage(outport)) {

                bool isImagePort = (dynamic_cast<ImageOutport*>(port) != nullptr);

                std::vector<const Layer *> layers;
                if (isImagePort) {
                    // register all color layers
                    for (std::size_t i=0; i < image->getNumberOfColorLayers(); ++i) {
                        layers.push_back(image->getColorLayer(i));
                    }
                    // register picking layer
                    layers.push_back(image->getPickingLayer());
                    // register depth layer
                    layers.push_back(image->getDepthLayer());
                }
                else {
                    // outport is not an ImageOutport, show only first color layer
                    layers.push_back(image->getColorLayer(0));
                }

                // add all layer images into the same row
                auto tableCell = t.append("tr").append("td");
                for (auto layer : layers) {
                    auto data = layer->getAsCodedBuffer(imageType);
                    QByteArray byteArray;

                    if (imageType == "png") {
                        // input buffer is already stored as png 
                        byteArray.setRawData(reinterpret_cast<const char*>(&(data->front())),
                                             static_cast<unsigned int>(data->size()));
                    }
                    else if (imageType == "raw") {
                        // do manual conversion from raw to png via QImage
                        QImage::Format format = QImage::Format_Invalid;
                        if (data->size() == portInspectorSize * portInspectorSize) {
#if QT_VERSION >= 0x050500
                            format = QImage::Format_Grayscale8;
#else
                            format = QImage::Format_RGB888;
                            // duplicate grayscale data into 3 channels
                            auto newData = std::make_unique<std::vector<unsigned char>>();
                            newData->reserve(data->size() * 3);

                            for (auto value : *data.get()) {
                                newData->insert(newData->end(), 3, value);
                            }
                            data = std::move(newData);
#endif
                        }
                        else if (data->size() == portInspectorSize * portInspectorSize * 3) {
                            format = QImage::Format_RGB888;
                        }
                        else if (data->size() == portInspectorSize * portInspectorSize * 4) {
                            format = QImage::Format_RGBA8888;
                        }

                        QImage qImage(reinterpret_cast<const unsigned char*>(&(data->front())),
                                     static_cast<int>(portInspectorSize), static_cast<int>(portInspectorSize), format);
                        QBuffer buffer(&byteArray);
                        qImage.save(&buffer, "PNG");
                    }
                    else {
                        throw Exception("Support for image type not yet implemented", IvwContext);
                    }
                    
                    tableCell.append(
                        "img", "",
                        { {"width", std::to_string(portInspectorSize)},
                         {"height", std::to_string(portInspectorSize)},
                         {"src", "data:image/png;base64," + std::string(byteArray.toBase64().data())} });
                }
            }
        }
    }


    if (portinfo) {
        t.append("tr").append("td", port->getContentInfo());
    }

    // Need to make sure that we have not pending qt stuff before showing tooltip
    // otherwise we might loose focus and the tooltip will go away...
    static_cast<InviwoApplicationQt*>(InviwoApplication::getPtr())->processEvents();

    showToolTipHelper(e, utilqt::toLocalQString(doc));
}

} // namespace

