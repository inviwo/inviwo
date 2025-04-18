/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2025 Inviwo Foundation
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
#include <inviwo/core/util/fileobserver.h>
#include <inviwo/core/util/filesystem.h>

#include <string>
#include <memory>
#include <unordered_map>
#include <ctime>
#include <filesystem>
#include <chrono>

namespace inviwo {

class InviwoApplication;

/**
 * \class InviwoModuleLibraryObserver
 * \brief Serializes the network, reloads modules and de-serializes the network when observed module
 * library changes.
 */
class IVW_CORE_API InviwoModuleLibraryObserver {
public:
    InviwoModuleLibraryObserver(InviwoApplication* app);
    virtual ~InviwoModuleLibraryObserver() = default;

    void observe(const std::filesystem::path& file);
    void reloadModules();

private:
    class Observer : public FileObserver {
    public:
        Observer(InviwoModuleLibraryObserver& imo, InviwoApplication* app);
        virtual void fileChanged(const std::filesystem::path& dir) override;

    private:
        InviwoModuleLibraryObserver& imo_;
    };

    void fileChanged(const std::filesystem::path& dir);

    InviwoApplication* app_;
    // Need to be pointer since we cannot initialize the observer before the application.
    std::unique_ptr<Observer> observer_;
    std::unordered_map<std::filesystem::path, std::filesystem::file_time_type, PathHash> observing_;
};

}  // namespace inviwo
