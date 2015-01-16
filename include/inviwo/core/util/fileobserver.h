/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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
#include <vector>
#include <utility>

namespace inviwo {

class IVW_CORE_API FileObserver {

public:
    FileObserver();
    virtual ~FileObserver();

    void startFileObservation(std::string fileName);
    void stopFileObservation(std::string fileName);
    std::vector<std::string> getFiles();

    bool isObserved(std::string fileName);
    void increaseNumObservers(std::string fileName);
    void decreaseNumObservers(std::string fileName);
    int getNumObservers(std::string fileName);

    virtual void fileChanged(std::string fileName) = 0;

private:
    std::vector<std::pair<std::string, int> > observedFiles_; ///< stores the files to be observed
    ///< plus the number of observers for each
};

} // namespace

#endif // IVW_FILEOBSERVER_H
