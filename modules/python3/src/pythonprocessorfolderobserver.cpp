/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2025 Inviwo Foundation
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

#include <inviwo/core/common/inviwomodule.h>                // for InviwoModule
#include <inviwo/core/processors/processorfactoryobject.h>  // for ProcessorFactoryObject
#include <inviwo/core/util/exception.h>                     // for Exception
#include <inviwo/core/util/fileobserver.h>                  // for FileObserver
#include <inviwo/core/util/filesystem.h>                    // for getFileExtension, directoryEx...
#include <inviwo/core/util/logcentral.h>                    // for LogCentral
#include <modules/python3/pythonprocessorfactoryobject.h>   // for PythonProcessorFactoryObject

#include <algorithm>  // for count
#include <exception>  // for exception
#include <fstream>    // for operator<<, basic_ifstream
#include <memory>     // for allocator, make_unique
#include <utility>    // for move

namespace inviwo {
class InviwoApplication;

PythonProcessorFolderObserver::PythonProcessorFolderObserver(InviwoApplication* app,
                                                             const std::filesystem::path& directory,
                                                             InviwoModule& module)
    : FileObserver(app), app_(app), directory_{directory}, module_{module} {

    if (std::filesystem::is_directory(directory)) {
        for (auto&& item : std::filesystem::recursive_directory_iterator{directory}) {
            if (item.is_regular_file() && item.path().extension() == ".py") {
                if (!registerFile(item)) {
                    startFileObservation(item);
                }
            }
        }
    }

    startFileObservation(directory);
}

bool PythonProcessorFolderObserver::registerFile(const std::filesystem::path& filename) {
    const auto isEmpty = [](const std::filesystem::path& file) {
        auto ifs = std::ifstream(file);
        ifs.seekg(0, std::ios::end);
        return ifs.tellg() == std::streampos(0);
    };

    if (std::count(registeredFiles_.begin(), registeredFiles_.end(), filename) == 0) {
        if (!std::filesystem::is_regular_file(filename)) return false;
        if (isEmpty(filename)) return false;

        try {
            auto pfo = std::make_unique<PythonProcessorFactoryObject>(app_, filename);
            module_.registerProcessor(std::move(pfo));
            registeredFiles_.push_back(filename);
            return true;
        } catch (const Exception& e) {
            log::exception(e);
        } catch (const std::exception& e) {
            log::exception(e);
        }
    }
    return false;
}

void PythonProcessorFolderObserver::fileChanged(const std::filesystem::path& changed) {
    if (changed == directory_) {
        if (std::filesystem::is_directory(directory_)) {
            auto files = filesystem::getDirectoryContents(directory_);
            for (const auto& file : files) {
                if (isObserved(directory_ / file)) continue;
                if (file.extension() != ".py") continue;

                if (registerFile(directory_ / file)) {
                    log::info("Loaded python processor: {}", directory_ / file);
                    stopFileObservation(directory_ / file);
                } else {
                    startFileObservation(directory_ / file);
                }
            }
        }
    } else {
        if (changed.extension() == ".py") {
            if (registerFile(changed)) {
                log::info("Loaded python processor: {}", changed);
                stopFileObservation(changed);
            }
        }
    }
}

}  // namespace inviwo
