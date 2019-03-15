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

#include <inviwo/core/io/imagewriterutil.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/fileextension.h>
#include <inviwo/core/util/filedialog.h>
#include <inviwo/core/util/dialogfactory.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/io/datawriter.h>
#include <inviwo/core/io/datawriterexception.h>
#include <inviwo/core/io/datawriterfactory.h>

namespace inviwo {

namespace util {

void saveLayer(const Layer& layer, const std::string& path, const FileExtension& extension) {
    auto factory = InviwoApplication::getPtr()->getDataWriterFactory();

    auto writer = std::shared_ptr<DataWriterType<Layer>>(
        factory->getWriterForTypeAndExtension<Layer>(extension));

    if (!writer) {
        // could not find a reader for the given extension, extension might be invalid
        // try to get reader for the extension extracted from the file name, i.e. path
        const auto ext = filesystem::getFileExtension(path);
        writer = std::shared_ptr<DataWriterType<Layer>>(
            factory->getWriterForTypeAndExtension<Layer>(ext));
        if (!writer) {
            LogInfoCustom(
                "ImageWriterUtil",
                "Could not find a writer for the specified file extension (\"" << ext << "\")");
            return;
        }
    }

    try {
        writer->setOverwrite(true);
        writer->writeData(&layer, path);
        LogInfoCustom("ImageWriterUtil", "Canvas layer exported to disk: " << path);
    } catch (DataWriterException const& e) {
        LogErrorCustom("ImageWriterUtil", e.getMessage());
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

    auto writerFactory = InviwoApplication::getPtr()->getDataWriterFactory();
    fileDialog->addExtensions(writerFactory->getExtensionsForType<Layer>());

    if (fileDialog->show()) {
        saveLayer(layer, fileDialog->getSelectedFile(), fileDialog->getSelectedFileExtension());
    }
}

}  // namespace util

}  // namespace inviwo
