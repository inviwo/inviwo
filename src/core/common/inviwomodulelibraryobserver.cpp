/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2023 Inviwo Foundation
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

#include <inviwo/core/common/inviwomodulelibraryobserver.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/network/workspacemanager.h>
#include <inviwo/core/common/runtimemoduleregistration.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/io/serialization/serializationexception.h>

#include <sstream>
#include <fmt/std.h>

namespace inviwo {

InviwoModuleLibraryObserver::Observer::Observer(InviwoModuleLibraryObserver& imo,
                                                InviwoApplication* app)
    : FileObserver(app), imo_(imo) {}

void InviwoModuleLibraryObserver::Observer::fileChanged(const std::filesystem::path& dir) {
    imo_.fileChanged(dir);
}

InviwoModuleLibraryObserver::InviwoModuleLibraryObserver(InviwoApplication* app) : app_(app) {}

void InviwoModuleLibraryObserver::observe(const std::filesystem::path& file) {
    // We cannot create the observer in the constructor since
    // deriving applications will implement observer behavior
    // and they will not have been created when InviwoApplication
    // constructor is called.
    if (!observer_) observer_ = std::make_unique<Observer>(*this, app_);
    if (observing_.count(file) != 0) return;
    auto dir = file.parent_path();
    if (!observer_->isObserved(dir)) observer_->startFileObservation(dir);

    observing_[file] = std::filesystem::last_write_time(file);
}

void InviwoModuleLibraryObserver::fileChanged(const std::filesystem::path& dir) {
    bool reload = false;
    for (const auto& f : filesystem::getDirectoryContents(dir)) {
        const auto file = dir / f;
        auto it = observing_.find(file);
        if (it != observing_.end()) {
            auto time = std::filesystem::last_write_time(file);
            if (time > it->second) {
                it->second = time;
                reload = true;
                LogInfo("Detected change in: " << file);
            }
        }
    }
    if (reload) {
        app_->getModuleManager().reloadModules();
    }
}


}  // namespace inviwo
