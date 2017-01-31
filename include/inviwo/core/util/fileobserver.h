/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2017 Inviwo Foundation
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

#ifndef IVW_FILEOBSERVER_H
#define IVW_FILEOBSERVER_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace inviwo {

/** \class FileObserver
 * Calls fileChanged when an observed file changes.
 * One or multiple files can be observed.
 */
class IVW_CORE_API FileObserver {

public:
    /** 
     * \brief Starts observing file if existing.
     *
     * @note Registers as a file observer in InviwoApplication.
     * 
     * @param filePath Full path to file to observe
     */
    FileObserver(const std::string& filePath = "");
    /**
     * Moves observed files in other, unregisters other and registers this observer. 
     */
    FileObserver(FileObserver&& other);
    /**
     * Unregisters file observer in InviwoApplication and stops observing all files.
     */
    virtual ~FileObserver();

    /** 
     * \brief Starts observing file if it exists.
     * Each call to startFileObservation should be matched with stopFileObservation
     * @param filePath Full path to file
     */
    void startFileObservation(const std::string filePath);
    /** 
     * \brief Stops observing the file if being observed.
     * 
     * @param filePath Full path to file
     * @return void 
     */
    void stopFileObservation(const std::string fileName);
    std::vector<std::string> getFiles() const;
    bool isObserved(const std::string& fileName) const;

    virtual void fileChanged(const std::string& fileName) = 0;

private:

// We can safely ignore the C4251 warning for private members.
#pragma warning( push )
#pragma warning( disable: 4251 )
    std::unordered_map<std::string, int> observedFiles_; ///< stores the files to be observed
#pragma warning( pop )
    ///< plus the number of observers for each
};

} // namespace

#endif // IVW_FILEOBSERVER_H
