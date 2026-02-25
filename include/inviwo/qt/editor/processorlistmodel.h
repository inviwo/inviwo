/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <inviwo/core/processors/processorinfo.h>
#include <inviwo/core/processors/processordocs.h>
#include <inviwo/core/util/transparentmaps.h>

#include <QAbstractItemModel>
#include <QIcon>
#include <QString>
#include <QVariant>

#include <functional>

namespace inviwo {

class IVW_QTEDITOR_API ProcessorListModel : public QAbstractItemModel {
public:
    enum class Role : std::uint16_t { Type = Qt::UserRole + 100, ClassIdentifier, Item, Sort };
    struct Item {
        ProcessorInfo info;
        help::HelpProcessor help;
        std::string moduleId;
        std::string metaInfo;
        std::time_t lastUsed;
        size_t useCount;
    };
    struct Node {
        enum class Type : std::uint8_t { Root, Group, Item };
        Type type = Type::Root;
        QString name;
        QVariant sort;
        Item* item = nullptr;
        Node* parent = nullptr;
        int row = 0;
        std::vector<std::unique_ptr<Node>> children;

        Node* child(int i) {
            if (i < 0 || i >= static_cast<int>(children.size())) return nullptr;
            return children[i].get();
        }
        int size() const { return static_cast<int>(children.size()); }
    };
    enum class Grouping : std::uint8_t {
        Alphabetical,
        Categorical,
        CodeState,
        Module,
        LastUsed,
        MostUsed
    };
    Q_ENUM(Grouping);

    explicit ProcessorListModel(QObject* parent = nullptr);
    ProcessorListModel(const ProcessorListModel&) = delete;
    ProcessorListModel& operator=(const ProcessorListModel&) = delete;
    ProcessorListModel(ProcessorListModel&&) = delete;
    ProcessorListModel& operator=(ProcessorListModel&&) = delete;
    virtual ~ProcessorListModel() = default;

    virtual QModelIndex index(int row, int column, const QModelIndex& parent) const override;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
    virtual QVariant data(const QModelIndex& index, int role) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    virtual QModelIndex parent(const QModelIndex& index) const override;
    virtual int rowCount(const QModelIndex& parent) const override;
    virtual int columnCount(const QModelIndex& parent) const override;

    void setItems(std::vector<Item> items);
    void setGrouping(Grouping grouping);

    bool updateItem(std::string_view classIdentifier, const std::function<bool(Item&)>& updater);
    void addItem(Item item);
    void removeItem(std::string_view classIdentifier);

private:
    Node* indexToNode(const QModelIndex& index) const;
    const QIcon* getCodeStateIcon(CodeState state) const;
    static std::pair<std::string, QVariant> categoryAndSort(const Item& item, Grouping grouping);
    void build();

    Grouping grouping_;
    std::vector<Item> items_;
    std::unique_ptr<Node> root_;
    std::unordered_map<Item*, Node*> itemToNode_;
    UnorderedStringMap<Item*> classIdentifierToItem_;

    QIcon iconStable_;
    QIcon iconExperimental_;
    QIcon iconBroken_;
    QIcon iconDeprecated_;
};

}  // namespace inviwo

Q_DECLARE_METATYPE(inviwo::ProcessorListModel::Item*);
