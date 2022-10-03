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
#pragma once

#include <modules/glfw/glfwmoduledefine.h>

#include <inviwo/core/util/filesystemobserver.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace inviwo {

class WatcherThread;
class FileObserver;
class InviwoApplication;

/**
 * An implementation for FileSystemObserver using the windows api.
 * Currently does nothing on Mac / Linux
 */
class IVW_MODULE_GLFW_API FileWatcher : public FileSystemObserver {
public:
    FileWatcher(InviwoApplication* app = nullptr);
    FileWatcher(const FileWatcher&) = delete;
    FileWatcher(FileWatcher&&) = delete;
    FileWatcher& operator=(const FileWatcher&) = delete;
    FileWatcher& operator=(FileWatcher&&) = delete;
    virtual ~FileWatcher();

    virtual void registerFileObserver(FileObserver* fileObserver) override;
    virtual void unRegisterFileObserver(FileObserver* fileObserver) override;

private:
    virtual void startFileObservation(const std::string& fileName) override;
    virtual void stopFileObservation(const std::string& fileName) override;

    InviwoApplication* app_;
    std::unique_ptr<WatcherThread> watcher_;
    std::vector<FileObserver*> fileObservers_;
    std::unordered_map<std::string, std::unordered_set<std::string>> observed_;
};

}  // namespace inviwo
