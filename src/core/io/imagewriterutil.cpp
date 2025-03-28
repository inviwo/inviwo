/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2025 Inviwo Foundation
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

#include <inviwo/core/io/imagewriterutil.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/fileextension.h>
#include <inviwo/core/util/filedialog.h>
#include <inviwo/core/util/dialogfactory.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/io/datawriterexception.h>
#include <inviwo/core/io/datawriterutil.h>

namespace inviwo {

namespace util {

void saveLayer(const Layer& layer, const std::filesystem::path& path,
               const FileExtension& extension) {
    try {
        util::saveData<Layer>(layer, path, extension, Overwrite::Yes);
        log::info("Canvas layer exported to disk: {}", path);
    } catch (const DataWriterException& e) {
        log::exception(e);
    }
}

void saveLayer(const Layer& layer) {
    auto fileDialog = util::dynamic_unique_ptr_cast<FileDialog>(
        InviwoApplication::getPtr()->getDialogFactory()->create("FileDialog"));
    if (!fileDialog) {
        return;
    }
    fileDialog->setTitle("Save Layer to File...");
    fileDialog->setAcceptMode(AcceptMode::Save);
    fileDialog->setFileMode(FileMode::AnyFile);
    fileDialog->setContentType("image");
    fileDialog->setCurrentDirectory("");

    auto writerFactory = InviwoApplication::getPtr()->getDataWriterFactory();
    fileDialog->addExtensions(writerFactory->getExtensionsForType<Layer>());

    if (fileDialog->show()) {
        saveLayer(layer, fileDialog->getSelectedFile(), fileDialog->getSelectedFileExtension());
    }
}

}  // namespace util

}  // namespace inviwo
