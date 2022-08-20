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

#include <inviwo/core/common/inviwocoredefine.h>

#include <string>

namespace inviwo {

class FileObserver;

/**
 * Abstraction around file system events, to allow different implementations to be provided.
 * A FileSystemObserver can be registered with the inviwo application to be used by the
 * FileObservers. A FileObserver can also use a FileSystemObserver directly.
 * @see FileObserver
 * @see InviwoApplication::setFileSystemObserver
 */
class IVW_CORE_API FileSystemObserver {
public:
    FileSystemObserver() = default;
    virtual ~FileSystemObserver() = default;

    /**
     * Register a FileObserver to get callback when files change.
     */
    virtual void registerFileObserver(FileObserver* fileObserver) = 0;
    /**
     * Unregister a FileObserver.
     */
    virtual void unRegisterFileObserver(FileObserver* fileObserver) = 0;

private:
    friend FileObserver;

    /**
     * Start observing a file or directory for changes.
     * The FileSystemObserver will call FileObserver::fileChanged on the registered observers
     * when changes are detected.
     * This function is only called by the FileObserver
     */
    virtual void startFileObservation(const std::string& fileName) = 0;

    /**
     * Stop observing changes to a file or directory.
     * This function is only called by the FileObserver
     */
    virtual void stopFileObservation(const std::string& fileName) = 0;
};

}  // namespace inviwo
