/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016 Inviwo Foundation
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
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/qt/editor/inviwomainwindow.h>
#include <inviwo/qt/editor/undomanager.h>
#include <inviwo/core/util/raiiutils.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QAction>
#include <QEvent>
#include <QApplication>
#include <QGuiApplication>

#include <warn/pop>

namespace inviwo {


UndoManager::UndoManager(InviwoMainWindow* mainWindow) : mainWindow_(mainWindow) {
    mainWindow_->getInviwoApplication()->getProcessorNetwork()->addObserver(this);

    interactionEndCallback_ =
        mainWindow_->getInviwoApplication()->getInteractionStateManager().onInteractionEnd(
            [&]() { if (dirty_) pushState(); });

    undoAction_ = new QAction(QAction::tr("&Undo"), mainWindow_);
    undoAction_->setShortcut(QKeySequence::Undo);
    undoAction_->connect(undoAction_, &QAction::triggered, [&]() {
        undoState();
    });

    redoAction_ = new QAction(QAction::tr("&Redo"), mainWindow_);
    redoAction_->setShortcut(QKeySequence::Redo);
    redoAction_->connect(redoAction_, &QAction::triggered, [&]() {
        redoState();
    });

    updateActions();

    pushState();
}

void UndoManager::pushState() {
    if (isRestoring) return;
    if (mainWindow_->getInviwoApplication()->getProcessorNetwork()->islocked()) return;
    if (mainWindow_->getInviwoApplication()->getInteractionStateManager().isInteracting()) return;

    auto path = mainWindow_->getCurrentWorkspace();
    Serializer s(path);
    mainWindow_->getInviwoApplication()->getProcessorNetwork()->serialize(s);
    std::stringstream stream;
    s.writeFile(stream);
    auto str = stream.str();
   
    if (head_ >= 0 && str == undoBuffer_[head_]) return; // No Change
   
    ++head_;
    auto offset = std::min(std::distance(undoBuffer_.begin(), undoBuffer_.end()), head_);
    undoBuffer_.erase(undoBuffer_.begin() + offset, undoBuffer_.end());
    undoBuffer_.emplace_back(str);
    dirty_ = false;
    updateActions();

    //LogInfo("Push state: " << head_ << " (" << undoBuffer_.size() << ")");
}
void UndoManager::undoState() {
    if (head_ > 0) {
        util::KeepTrueWhileInScope restore(&isRestoring);
        auto path = mainWindow_->getCurrentWorkspace();

        std::stringstream stream;
        --head_;
        stream << undoBuffer_[head_];
        Deserializer d(mainWindow_->getInviwoApplication(), stream, path);
        mainWindow_->getInviwoApplication()->getProcessorNetwork()->deserialize(d);

        dirty_ = false;
        updateActions();

        //LogInfo("Undo state: " << head_ << " (" << undoBuffer_.size() << ")");
    }
}
void UndoManager::redoState() {
    if (head_ >= -1 && head_ < static_cast<DiffType>(undoBuffer_.size())-1) {

        util::KeepTrueWhileInScope restore(&isRestoring);
        auto path = mainWindow_->getCurrentWorkspace();

        std::stringstream stream;
        ++head_;
        stream << undoBuffer_[head_];
        Deserializer d(mainWindow_->getInviwoApplication(), stream, path);
        mainWindow_->getInviwoApplication()->getProcessorNetwork()->deserialize(d);

        dirty_ = false;
        updateActions();

        //LogInfo("Redo state: " << head_ << " (" << undoBuffer_.size() << ")");
    }
}

QAction* UndoManager::getUndoAction() const {
    return undoAction_;
}

QAction* UndoManager::getRedoAction() const {
    return redoAction_;
}

void UndoManager::updateActions() {
    undoAction_->setEnabled(head_>0);
    redoAction_->setEnabled(head_ >= -1 && head_ < static_cast<DiffType>(undoBuffer_.size()) - 1);
}

void UndoManager::onProcessorNetworkUnlocked() {
    if (dirty_) pushState(); 
}

void UndoManager::onProcessorNetworkChange() {
    dirty_ = true;
    pushState();
}
void UndoManager::onProcessorNetworkDidAddProcessor(Processor* processor) {
    dirty_ = true;
    pushState();
}
void UndoManager::onProcessorNetworkDidRemoveProcessor(Processor* processor) {
    dirty_ = true;
    pushState();
}
void UndoManager::onProcessorNetworkDidAddConnection(const PortConnection& connection) {
    dirty_ = true;
    pushState();
}
void UndoManager::onProcessorNetworkDidRemoveConnection(const PortConnection& connection) {
    dirty_ = true;
    pushState();
}
void UndoManager::onProcessorNetworkDidAddLink(const PropertyLink& propertyLink) {
    dirty_ = true;
    pushState();
}
void UndoManager::onProcessorNetworkDidRemoveLink(const PropertyLink& propertyLink) {
    dirty_ = true;
    pushState();
}
}  // namespace
