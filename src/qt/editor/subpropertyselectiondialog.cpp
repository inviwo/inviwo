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
#include <inviwo/core/properties/propertyowner.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/util/stdextensions.h>
#include <modules/qtwidgets/inviwoqtutils.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QHBoxLayout>

#include <QTreeView>
#include <QAbstractItemModel>

#include <QPlainTextEdit>
#include <warn/pop>

#include <vector>
#include <unordered_map>
#include <limits>

namespace inviwo {

class NetworkTreeModel : public QAbstractItemModel {
public:
    NetworkTreeModel(ProcessorNetwork& network, QObject* parent)
        : QAbstractItemModel(parent), processors_{network.getProcessors()} {}

    // Overrides
    virtual QModelIndex index(int row, int column,
                              const QModelIndex& parent = QModelIndex()) const override {

        if (parent.isValid()) {
            if (auto* owner = toOwner(parent)) {
                return createIndex(row, column, owner);
            } else {
                return {};
            }
        } else if (row >= 0 && row < ssize(processors_)) {
            return createIndex(row, column, nullptr);
        }
        return {};
    }
    virtual QModelIndex parent(const QModelIndex& index) const override {
        if (index.isValid()) {
            if (auto parent = getParent(index)) {
                if (auto grandParent = parent->getOwner()) {
                    auto& siblings = grandParent->getProperties();
                    size_t row =
                        std::distance(siblings.begin(), std::find(siblings.begin(), siblings.end(),
                                                                  dynamic_cast<Property*>(parent)));
                    createIndex(row, 0, grandParent);
                } else if (auto* processor = dynamic_cast<Processor*>(parent)) {
                    size_t row =
                        std::distance(processors_.begin(),
                                      std::find(processors_.begin(), processors_.end(), parent));
                    createIndex(row, 0, nullptr);
                }
            }
        }
        return {};
    }
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override {
        if (parent.isValid()) {
            if (auto owner = toOwner(parent)) {
                return owner->size();
            } else {
                return 0;
            }
        } else {
            return processors_.size();
        }
    }
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override { return 1; }
    virtual QVariant data(const QModelIndex& index, int role) const override {
        if (!index.isValid()) return {};

        if (auto prop = toProperty(index)) {
            LogWarn("Data Property role:" << role << " row: " << index.row()
                                          << " path: " << prop->getPath());
            switch (role) {
                case Qt::DisplayRole:
                    return utilqt::toQString(prop->getIdentifier());
            }
        } else if (auto owner = toOwner(index)) {
            LogWarn("Data Owner role:" << role << " row: " << index.row()
                                       << " path: " << prop->getPath());
            switch (role) {
                case Qt::DisplayRole:
                    return utilqt::toQString(owner->getIdentifier());
            }
        }

        return {};
    }

private:
    template <typename T>
    static int ssize(const T& cont) {
        return static_cast<int>(cont.size());
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

    std::vector<Processor*> processors_;
};

SubPropertySelectionDialog::SubPropertySelectionDialog(CompositeProcessor* processor,
                                                       QWidget* parent)
    : InviwoDockWidget("Add Sub Properties", parent, "SubPropertySelectionWidget") {

    setFloating(true);
    setAttribute(Qt::WA_DeleteOnClose);
    setAllowedAreas(Qt::NoDockWidgetArea);
    setMinimumWidth(utilqt::emToPx(this, 50));
    setMinimumHeight(utilqt::emToPx(this, 50));

    loadState();

    auto& net = processor->getSubNetwork();
    auto model = new NetworkTreeModel(net, this);

    auto layout = new QHBoxLayout();
    auto tree = new QTreeView(this);
    tree->setModel(model);

    QSizePolicy sp = tree->sizePolicy();
    sp.setHorizontalStretch(3);
    sp.setVerticalStretch(3);
    sp.setVerticalPolicy(QSizePolicy::Preferred);
    tree->setSizePolicy(sp);

    layout->addWidget(tree);
    setContents(layout);
}

}  // namespace inviwo
