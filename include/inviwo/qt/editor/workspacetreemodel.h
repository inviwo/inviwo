/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2023 Inviwo Foundation
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
#include <inviwo/core/common/inviwomodule.h>
#include <inviwo/qt/editor/workspacemodelroles.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QMetaType>
#include <QString>
#include <QVariant>
#include <QImage>
#include <QAbstractItemModel>
#include <warn/pop>

#include <memory>
#include <mutex>

namespace inviwo {

class TreeItem;
class InviwoApplication;

struct IVW_QTEDITOR_API WorkspaceInfo {
    QString title;
    QString author;
    QString tags;
    QString categories;
    QString description;
    QImage image;
    QStringList processors;
};

}  // namespace inviwo
Q_DECLARE_METATYPE(inviwo::WorkspaceInfo);  // To be able to use queued Qt connect

namespace inviwo {

class IVW_QTEDITOR_API WorkspaceInfoLoader
    : public QObject,
      public std::enable_shared_from_this<WorkspaceInfoLoader> {
#include <warn/push>
#include <warn/ignore/all>
    Q_OBJECT
#include <warn/pop>
public:
    WorkspaceInfoLoader() = default;
    WorkspaceInfoLoader(const std::filesystem::path& filename, InviwoApplication* app)
        : filename_{filename}, app_{app} {}

    void operator()();
    void submit();

signals:
    void workspaceInfoLoaded(WorkspaceInfo info);

private:
    std::filesystem::path filename_;
    InviwoApplication* app_;
    std::once_flag flag_;
};

class IVW_QTEDITOR_API WorkspaceTreeModel : public QAbstractItemModel {
public:
    using Role = WorkspaceModelRole;
    using Type = WorkspaceModelType;

    explicit WorkspaceTreeModel(InviwoApplication* app, QObject* parent = nullptr);
    virtual ~WorkspaceTreeModel();

    virtual QModelIndex index(int row, int column,
                              const QModelIndex& parent = QModelIndex()) const override;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
    virtual QVariant data(const QModelIndex& index, int role) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role = Qt::DisplayRole) const override;
    virtual QModelIndex parent(const QModelIndex& index) const override;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    void updateRecentWorkspaces(const QStringList& recentFiles);
    void updateExampleEntries();
    void updateRegressionTestEntries();

    QModelIndex getCategoryIndex(std::string_view category);
    static constexpr std::string_view recent = "Recent Workspaces";
    static constexpr std::string_view examples = "Examples";
    static constexpr std::string_view tests = "Regression Tests";

private:
    void updateModules(std::string_view category, ModulePath pathType, bool recursive);

    QModelIndex getIndex(TreeItem* item, int column = 0) const;
    void updateCategory(TreeItem* item, std::vector<std::unique_ptr<TreeItem>> children);
    void addEntry(TreeItem* root, std::unique_ptr<TreeItem> child);

    TreeItem* getCategory(std::string_view name) const;

    TreeItem* getItem(const QModelIndex& index) const;
    InviwoApplication* app_;

    std::unique_ptr<TreeItem> root_;
};

}  // namespace inviwo
