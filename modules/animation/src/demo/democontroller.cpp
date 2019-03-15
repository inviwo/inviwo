/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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
#include <modules/animation/demo/democontrollerobserver.h>
#include <inviwo/core/network/networklock.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/stringconversion.h>

namespace inviwo {

namespace animation {

DemoController::DemoController(InviwoApplication* app)
    : app_(app), demoFolder_("demoFolder", "Folder", ""), demoFile_("demoFile", "File") {

    demoFolder_.setFileMode(FileMode::DirectoryOnly);
    addProperty(demoFolder_);
    addProperty(demoFile_);
    demoFolder_.onChange([this]() { setFileOptions(); });

    demoFile_.onChange([this]() {
        if (updateWorkspace_) onChangeSelection(Offset::Reload);
    });
}

DemoController::~DemoController() = default;

void loadWorkspaceApp(const std::string& path);

void DemoController::setFileOptions() {

    std::string selectedPath = filesystem::cleanupPath(demoFolder_.get());

    updateWorkspace_ = false;
    // Get all files and gather interesting ones.
    demoFile_.clearOptions();
    std::vector<std::string> elements =
        filesystem::getDirectoryContents(selectedPath, filesystem::ListMode::Files);

    int invNum = 0;
    for (size_t elIdx = 0; elIdx < elements.size(); ++elIdx) {
        std::string& elem = elements[elIdx];
        std::string ext = filesystem::getFileExtension(elem);
        ext = toLower(ext);
        if (ext.compare("inv") == 0) {  // Found a workspace
            auto cleanName = filesystem::getFileNameWithoutExtension(elem);
            auto cleanNameExt = filesystem::cleanupPath(elem);
            demoFile_.addOption(cleanNameExt, cleanName, invNum++);
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
        default:
            return;
    }

    // No need to load anything
    if (nextFileIndex < 0 || nextFileIndex >= numFiles ||
        (offset != Offset::Reload && nextFileIndex == demoFile_.get()))
        return;

    updateWorkspace_ = false;
    demoFile_.set(nextFileIndex);
    loadWorkspaceApp(demoFolder_.get() + "/" + demoFile_.getIdentifiers()[demoFile_.get()]);

    updateWorkspace_ = true;
}

void DemoController::setFolder(const std::string& path) { demoFolder_.set(path); }

// Copied from InviwoMainWindow::openWorkspace
void loadWorkspaceApp(const std::string& fileName) {

    InviwoApplication* app = InviwoApplication::getPtr();

    NetworkLock lock(app->getProcessorNetwork());
    app->getWorkspaceManager()->clear();
    try {
        app->getWorkspaceManager()->load(fileName, [&](ExceptionContext ec) {
            try {
                throw;
            } catch (const IgnoreException& e) {
                util::log(e.getContext(),
                          "Incomplete network loading " + fileName + " due to " + e.getMessage(),
                          LogLevel::Error);
            }
        });
    } catch (const Exception& e) {
        util::log(e.getContext(),
                  "Unable to load network " + fileName + " due to " + e.getMessage(),
                  LogLevel::Error);
        app->getWorkspaceManager()->clear();
    }
}

}  // namespace animation

}  // namespace inviwo
