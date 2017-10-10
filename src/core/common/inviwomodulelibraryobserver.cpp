/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016 Inviwo Foundation
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
#include <inviwo/core/network/networklock.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/network/workspacemanager.h>
#include <inviwo/core/util/filesystem.h>

#include <sstream>

namespace inviwo {

InviwoModuleLibraryObserver::InviwoModuleLibraryObserver(const std::string& filePath /*= ""*/)
    : FileObserver(filePath) {}

void InviwoModuleLibraryObserver::fileChanged(const std::string& fileName) {
    // 1. Serialize network
    // 2. Clear modules/unload module libraries
    // 3. Load module libraries and register them
    // 4. De-serialize network

    // Serialize network
    std::stringstream stream;
    auto app = InviwoApplication::getPtr();

    try {
        app->getWorkspaceManager()->save(stream, app->getBasePath());
    } catch (SerializationException& exception) {
        util::log(exception.getContext(), "Unable to save network due to " + exception.getMessage(),
                  LogLevel::Error);
        return;
    }
    // Unregister modules and clear network
    app->getModuleManager().unregisterModules();

    // Register modules again
    app->getModuleManager().registerModules(std::vector<std::string>(
        1, inviwo::filesystem::getFileDirectory(inviwo::filesystem::getExecutablePath())));

    // De-serialize network
    try {
        // Lock the network that so no evaluations are triggered during the de-serialization
        app->getWorkspaceManager()->load(stream, app->getBasePath());
    } catch (SerializationException& exception) {
        util::log(exception.getContext(), "Unable to load network due to " + exception.getMessage(),
                  LogLevel::Error);
        return;
    }
}

} // namespace

