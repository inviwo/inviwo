/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2019 Inviwo Foundation
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

#ifndef IVW_INVIWOFILEDIALOG_H
#define IVW_INVIWOFILEDIALOG_H

#include <inviwo/core/common/inviwoapplication.h>
#include <modules/qtwidgets/qtwidgetsmoduledefine.h>
#include <inviwo/core/util/fileextension.h>
#include <inviwo/core/util/filedialog.h>
#include <string>
#include <unordered_map>

#include <warn/push>
#include <warn/ignore/all>
#include <QFileDialog>
#include <QUrl>
#include <QSettings>
#include <warn/pop>

namespace inviwo {

class IVW_MODULE_QTWIDGETS_API InviwoFileDialog : public QFileDialog, public FileDialog {
#include <warn/push>
#include <warn/ignore/all>
    Q_OBJECT
#include <warn/pop>
public:
    InviwoFileDialog(QWidget *parent = nullptr, const std::string &title = "",
                     const std::string &pathType = "default", const std::string &path = "");

    virtual bool show() override;

    virtual int exec() override;

    virtual void setTitle(const std::string &title) override;

    virtual void setAcceptMode(inviwo::AcceptMode mode) override;
    virtual inviwo::AcceptMode getAcceptMode() const override;

    virtual void setFileMode(inviwo::FileMode mode) override;
    virtual inviwo::FileMode getFileMode() const override;

    virtual void setContentType(const std::string &contentType) override;
    virtual std::string getContentType() const override;

    /**
     * \brief sets the current directory of the file dialog to the parent directory of the given
     *   file name or, if it is referring to a directory, to the given path. The file will be
     *   selected when the dialog is shown.
     *
     * @param filename  path and name of the file (can be either a file name or directory name
     * including the full path)
     */
    virtual void setCurrentFile(const std::string &filename) override;
    virtual std::vector<std::string> getSelectedFiles() const override;

    /**
     * \brief set the current directory of the file dialog
     *
     * @param path  given path, must not contain a file name
     */
    virtual void setCurrentDirectory(const std::string &path) override;

    virtual FileExtension getSelectedFileExtension() const override;
    virtual void setSelectedExtension(const FileExtension &ext) override;

    virtual void addExtension(const FileExtension &fileExt) override;
    virtual void addExtension(const std::string &ext, const std::string &description) override;
    virtual void addExtension(const std::string &extString) override;
    virtual void addExtensions(const std::vector<FileExtension> &extensions) override;

    void addSidebarPath(const PathType &path);
    void addSidebarPath(const std::string &path);
    void addSidebarPath(const QString &path);

    void useNativeDialog(const bool &use = true);

    static QString getPreviousPath(const QString &pathType);
    static void setPreviousPath(const QString &pathType, const QString &path);

    static QString getPreviousExtension(const QString &pathType);
    static void setPreviousExtension(const QString &pathType, const QString &path);

protected slots:
    void filterSelectionChanged(const QString &filter);

protected:
    FileExtension getMatchingFileExtension(const QString &extStr);

    QList<QUrl> sidebarURLs_;
    QStringList extensions_;
    QString selectedExtension_;
    QString pathType_;
    QString currentPath_;

    FileExtension selectedFilter_;

    using FileExtMap = std::unordered_map<std::string, FileExtension>;
    FileExtMap extmap_;
};
}  // namespace inviwo

#endif
