/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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
#include <inviwo/qt/applicationbase/inviwoapplicationqt.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QAction>
#include <QEvent>
#include <QApplication>
#include <QGuiApplication>
#include <warn/pop>

#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <vector>
#include <string>

namespace inviwo {

class AutoSaver {
public:
    AutoSaver()
        : path_{filesystem::getPath(PathType::Settings)}
        , restored_{[this]() -> std::optional<std::string> {
            if (filesystem::fileExists(path_ + "/autosave.inv")) {
                auto ifstream = filesystem::ifstream(path_ + "/autosave.inv");
                std::stringstream buffer;
                buffer << ifstream.rdbuf();
                return std::move(buffer).str();
            }

            return std::nullopt;
        }()}
        , quit_{false}
        , saver_{[this]() {
            for (;;) {

                std::shared_ptr<const std::string> str;
                {
                    std::unique_lock<std::mutex> lock(mutex_);
                    while (!quit_ && toSave_.empty()) {
                        condition_.wait(lock);
                    }

                    if (quit_) return;

                    if (!toSave_.empty()) {
                        str = std::move(toSave_.back());
                        toSave_.clear();
                    }
                }

                if (str) {
                    auto ofstream = filesystem::ofstream(path_ + "/autosave.inv.tmp");
                    ofstream << *str;
                    filesystem::copyFile(path_ + "/autosave.inv.tmp", path_ + "/autosave.inv");
                }
            }
        }} {}

    ~AutoSaver() {
        quit_ = true;
        condition_.notify_one();
        saver_.join();
    }

    const std::optional<std::string> &getRestored() const { return restored_; }

    void save(std::shared_ptr<const std::string> str) {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            toSave_.push_back(str);
        }
        condition_.notify_one();
    }

private:
    std::string path_;
    std::optional<std::string> restored_;
    std::atomic<bool> quit_;
    std::condition_variable condition_;
    std::mutex mutex_;
    std::vector<std::shared_ptr<const std::string>> toSave_;

    std::thread saver_;
};

UndoManager::UndoManager(InviwoMainWindow *mainWindow)
    : mainWindow_(mainWindow)
    , manager_{mainWindow_->getInviwoApplication()->getWorkspaceManager()}
    , refPath_{filesystem::findBasePath()}
    , autoSaver_{std::make_unique<AutoSaver>()} {

    mainWindow_->getInviwoApplicationQt()->setUndoTrigger([this]() { pushStateIfDirty(); });
    mainWindow_->getInviwoApplication()->getProcessorNetwork()->addObserver(this);

    undoAction_ = new QAction(QIcon(":/svgicons/undo.svg"), QAction::tr("&Undo"), mainWindow_);
    undoAction_->setShortcut(QKeySequence::Undo);
    undoAction_->connect(undoAction_, &QAction::triggered, [this]() { undoState(); });

    redoAction_ = new QAction(QIcon(":/svgicons/redo.svg"), QAction::tr("&Redo"), mainWindow_);
    redoAction_->setShortcut(QKeySequence::Redo);
    redoAction_->connect(redoAction_, &QAction::triggered, [this]() { redoState(); });

    clearHandle_ = manager_->onClear([&]() {
        clear();
        pushState();
    });
    loadHandle_ = manager_->onLoad([&](Deserializer &) {
        if (isRestoring) return;
        clear();
        pushState();
    });

    updateActions();
    pushState();
}

UndoManager::~UndoManager() = default;

void UndoManager::pushStateIfDirty() {
    if (dirty_) pushState();
}
void UndoManager::markDirty() { dirty_ = true; }

void UndoManager::pushState() {
    if (isRestoring) return;

    std::stringstream stream;
    try {
        manager_->save(stream, refPath_, [](ExceptionContext context) -> void { throw; },
                       WorkspaceSaveMode::Undo);
    } catch (...) {
        return;
    }
    auto str = std::make_shared<const std::string>(std::move(stream).str());

    dirty_ = false;
    if (head_ >= 0 && *str == *undoBuffer_[head_]) return;  // No Change

    ++head_;
    auto offset = std::min(std::distance(undoBuffer_.begin(), undoBuffer_.end()), head_);
    undoBuffer_.erase(undoBuffer_.begin() + offset, undoBuffer_.end());
    undoBuffer_.push_back(str);

    autoSaver_->save(str);

    updateActions();
}
void UndoManager::undoState() {
    if (head_ > 0) {
        util::KeepTrueWhileInScope restore(&isRestoring);
        --head_;

        std::stringstream stream;
        stream << *undoBuffer_[head_];
        manager_->load(stream, refPath_);

        dirty_ = false;
        updateActions();
    }
}
void UndoManager::redoState() {
    if (head_ >= -1 && head_ < static_cast<DiffType>(undoBuffer_.size()) - 1) {

        util::KeepTrueWhileInScope restore(&isRestoring);
        ++head_;

        std::stringstream stream;
        stream << *undoBuffer_[head_];
        manager_->load(stream, refPath_);

        dirty_ = false;
        updateActions();
    }
}

void UndoManager::clear() {
    head_ = -1;
    undoBuffer_.clear();
}

QAction *UndoManager::getUndoAction() const { return undoAction_; }

QAction *UndoManager::getRedoAction() const { return redoAction_; }

bool UndoManager::hasRestore() const {
    if (auto &str = autoSaver_->getRestored()) {
        return !str->empty();
    }
    return false;
}

void UndoManager::restore() {
    if (const auto &str = autoSaver_->getRestored()) {
        std::stringstream stream;
        if (!str->empty()) {
            stream << *str;
            manager_->load(stream, refPath_);
        }
    }
}

void UndoManager::updateActions() {
    undoAction_->setEnabled(head_ > 0);
    redoAction_->setEnabled(head_ >= -1 && head_ < static_cast<DiffType>(undoBuffer_.size()) - 1);
}

void UndoManager::onProcessorNetworkChange() { dirty_ = true; }
void UndoManager::onProcessorNetworkDidAddProcessor(Processor *) { dirty_ = true; }
void UndoManager::onProcessorNetworkDidRemoveProcessor(Processor *) { dirty_ = true; }
void UndoManager::onProcessorNetworkDidAddConnection(const PortConnection &) { dirty_ = true; }
void UndoManager::onProcessorNetworkDidRemoveConnection(const PortConnection &) { dirty_ = true; }
void UndoManager::onProcessorNetworkDidAddLink(const PropertyLink &) { dirty_ = true; }
void UndoManager::onProcessorNetworkDidRemoveLink(const PropertyLink &) { dirty_ = true; }
}  // namespace inviwo
