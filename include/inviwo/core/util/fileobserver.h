/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2020 Inviwo Foundation
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

#include <inviwo/core/common/inviwocoredefine.h>

#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

namespace inviwo {

class InviwoApplication;
class FileSystemObserver;

/** \class FileObserver
 * Calls fileChanged when an observed file/directory changes.
 * One or multiple files/directories can be observed.
 */
class IVW_CORE_API FileObserver {
public:
    /**
     * @note Registers as a file observer in InviwoApplication.
     */
    FileObserver(InviwoApplication* app);

    /**
     * @note Registers as a file observer in FileSystemObserver.
     */
    FileObserver(FileSystemObserver* app);

    FileObserver(const FileObserver&) = delete;
    FileObserver& operator=(const FileObserver&) = delete;
    FileObserver(FileObserver&& rhs);
    FileObserver& operator=(FileObserver&& that);

    /**
     * Unregisters file observer in InviwoApplication and stops observing all files.
     */
    virtual ~FileObserver();

    /**
     * \brief Starts observing file if it exists.
     * @param filePath Full path to file
     */
    bool startFileObservation(const std::string& filePath);
    /**
     * \brief Stops observing the file if being observed.
     * @param filePath Full path to file
     */
    bool stopFileObservation(const std::string& filePath);

    /**
     * Stop observation of all observed files
     */
    void stopAllObservation();

    const std::unordered_set<std::string>& getFiles() const;
    bool isObserved(const std::string& fileName) const;

    virtual void fileChanged(const std::string& fileName) = 0;

protected:
    FileSystemObserver* fileSystemObserver_;

private:
    std::unordered_set<std::string> observedFiles_;
};

}  // namespace inviwo
