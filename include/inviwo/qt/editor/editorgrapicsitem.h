/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2024 Inviwo Foundation
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

#pragma once

#include <inviwo/qt/editor/inviwoqteditordefine.h>
#include <modules/qtwidgets/properties/propertywidgetqt.h>
#include <warn/push>
#include <warn/ignore/all>
#include <QGraphicsItem>
#include <QGraphicsRectItem>
#include <QGraphicsSceneHelpEvent>
#include <warn/pop>

namespace inviwo {

class NetworkEditor;

enum InviwoUserGraphicsItemType {
    ProcessorGraphicsType = 1,
    CurveGraphicsType,
    ConnectionDragGraphicsType,
    ConnectionInportDragGraphicsType,
    ConnectionGraphicsType,
    LinkGraphicsType,
    LinkConnectionDragGraphicsType,
    LinkConnectionGraphicsType,
    ProcessorProgressGraphicsType,
    ProcessorStatusGraphicsType,
    ProcessorLinkGraphicsType,
    ProcessorInportGraphicsType,
    ProcessorOutportGraphicsType,
    ProcessorErrorItemType
};

namespace depth {
// Z value for various graphics items.
static constexpr double dragItem = 4.0;
static constexpr double processorSelected = 3.0;
static constexpr double processorError = 2.5;
static constexpr double processor = 2.0;
static constexpr double connection = 1.0;
static constexpr double link = 0.0;
}  // namespace depth

class Port;

class IVW_QTEDITOR_API EditorGraphicsItem : public QGraphicsRectItem {
public:
    EditorGraphicsItem();
    EditorGraphicsItem(QGraphicsItem* parent);
    EditorGraphicsItem(const EditorGraphicsItem&) = delete;
    EditorGraphicsItem(EditorGraphicsItem&&) = delete;
    EditorGraphicsItem& operator=(const EditorGraphicsItem&) = delete;
    EditorGraphicsItem& operator=(EditorGraphicsItem&&) = delete;
    virtual ~EditorGraphicsItem();

    QPoint mapPosToSceen(QPointF pos) const;

    virtual void showToolTip(QGraphicsSceneHelpEvent* event);
    void showPortInfo(QGraphicsSceneHelpEvent* e, Port* port) const;

protected:
    void showToolTipHelper(QGraphicsSceneHelpEvent* event, QString string) const;
    NetworkEditor* getNetworkEditor() const;
};

}  // namespace inviwo
