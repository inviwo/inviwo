/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2023-2024 Inviwo Foundation
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

#include <inviwo/qt/editor/editorsettings.h>

namespace inviwo {

EditorSettings::EditorSettings(InviwoApplication* app)
    : Settings("Editor Settings", app)
    , workspaceAuthor{"workspaceAuthor", "Default Workspace Author", ""}
    , numRecentFiles{"numRecentFiles",
                     "Number of Recent Files",
                     12,
                     {1, ConstraintBehavior::Immutable},
                     {100, ConstraintBehavior::Ignore}}
    , numRestoreFiles{"numRestoreFiles",
                      "Number of Restore Files",
                      "The maximum number of backup files to keep, the oldest will be removed first"_help,
                      24,
                      {1, ConstraintBehavior::Immutable},
                      {100, ConstraintBehavior::Ignore}}
    , restoreFrequency{"restoreFrequency",
                       "Restore Frequency",
                       "Minutes between new backup files"_help,
                       10,
                       {1, ConstraintBehavior::Immutable},
                       {100, ConstraintBehavior::Ignore}}
    , workspaceDirectories{"workspaceDirectories", "Workspace Directories",
                           std::make_unique<FileProperty>("directory", "Directory",
                                                          "Workspace directory"_help, "",
                                                          AcceptMode::Open, FileMode::Directory)}
    , showProcessorEvaluationCounts{"showProcessorEvaluationCounts",
                                    "Show Processor Evaluation Counts", true} {

    addProperties(workspaceAuthor, numRecentFiles, numRestoreFiles, restoreFrequency,
                  workspaceDirectories, showProcessorEvaluationCounts);

#ifndef IVW_PROFILING
    showProcessorEvaluationCounts.set(false);
    showProcessorEvaluationCounts.setReadOnly(true);
    showProcessorEvaluationCounts.setHelp("Enable 'IVW_PROFILING' in cmake to show counts"_help);
    showProcessorEvaluationCounts.setCurrentStateAsDefault();
#endif

    workspaceDirectories.setHelp(
        "Additional workspace directories listed on the Get Started screen."_help);

    load();
}

}  // namespace inviwo
