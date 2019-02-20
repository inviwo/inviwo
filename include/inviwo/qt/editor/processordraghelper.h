/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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
#include <inviwo/core/common/inviwo.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QObject>
#include <warn/pop>

#include <unordered_map>
#include <memory>

class QEvent;
class QGraphicsSceneDragDropEvent;

namespace inviwo {

class Processor;
class ProcessorMimeData;
class ConnectionGraphicsItem;
class NetworkEditor;
class AutoLinker;
class LinkConnectionDragGraphicsItem;
class ConnectionDragGraphicsItem;
class ProcessorGraphicsItem;
class Property;
class Inport;
class Outport;

class IVW_QTEDITOR_API ProcessorDragHelper : public QObject {
#include <warn/push>
#include <warn/ignore/all>
    Q_OBJECT
#include <warn/pop>

public:
    ProcessorDragHelper(NetworkEditor& editor);
    virtual ~ProcessorDragHelper();

    virtual bool eventFilter(QObject* obj, QEvent* event) override;

    void clear(ConnectionGraphicsItem* connection);
    void clear(ProcessorGraphicsItem* processor);

    bool enter(QGraphicsSceneDragDropEvent* e, const ProcessorMimeData* mime);
    bool move(QGraphicsSceneDragDropEvent* e, const ProcessorMimeData* mime);
    bool leave(QGraphicsSceneDragDropEvent* e, const ProcessorMimeData* mime);
    bool drop(QGraphicsSceneDragDropEvent* e, const ProcessorMimeData* mime);

private:
    void updateAutoConnections(QGraphicsSceneDragDropEvent* e);
    void updateAutoLinks(QGraphicsSceneDragDropEvent* e);

    void setText(QGraphicsSceneDragDropEvent* e, const std::string& processor);

    static bool canSplitConnection(Processor* p, ConnectionGraphicsItem* connection);

    void resetConnection();
    void resetProcessor();
    void resetAutoConnections();
    void resetAutoLinks();

    NetworkEditor& editor_;
    ConnectionGraphicsItem* connectionTarget_ = nullptr;
    ProcessorGraphicsItem* processorTarget_ = nullptr;

    std::vector<std::pair<Inport*, std::vector<Outport*>>> autoConnectCandidates_;
    std::unordered_map<Inport*, std::unique_ptr<ConnectionDragGraphicsItem>> autoConnections_;

    std::unique_ptr<AutoLinker> autoLinker_;
    std::unordered_map<Processor*, std::unique_ptr<LinkConnectionDragGraphicsItem>> autoLinks_;
};

}  // namespace inviwo
