/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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
#include <modules/qtwidgets/inviwoqtutils.h>
#include <inviwo/core/util/stringconversion.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QListView>
#include <QTableView>
#include <QHeaderView>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QPushButton>
#include <QToolButton>
#include <QPainter>
#include <QPixmap>
#include <QIcon>
#include <QCheckBox>
#include <warn/pop>

namespace inviwo {

int ResourceRole = Qt::UserRole + 1;

class ResourceManagerItemModel : public QStandardItemModel {
public:
    ResourceManagerItemModel(QObject *parent) : QStandardItemModel(parent) { setColumnCount(3); }

    virtual Qt::ItemFlags flags(const QModelIndex & /*index*/) const override {
        return Qt::ItemIsEnabled;
    }

    Resource *getResource(int row) const {
        auto qvar = data(this->index(row, 0), ResourceRole);
        return static_cast<Resource *>(qvar.value<void *>());
    }

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {

        if (role == Qt::ToolTipRole) {
            auto resource = getResource(index.row());
            auto toolTip = resource->info();
            return QString(utilqt::toQString(toolTip));
        }
        return QStandardItemModel::data(index, role);
    }
};

ResourceManagerDockWidget::ResourceManagerDockWidget(QWidget *parent, ResourceManager &manager)
    : InviwoDockWidget("Resource Manager", parent, "ResourceManager"), manager_(manager) {
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    resize(utilqt::emToPx(this, QSizeF(40, 70)));  // default size

    manager_.addObserver(this);

    auto layout = new QVBoxLayout();
    setContents(layout);

    tableView_ = new QTableView();
    tableView_->setSortingEnabled(true);
    tableView_->verticalHeader()->hide();

    model_ = new ResourceManagerItemModel(this);

    model_->setHorizontalHeaderLabels({"Name", "Type", ""});

    tableView_->setModel(model_);
    tableView_->setGridStyle(Qt::NoPen);
    tableView_->setCornerButtonEnabled(false);

    layout->addWidget(tableView_);

    tableView_->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    tableView_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    tableView_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    tableView_->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
    tableView_->setColumnWidth(2, 32);

    auto buttomRowLayout = new QHBoxLayout();
    layout->addLayout(buttomRowLayout);

    disabledCheckBox_ = new QCheckBox("Disable");
    disabledCheckBox_->setChecked(!manager_.isEnabled());
    buttomRowLayout->addWidget(disabledCheckBox_);

    buttomRowLayout->addStretch();

    auto clearAllButton = new QPushButton("Clear all");
    buttomRowLayout->addWidget(clearAllButton);

    connect(clearAllButton, &QPushButton::pressed, [rm = &this->manager_]() { rm->clear(); });

    connect(disabledCheckBox_, &QCheckBox::stateChanged, [this](int state) {
        disabledCheckBox_->blockSignals(true);
        manager_.setEnabled(state == Qt::Unchecked);
        disabledCheckBox_->blockSignals(false);
    });
}

ResourceManagerDockWidget::~ResourceManagerDockWidget() { manager_.removeObserver(this); }

void ResourceManagerDockWidget::onResourceAdded(const std::string &key, const std::type_index &type,
                                                Resource *resource) {
    QList<QStandardItem *> row;

    auto keyC = key;
    replaceInString(keyC, "\\", "/");
    auto keySplit = splitString(keyC, '/').back();

    row.append(new QStandardItem(utilqt::toQString(keySplit)));
    row.append(new QStandardItem(utilqt::toQString(resource->typeDisplayName())));
    auto rowID = model_->rowCount();
    model_->appendRow(row);
    auto btn = new QToolButton();
    btn->setIcon(QIcon(":/svgicons/edit-delete.svg"));
    btn->setToolTip("Remove resource");
    tableView_->setIndexWidget(model_->index(rowID, 2), btn);

    model_->setData(model_->index(rowID, 0), QVariant::fromValue((void *)resource), ResourceRole);

    connect(btn, &QPushButton::pressed,
            [key, t = type, rm = &this->manager_]() { rm->removeResource(key, t); });
}

void ResourceManagerDockWidget::onResourceRemoved(const std::string &, const std::type_index &,
                                                  Resource *resource) {
    for (int i = 0; i < model_->rowCount(); i++) {
        if (model_->getResource(i) == resource) {
            model_->removeRow(i);
            break;
        }
    }
}

void ResourceManagerDockWidget::onResourceManagerEnableStateChanged() {
    disabledCheckBox_->blockSignals(true);
    disabledCheckBox_->setChecked(!manager_.isEnabled());
    disabledCheckBox_->blockSignals(false);
}

}  // namespace inviwo
