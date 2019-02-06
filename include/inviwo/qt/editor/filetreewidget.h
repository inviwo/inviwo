/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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
#pragma once

#include <inviwo/qt/editor/inviwoqteditordefine.h>
#include <inviwo/core/common/inviwo.h>

#include <warn/push>
#include <warn/ignore/all>

#include <QTreeWidget>
#include <QIcon>

#include <warn/pop>

class QTreeWidgetItem;

namespace inviwo {

class InviwoApplication;

class IVW_QTEDITOR_API FileTreeWidget : public QTreeWidget {
#include <warn/push>
#include <warn/ignore/all>
    Q_OBJECT
#include <warn/pop>
public:
    enum ListElemType { File = 1, Section, SubSection };

    enum ItemRoles { FileName = Qt::UserRole + 100, Path, Type, ExampleWorkspace };

    explicit FileTreeWidget(InviwoApplication *app, QWidget *parent = nullptr);
    virtual ~FileTreeWidget() = default;

    void updateRecentWorkspaces(const QStringList &recentFiles);
    void updateExampleEntries();

    bool selectRecentWorkspace(int index);

signals:
    void selectedFileChanged(const QString &filename, bool isExample);
    void loadFile(const QString &filename, bool isExample);

private:
    static QTreeWidgetItem *createCategory(const QString &caption,
                                           ListElemType = ListElemType::Section);
    static QTreeWidgetItem *createFileEntry(const QIcon &icon, const std::string &filename,
                                            bool isExample = false);

    static void removeChildren(QTreeWidgetItem *node);

    InviwoApplication *inviwoApp_;

    QTreeWidgetItem *recentWorkspaceItem_ = nullptr;
    QTreeWidgetItem *examplesItem_ = nullptr;

    QIcon fileIcon_;
};

}  // namespace inviwo
