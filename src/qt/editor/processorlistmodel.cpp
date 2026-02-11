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

#include <inviwo/qt/editor/processorlistmodel.h>

#include <modules/qtwidgets/inviwoqtutils.h>

#include <QFont>

namespace inviwo {

namespace {

Document tooltip(const ProcessorListModel::Item& item) {
    Document doc;
    using P = Document::PathComponent;
    auto b = doc.append("html").append("body");
    b.append("b", item.info.displayName, {{"style", "color:white;"}});
    using H = utildoc::TableBuilder::Header;
    utildoc::TableBuilder tb(b, P::end(), {{"class", "propertyInfo"}});

    tb(H("Module"), item.moduleId);
    tb(H("Identifier"), item.info.classIdentifier);
    tb(H("Category"), item.info.category);
    tb(H("Code"), item.info.codeState);
    tb(H("Tags"), item.info.tags.getString());
    tb(H("Uses"), item.useCount);
    tb(H("Last"), item.lastUsed == 0 ? "Never" : std::ctime(&item.lastUsed));

    b.append(item.metaInfo);
    return doc;
}

}  // namespace

ProcessorListModel::ProcessorListModel(QObject* parent)
    : QAbstractItemModel(parent)
    , grouping_{Grouping::Categorical}
    , root_{std::make_unique<Node>()}
    , iconStable_{":/svgicons/processor-stable.svg"}
    , iconExperimental_{":/svgicons/processor-experimental.svg"}
    , iconBroken_{":/svgicons/processor-broken.svg"}
    , iconDeprecated_{":/svgicons/processor-deprecated.svg"} {}

QModelIndex ProcessorListModel::index(int row, int column, const QModelIndex& parent) const {
    Node* parentNode = indexToNode(parent);
    if (!parentNode) return {};

    Node* childNode = parentNode->child(row);
    if (childNode) return createIndex(row, column, childNode);

    return {};
}
Qt::ItemFlags ProcessorListModel::flags(const QModelIndex& index) const {
    if (!index.isValid()) return Qt::NoItemFlags;

    auto* node = indexToNode(index);
    switch (node->type) {
        case Node::Type::Root:
            return Qt::NoItemFlags;
        case Node::Type::Group:
            return Qt::NoItemFlags;
        case Node::Type::Item:
            return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemNeverHasChildren;
    }
}
QVariant ProcessorListModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) return {};
    auto* node = indexToNode(index);

    switch (node->type) {
        case Node::Type::Root: {
            return {};
        }
        case Node::Type::Group:
            switch (role) {
                case Qt::DisplayRole:
                    if (index.column() == 0) {
                        return node->name;
                    }
                    return {};
                case Qt::ToolTipRole:
                    return {};
                case static_cast<int>(Role::Type):
                    return static_cast<int>(Node::Type::Group);
                case static_cast<int>(Role::Sort):
                    return node->sort;
                default:
                    return {};
            }
        case Node::Type::Item:
            switch (role) {
                case Qt::DisplayRole:
                    switch (index.column()) {
                        case 0:
                            return node->name;
                        case 1: {
                            auto platformTags = util::getPlatformTags(node->item->info.tags);
                            return utilqt::toQString(platformTags.getString());
                        }
                        default:
                            break;
                    }
                    return node->name;
                case Qt::DecorationRole:
                    if (index.column() == 0) {
                        return *getCodeStateIcon(node->item->info.codeState);
                    }
                    return {};
                case Qt::FontRole: {
                    if (index.column() == 1) {
                        QFont font;
                        font.setWeight(QFont::Bold);
                        return font;
                    }
                    return {};
                }
                case Qt::ToolTipRole:
                    return utilqt::toQString(tooltip(*node->item).str());
                case static_cast<int>(Role::Type):
                    return static_cast<int>(Node::Type::Item);
                case static_cast<int>(Role::ClassIdentifier):
                    return utilqt::toQString(node->item->info.classIdentifier);
                case static_cast<int>(Role::Item):
                    return QVariant::fromValue(node->item);
                default:
                    return {};
            }
    }
    return {};
}
QVariant ProcessorListModel::headerData(int /*section*/, Qt::Orientation /*orientation*/,
                                        int /*role*/) const {
    return {};
}
QModelIndex ProcessorListModel::parent(const QModelIndex& index) const {

    if (!index.isValid()) return QModelIndex{};

    if (auto* node = indexToNode(index)) {
        if (auto* parent = node->parent) {
            if (parent != root_.get()) {
                return createIndex(parent->row, 0, parent);
            }
        }
    }
    return QModelIndex{};
}
int ProcessorListModel::rowCount(const QModelIndex& parent) const {
    auto* node = indexToNode(parent);
    return node ? node->size() : 0;
}
int ProcessorListModel::columnCount(const QModelIndex&) const { return 2; }

void ProcessorListModel::setItems(std::vector<Item> items) {
    beginResetModel();
    items_ = std::move(items);
    build();
    endResetModel();
}

void ProcessorListModel::setGrouping(Grouping grouping) {
    beginResetModel();
    grouping_ = grouping;
    build();
    endResetModel();
}

auto ProcessorListModel::indexToNode(const QModelIndex& index) const -> Node* {
    if (!index.isValid()) return root_.get();
    return static_cast<Node*>(index.internalPointer());
}

const QIcon* ProcessorListModel::getCodeStateIcon(CodeState state) const {
    switch (state) {
        case CodeState::Stable:
            return &iconStable_;
        case CodeState::Experimental:
            return &iconExperimental_;
        case CodeState::Deprecated:
            return &iconDeprecated_;
        case CodeState::Broken:
            return &iconBroken_;
        default:
            return &iconExperimental_;
    }
}

std::pair<std::string, QVariant> ProcessorListModel::categoryAndSort(const Item& item,
                                                                     Grouping grouping) {
    switch (grouping) {
        using enum Grouping;
        case Alphabetical:
            return {item.info.displayName.substr(0, 1), item.info.displayName.front()};
        case Categorical:
            return {item.info.category, utilqt::toQString(item.info.category)};
        case CodeState:
            return {toString(item.info.codeState), static_cast<int>(item.info.codeState)};
        case Module:
            return {item.moduleId, utilqt::toQString(item.moduleId)};
        case LastUsed: {
            if (item.lastUsed == 0) {
                return {"Never", std::numeric_limits<int>::max()};
            }
            const auto now = std::chrono::system_clock::now();
            const auto midnight = std::chrono::floor<std::chrono::days>(now);
            const auto use = std::chrono::system_clock::from_time_t(item.lastUsed);

            if (use > midnight) {
                return {"Today", 24};
            } else if (use > midnight - std::chrono::days{1}) {
                return {"Yesterday", 48};
            } else if (use > midnight - std::chrono::weeks{1}) {
                return {"This Week", 7 * 24};
            } else if (use > midnight - std::chrono::months{1}) {
                return {"This Month", 30 * 24};
            } else if (use > midnight - std::chrono::years{1}) {
                return {"This Year", 30 * 24};
            } else {
                return {"Older", 100 * 24};
            }
        }
        case MostUsed: {
            if (item.useCount == 0) {
                return {"Never", 0};
            } else if (item.useCount < 10) {
                return {"One or More times", -1};
            } else {
                const auto tens = static_cast<int>(item.useCount / 10);
                return {fmt::format("More than {} times", tens * 10), -(tens + 1)};
            }
        }
        default:
            return {"Unknown", QVariant()};
    }
}

void ProcessorListModel::build() {
    root_ = std::make_unique<Node>();
    itemToNode_.clear();

    StringMap<std::vector<Item*>> groups;
    StringMap<QVariant> groupToSort;
    for (auto& item : items_) {
        auto [group, sort] = categoryAndSort(item, grouping_);
        groups[group].push_back(&item);
        groupToSort[group] = sort;
    }

    for (const auto& [group, items] : groups) {
        auto groupNode = std::make_unique<Node>(Node{.type = Node::Type::Group,
                                                     .name = utilqt::toQString(group),
                                                     .sort = groupToSort[group],
                                                     .item = nullptr,
                                                     .parent = root_.get(),
                                                     .row = root_->size(),
                                                     .children = {}});
        for (auto* item : items) {
            groupNode->children.push_back(
                std::make_unique<Node>(Node{.type = Node::Type::Item,
                                            .name = utilqt::toQString(item->info.displayName),
                                            .sort = QVariant{},
                                            .item = item,
                                            .parent = groupNode.get(),
                                            .row = groupNode->size(),
                                            .children = {}}));
            itemToNode_[item] = groupNode->children.back().get();
            classIdentifierToItem_[item->info.classIdentifier] = item;
        }
        root_->children.push_back(std::move(groupNode));
    }
}

bool ProcessorListModel::updateItem(std::string_view classIdentifier,
                                    const std::function<bool(Item&)>& updater) {
    auto it = classIdentifierToItem_.find(classIdentifier);
    if (it != classIdentifierToItem_.end()) {
        if (updater(*it->second)) {
            if (grouping_ == Grouping::LastUsed || grouping_ == Grouping::MostUsed) {
                beginResetModel();
                build();
                endResetModel();
            } else {
                auto* node = itemToNode_[it->second];
                auto index = createIndex(node->row, 0, node);
                dataChanged(index, index);
                return true;
            }
        }
    }
    return false;
}

void ProcessorListModel::addItem(Item item) {
    beginResetModel();
    items_.push_back(std::move(item));
    build();
    endResetModel();
}
void ProcessorListModel::removeItem(std::string_view classIdentifier) {
    beginResetModel();
    std::erase_if(items_,
                  [&](const auto& item) { return item.info.classIdentifier == classIdentifier; });
    build();
    endResetModel();
}

}  // namespace inviwo
