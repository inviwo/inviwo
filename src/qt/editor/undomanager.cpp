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

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/qt/editor/inviwomainwindow.h>
#include <inviwo/qt/editor/undomanager.h>
#include <inviwo/core/util/raiiutils.h>
#include <inviwo/core/util/filesystem.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QAction>
#include <QEvent>
#include <QApplication>
#include <QGuiApplication>
#include <QMouseEvent>
#include <QTouchEvent>
#include <warn/pop>

namespace inviwo {

UndoManager::UndoManager(InviwoMainWindow* mainWindow)
    : mainWindow_(mainWindow)
    , manager_{mainWindow_->getInviwoApplication()->getWorkspaceManager()}
    , refPath_{filesystem::findBasePath()} {

    mainWindow_->getInviwoApplication()->getProcessorNetwork()->addObserver(this);

    undoAction_ = new QAction(QAction::tr("&Undo"), mainWindow_);
    undoAction_->setShortcut(QKeySequence::Undo);
    undoAction_->connect(undoAction_, &QAction::triggered, this, [&]() { undoState(); });

    redoAction_ = new QAction(QAction::tr("&Redo"), mainWindow_);
    redoAction_->setShortcut(QKeySequence::Redo);
    redoAction_->connect(redoAction_, &QAction::triggered, this, [&]() { redoState(); });

    clearHandle_ = manager_->onClear([&]() { 
        clear();
        pushState();
    });
    loadHandle_ = manager_->onLoad([&](Deserializer& d) { 
        if (isRestoring) return;
        clear();
        pushState();
    });

    updateActions();
    pushState();
}

void UndoManager::pushState() {
    if (isRestoring) return;

    std::stringstream stream;
    manager_->save(stream, refPath_);
    auto str = stream.str();
   
    if (head_ >= 0 && str == undoBuffer_[head_]) return; // No Change
   
    ++head_;
    auto offset = std::min(std::distance(undoBuffer_.begin(), undoBuffer_.end()), head_);
    undoBuffer_.erase(undoBuffer_.begin() + offset, undoBuffer_.end());
    undoBuffer_.emplace_back(str);
    dirty_ = false;
    updateActions();
}
void UndoManager::undoState() {
    if (head_ > 0) {
        util::KeepTrueWhileInScope restore(&isRestoring);
        --head_;

        std::stringstream stream;
        stream << undoBuffer_[head_];
        manager_->load(stream, refPath_);

        dirty_ = false;
        updateActions();
    }
}
void UndoManager::redoState() {
    if (head_ >= -1 && head_ < static_cast<DiffType>(undoBuffer_.size())-1) {

        util::KeepTrueWhileInScope restore(&isRestoring);
        ++head_;
        
        std::stringstream stream;
        stream << undoBuffer_[head_];     
        manager_->load(stream, refPath_);

        dirty_ = false;
        updateActions();
    }
}

void UndoManager::clear() {
    head_ = -1;
    undoBuffer_.clear();
}

QAction* UndoManager::getUndoAction() const {
    return undoAction_;
}

QAction* UndoManager::getRedoAction() const {
    return redoAction_;
}

#include <warn/push>
#include <warn/ignore/switch-enum>
bool UndoManager::eventFilter(QObject *obj, QEvent *event) {
    auto res = QObject::eventFilter(obj, event);

    // https://bugreports.qt.io/browse/QTBUG-39592 we get the same event multiple times.
    if (!obj->isWindowType()) return res;

    switch (event->type()) {
        case QEvent::MouseButtonRelease: {
            if (dirty_) pushState(); 
            break;
        }
        case QEvent::TouchEnd: {
            auto te = static_cast<QTouchEvent *>(event);
            if (util::all_of(te->touchPoints(), [](const QTouchEvent::TouchPoint &tp) {
                return tp.state() == Qt::TouchPointReleased;
            })) {
                if (dirty_) pushState(); 
                break;
            }
            break;
        }
        case QEvent::KeyRelease: {
            if (dirty_) pushState(); 
        }
        default:
            break;
    }
    return res;
}
#include <warn/pop>

void UndoManager::updateActions() {
    undoAction_->setEnabled(head_>0);
    redoAction_->setEnabled(head_ >= -1 && head_ < static_cast<DiffType>(undoBuffer_.size()) - 1);
}

void UndoManager::onProcessorNetworkChange() {
    dirty_ = true;
}
void UndoManager::onProcessorNetworkDidAddProcessor(Processor* processor) {
    dirty_ = true;
}
void UndoManager::onProcessorNetworkDidRemoveProcessor(Processor* processor) {
    dirty_ = true;
}
void UndoManager::onProcessorNetworkDidAddConnection(const PortConnection& connection) {
    dirty_ = true;
}
void UndoManager::onProcessorNetworkDidRemoveConnection(const PortConnection& connection) {
    dirty_ = true;
}
void UndoManager::onProcessorNetworkDidAddLink(const PropertyLink& propertyLink) {
    dirty_ = true;
}
void UndoManager::onProcessorNetworkDidRemoveLink(const PropertyLink& propertyLink) {
    dirty_ = true;
}
}  // namespace
