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

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/qt/applicationbase/undomanager.h>
#include <inviwo/core/util/raiiutils.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/threadutil.h>

#include <QAction>
#include <QEvent>
#include <QApplication>
#include <QGuiApplication>
#include <QMouseEvent>
#include <QTouchEvent>
#include <QTimer>

#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <vector>
#include <string>
#include <filesystem>

#include <fmt/format.h>
#include <fmt/chrono.h>

namespace inviwo {

class AutoSaver {
public:
    using clock_t = std::chrono::system_clock;

    AutoSaver(
        std::function<int()> numRestoreFiles = []() -> int { return 100000; },
        std::function<int()> restoreFrequency = []() -> int { return 1440; })
        : path_{filesystem::getPath(PathType::Settings)}
        , sessionStart_{clock_t::now()}
        , restored_{[this]() -> std::optional<std::string> {
            if (std::filesystem::is_regular_file(path_ / "autosave.inv")) {
                auto ifstream = std::ifstream(path_ / "autosave.inv");
                std::stringstream buffer;
                buffer << ifstream.rdbuf();
                return std::move(buffer).str();
            }

            return std::nullopt;
        }()}
        , quit_{false}
        , saver_{[this, numRestoreFiles, restoreFrequency]() {
            util::setThreadDescription("Inviwo AutoSave");
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
                    try {
                        std::filesystem::create_directories(path_ / "autosaves");
                        {
                            // make sure we have closed the file _before_ we copy it.
                            auto ofstream = std::ofstream(path_ / "autosave.inv.tmp");
                            ofstream << *str;
                        }

                        std::filesystem::copy(path_ / "autosave.inv.tmp", path_ / "autosave.inv",
                                              std::filesystem::copy_options::overwrite_existing);

                        if (clock_t::now() - sessionStart_ >
                            std::chrono::minutes(restoreFrequency())) {
                            sessionStart_ = clock_t::now();
                        }
                        std::filesystem::copy(path_ / "autosave.inv.tmp",
                                              path_ / "autosaves" / sessionName(sessionStart_),
                                              std::filesystem::copy_options::overwrite_existing);

                        clearOldSaves(path_ / "autosaves", numRestoreFiles());
                    } catch (const std::exception& e) {
                        LogInfo("Error saving auto save: " << e.what());
                    }
                }
            }
        }} {}

    ~AutoSaver() {
        quit_ = true;
        condition_.notify_one();
        saver_.join();
    }

    const std::optional<std::string>& getRestored() const { return restored_; }

    void save(std::shared_ptr<const std::string> str) {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            toSave_.push_back(str);
        }
        condition_.notify_one();
    }

private:
    static std::filesystem::path sessionName(clock_t::time_point now) {
        return fmt::format("autosave-{}-{:%Y-%m-%d-%H-%M}.inv", util::getPid(), now);
    }

    static void clearOldSaves(const std::filesystem::path& path, size_t maxSaves) {
        std::filesystem::directory_iterator it(path);

        std::vector<std::filesystem::path> files{it, end(it)};

        std::sort(files.begin(), files.end(),
                  [](const std::filesystem::path& a, const std::filesystem::path& b) {
                      return std::filesystem::last_write_time(a) >
                             std::filesystem::last_write_time(b);
                  });

        std::for_each(files.begin() + std::min(files.size(), size_t{maxSaves}), files.end(),
                      [](const std::filesystem::path& a) { std::filesystem::remove(a); });
    }

    std::filesystem::path path_;
    clock_t::time_point sessionStart_;

    std::optional<std::string> restored_;
    std::atomic<bool> quit_;
    std::condition_variable condition_;
    std::mutex mutex_;
    std::vector<std::shared_ptr<const std::string>> toSave_;

    std::thread saver_;
};

UndoManager::UndoManager(WorkspaceManager* wm, ProcessorNetwork* network,
                         std::function<int()> numRestoreFiles,
                         std::function<int()> restoreFrequency)
    : network_{network}
    , manager_{wm}
    , refPath_{filesystem::findBasePath()}
    , autoSaver_{std::make_unique<AutoSaver>(numRestoreFiles, restoreFrequency)} {

    QApplication::instance()->installEventFilter(this);

    undoAction_ = new QAction(QIcon(":/svgicons/undo.svg"), QAction::tr("&Undo"), this);
    undoAction_->setShortcut(QKeySequence::Undo);
    undoAction_->connect(undoAction_, &QAction::triggered, [this]() { undoState(); });

    redoAction_ = new QAction(QIcon(":/svgicons/redo.svg"), QAction::tr("&Redo"), this);
    redoAction_->setShortcut(QKeySequence::Redo);
    redoAction_->connect(redoAction_, &QAction::triggered, [this]() { redoState(); });

    clearHandle_ = manager_->onClear([&]() {
        clear();
        pushState();
    });
    loadHandle_ = manager_->onLoad([&](Deserializer&) {
        if (isRestoring) return;
        clear();
        pushState();
    });

    modifiedHandle_ = wm->onModified([this](bool modified) {
        if (modified) {
            dirty_ = true;
        }
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
        manager_->save(
            stream, refPath_, [](ExceptionContext context) -> void { throw; },
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

    if (!network_->empty()) {
        autoSaver_->save(str);
    }

    updateActions();
}
void UndoManager::undoState() {
    if (head_ > 0) {
        util::KeepTrueWhileInScope restore(&isRestoring);
        --head_;

        std::stringstream stream;
        stream << *undoBuffer_[head_];
        manager_->load(stream, refPath_, StandardExceptionHandler{}, WorkspaceSaveMode::Undo);

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
        manager_->load(stream, refPath_, StandardExceptionHandler{}, WorkspaceSaveMode::Undo);

        dirty_ = false;
        updateActions();
    }
}

void UndoManager::clear() {
    head_ = -1;
    undoBuffer_.clear();
}

QAction* UndoManager::getUndoAction() const { return undoAction_; }

QAction* UndoManager::getRedoAction() const { return redoAction_; }

bool UndoManager::hasRestore() const {
    if (auto& str = autoSaver_->getRestored()) {
        return !str->empty();
    }
    return false;
}

void UndoManager::restore() {
    if (const auto& str = autoSaver_->getRestored()) {
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

#include <warn/push>
#include <warn/ignore/switch-enum>
bool UndoManager::eventFilter(QObject*, QEvent* e) {
    const bool triggerPush = [&]() {
        switch (e->type()) {
            case QEvent::MouseButtonRelease: {
                return true;
            }
            case QEvent::NonClientAreaMouseButtonRelease: {
                return true;
            }
            case QEvent::TouchEnd: {
                auto te = static_cast<QTouchEvent*>(e);
                return util::all_of(te->points(), [](const QTouchEvent::TouchPoint& tp) {
                    return tp.state() == QEventPoint::Released;
                });
            }
            case QEvent::KeyRelease: {
                return true;
            }
            case QEvent::Drop: {
                return true;
            }
            default:
                return false;
        }
    }();

    if (triggerPush) {
        ++triggerId_;
        QTimer::singleShot(100, this, [this, id = triggerId_]() {
            if (id == triggerId_) {
                pushStateIfDirty();
            }
        });
    }

    return false;
}
#include <warn/pop>

}  // namespace inviwo
