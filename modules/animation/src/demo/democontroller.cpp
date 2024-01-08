/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2024 Inviwo Foundation
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

#include <modules/animation/demo/democontroller.h>

#include <inviwo/core/common/inviwoapplication.h>   // for InviwoApplication
#include <inviwo/core/network/networklock.h>        // for NetworkLock
#include <inviwo/core/network/workspacemanager.h>   // for WorkspaceManager
#include <inviwo/core/properties/fileproperty.h>    // for FileProperty
#include <inviwo/core/properties/optionproperty.h>  // for OptionPropertyInt
#include <inviwo/core/util/exception.h>             // for IgnoreException, Exception, Exception...
#include <inviwo/core/util/filedialogstate.h>       // for FileMode, FileMode::DirectoryOnly
#include <inviwo/core/util/filesystem.h>            // for cleanupPath, getDirectoryContents
#include <inviwo/core/util/logcentral.h>            // for log, LogLevel, LogLevel::Error
#include <inviwo/core/util/stringconversion.h>      // for toLower
#include <inviwo/core/util/utilities.h>

#include <cstddef>      // for size_t
#include <functional>   // for __base
#include <string_view>  // for string_view
#include <vector>       // for vector

#include <fmt/std.h>

namespace inviwo {

namespace animation {

DemoController::DemoController(InviwoApplication* app)
    : app_(app), demoFolder_("demoFolder", "Folder", ""), demoFile_("demoFile", "File") {

    demoFolder_.setFileMode(FileMode::Directory);
    addProperty(demoFolder_);
    addProperty(demoFile_);
    demoFolder_.onChange([this]() { setFileOptions(); });

    demoFile_.onChange([this]() {
        if (updateWorkspace_) onChangeSelection(Offset::Reload);
    });
}

DemoController::~DemoController() = default;

void DemoController::setFileOptions() {
    updateWorkspace_ = false;
    // Get all files and gather interesting ones.
    demoFile_.clearOptions();
    auto elements =
        filesystem::getDirectoryContents(demoFolder_.get(), filesystem::ListMode::Files);

    int invNum = 0;
    for (size_t elIdx = 0; elIdx < elements.size(); ++elIdx) {
        auto& elem = elements[elIdx];
        std::string ext = elem.extension().string();
        ext = toLower(ext);
        if (ext.compare(".inv") == 0) {  // Found a workspace
            auto cleanName = elem.stem();
            auto cleanNameExt = util::stripIdentifier(elem.string());
            demoFile_.addOption(cleanNameExt, cleanName.string(), invNum++);
        }
    }

    updateWorkspace_ = true;

    // Select first element, call file callback.
    if (demoFile_.getValues().size() > 0) {
        demoFile_.set(0);
        onChangeSelection(Offset::Reload);
    }
}

void DemoController::onChangeSelection(Offset offset) {

    if (offset == Offset::None) return;

    int numFiles = static_cast<int>(demoFile_.getValues().size());
    int nextFileIndex = demoFile_.get();

    switch (offset) {
        case Offset::First:
            nextFileIndex = 0;
            break;
        case Offset::Previous:
            nextFileIndex--;
            break;
        case Offset::Next:
            nextFileIndex++;
            break;
        case Offset::Last:
            nextFileIndex = numFiles - 1;
            break;
        default:
            return;
    }

    // No need to load anything
    if (nextFileIndex < 0 || nextFileIndex >= numFiles ||
        (offset != Offset::Reload && nextFileIndex == demoFile_.get()))
        return;

    updateWorkspace_ = false;
    demoFile_.set(nextFileIndex);
    loadWorkspaceApp(demoFolder_.get() / demoFile_.getIdentifiers()[demoFile_.get()]);

    updateWorkspace_ = true;
}

void DemoController::setFolder(const std::filesystem::path& path) { demoFolder_.set(path); }

// Copied from InviwoMainWindow::openWorkspace
void DemoController::loadWorkspaceApp(const std::filesystem::path& fileName) {
    NetworkLock lock(app_->getProcessorNetwork());
    app_->getWorkspaceManager()->clear();
    try {
        app_->getWorkspaceManager()->load(fileName, [&](ExceptionContext ec) {
            try {
                throw;
            } catch (const IgnoreException& e) {
                util::log(e.getContext(),
                          fmt::format("Incomplete network loading {} due to {}", fileName,
                                      e.getMessage()),
                          LogLevel::Error);
            }
        });
    } catch (const Exception& e) {
        util::log(e.getContext(),
                  fmt::format("Unable to load network {} due to {}", fileName, e.getMessage()),
                  LogLevel::Error);
        app_->getWorkspaceManager()->clear();
    }
}

InviwoApplication* DemoController::getInviwoApplication() { return app_; }

}  // namespace animation

}  // namespace inviwo
