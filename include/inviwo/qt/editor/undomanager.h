/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2017 Inviwo Foundation
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

#ifndef IVW_UNDOMANAGER_H
#define IVW_UNDOMANAGER_H

#include <inviwo/qt/editor/inviwoqteditordefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/network/processornetworkobserver.h>
#include <inviwo/core/network/workspacemanager.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QObject>
#include <warn/pop>

class QAction;
class QEvent;

namespace inviwo {

class InviwoMainWindow;


/**
 * \class UndoManager
 */
class IVW_QTEDITOR_API UndoManager : public QObject, public ProcessorNetworkObserver {
public:
    UndoManager(InviwoMainWindow* mainWindow);
    virtual ~UndoManager() = default;

    void pushState();
    void undoState();
    void redoState();
    void clear();

    QAction* getUndoAction() const;
    QAction* getRedoAction() const;

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:
    using DiffType = std::vector<std::string>::iterator::difference_type;

    void updateActions();

    // ProcessorNetworkObserver overrides;
    virtual void onProcessorNetworkChange() override;
    virtual void onProcessorNetworkDidAddProcessor(Processor* processor) override;
    virtual void onProcessorNetworkDidRemoveProcessor(Processor* processor) override;
    virtual void onProcessorNetworkDidAddConnection(const PortConnection& connection) override;
    virtual void onProcessorNetworkDidRemoveConnection(const PortConnection& connection) override;
    virtual void onProcessorNetworkDidAddLink(const PropertyLink& propertyLink) override;
    virtual void onProcessorNetworkDidRemoveLink(const PropertyLink& propertyLink) override;

    InviwoMainWindow* mainWindow_;
    WorkspaceManager* manager_;
    std::string refPath_;

    bool dirty_ = true;
    bool isRestoring = false;
    DiffType head_ = -1;
    std::vector<std::string> undoBuffer_;

    QAction* undoAction_;
    QAction* redoAction_;

    WorkspaceManager::ClearHandle clearHandle_;
    WorkspaceManager::DeserializationHandle loadHandle_;
};

} // namespace

#endif // IVW_UNDOMANAGER_H

