/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2024 Inviwo Foundation
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

#include <inviwo/qt/editor/resourcemanager/resourcemanagerdockwidget.h>

#include <inviwo/core/resourcemanager/resourcemanager.h>
#include <inviwo/core/resourcemanager/resource.h>
#include <modules/qtwidgets/inviwoqtutils.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/util/glm.h>
#include <inviwo/core/util/formatconversion.h>
#include <inviwo/core/resourcemanager/resourcemanagerobserver.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QListView>
#include <QTreeView>
#include <QHeaderView>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QPushButton>
#include <QToolButton>
#include <QPainter>
#include <QPixmap>
#include <QIcon>
#include <QCheckBox>
#include <QSortFilterProxyModel>
#include <warn/pop>

namespace inviwo {

class ResourceManagerItemModel : public QAbstractItemModel, public ResourceManagerObserver {
public:
    static constexpr int SortRole = Qt::UserRole + 1;

    ResourceManagerItemModel(ResourceManager* manager, QObject* parent)
        : QAbstractItemModel(parent), manager_(manager) {

        manager_->addObserver(this);
    }

    virtual QModelIndex index(int row, int column,
                              const QModelIndex& parent = QModelIndex()) const override {

        if (parent == QModelIndex{}) {
            return createIndex(row, column, quintptr(-1));
        } else if (parent.parent() == QModelIndex{}) {
            if (row < static_cast<int>(manager_->size(static_cast<size_t>(parent.row())))) {
                return createIndex(row, column, parent.row());
            }
        }
        return {};
    }
    virtual Qt::ItemFlags flags(const QModelIndex& index) const override {
        return Qt::ItemIsEnabled;
    }

    virtual QModelIndex parent(const QModelIndex& index) const override {
        const auto parent = index.internalId();
        if (parent < std::tuple_size_v<ResourceManager::Keys>) {
            return createIndex(static_cast<int>(parent), 0, quintptr(-1));
        }
        return {};
    }

    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override {
        if (parent == QModelIndex{}) {
            return std::tuple_size_v<ResourceManager::Keys>;
        } else if (parent.parent() == QModelIndex{}) {
            return static_cast<int>(manager_->size(static_cast<size_t>(parent.row())));
        } else {
            return 0;
        }
    }
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override { return 4; }

    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role = Qt::DisplayRole) const override {

        constexpr std::array cols{std::string_view{"Dimensions"}, std::string_view{"Format"},
                                  std::string_view{"Description"}, std::string_view{"Size"}};

        if (role == Qt::DisplayRole && section < static_cast<int>(cols.size())) {
            return utilqt::toQString(cols[section]);
        }

        return {};
    }
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override {
        if (role == Qt::DisplayRole) {
            if (index.parent() == QModelIndex{}) {
                if (index.row() < static_cast<int>(ResourceManager::names.size())) {
                    if (index.column() == 0) {
                        return utilqt::toQString(ResourceManager::names[index.row()]);
                    } else if (index.column() == 1) {
                        return static_cast<int>(manager_->size(static_cast<size_t>(index.row())));
                    } else if (index.column() == 3) {
                        return utilqt::toQString(util::formatBytesToString(
                            manager_->totalByteSize(static_cast<size_t>(index.row()))));
                    }
                }
            } else {
                const auto group = index.parent().row();
                const auto item = index.row();

                if (const auto* resource = manager_->get(group, item)) {
                    if (index.column() == 0) {
                        fmt::memory_buffer buff;
                        auto out = fmt::appender(buff);
                        fmt::format_to(out, "{}", resource->dims.x);
                        if (resource->dims.y != 0) {
                            fmt::format_to(out, "x{}", resource->dims.y);
                        }
                        if (resource->dims.z != 0) {
                            fmt::format_to(out, "x{}", resource->dims.z);
                        }
                        if (resource->dims.w != 0) {
                            fmt::format_to(out, "x{}", resource->dims.w);
                        }

                        return utilqt::toQString(std::string_view{buff.data(), buff.size()});
                    } else if (index.column() == 1) {
                        return utilqt::toQString(fmt::to_string(resource->format));
                    } else if (index.column() == 2) {
                        return utilqt::toQString(resource->desc);
                    } else if (index.column() == 3) {
                        return utilqt::toQString(
                            util::formatBytesToString(resource->sizeInBytes()));
                    }
                }
            }
        }
        if (role == SortRole) {
            if (index.parent() == QModelIndex{}) {
                return {};
            } else {
                const auto group = index.parent().row();
                const auto item = index.row();

                if (const auto* resource = manager_->get(group, item)) {
                    if (index.column() == 0) {
                        return static_cast<qulonglong>(
                            glm::compMul(glm::max(resource->dims, glm::size4_t{1, 1, 1, 1})));
                    } else if (index.column() == 1) {
                        return utilqt::toQString(fmt::to_string(resource->format));
                    } else if (index.column() == 2) {
                        return utilqt::toQString(resource->desc);
                    } else if (index.column() == 3) {
                        return static_cast<qulonglong>(resource->sizeInBytes());
                    }
                }
            }
        }
        return {};
    }

    virtual void onWillAddResource(size_t group, size_t item, const Resource&) override {
        const auto parentIndex = index(static_cast<int>(group), 0);
        beginInsertRows(parentIndex, static_cast<int>(item), static_cast<int>(item));
    }
    virtual void onDidAddResource(size_t, size_t, const Resource&) override { endInsertRows(); }
    virtual void onWillUpdateResource(size_t group, size_t item, const Resource&) override {}
    virtual void onDidUpdateResource(size_t group, size_t item, const Resource&) override {
        const auto parentIndex = index(static_cast<int>(group), 0);
        dataChanged(index(static_cast<int>(item), 0, parentIndex),
                    index(static_cast<int>(item), 3, parentIndex));
    }
    virtual void onWillRemoveResource(size_t group, size_t item, const Resource&) override {
        const auto parentIndex = index(static_cast<int>(group), 0);
        beginRemoveRows(parentIndex, static_cast<int>(item), static_cast<int>(item));
    }
    virtual void onDidRemoveResource(size_t, size_t, const Resource&) override { endRemoveRows(); }

private:
    ResourceManager* manager_;  // should not be null
};

ResourceManagerDockWidget::ResourceManagerDockWidget(QWidget* parent, ResourceManager& manager)
    : InviwoDockWidget("Resource Manager", parent, "ResourceManager"), manager_(manager) {
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    resize(utilqt::emToPx(this, QSizeF(40, 70)));  // default size

    auto layout = new QVBoxLayout();
    setContents(layout);

    view_ = new QTreeView();
    view_->setSortingEnabled(true);

    model_ = new ResourceManagerItemModel(&manager_, this);

    auto* sortProxy = new QSortFilterProxyModel(this);
    sortProxy->setSourceModel(model_);
    sortProxy->setSortRole(ResourceManagerItemModel::SortRole);

    view_->setModel(sortProxy);
    view_->setIndentation(utilqt::emToPx(this, 1.0));

    layout->addWidget(view_);

    view_->header()->setDefaultAlignment(Qt::AlignLeft);
    view_->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    view_->header()->setSectionResizeMode(1, QHeaderView::Stretch);
    view_->header()->setSectionResizeMode(2, QHeaderView::Stretch);
    view_->header()->setSectionResizeMode(3, QHeaderView::Stretch);

    view_->expandRecursively({});
}

}  // namespace inviwo
