/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017 Inviwo Foundation
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
#include <warn/pop>

namespace inviwo {

namespace detail {
class ItemModel : public QStandardItemModel {
public:
    ItemModel(QObject *parent) : QStandardItemModel(parent) { setColumnCount(3); }

    virtual Qt::ItemFlags flags(const QModelIndex &index) const override {
        return Qt::ItemIsEnabled;
    }

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
        /*if (role == Qt::SizeHintRole) {
            return QSize(200, 25);
        }*/
        return QStandardItemModel::data(index, role);
    }
};
}  // namespace detail

ResourceManagerDockWidget::ResourceManagerDockWidget(QWidget *parent, ResourceManager &manager)
    : InviwoDockWidget("Resource Manager", parent, "ResourceManager"), manager_(manager) {
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    resize(QSize(400, 700));  // default size

    manager_.addObserver(this);

    auto layout = new QVBoxLayout();
    setContents(layout);

    tableView_ = new QTableView();
    tableView_->setSortingEnabled(true);
    tableView_->verticalHeader()->hide();

    model_ = new detail::ItemModel(this);

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
    buttomRowLayout->addStretch();

    auto clearAllButton = new QPushButton("Clear all");
    buttomRowLayout->addWidget(clearAllButton);

    connect(clearAllButton,&QPushButton::pressed,[rm=&this->manager_](){
        rm->clear();
    });


}

ResourceManagerDockWidget::~ResourceManagerDockWidget() { manager_.removeObserver(this); }

void ResourceManagerDockWidget::onResourceAdded(const std::string &key, const std::string &type,
                                                Resource *resource) {
    QList<QStandardItem *> row;

    auto keyC = key;
    replaceInString(keyC, "\\", "/");
    auto keySplit = splitString(keyC, '/').back();

    auto item = new QStandardItem(utilqt::toQString(keySplit));
    item->setToolTip(utilqt::toQString(keyC));
    auto item2 = new QStandardItem(utilqt::toQString(type));
    row.append(item);
    row.append(item2);
    auto rowID = model_->rowCount();
    model_->appendRow(row);
    auto btn = new QToolButton();
    btn->setIcon(QIcon(":/icons/edit-delete.png"));
    btn->setToolTip("Remove resource");
    tableView_->setIndexWidget(model_->index(rowID, 2), btn);
    connect(btn, &QPushButton::pressed,
            [ key, type, rowID, m = this->model_, rm = &this->manager_ ]() {
                rm->removeResource(key, type);
            });
}

void ResourceManagerDockWidget::onResourceRemoved(const std::string &key, const std::string &type,
                                                  Resource *resource) {
    auto keyC = key;
    replaceInString(keyC, "\\", "/");
    auto keySplit = splitString(keyC, '/').back();
    for (int i = 0; i < model_->rowCount(); i++) {

        //TODO should check for full key, not keySplit
        if (utilqt::fromQString(model_->item(i, 0)->text()) == keySplit &&
            utilqt::fromQString(model_->item(i, 1)->text()) == type) {
            model_->removeRow(i);
            break;
        }
    }
}

}  // namespace inviwo
