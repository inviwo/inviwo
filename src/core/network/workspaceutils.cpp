/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2023 Inviwo Foundation
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
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/logerrorcounter.h>

#include <fmt/format.h>

namespace inviwo {

namespace util {

void forEachWorkspaceInDirRecusive(std::string_view path,
                                   std::function<void(std::string_view)> callback) {

    for (const auto& file : filesystem::getDirectoryContentsRecursively(
             std::string(path), filesystem::ListMode::Files)) {
        if (filesystem::wildcardStringMatch("*.inv", file)) {
            callback(file);
        }
    }
}

void updateWorkspaces(InviwoApplication* app, std::string_view path, DryRun dryRun) {
    auto update = [&](std::string_view fileName) {
        LogInfoCustom("util::updateWorkspaces", "Updating workspace: " << fileName);
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

            do {
                app->processFront();
                app->waitForPool();
            } while (app->getProcessorNetwork()->runningBackgroundJobs() > 0);

            if (dryRun == DryRun::No) {
                app->getWorkspaceManager()->save(fileName, [&](ExceptionContext ec) {
                    try {
                        throw;
                    } catch (const IgnoreException& e) {
                        util::log(e.getContext(), e.getMessage(), LogLevel::Info);
                    }
                });
            }
        } catch (const Exception& e) {
            util::log(
                e.getContext(),
                fmt::format("Unable to convert network {} due to {}", fileName, e.getMessage()),
                LogLevel::Error);
            NetworkLock lock(app->getProcessorNetwork());
            app->getWorkspaceManager()->clear();
        }
    };

    forEachWorkspaceInDirRecusive(path, update);
}

void updateExampleWorkspaces(InviwoApplication* app, DryRun dryRun) {
    updateWorkspaces(app, filesystem::getPath(PathType::Workspaces), dryRun);

    for (const auto& m : app->getModules()) {
        updateWorkspaces(app, m->getPath(ModulePath::Workspaces), dryRun);
    }
}

void updateRegressionWorkspaces(InviwoApplication* app, DryRun dryRun) {
    for (const auto& m : app->getModules()) {
        updateWorkspaces(app, m->getPath(ModulePath::RegressionTests), dryRun);
    }
}

}  // namespace util

}  // namespace inviwo
