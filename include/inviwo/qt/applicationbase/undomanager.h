/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2024 Inviwo Foundation
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

#include <inviwo/qt/applicationbase/qtapplicationbasemoduledefine.h>

#include <inviwo/core/network/workspacemanager.h>

#include <memory>
#include <optional>
#include <string>

#include <QObject>

class QAction;
class QEvent;

namespace inviwo {

class AutoSaver;
class ProcessorNetwork;

/**
 * \class UndoManager
 */
class IVW_QTAPPLICATIONBASE_API UndoManager : public QObject {
public:
    UndoManager(
        WorkspaceManager* wm, ProcessorNetwork* network,
        std::function<int()> numRestoreFiles = []() -> int { return 100000; },
        std::function<int()> restoreFrequency = []() -> int { return 1440; });
    UndoManager(const UndoManager&) = delete;
    UndoManager(UndoManager&&) = delete;
    UndoManager& operator=(const UndoManager&) = delete;
    UndoManager& operator=(UndoManager&&) = delete;
    virtual ~UndoManager();

    void pushStateIfDirty();
    void markDirty();

    void pushState();
    void undoState();
    void redoState();
    void clear();

    QAction* getUndoAction() const;
    QAction* getRedoAction() const;

    bool hasRestore() const;
    void restore();

    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    using DiffType = std::vector<std::pmr::string>::iterator::difference_type;

    void updateActions();

    ProcessorNetwork* network_;
    WorkspaceManager* manager_;
    std::filesystem::path refPath_;

    bool dirty_ = true;
    size_t triggerId_ = 0;
    bool isRestoring = false;
    DiffType head_ = -1;
    std::vector<std::shared_ptr<const std::pmr::string>> undoBuffer_;

    QAction* undoAction_;
    QAction* redoAction_;

    WorkspaceManager::ClearHandle clearHandle_;
    WorkspaceManager::DeserializationHandle loadHandle_;
    WorkspaceManager::ModifiedHandle modifiedHandle_;

    std::unique_ptr<AutoSaver> autoSaver_;
};

}  // namespace inviwo
