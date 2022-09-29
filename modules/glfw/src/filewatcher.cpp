/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2022 Inviwo Foundation
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

#include <modules/glfw/filewatcher.h>

#include <inviwo/core/util/assertion.h>      // for IVW_ASSERT
#include <inviwo/core/util/logcentral.h>     // for LogCentral, LogWarn
#include <inviwo/core/util/stdextensions.h>  // for erase_remove

#include <algorithm>                         // for find

#ifdef WIN32
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/threadutil.h>
#include <inviwo/core/util/zip.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/util/fileobserver.h>

#include <Windows.h>
#include <array>
#include <mutex>
#include <atomic>
#include <chrono>
#endif

namespace inviwo {

#ifdef WIN32

class WatcherThread {
public:
    enum class Action { Added, Removed, Modified };

    WatcherThread(
        std::function<void(const std::string&, const std::string&, Action)> changeCallback)
        : changeCallback_{std::move(changeCallback)} {

        util::setThreadDescription(thread_, "Inviwo File Watcher Thread");
    }

    ~WatcherThread() {
        stop_ = true;
        thread_.join();
    }

    bool addObservation(const std::string& path) {
        std::scoped_lock lock{mutex_};
        if (active_ + toAdd_.size() - toRemove_.size() + 1 < MAXIMUM_WAIT_OBJECTS) {
            toAdd_.push_back(path);
            return true;
        } else {
            return false;
        }
    }
    void removeObservation(const std::string& path) {
        std::scoped_lock lock{mutex_};
        toRemove_.push_back(path);
    }

private:
    void remove(const std::vector<std::string>& toRemove) {
        auto range = util::zip(handles_, observed_);
        auto it = std::remove_if(range.begin(), range.end(), [&](auto&& elem) {
            return std::find(toRemove.begin(), toRemove.end(), elem.second().first) !=
                   toRemove.end();
        });
        for (auto&& [handle, observed] : util::as_range(it, range.end())) {
            FindCloseChangeNotification(handle);
            observed.first.clear();
            observed.second.clear();
            --active_;
        }
    }

    void add(const std::vector<std::string>& toAdd) {
        for (auto& path : toAdd) {
            auto wpath = util::toWstring(path);
            const auto handle = FindFirstChangeNotification(wpath.c_str(), TRUE, filter);

            if (handle == INVALID_HANDLE_VALUE || handle == nullptr) {
                LogError("FindFirstChangeNotification function failed.");
                continue;
            }

            handles_[active_] = handle;
            observed_[active_].first = path;
            for (auto& elem : filesystem::getDirectoryContentsRecursively(
                     path, filesystem::ListMode::FilesAndDirectories)) {
                observed_[active_].second[elem] = filesystem::fileModificationTime(elem);
            }

            ++active_;
        }
    }

    void watch() {
        while (!stop_) {
            {
                std::scoped_lock lock{mutex_};
                if (!toRemove_.empty()) {
                    remove(toRemove_);
                    toRemove_.clear();
                }
                if (!toAdd_.empty()) {
                    add(toAdd_);
                    toAdd_.clear();
                }
            }
            if (active_ == 0) {
                std::this_thread::sleep_for(timeout_);
            } else {
                const auto status =
                    WaitForMultipleObjects(static_cast<DWORD>(active_), handles_.data(), FALSE,
                                           static_cast<DWORD>(timeout_.count()));
                if (status >= WAIT_OBJECT_0 && status < WAIT_OBJECT_0 + MAXIMUM_WAIT_OBJECTS) {

                    const auto& path = observed_[status - WAIT_OBJECT_0].first;
                    auto& files = observed_[status - WAIT_OBJECT_0].second;

                    const auto changedFiles = getChangedAndUpdateFiles(path, files);
                    for (auto&& [changedFile, action] : changedFiles) {
                        changeCallback_(path, changedFile, action);
                    }

                    const auto handle = handles_[status - WAIT_OBJECT_0];
                    FindNextChangeNotification(handle);
                }
            }
        }
        for (auto handle : util::as_range(handles_.begin(), handles_.begin() + active_)) {
            FindCloseChangeNotification(handle);
        }
    }

    // Update the time stamps on all files and return the changed ones.
    std::vector<std::pair<std::string, Action>> getChangedAndUpdateFiles(
        const std::string& path, std::unordered_map<std::string, time_t>& files) {

        std::vector<std::pair<std::string, Action>> changed;

        util::map_erase_remove_if(files, [&](auto& item) {
            if (!filesystem::fileExists(item.first)) {
                changed.emplace_back(item.first, Action::Removed);
                return true;
            } else {
                return false;
            }
        });

        for (auto& elem : filesystem::getDirectoryContentsRecursively(
                 path, filesystem::ListMode::FilesAndDirectories)) {

            auto it = files.find(elem);
            if (it == files.end()) {
                changed.emplace_back(elem, Action::Removed);
                files[elem] = filesystem::fileModificationTime(elem);
            } else {
                auto newTime = filesystem::fileModificationTime(elem);
                if (newTime > it->second) {
                    changed.emplace_back(elem, Action::Modified);
                    it->second = newTime;
                }
            }
        }

        return changed;
    }

    static constexpr DWORD filter =
        FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE;

    std::array<HANDLE, MAXIMUM_WAIT_OBJECTS> handles_{};
    std::array<std::pair<std::string, std::unordered_map<std::string, time_t>>,
               MAXIMUM_WAIT_OBJECTS>
        observed_{};
    std::atomic<size_t> active_ = 0;

    std::function<void(const std::string&, const std::string&, Action)> changeCallback_;
    std::mutex mutex_;
    std::vector<std::string> toAdd_;
    std::vector<std::string> toRemove_;
    std::atomic<bool> stop_{false};
    std::chrono::milliseconds timeout_{1000};
    std::thread thread_{[this]() { watch(); }};
};

FileWatcher::FileWatcher(InviwoApplication* app)
    : app_{app}
    , watcher_{std::make_unique<WatcherThread>(
          [this](const std::string& dir, const std::string& path, WatcherThread::Action) {
              auto notifyAboutChanges = [this, dir, path]() {
                  if (filesystem::fileExists(path)) {
                      // don't use iterators here, they might be invalidated.
                      const auto orgSize = fileObservers_.size();
                      for (size_t i = 0; i < orgSize && i < fileObservers_.size(); ++i) {
                          if (fileObservers_[i]->isObserved(path)) {
                              fileObservers_[i]->fileChanged(path);
                          }
                      }
                  }
                  if (filesystem::directoryExists(dir)) {
                      // don't use iterators here, they might be invalidated.
                      const auto orgSize = fileObservers_.size();
                      for (size_t i = 0; i < orgSize && i < fileObservers_.size(); ++i) {
                          if (fileObservers_[i]->isObserved(dir)) {
                              fileObservers_[i]->fileChanged(dir);
                          }
                      }
                  }
              };

              if (app_) {
                  app_->dispatchFront(notifyAboutChanges);
              } else {
                  notifyAboutChanges();
              }
          })} {}

FileWatcher::~FileWatcher() = default;

void FileWatcher::registerFileObserver(FileObserver* fileObserver) {
    IVW_ASSERT(std::find(fileObservers_.cbegin(), fileObservers_.cend(), fileObserver) ==
                   fileObservers_.cend(),
               "File observer already registered.");
    fileObservers_.push_back(fileObserver);
}

void FileWatcher::unRegisterFileObserver(FileObserver* fileObserver) {
    const auto it = std::find(fileObservers_.begin(), fileObservers_.end(), fileObserver);
    if (it != fileObservers_.end()) {
        fileObservers_.erase(it);
    }
}

void FileWatcher::startFileObservation(const std::string& fileName) {
    const bool isDirectory = filesystem::directoryExists(fileName);
    const auto dir = isDirectory ? fileName : filesystem::getFileDirectory(fileName);

    const auto it = observed_.find(dir);
    if (it == observed_.end()) {
        observed_[dir].insert(fileName);
        if (!watcher_->addObservation(dir)) {
            LogError("Can't watch more files");
        }
    } else {
        it->second.insert(fileName);
    }
}

void FileWatcher::stopFileObservation(const std::string& fileName) {
    auto observerit =
        std::find_if(std::begin(fileObservers_), std::end(fileObservers_),
                     [fileName](const auto observer) { return observer->isObserved(fileName); });
    // Make sure that no observer is observing the file
    if (observerit == std::end(fileObservers_)) {
        const bool isDirectory = filesystem::directoryExists(fileName);
        const auto dir = isDirectory ? fileName : filesystem::getFileDirectory(fileName);

        const auto it = observed_.find(dir);
        if (it != observed_.end()) {
            it->second.erase(fileName);
            if (it->second.empty()) {
                watcher_->removeObservation(dir);
                observed_.erase(it);
            }
        }
    }
}

#else

class WatcherThread {
public:
    WatcherThread() = default;
};

FileWatcher::FileWatcher(InviwoApplication* app) : app_{app} {
    (void)app_;
    LogWarn("FileObserver are currently not supported using GLFW on this platform");
}

FileWatcher::~FileWatcher() = default;

void FileWatcher::registerFileObserver(FileObserver* fileObserver) {
    IVW_ASSERT(std::find(fileObservers_.cbegin(), fileObservers_.cend(), fileObserver) ==
                   fileObservers_.cend(),
               "File observer already registered.");
    fileObservers_.push_back(fileObserver);
}

void FileWatcher::unRegisterFileObserver(FileObserver* fileObserver) {
    util::erase_remove(fileObservers_, fileObserver);
}

void FileWatcher::stopFileObservation(const std::string& fileName) {}
void FileWatcher::startFileObservation(const std::string& fileName) {}

#endif

}  // namespace inviwo
