/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018 Inviwo Foundation
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

#include <modules/qtwidgets/tf/tfutils.h>

#include <inviwo/core/network/networklock.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/logcentral.h>
#include <inviwo/core/util/fileextension.h>
#include <inviwo/core/io/datareaderexception.h>
#include <inviwo/core/io/datawriterexception.h>

#include <modules/qtwidgets/inviwofiledialog.h>
#include <modules/qtwidgets/inviwoqtutils.h>

namespace inviwo {

namespace util {

void importFromFile(TFPrimitiveSet& primitiveSet, QWidget* parent) {
    InviwoFileDialog importFileDialog(parent, "Import " + primitiveSet.getTitle(),
                                      "transferfunction");
    importFileDialog.setAcceptMode(AcceptMode::Open);
    importFileDialog.setFileMode(FileMode::ExistingFile);
    for (auto& ext : primitiveSet.getSupportedExtensions()) {
        importFileDialog.addExtension(ext);
    }
    importFileDialog.addExtension(FileExtension::all());

    if (importFileDialog.exec()) {
        const auto filename = utilqt::fromQString(importFileDialog.selectedFiles().at(0));
        try {
            NetworkLock lock;
            primitiveSet.load(filename, importFileDialog.getSelectedFileExtension());
        } catch (DataReaderException& e) {
            util::log(e.getContext(), e.getMessage(), LogLevel::Error, LogAudience::User);
        }
    }
}

void exportToFile(const TFPrimitiveSet& primitiveSet, QWidget* parent) {
    InviwoFileDialog exportFileDialog(parent, "Export " + primitiveSet.getTitle(),
                                      "transferfunction");
    exportFileDialog.setAcceptMode(AcceptMode::Save);
    exportFileDialog.setFileMode(FileMode::AnyFile);
    for (auto& ext : primitiveSet.getSupportedExtensions()) {
        exportFileDialog.addExtension(ext);
    }
    exportFileDialog.addExtension(FileExtension::all());

    if (exportFileDialog.exec()) {
        const auto filename = utilqt::fromQString(exportFileDialog.selectedFiles().at(0));
        const auto fileExt = exportFileDialog.getSelectedFileExtension();
        try {
            primitiveSet.save(filename, fileExt);
            util::log(IvwContextCustom("util::exportToFile"), "Data exported to disk: " + filename,
                      LogLevel::Info, LogAudience::User);
        } catch (DataWriterException& e) {
            util::log(e.getContext(), e.getMessage(), LogLevel::Error, LogAudience::User);
        }
    }
}

}  // namespace util

}  // namespace inviwo
