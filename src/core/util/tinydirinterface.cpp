/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015 Inviwo Foundation
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

#include <inviwo/core/util/tinydirinterface.h>
#include <inviwo/core/util/exception.h>
#include <algorithm>
#include <cerrno>

namespace inviwo {

TinyDirInterface::TinyDirInterface() 
    : isOpen_(false)
    , mode_(ListMode::FilesOnly)
    , path_() 
{
    // initialize tinydir struct
    memset(&resource_, 0, sizeof(tinydir_dir));
}

TinyDirInterface::~TinyDirInterface() {
    close();
}

bool TinyDirInterface::open(const std::string &path) {
    // close previous directory
    close();

    if (path.empty())
        return false;

    path_ = path;
    int errCode = tinydir_open(&resource_, path_.c_str());
    isOpen_ = (errCode == 0);

    return isOpen_;
}

void TinyDirInterface::close() {
    if (isOpen_) {
        path_ = std::string();
        tinydir_close(&resource_);
        isOpen_ = false;
    }
}


void TinyDirInterface::setListMode(ListMode mode) {
    mode_ = mode;
}

TinyDirInterface::ListMode TinyDirInterface::getListMode() const {
    return mode_;
}

bool TinyDirInterface::isOpen() const {
    return isOpen_;
}

bool TinyDirInterface::isNextEntryAvailable() const {
    return (isOpen_ && resource_.has_next);
}

std::string TinyDirInterface::getNextEntry() {
    return getNextEntry(false);

}

std::string TinyDirInterface::getNextEntryWithBasePath() {
    return getNextEntry(true);
}

std::vector<std::string> TinyDirInterface::getContents() {
    std::vector<std::string> files;
    while (isNextEntryAvailable()) {
        std::string entry{ getNextEntry(false) };
        // getNextEntry might return an empty string depending on the ListMode setting
        if (!entry.empty()) {
            files.push_back(entry);
        }
    }

    // ascending sort based on file name and then on the extension
    std::sort(files.begin(), files.end(), [](const std::string &a, const std::string &b) {
        std::size_t pos = a.rfind(".");
        bool extFound = (pos != std::string::npos);
        std::string filenameA{ extFound ? a.substr(0, pos) : a };
        std::string extA{ extFound ? a.substr(pos + 1, std::string::npos) : "" };

        pos = b.rfind(".");
        extFound = (pos != std::string::npos);
        std::string filenameB{ extFound ? b.substr(0, pos) : b };
        std::string extB{ extFound ? b.substr(pos + 1, std::string::npos) : "" };

        int nameComp = filenameA.compare(filenameB);
        // file a is to be sorted before b, if the file name is 'smaller' in 
        // lexical order or if the file names are identical and the extension is 'smaller'
        return ((nameComp < 0) || ((nameComp == 0) && (extA.compare(extB) < 0)));
    });
    return files;
}

std::vector<std::string> TinyDirInterface::getContentsWithBasePath() {
    std::vector<std::string> files;
    while (isNextEntryAvailable()) {
        files.push_back(getNextEntry(true));
    }
    return files;
}

std::string TinyDirInterface::getNextEntry(bool includeBasePath) {
    if (!isOpen_)
        return{};

    std::string str{};
    bool foundEntry = false;
    while (resource_.has_next && !foundEntry) {
        // query next entry
        tinydir_file file;
        int errnum = tinydir_readfile(&resource_, &file);
        if (errnum != 0) {
            // cannot access entry
            std::string errMsg{ "Cannot access entry in \"" };
            errMsg.append(resource_.path);
            errMsg.append("\": ");
            errMsg.append(strerror(errnum));

            throw FileException(errMsg);
        }

        // check whether entry matches current ListMode setting
        foundEntry = ((mode_ == ListMode::FilesAndDirectories)
            || ((file.is_dir == 0) != (mode_ == ListMode::DirectoriesOnly)));
        if (foundEntry) {
            str = (includeBasePath ? file.path : file.name);
        }
        tinydir_next(&resource_);
    }
    // close resource if no more entries are available
    if (!resource_.has_next) {
        close();
    }
    return str;
}

} // namespace inviwo
