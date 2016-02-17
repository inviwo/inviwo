/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#ifndef IVW_LINKDIALOG_H
#define IVW_LINKDIALOG_H

#include <inviwo/qt/editor/inviwoqteditordefine.h>
#include <inviwo/qt/widgets/inviwodockwidget.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QComboBox>
#include <QEventLoop>
#include <warn/pop>

class QStandardItemModel;
class QStandardItem;
class QPushButton;

namespace inviwo {

class Processor;
class Property;
class PropertyOwner;
class LinkDialogGraphicsView;
class LinkDialogGraphicsScene;

class IVW_QTEDITOR_API CheckableQComboBox : public QComboBox {
#include <warn/push>
#include <warn/ignore/all>
    Q_OBJECT
#include <warn/pop>
public:
    CheckableQComboBox(QWidget* parent, std::string widgetName, std::vector<std::string> options);
    virtual ~CheckableQComboBox();
    bool isItemChecked(int i);
    std::vector<std::string> getCheckedItems();
public slots:
    void onSmartLinkOptionChecked(const QModelIndex&, const QModelIndex&);

private:
    QStandardItemModel* stdandardModel_;
    std::vector<QStandardItem*> standardItems_;
    std::string widgetName_;
};

class IVW_QTEDITOR_API LinkDialog : public InviwoDockWidget {
#include <warn/push>
#include <warn/ignore/all>
    Q_OBJECT
#include <warn/pop>
public:
    LinkDialog(Processor* src, Processor* dest, QWidget* parent);

    virtual void closeEvent(QCloseEvent* event);
    int exec();
    virtual QSize sizeHint() const;
    virtual ~LinkDialog();

private slots:
    void closeLinkDialog();
    void clickedSmartLinkPushButton();
    void clickedDeleteAllLinksPushButton();
    void expandCompositeProperties();

private:
    LinkDialogGraphicsView* linkDialogView_;
    LinkDialogGraphicsScene* linkDialogScene_;
    QPushButton* smartLinkPushButton_;
    CheckableQComboBox* smartLinkOptions_;
    QPushButton* expandCompositeButton_;
    QPushButton* deleteAllLinkPushButton_;
    Processor* src_;
    Processor* dest_;
    QEventLoop eventLoop_;
};

}  // namespace

#endif  // IVW_LINKDIALOG_H
