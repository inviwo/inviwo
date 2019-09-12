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

#include <QString>
#include <QVariant>
#include <QIcon>
#include <QAbstractItemModel>

#include <warn/pop>

#include <memory>

namespace inviwo {

class TreeItem;

class IVW_QTEDITOR_API FileTreeModel : public QAbstractItemModel {
#include <warn/push>
#include <warn/ignore/all>
    Q_OBJECT
#include <warn/pop>
public:
    enum ItemRoles { FileName = Qt::UserRole + 100, Path, Type, ExampleWorkspace };

    enum class ListElemType { File = 1, Section, SubSection, None };

    friend bool operator==(const QVariant&, ListElemType);
    friend bool operator==(ListElemType, const QVariant&);
    friend bool operator!=(const QVariant&, ListElemType);
    friend bool operator!=(ListElemType, const QVariant&);

    explicit FileTreeModel(QObject* parent = nullptr);
    virtual ~FileTreeModel() = default;

    virtual QModelIndex index(int row, int column,
                              const QModelIndex& parent = QModelIndex()) const override;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
    virtual QVariant data(const QModelIndex& index, int role) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role = Qt::DisplayRole) const override;
    virtual QModelIndex parent(const QModelIndex& index) const override;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    virtual bool insertRows(int position, int rows,
                            const QModelIndex& parent = QModelIndex()) override;
    virtual bool removeRows(int position, int rows,
                            const QModelIndex& parent = QModelIndex()) override;

    void updateCategory(TreeItem* item, std::vector<std::unique_ptr<TreeItem>> children);

    void addEntry(TreeItem* root, std::unique_ptr<TreeItem> child);
    bool removeEntry(TreeItem* node);
    bool removeChildren(TreeItem* root);
    QModelIndex getIndex(TreeItem* item, int column = 0) const;

private:
    TreeItem* getItem(const QModelIndex& index) const;

    std::unique_ptr<TreeItem> root_;
};

class IVW_QTEDITOR_API TreeItem {
public:
    explicit TreeItem(TreeItem* parent = nullptr);
    TreeItem(const QString& caption, FileTreeModel::ListElemType type, TreeItem* parent = nullptr);
    TreeItem(const QIcon& icon, const std::string& filename, bool isExample = false,
             TreeItem* parent = nullptr);
    TreeItem(const TreeItem&) = delete;
    TreeItem& operator=(const TreeItem&) = delete;
    ~TreeItem() = default;

    void addChild(std::unique_ptr<TreeItem> child);
    void addChildren(std::vector<std::unique_ptr<TreeItem>> children);

    bool insertChildren(int position, int count);
    bool removeChildren(int position, int count);
    void removeChildren();

    TreeItem* child(int row) const;
    int row() const;
    int childCount() const;
    int columnCount() const;
    TreeItem* parent() const;

    QVariant data(int column, int role) const;
    FileTreeModel::ListElemType type() const;

    void setData(const QString& caption, FileTreeModel::ListElemType type);
    void setData(const QIcon& icon, const std::string& filename, bool isExample);

    bool operator==(const TreeItem& tree) const;
    bool operator!=(const TreeItem& tree) const;

private:
    TreeItem* parent_;
    std::vector<std::unique_ptr<TreeItem>> childItems_;

    FileTreeModel::ListElemType type_ = FileTreeModel::ListElemType::None;

    QIcon icon_;
    QString caption_;
    QString file_;
    QString path_;
    bool isExample_ = false;
};

}  // namespace inviwo
