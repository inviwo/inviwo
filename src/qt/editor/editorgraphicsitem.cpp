/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2025 Inviwo Foundation
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

#include <inviwo/qt/editor/editorgraphicsitem.h>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/datastructures/image/image.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/ports/port.h>
#include <inviwo/core/util/document.h>
#include <inviwo/core/util/settings/systemsettings.h>
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
        return QPoint(0, 0);
    }
}

void EditorGraphicsItem::showToolTip(QGraphicsSceneHelpEvent*) {}

void EditorGraphicsItem::showToolTipHelper(QGraphicsSceneHelpEvent* e, QString string) const {
    if (!scene() || scene()->views().empty()) return;

    QGraphicsView* v = scene()->views().first();
    QRectF rect = this->mapRectToScene(this->rect());
    QRect viewRect = v->mapFromScene(rect).boundingRect();
    e->accept();
    QToolTip::showText(e->screenPos(), string, v, viewRect);
}

NetworkEditor* EditorGraphicsItem::getNetworkEditor() const {
    return qobject_cast<NetworkEditor*>(scene());
}

namespace {
std::vector<std::pair<std::string, const Layer*>> getLayersForImagePort(
    const std::shared_ptr<const Image>& image, const std::shared_ptr<const Image>& imageData) {
    std::vector<std::pair<std::string, const Layer*>> layers;
    for (std::size_t i = 0; i < image->getNumberOfColorLayers(); ++i) {
        const auto* layerData = imageData->getColorLayer(i);
        const auto* layer = image->getColorLayer(i);
        layers.emplace_back(
            fmt::format("Color Layer {} {}", i, layerData->getDataFormat()->getString()), layer);
    }

    // register picking layer
    layers.emplace_back(
        fmt::format("Picking Layer {}", imageData->getPickingLayer()->getDataFormat()->getString()),
        image->getPickingLayer());
    // register depth layer
    layers.emplace_back(
        fmt::format("Depth Layer {}", imageData->getDepthLayer()->getDataFormat()->getString()),
        image->getDepthLayer());

    return layers;
}

std::string layerToString(const Layer& layer) {
    const auto imgbuf = layer.getAsCodedBuffer("png");
    // imgbuf might be null, if we don't have a data writer factory function to save
    // the layer. Happens if cimg not used, and no other data writer is registered.
    if (!imgbuf) return {};

    const QByteArray byteArray(reinterpret_cast<const char*>(imgbuf->data()),  // NOLINT
                               static_cast<int>(imgbuf->size()));

    return {byteArray.toBase64().data()};
}

void addLayersTable(Document::DocumentHandle& body,
                    const std::vector<std::pair<std::string, const Layer*>>& layers, size_t size) {
    auto p = body.append("p");
    p.append("b", "Port Inspector", {{"style", "color:white;"}});
    auto t = p.append("table");
    auto tableRow = t.append("tr");
    const size_t perRow = static_cast<size_t>(std::ceil(std::sqrt(layers.size())));
    size_t rowCount = 0;
    for (const auto& [name, layer] : layers) {
        if (rowCount >= perRow) {
            rowCount = 0;
            tableRow = t.append("tr");
        }

        const auto layerStr = layerToString(*layer);
        if (layerStr.empty()) continue;

        auto table = tableRow.append("td").append("table");

        table.append("tr").append("td").append("img", "",
                                               {{"width", std::to_string(size)},
                                                {"height", std::to_string(size)},
                                                {"src", "data:image/png;base64," + layerStr}});
        if (!name.empty()) {
            table.append("tr").append("td", name);
        }
        ++rowCount;
    }
}

}  // namespace

void EditorGraphicsItem::showPortInfo(QGraphicsSceneHelpEvent* e, Port* port) const {
    if (!scene() || scene()->views().empty()) return;
    QGraphicsView* view = scene()->views().first();
    const QRectF rect = this->mapRectToScene(this->rect());
    const QRect viewRect = view->mapFromScene(rect).boundingRect();
    e->accept();

    auto* settings = InviwoApplication::getPtr()->getSettingsByType<SystemSettings>();
    const bool inspector = settings->enablePortInspectors_.get();
    const auto portInspectorSize = static_cast<size_t>(settings->portInspectorSize_.get());

    Document desc{};
    auto html = desc.append("html");
    html.append("head").append("style", R"(
        div.name {
            font-size: 13pt;
            color: #c8ccd0;;
            font-weight: bold;
        }
        div.help {
            font-size: 12pt;
            margin: 10px 0px 10px 0px;
            padding: 0px 0px 0px 0px;
        }
        table {
            margin: 10px 0px 0px 0px;
            padding: 0px 0px 0px 0px;
        }
        )"_unindent);

    auto body = html.append("body");
    body.append(port->getInfo());

    auto inport = dynamic_cast<const Inport*>(port);
    auto outport = dynamic_cast<Outport*>(port);
    if (!outport && inport) {
        outport = inport->getConnectedOutport();
    }

    if (inspector && outport) {
        if (auto image = getNetworkEditor()->renderPortInspectorImage(outport)) {
            std::vector<std::pair<std::string, const Layer*>> layers;
            if (auto* imageOutport = dynamic_cast<ImageOutport*>(outport)) {
                if (auto imageData = imageOutport->getData()) {
                    // register all color layers
                    layers = getLayersForImagePort(image, imageData);
                }
            } else {
                // outport is not an ImageOutport, show only first color layer
                layers.emplace_back("", image->getColorLayer(0));
            }
            addLayersTable(body, layers, portInspectorSize);
        }
    }

    // Need to make sure that we have not pending qt stuff before showing tooltip
    // otherwise we might loose focus and the tooltip will go away...
    qApp->processEvents();

    // don't use showToolTipHelper here, since this might have been deleted in processEvents.
    QToolTip::showText(e->screenPos(), utilqt::toLocalQString(desc), view, viewRect);
}

}  // namespace inviwo
