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

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/settings/systemsettings.h>
#include <inviwo/core/util/tooltiphelper.h>
#include <inviwo/qt/editor/editorgrapicsitem.h>
#include <inviwo/qt/editor/networkeditor.h>
#include <inviwo/core/ports/port.h>
#include <inviwo/qt/widgets/inviwoqtutils.h>

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

EditorGraphicsItem::~EditorGraphicsItem() {}

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

void EditorGraphicsItem::showPortInfo(QGraphicsSceneHelpEvent* e, Port* port) const {
    SystemSettings* settings = InviwoApplication::getPtr()->getSettingsByType<SystemSettings>();
    bool portinfo = settings->enablePortInformationProperty_.get();
    bool inspector = settings->enablePortInspectorsProperty_.get();

    if (!inspector && !portinfo) return;

    std::unique_ptr<std::vector<unsigned char>> data;
    int size = settings->portInspectorSize_.get();

    ToolTipHelper t;
    t.tableTop();

    std::string imageType = "png";

    if (inspector) {
        if (auto outport = dynamic_cast<Outport*>(port)) {
            data = NetworkEditor::getPtr()->renderPortInspectorImage(outport, imageType);
        }
    }

    if (portinfo) {
        ToolTipHelper head(port->getClassIdentifier());
        head.tableTop();
        head.row("Identifier", port->getIdentifier());
        head.tableBottom();
        t << "<tr><td>" << std::string(head) << "</td></tr>";
    }

    if (data) {
        QByteArray byteArray;

        if (imageType != "png"){
            QImage::Format format = QImage::Format_Invalid;
            if (data->size() == size*size){
                format = QImage::Format_Indexed8;
            }
            else if (data->size() == size*size * 3){
                format = QImage::Format_RGB888;
            }
            else if (data->size() == size*size * 4){
                #if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
            format = QImage::Format_RGBA8888;
        #else
            format = QImage::Format_RGB888;
        #endif
            }

            QImage image(reinterpret_cast<const unsigned char*>(&(data->front())), size, size, format);
            QBuffer buffer(&byteArray);
            image.save(&buffer, "PNG");
        }
        else{
            byteArray.setRawData(reinterpret_cast<const char*>(&(data->front())),
                static_cast<unsigned int>(data->size()));
        }
        
        t << "<tr><td><img width='"<< size <<"' height='"<< size << "' ";
        t << "src=\"data:image/png;base64,"<< byteArray.toBase64().data() <<"\"/></td></tr>";
    }

    if (portinfo) {
        t << "<tr><td>" << port->getContentInfo() << "</td></tr>";
    }

    t.tableBottom();

    showToolTipHelper(e, utilqt::toLocalQString(t));
}

} // namespace

