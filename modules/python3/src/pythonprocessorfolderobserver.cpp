/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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

#include <modules/python3/pythonprocessorfolderobserver.h>
#include <modules/python3/pythonprocessorfactoryobject.h>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/filesystem.h>

#include <algorithm>

namespace inviwo {

PythonProcessorFolderObserver::PythonProcessorFolderObserver(InviwoApplication* app,
                                                             const std::string& directory,
                                                             InviwoModule& module)
    : FileObserver(app), app_(app), directory_{directory}, module_{module} {

    if (filesystem::directoryExists(directory)) {
        const auto files = filesystem::getDirectoryContents(directory);
        for (const auto& file : files) {
            if (filesystem::getFileExtension(file) == "py") {
                if (!registerFile(directory + "/" + file)) {
                    startFileObservation(directory_ + "/" + file);
                }
            }
        }
    }

    startFileObservation(directory);
}

bool PythonProcessorFolderObserver::registerFile(const std::string& filename) {
    const auto isEmpty = [](const std::string& file) {
        auto ifs = filesystem::ifstream(file);
        ifs.seekg(0, std::ios::end);
        return ifs.tellg() == std::streampos(0);
    };

    if (std::count(registeredFiles_.begin(), registeredFiles_.end(), filename) == 0) {
        if (!filesystem::fileExists(filename)) return false;
        if (isEmpty(filename)) return false;

        try {
            auto pfo = std::make_unique<PythonProcessorFactoryObject>(app_, filename);
            module_.registerProcessor(std::move(pfo));
            registeredFiles_.push_back(filename);
            return true;
        } catch (const std::exception& e) {
            LogError(e.what());
        }
    }
    return false;
}

void PythonProcessorFolderObserver::fileChanged(const std::string& changed) {
    if (changed == directory_) {
        if (filesystem::directoryExists(directory_)) {
            auto files = filesystem::getDirectoryContents(directory_);
            for (const auto& file : files) {
                if (isObserved(directory_ + "/" + file)) continue;
                if (filesystem::getFileExtension(file) != "py") continue;

                if (registerFile(directory_ + "/" + file)) {
                    LogInfo("Loaded python processor: " << directory_ + "/" + file);
                    stopFileObservation(directory_ + "/" + file);
                } else {
                    startFileObservation(directory_ + "/" + file);
                }
            }
        }
    } else {
        if (filesystem::getFileExtension(changed) == "py") {
            if (registerFile(changed)) {
                LogInfo("Loaded python processor: " << directory_ + "/" + changed);
                stopFileObservation(directory_ + "/" + changed);
            }
        }
    }
}

}  // namespace inviwo
