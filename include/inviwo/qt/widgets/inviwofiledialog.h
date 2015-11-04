/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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
#include <inviwo/qt/widgets/inviwoqtwidgetsdefine.h>
#include <inviwo/core/util/fileextension.h>
#include <string>
#include <unordered_map>

#include <warn/push>
#include <warn/ignore/all>
#include <QFileDialog>
#include <QUrl>
#include <QSettings>
#include <warn/pop>


namespace inviwo {

class IVW_QTWIDGETS_API InviwoFileDialog : public QFileDialog {
    #include <warn/push>
    #include <warn/ignore/all>
    Q_OBJECT
    #include <warn/pop>
public:
    InviwoFileDialog(QWidget *parent, const std::string &title,
        const std::string &pathType = "default",
        const std::string &path="");

    void addExtension(const FileExtension &fileExt);
    void addExtension(const std::string &ext, const std::string &description);
    void addExtension(const std::string &extString);

    void addSidebarPath(const InviwoApplication::PathType &path);
    void addSidebarPath(const std::string &path);
    void addSidebarPath(const QString &path);

    void useNativeDialog(const bool &use = true);

    void setCurrentDirectory(const std::string &path);

    FileExtension getSelectedFileExtension() const;

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    virtual int exec() override;
#else
    virtual int exec();
#endif

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

    static QSettings globalSettings_;
};
}  // namespace

#endif
