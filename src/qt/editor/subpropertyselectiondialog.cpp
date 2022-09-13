/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022 Inviwo Foundation
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

#include <inviwo/qt/editor/subpropertyselectiondialog.h>

#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/network/lambdanetworkvisitor.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/compositeprocessor.h>
#include <inviwo/core/processors/compositesink.h>
#include <inviwo/core/processors/compositesource.h>
#include <inviwo/core/properties/propertyowner.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/assertion.h>
#include <inviwo/core/util/transformiterator.h>
#include <modules/qtwidgets/inviwoqtutils.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QLabel>
#include <QGridLayout>
#include <QTreeView>
#include <QHeaderView>
#include <QAbstractItemModel>
#include <QMimeData>
#include <QIODevice>
#include <QByteArray>
#include <QDataStream>
#include <QStyledItemDelegate>
#include <QEvent>
#include <QMouseEvent>
#include <QToolButton>
#include <QItemSelectionModel>
#include <warn/pop>

#include <fmt/format.h>

#include <vector>
#include <unordered_map>
#include <limits>

namespace inviwo {

SuperPropertyMimeData::SuperPropertyMimeData(std::vector<Property*> someProperties)
    : QMimeData{}, properties{someProperties} {

    auto str = utilqt::toQString(fmt::format(
        "{}", fmt::join(util::transformRange(properties, [](Property* p) { return p->getPath(); }),
                        ", ")));

    QByteArray byteData;
    QDataStream ds(&byteData, QIODevice::WriteOnly);
    ds << str;
    setData(utilqt::toQString(getMimeTag()), byteData);
    setText(str);
}

const std::string& SuperPropertyMimeData::getMimeTag() {
    static std::string tag{"application/x.vnd.inviwo.superproperties+txt"};
    return tag;
}

const SuperPropertyMimeData* SuperPropertyMimeData::toSuperPropertyMimeData(const QMimeData* data) {
    return qobject_cast<const SuperPropertyMimeData*>(data);
}

class NetworkTreeModel : public QAbstractItemModel,
                         public PropertyOwnerObserver,
                         public PropertyObserver {
public:
    NetworkTreeModel(const std::vector<Processor*>& processors, QObject* parent)
        : QAbstractItemModel(parent), processors_{processors} {

        LambdaNetworkVisitor ownerVisitor{[&](PropertyOwner& o) { o.addObserver(this); }};
        for (auto p : processors_) {
            p->accept(ownerVisitor);
        }

        LambdaNetworkVisitor propVisitor{[&](Property& o) { o.addObserver(this); }};
        for (auto p : processors_) {
            p->accept(propVisitor);
        }
    }

    NetworkTreeModel(ProcessorNetwork& network, QObject* parent)
        : NetworkTreeModel(networkToProcessors(network), parent) {}

    // Overrides
    virtual QModelIndex index(int row, int column,
                              const QModelIndex& parent = QModelIndex()) const override {
        if (auto* owner = toOwner(parent)) {
            return createIndex(row, column, owner);
        } else {
            return createIndex(row, column, nullptr);
        }
    }
    virtual QModelIndex parent(const QModelIndex& index) const override {
        if (auto parent = getParent(index)) {
            if (auto grandParent = parent->getOwner()) {
                return createIndex(
                    position(grandParent->getProperties(), dynamic_cast<Property*>(parent)), 0,
                    grandParent);
            } else if (auto* processor = dynamic_cast<Processor*>(parent)) {
                return createIndex(position(processors_, processor), 0, nullptr);
            }
        }

        return {};
    }

    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override {
        if (!parent.isValid()) {
            return processors_.size();
        } else if (auto owner = toOwner(parent)) {
            return owner->size();
        } else {
            return 0;
        }
    }
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override { return 3; }

    Qt::ItemFlags flags(const QModelIndex& index) const override {
        if (index.column() == 0) {
            Qt::ItemFlags flags = Qt::NoItemFlags;
            if (auto prop = toProperty(index)) {
                if (!isExposed(prop)) {
                    flags =
                        flags | Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled;
                }
            }
            return flags;
        } else {
            return Qt::NoItemFlags;
        }
    }

    virtual QVariant data(const QModelIndex& index, int role) const override {
        if (!index.isValid()) return {};

        auto name = [](auto* item) {
            if (!item->getDisplayName().empty()) {
                return utilqt::toQString(item->getDisplayName());
            } else {
                return utilqt::toQString(item->getIdentifier());
            }
        };

        auto visible = [](Property* prop) {
            if (prop->getVisible()) {
                return QIcon(":/svgicons/canvas-show.svg");
            } else {
                return QIcon(":/svgicons/canvas-hide.svg");
            }
        };

        auto readOnly = [](Property* prop) {
            if (prop->getReadOnly()) {
                return QIcon(":/svgicons/locked.svg");
            } else {
                return QIcon(":/svgicons/unlocked.svg");
            }
        };

        auto tooltip = [](Property* prop) {
            Document desc{};
            auto html = desc.append("html");
            html.append("head").append("style", R"(
                div.name {
                    font-size: 13pt;
                    color: #c8ccd0;;
                    font-weight: bold;
                }
                div.help {
                    font-size: 12pt;
                    margin: 10px 0px 10px 0px;
                    padding: 0px 0px 0px 0px;
                }
                table {
                    margin: 10px 0px 0px 0px;
                    padding: 0px 0px 0px 0px;
                }
            )"_unindent);

            html.append("body").append(prop->getDescription());

            return utilqt::toQString(desc);
        };

        if (index.column() == 0) {
            switch (role) {
                case Qt::DisplayRole: {
                    if (auto prop = toProperty(index)) {
                        return name(prop);
                    } else if (auto proc = toProcessor(index)) {
                        return name(proc);
                    }
                }
                case Qt::EditRole: {
                    if (auto prop = toProperty(index)) {
                        return name(prop);
                    } else if (auto proc = toProcessor(index)) {
                        return name(proc);
                    }
                }
                case Qt::ToolTipRole: {
                    if (auto prop = toProperty(index)) {
                        return tooltip(prop);
                    }
                    break;
                }
                case Qt::ForegroundRole: {
                    if (auto prop = toProperty(index); prop && isExposed(prop)) {
                        return QBrush{QColor("#7d7975")};
                    }
                    break;
                }
            }
        } else if (index.column() == 1) {
            switch (role) {
                case Qt::DecorationRole: {
                    if (auto prop = toProperty(index)) {
                        return visible(prop);
                    }
                    break;
                }
                case Qt::EditRole: {
                    if (auto prop = toProperty(index)) {
                        return prop->getVisible();
                    }
                    break;
                }
                case Qt::ToolTipRole: {
                    if (auto prop = toProperty(index)) {
                        return prop->getVisible() ? "Visible" : "Hidden";
                    }
                    break;
                }
            }
        } else if (index.column() == 2) {
            switch (role) {
                case Qt::DecorationRole: {
                    if (auto prop = toProperty(index)) {
                        return readOnly(prop);
                    }
                    break;
                }
                case Qt::EditRole: {
                    if (auto prop = toProperty(index)) {
                        return prop->getReadOnly();
                    }
                    break;
                }
                case Qt::ToolTipRole: {
                    if (auto prop = toProperty(index)) {
                        return prop->getReadOnly() ? "ReadOnly" : "Editable";
                    }
                    break;
                }
            }
        }

        return {};
    }

    QStringList mimeTypes() const override {
        return {utilqt::toQString(SuperPropertyMimeData::getMimeTag())};
    }

    QMimeData* mimeData(const QModelIndexList& indexes) const override {
        if (indexes.empty()) return nullptr;

        std::vector<Property*> props;
        for (auto index : indexes) {
            if (auto prop = toProperty(index)) {
                props.push_back(prop);
            }
        }
        QMimeData* data = new SuperPropertyMimeData(props);
        return data;
    }

    // PropertyOwnerObserver overrides
    virtual void onWillAddProperty(PropertyOwner* owner, Property* property, size_t pos) override {
        beginInsertRows(index(owner), pos, pos);
    }
    virtual void onDidAddProperty(Property* property, size_t) override {
        endInsertRows();
        LambdaNetworkVisitor ownerVisitor{[&](PropertyOwner* o) { o->addObserver(this); }};
        property->accept(ownerVisitor);

        LambdaNetworkVisitor propVisitor{[&](Property* o) { o->addObserver(this); }};
        property->accept(propVisitor);
    };

    virtual void onWillRemoveProperty(Property* property, size_t pos) override {
        LambdaNetworkVisitor ownerVisitor{[&](PropertyOwner* o) { o->removeObserver(this); }};
        property->accept(ownerVisitor);

        LambdaNetworkVisitor propVisitor{[&](Property* o) { o->removeObserver(this); }};
        property->accept(propVisitor);
        beginRemoveRows(index(property->getOwner()), pos, pos);
    }
    virtual void onDidRemoveProperty(PropertyOwner*, Property*, size_t) override {
        endRemoveRows();
    };

    virtual void onSetDisplayName(Property* property, const std::string&) override {
        QModelIndex pos = index(property);
        dataChanged(pos, pos, {Qt::DisplayRole});
    }

protected:
    static bool isExposed(Property* prop) {
        auto exposed = prop->getMetaData<BoolMetaData>("CompositeProcessorExposed", false);
        if (exposed) return true;
        for (auto owner = dynamic_cast<Property*>(prop->getOwner()); owner;
             owner = dynamic_cast<Property*>(owner->getOwner())) {
            exposed = owner->getMetaData<BoolMetaData>("CompositeProcessorExposed", false);
            if (exposed) return true;
        }
        return false;
    };

    QModelIndex index(Property* property) const {
        auto parent = property->getOwner();
        IVW_ASSERT(parent, "Should always be a parent for a property");

        auto pos = position(parent->getProperties(), property);
        return index(pos, 0, index(parent));
    }
    QModelIndex index(PropertyOwner* owner) const {
        if (auto parent = owner->getOwner()) {
            auto pos = position(parent->getProperties(), dynamic_cast<Property*>(owner));
            return index(pos, 0, index(parent));
        } else {
            auto pos = position(processors_, dynamic_cast<Processor*>(owner));
            return index(pos, 0, QModelIndex{});
        }
    }

    static std::vector<Processor*> networkToProcessors(ProcessorNetwork& net) {
        auto processors = net.getProcessors();
        processors.erase(std::remove_if(processors.begin(), processors.end(),
                                        [](Processor* p) {
                                            return dynamic_cast<CompositeSinkBase*>(p) ||
                                                   dynamic_cast<CompositeSourceBase*>(p);
                                        }),
                         processors.end());
        return processors;
    }

    template <typename T>
    static int ssize(const T& cont) {
        return static_cast<int>(cont.size());
    }

    template <typename Cont, typename Elem>
    static int position(const Cont& cont, const Elem& elem) {
        return static_cast<int>(
            std::distance(cont.begin(), std::find(cont.begin(), cont.end(), elem)));
    }

    PropertyOwner* getParent(const QModelIndex& index) const {
        return static_cast<PropertyOwner*>(index.internalPointer());
    }

    Property* toProperty(const QModelIndex& index) const {
        if (auto parent = getParent(index)) {
            if (index.row() >= 0 && index.row() < ssize(*parent)) {
                return (*parent)[index.row()];
            }
        }
        return nullptr;
    }
    PropertyOwner* toOwner(const QModelIndex& index) const {
        if (auto parent = getParent(index)) {
            if (index.row() >= 0 && index.row() < ssize(*parent)) {
                return dynamic_cast<PropertyOwner*>((*parent)[index.row()]);
            }
        } else if (index.row() >= 0 && index.row() < ssize(processors_)) {
            return processors_[index.row()];
        }
        return nullptr;
    }
    Processor* toProcessor(const QModelIndex& index) const {
        if (auto parent = getParent(index)) {
            if (index.row() >= 0 && index.row() < ssize(*parent)) {
                return dynamic_cast<Processor*>((*parent)[index.row()]);
            }
        } else if (index.row() >= 0 && index.row() < ssize(processors_)) {
            return processors_[index.row()];
        }
        return nullptr;
    }

    std::vector<Processor*> processors_;
};

class CompositeProcessorTreeModel : public NetworkTreeModel {
public:
    CompositeProcessorTreeModel(CompositeProcessor* cp, QObject* parent)
        : NetworkTreeModel{{cp}, parent}, cp_{cp} {}

    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override { return 4; }

    Qt::ItemFlags flags(const QModelIndex& index) const override {
        if (index.column() == 0) {
            Qt::ItemFlags flags = Qt::NoItemFlags;
            if (auto prop = toProperty(index)) {
                flags = flags | Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable |
                        Qt::ItemIsDragEnabled;
            }
            if (auto owner = toOwner(index)) {
                flags = flags | Qt::ItemIsDropEnabled;
            }
            return flags;
        } else {
            return Qt::ItemIsEnabled;
        }
    }

    virtual QVariant data(const QModelIndex& index, int role) const override {
        if (index.column() == 3) {
            switch (role) {
                case Qt::DecorationRole: {
                    if (auto prop = toProperty(index)) {
                        if (cp_->getSubProperty(prop)) {
                            return QIcon(":/svgicons/close.svg");
                        }
                    }
                    break;
                }
                case Qt::ToolTipRole: {
                    if (auto prop = toProperty(index)) {
                        return "Remove";
                    }
                    break;
                }
            }
            return {};
        } else {
            return NetworkTreeModel::data(index, role);
        }
    }

    bool setData(const QModelIndex& index, const QVariant& value,
                 int role = Qt::EditRole) override {

        if (role != Qt::EditRole) return false;

        if (index.column() == 0) {
            if (auto str = value.toString(); !str.isEmpty()) {
                if (auto property = toProperty(index)) {
                    property->setDisplayName(utilqt::fromQString(str));
                    return true;
                }
            }
        } else if (index.column() == 1) {
            if (auto property = toProperty(index)) {
                property->setVisible(value.toBool());
                return true;
            }
        } else if (index.column() == 2) {
            if (auto property = toProperty(index)) {
                property->setReadOnly(value.toBool());
                return true;
            }
        }

        return false;
    }

    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override {
        std::vector<Property*> toRemove;

        for (int i = 0; i < count; ++i) {
            if (auto prop = toProperty(index(row, 0, parent))) {
                if (cp_->getSubProperty(prop)) {
                    toRemove.emplace_back(prop);
                }
            }
        }

        if (toRemove.empty()) {
            return false;
        }

        for (auto prop : toRemove) {
            auto i = index(prop);

            beginRemoveRows(i.parent(), i.row(), i.row());
            cp_->removeSuperProperty(cp_->getSubProperty(prop));
            endRemoveRows();
        }

        return true;
    }

    bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column,
                      const QModelIndex& parent) override {
        try {
            if (auto* propData = SuperPropertyMimeData::toSuperPropertyMimeData(data)) {
                for (auto* subProperty : propData->properties) {
                    cp_->addSuperProperty(subProperty);
                }
                return true;
            } else {
                return false;
            }
        } catch (const Exception& e) {
            util::log(e.getContext(), e.getMessage());
            return false;
        }
    }

protected:
    CompositeProcessor* cp_;
};

class ItemDelegate : public QStyledItemDelegate {
    using QStyledItemDelegate::QStyledItemDelegate;

    virtual bool editorEvent(QEvent* event, QAbstractItemModel* model,
                             const QStyleOptionViewItem& option,
                             const QModelIndex& index) override {

        if (index.column() >= 1 && index.column() <= 2) {
            if (event->type() == QEvent::MouseButtonPress) {
                event->accept();
                return true;
            } else if (event->type() == QEvent::MouseButtonRelease) {
                model->setData(index, !model->data(index, Qt::EditRole).toBool());
                event->accept();
                return true;
            }
        }

        if (index.column() == 3) {
            if (event->type() == QEvent::MouseButtonPress) {
                event->accept();
                return true;
            } else if (event->type() == QEvent::MouseButtonRelease) {
                model->removeRow(index.row(), index.parent());
                event->accept();
                return true;
            }
        }

        return QStyledItemDelegate::editorEvent(event, model, option, index);
    }
};

SubPropertySelectionDialog::SubPropertySelectionDialog(CompositeProcessor* processor,
                                                       QWidget* parent)
    : InviwoDockWidget(
          utilqt::toQString(fmt::format("Configure Properties ({})", processor->getIdentifier())),
          parent, "ConfigureCompositeProcessorWidget")
    , cp_{processor} {

    processor->getNetwork()->addObserver(this);

    setFloating(true);
    setAttribute(Qt::WA_DeleteOnClose);
    setAllowedAreas(Qt::NoDockWidgetArea);
    setMinimumWidth(utilqt::emToPx(this, 20));
    setMinimumHeight(utilqt::emToPx(this, 20));

    loadState();

    auto layout = new QGridLayout();

    auto config = [&](QTreeView* tree) {
        tree->header()->hide();
        tree->setUniformRowHeights(true);
        tree->setAnimated(true);
        tree->setSelectionMode(QAbstractItemView::ExtendedSelection);
        tree->setDragEnabled(true);

        tree->header()->setSectionsMovable(false);
        tree->header()->setStretchLastSection(false);
        tree->header()->setDefaultSectionSize(0);
        tree->header()->setMinimumSectionSize(utilqt::emToPx(this, 2));
        tree->header()->setMaximumSectionSize(utilqt::emToPx(this, 2));
        tree->header()->resizeSection(0, 80);
        tree->header()->resizeSection(1, utilqt::emToPx(this, 2));
        tree->header()->resizeSection(2, utilqt::emToPx(this, 2));

        tree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
        tree->header()->setSectionResizeMode(1, QHeaderView::Fixed);
        tree->header()->setSectionResizeMode(2, QHeaderView::Fixed);
        tree->header()->setStretchLastSection(false);

        QSizePolicy sp = tree->sizePolicy();
        sp.setHorizontalStretch(3);
        sp.setVerticalStretch(3);
        sp.setVerticalPolicy(QSizePolicy::Expanding);
        sp.setHorizontalPolicy(QSizePolicy::MinimumExpanding);
        tree->setSizePolicy(sp);
    };

    layout->addWidget(new QLabel("Composite Network"), 0, 0);
    layout->addWidget(new QLabel("Exposed Properties"), 0, 2);

    // Sub tree
    auto& net = processor->getSubNetwork();
    auto subModel = new NetworkTreeModel(net, this);
    auto subTree = new QTreeView(this);
    subTree->setModel(subModel);
    config(subTree);
    subTree->setDragDropMode(QAbstractItemView::DragOnly);
    layout->addWidget(subTree, 1, 0);

    // Add button
    auto add = new QToolButton(this);
    add->setIcon(QIcon(":/svgicons/link-right.svg"));
    add->setMinimumSize(utilqt::emToPx(this, QSizeF{3, 3}));
    add->setIconSize(utilqt::emToPx(this, QSizeF{3, 3}));
    add->setAutoRaise(true);
    layout->addWidget(add, 1, 1);

    connect(subTree->selectionModel(), &QItemSelectionModel::selectionChanged, this,
            [subTree, add](const QItemSelection&, const QItemSelection&) {
                bool empty = subTree->selectionModel()->selectedIndexes().empty();
                add->setDisabled(empty);
            });
            
    

    // Super tree
    auto superModel = new CompositeProcessorTreeModel(processor, this);
    auto superTree = new QTreeView(this);
    superTree->setModel(superModel);
    superTree->setDragDropMode(QAbstractItemView::DragDrop);
    superTree->setItemDelegate(new ItemDelegate(this));
    config(superTree);
    layout->addWidget(superTree, 1, 2);
    superTree->setRootIndex(superModel->index(0, 0));

    setContents(layout);
}

void SubPropertySelectionDialog::onProcessorNetworkWillRemoveProcessor(Processor* processor) {
    if (processor == cp_) {
        close();
    }
}

}  // namespace inviwo
