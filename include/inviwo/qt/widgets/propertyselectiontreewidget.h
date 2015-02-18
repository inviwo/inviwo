/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#ifndef IVW_PROPERTY_SELECTION_TREE_WIDGET_H
#define IVW_PROPERTY_SELECTION_TREE_WIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QTreeWidget>
#include <QApplication>
#include <QDialog>
#include <QPushButton>
#include <inviwo/qt/widgets/inviwoqtwidgetsdefine.h>

namespace inviwo {

class Property;
class ProcessorNetwork;

class IVW_QTWIDGETS_API PropertySelectionTree : public QTreeWidget {

public:
    PropertySelectionTree(QWidget* parent) : QTreeWidget(parent) {};
    ~PropertySelectionTree() {};

protected:
    void mousePressEvent(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent* e);
private:
    QPoint dragStartPosition_;
};

class IVW_QTWIDGETS_API PropertySelectionTreeWidget : public QWidget {
    Q_OBJECT
public:
    PropertySelectionTreeWidget();
    ~PropertySelectionTreeWidget();
    void addProcessorNetwork(ProcessorNetwork* processorNetwork, std::string workspaceFileName="CurrentWorkspace");
public slots:
    void clear();
    std::vector<Property*> getSelectedProperties(ProcessorNetwork* processorNetwork);
private:
    PropertySelectionTree* propertySelectionTree_;
    QPoint dragStartPosition_;
    QVBoxLayout* vLayout_;
};

class IVW_QTWIDGETS_API PropertySelectionTreeDialog : public QDialog {
    Q_OBJECT
public:
    PropertySelectionTreeDialog(ProcessorNetwork* processorNetwork,
                                std::vector<Property*>& selectedProperty,
                                QWidget* parent);
    ~PropertySelectionTreeDialog() {}
private slots:
    void clickedOkayButton();
    void clickedCancelButton();
private:
    void initDialog();
    PropertySelectionTreeWidget* selectionTree_;
    std::vector<Property*>& selectedProperties_;
    ProcessorNetwork* processorNetwork_;
};


} // namespace

#endif // IVW_PROPERTY_SELECTION_TREE_WIDGET_H