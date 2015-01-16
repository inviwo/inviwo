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

#include <inviwo/core/util/fileobserver.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/common/inviwoapplication.h>

namespace inviwo {

FileObserver::FileObserver() {
    //  observedFiles_ = new std::vector<std::pair<std::string, int> >();
}

FileObserver::~FileObserver() {
    //    delete observedFiles_;
}

void FileObserver::startFileObservation(std::string fileName) {
    if (isObserved(fileName))
        increaseNumObservers(fileName);
    else {
        if (filesystem::fileExists(fileName)) {
            observedFiles_.push_back(std::pair<std::string,int>(fileName, 1));
            InviwoApplication::getPtr()->startFileObservation(fileName);
        }
    }
}

void FileObserver::stopFileObservation(std::string fileName) {
    if (isObserved(fileName)) {
        if (getNumObservers(fileName) > 1)
            decreaseNumObservers(fileName);
        else {
            observedFiles_.erase(std::remove(observedFiles_.begin(), observedFiles_.end(), std::pair<std::string,int>(fileName,
                                             getNumObservers(fileName))), observedFiles_.end());
            InviwoApplication::getPtr()->stopFileObservation(fileName);
        }
    }
}

void FileObserver::increaseNumObservers(std::string fileName) {
    for (size_t i=0; i<observedFiles_.size(); i++)
        if (observedFiles_.at(i).first == fileName)
            observedFiles_.at(i).second++;
}

void FileObserver::decreaseNumObservers(std::string fileName) {
    for (size_t i=0; i<observedFiles_.size(); i++)
        if (observedFiles_.at(i).first == fileName)
            observedFiles_.at(i).second--;
}

int FileObserver::getNumObservers(std::string fileName) {
    for (size_t i=0; i<observedFiles_.size(); i++)
        if (observedFiles_.at(i).first == fileName)
            return observedFiles_.at(i).second;

    return 0;
}

bool FileObserver::isObserved(std::string fileName) {
    return (getNumObservers(fileName) > 0);
}

std::vector<std::string> FileObserver::getFiles() {
    std::vector<std::string> files;

    for (size_t i=0; i<observedFiles_.size(); i++)
        files.push_back(observedFiles_.at(i).first);

    return files;
}

} // namespace