/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
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

#include <inviwo/core/network/workspaceutils.h>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/common/inviwomodule.h>
#include <inviwo/core/network/workspacemanager.h>
#include <inviwo/core/network/networklock.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/logerrorcounter.h>

namespace inviwo {

namespace util {

void updateWorkspaces(InviwoApplication* app) {
    updateWorkspaces(app, filesystem::getPath(PathType::Workspaces));

    for (const auto& m : app->getModules()) {
        updateWorkspaces(app, m->getPath(ModulePath::Workspaces));
    }
}

void updateRegressionWorkspaces(InviwoApplication* app) {
    for (const auto& m : app->getModules()) {
        updateWorkspaces(app, m->getPath(ModulePath::RegressionTests));
    }
}

void updateWorkspaces(InviwoApplication* app, const std::string& path) {
    auto update = [&](const std::string& fileName) {
        auto errorCounter = std::make_shared<LogErrorCounter>();
        LogCentral::getPtr()->registerLogger(errorCounter);

        {
            NetworkLock lock(app->getProcessorNetwork());
            app->getWorkspaceManager()->clear();
        }
        try {
            {
                NetworkLock lock(app->getProcessorNetwork());
                app->getWorkspaceManager()->load(fileName, [&](ExceptionContext ec) {
                    try {
                        throw;
                    } catch (const IgnoreException& e) {
                        util::log(e.getContext(), e.getMessage(), LogLevel::Info);
                    }
                });

                if (errorCounter->getErrorCount() > 0) {
                    throw Exception("Error messages found!",
                                    IVW_CONTEXT_CUSTOM("util::updateWorkspaces"));
                }
            }
            app->waitForPool();
            {
                app->getWorkspaceManager()->save(fileName, [&](ExceptionContext ec) {
                    try {
                        throw;
                    } catch (const IgnoreException& e) {
                        util::log(e.getContext(), e.getMessage(), LogLevel::Info);
                    }
                });
            }

        } catch (const Exception& e) {
            util::log(e.getContext(),
                      "Unable to convert network " + fileName + " due to " + e.getMessage(),
                      LogLevel::Error);
            NetworkLock lock(app->getProcessorNetwork());
            app->getWorkspaceManager()->clear();
        }
    };

    std::function<void(const std::string&)> updatePath = [&](const std::string& path) {
        for (const auto& file :
             filesystem::getDirectoryContents(path, filesystem::ListMode::Files)) {
            if (filesystem::wildcardStringMatch("*.inv", file)) {
                const auto workspace = path + "/" + file;
                LogInfoCustom("util::updateWorkspaces", "Updating workspace: " << workspace);
                update(workspace);
            }
        }
        for (const auto& dir :
             filesystem::getDirectoryContents(path, filesystem::ListMode::Directories)) {
            if (dir != "." && dir != "..") updatePath(path + "/" + dir);
        }
    };

    updatePath(path);
}

}  // namespace util

}  // namespace inviwo
