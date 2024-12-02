/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024 Inviwo Foundation
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
#include <inviwo/qt/editor/connectiongraphicsitem.h>
#include <inviwo/qt/editor/linkgraphicsitem.h>
#include <inviwo/core/network/autolinker.h>

#include <unordered_map>
#include <vector>

#include <QPointF>
#include <Qt>

namespace inviwo {

class Inport;
class Outport;
class Processor;
class ProcessorNetwork;

class NetworkEditor;
class ProcessorGraphicsItem;

class IVW_QTEDITOR_API NetworkAutomation {
public:
    NetworkAutomation(NetworkEditor& editor);
    NetworkAutomation(const NetworkAutomation&) = delete;
    NetworkAutomation(NetworkAutomation&&) = delete;
    NetworkAutomation& operator=(const NetworkAutomation&) = delete;
    NetworkAutomation& operator=(NetworkAutomation&&) = delete;

    void enter(QPointF scenePos, Qt::KeyboardModifiers modifiers, Processor& processor,
               double zoom = 1.0);
    void move(QPointF scenePos, Qt::KeyboardModifiers modifiers, Processor& processor,
              double zoom = 1.0);
    void leave();

    enum class Result { None, Replace, Split, AutoConnect };
    Result drop(QPointF scenePos, Qt::KeyboardModifiers modifiers, Processor& processor);

    void clear(ConnectionGraphicsItem* connection);
    void clear(ProcessorGraphicsItem* processor);

    NetworkEditor& editor_;

    struct IVW_QTEDITOR_API AutoIn {
        AutoIn() = default;
        AutoIn(const AutoIn&) = delete;
        AutoIn(AutoIn&&) = delete;
        AutoIn& operator=(const AutoIn&) = delete;
        AutoIn& operator=(AutoIn&&) = delete;
        std::vector<std::pair<Inport*, std::vector<Outport*>>> candidates;
        std::unordered_map<Inport*, std::unique_ptr<ConnectionDragGraphicsItem>> connections;

        void findCandidates(Processor& processor, ProcessorNetwork& network);

        void update(QPointF scenePos, Qt::KeyboardModifiers modifiers, NetworkEditor& editor,
                    double zoom);
        void clear();
    };
    struct IVW_QTEDITOR_API AutoOut {
        AutoOut() = default;
        AutoOut(const AutoOut&) = delete;
        AutoOut(AutoOut&&) = delete;
        AutoOut& operator=(const AutoOut&) = delete;
        AutoOut& operator=(AutoOut&&) = delete;
        std::vector<std::pair<Outport*, std::vector<Inport*>>> candidates;
        std::unordered_map<Outport*, std::unique_ptr<ConnectionInportDragGraphicsItem>> connections;

        void findCandidates(Processor& processor, ProcessorNetwork& network);

        void update(QPointF scenePos, Qt::KeyboardModifiers modifiers, NetworkEditor& editor,
                    double zoom);
        void clear();
    };

    struct IVW_QTEDITOR_API AutoLink {
        AutoLink() = default;
        AutoLink(const AutoLink&) = delete;
        AutoLink(AutoLink&&) = delete;
        AutoLink& operator=(const AutoLink&) = delete;
        AutoLink& operator=(AutoLink&&) = delete;

        void findCandidates(Processor& processor, ProcessorNetwork& network);

        void update(QPointF scenePos, Qt::KeyboardModifiers modifiers, NetworkEditor& editor,
                    double zoom);
        void clear();

        AutoLinker linker;
        std::unordered_map<Processor*, std::unique_ptr<LinkConnectionDragGraphicsItem>> links;
    };

    AutoIn inports_;
    AutoOut outports_;
    AutoLink links_;

    ConnectionGraphicsItem* connectionTarget_ = nullptr;
    ProcessorGraphicsItem* processorTarget_ = nullptr;

    static constexpr Qt::KeyboardModifier noAutoLink = Qt::AltModifier;
    static constexpr Qt::KeyboardModifier autoInport = Qt::ShiftModifier;
    static constexpr Qt::KeyboardModifier autoOutport = Qt::ControlModifier;
};

}  // namespace inviwo
