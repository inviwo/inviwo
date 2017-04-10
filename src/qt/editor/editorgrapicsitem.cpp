/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2017 Inviwo Foundation
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
#include <modules/qtwidgets/inviwoqtutils.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QToolTip>
#include <QBuffer>
#include <QApplication>
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

                std::vector<std::pair<std::string, const Layer*>> layers;
                if (isImagePort) {
                    // register all color layers
                    for (std::size_t i = 0; i < image->getNumberOfColorLayers(); ++i) {
                        const auto layer = image->getColorLayer(i);
                        std::stringstream ss;
                        ss << "Color Layer " << i << " " << layer->getDataFormat()->getString();
                        layers.push_back({ss.str(), layer});
                    }

                    // register picking layer
                    {
                        std::stringstream ss;
                        ss << "Picking Layer "
                           << image->getPickingLayer()->getDataFormat()->getString();
                        layers.push_back({ss.str(), image->getPickingLayer()});
                    }
                    // register depth layer
                    {
                        std::stringstream ss;
                        ss << "Depth Layer "
                           << image->getDepthLayer()->getDataFormat()->getString();
                        layers.push_back({ss.str(), image->getDepthLayer()});
                    }
                } else {
                    // outport is not an ImageOutport, show only first color layer
                    layers.push_back({"", image->getColorLayer(0)});
                }

                // add all layer images into the same row
                auto tableCell = t.append("tr").append("td").append("table").append("tr");
                size_t perRow = std::ceil(std::sqrt(layers.size()));
                size_t rowCount = 0;
                for (auto item : layers) {
                    if (rowCount >= perRow) {
                        rowCount = 0;
                        tableCell = t.append("tr").append("td").append("table").append("tr");
                    }
                    auto name = item.first;
                    auto layer = item.second;

                    auto imgbuf = layer->getAsCodedBuffer("png");
                    QByteArray byteArray(reinterpret_cast<char*>(imgbuf->data()), static_cast<int>(imgbuf->size()));

                    auto table = tableCell.append("td").append("table");

                    table.append("tr").append("td").append(
                        "img", "",
                        {{"width", std::to_string(portInspectorSize)},
                         {"height", std::to_string(portInspectorSize)},
                         {"src",
                          "data:image/png;base64," + std::string(byteArray.toBase64().data())}});
                    if (!name.empty()) {
                        table.append("tr").append("td", name);
                    }
                    ++rowCount;
                }
            }
        }
    }


    if (portinfo) {
        t.append("tr").append("td", port->getContentInfo());
    }

    // Need to make sure that we have not pending qt stuff before showing tooltip
    // otherwise we might loose focus and the tooltip will go away...
    qApp->processEvents();

    showToolTipHelper(e, utilqt::toLocalQString(doc));
}

} // namespace

