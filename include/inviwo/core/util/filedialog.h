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

#ifndef IVW_FILEDIALOG_H
#define IVW_FILEDIALOG_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/util/dialog.h>
#include <inviwo/core/util/filedialogstate.h>
#include <inviwo/core/util/fileextension.h>

namespace inviwo {

class IVW_CORE_API FileDialog : public Dialog {
public:
    FileDialog();
    virtual ~FileDialog();

    virtual bool show() = 0;

    virtual void setTitle(const std::string &title) = 0;

    virtual void setAcceptMode(AcceptMode mode) = 0;
    virtual AcceptMode getAcceptMode() const = 0;

    virtual void setFileMode(FileMode mode) = 0;
    virtual FileMode getFileMode() const = 0;

    virtual void setContentType(const std::string &contentType) = 0;
    virtual std::string getContentType() const = 0;

    /**
     * \brief sets the current directory of the file dialog to the parent directory of the given
     *   file name or, if it is referring to a directory, to the given path. The file will be
     *   selected when the dialog is shown.
     *
     * @param filename  path and name of the file (can be either a file name or directory name
     * including the full path)
     */
    virtual void setCurrentFile(const std::string &filename) = 0;
    std::string getSelectedFile() const;
    virtual std::vector<std::string> getSelectedFiles() const = 0;
    /**
     * \brief set the current directory of the file dialog
     *
     * @param path  given path, must not contain a file name
     */
    virtual void setCurrentDirectory(const std::string &path) = 0;

    virtual void setSelectedExtension(const FileExtension &ext) = 0;
    virtual FileExtension getSelectedFileExtension() const = 0;

    virtual void addExtension(const FileExtension &fileExt) = 0;
    virtual void addExtension(const std::string &ext, const std::string &description) = 0;
    virtual void addExtension(const std::string &extString) = 0;
    virtual void addExtensions(const std::vector<FileExtension> &extensions) = 0;
};

}  // namespace inviwo

#endif  // IVW_FILEDIALOG_H
