/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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

#ifndef IVW_PROCESSORLISTWIDGET_H
#define IVW_PROCESSORLISTWIDGET_H

#include <inviwo/qt/editor/inviwoqteditordefine.h>
#include <modules/qtwidgets/inviwodockwidget.h>
#include <inviwo/core/processors/processorfactoryobject.h>
#include <inviwo/core/processors/processorfactory.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QComboBox>
#include <QLineEdit>
#include <QListWidget>
#include <QMouseEvent>
#include <QTreeWidget>
#include <QDrag>
#include <warn/pop>

namespace inviwo {

class HelpWidget;
class InviwoMainWindow;
class InviwoApplication;
class ProcessorTreeWidget;
class Processor;

class IVW_QTEDITOR_API ProcessorTree : public QTreeWidget {

public:
    ProcessorTree(ProcessorTreeWidget* parent);
    ~ProcessorTree() = default;

    static const int identifierRole;
    static const int sortRole;
    static const int viewRole;
    static const int typeRole;
    enum ItemType { GroupType, ProcessoorType };

protected:
    virtual void mousePressEvent(QMouseEvent* e) override;
    virtual void mouseMoveEvent(QMouseEvent* e) override;

private:
    void showContextMenu(const QPoint& p);

    ProcessorTreeWidget* processorTreeWidget_;
    QPoint dragStartPosition_;
};

class IVW_QTEDITOR_API ProcessorTreeItem : public QTreeWidgetItem {
    using QTreeWidgetItem::QTreeWidgetItem;
    virtual bool operator<(const QTreeWidgetItem& other) const;
};

class IVW_QTEDITOR_API ProcessorTreeWidget : public InviwoDockWidget,
                                             public FactoryObserver<ProcessorFactoryObject> {
public:
    enum class Grouping { Alphabetical, Categorical, CodeState, Module, LastUsed, MostUsed };
    Q_ENUM(Grouping);

    ProcessorTreeWidget(InviwoMainWindow* parent, HelpWidget* helpWidget);
    ~ProcessorTreeWidget();

    void focusSearch();
    void addSelectedProcessor();
    void addProcessor(QString className);
    void addProcessorsToTree(ProcessorFactoryObject* item = nullptr);
    void recordProcessorUse(const std::string& id);

    std::unique_ptr<Processor> createProcessor(QString cid);

    Grouping getGrouping() const;

protected:
    bool processorFits(ProcessorFactoryObject* processor, const QString& filter);
    const QIcon* getCodeStateIcon(CodeState) const;

private:
    void currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);

    void extractInfoAndAddProcessor(ProcessorFactoryObject* processor, InviwoModule* elem);
    QTreeWidgetItem* addToplevelItemTo(QString title, const std::string& desc);

    virtual void onRegister(ProcessorFactoryObject* item) override;
    virtual void onUnRegister(ProcessorFactoryObject*) override;

    virtual void closeEvent(QCloseEvent* event) override;

    InviwoApplication* app_;
    ProcessorTree* processorTree_;
    QComboBox* listView_;
    QLineEdit* lineEdit_;

    QPoint dragStartPosition_;

    QIcon iconStable_;
    QIcon iconExperimental_;
    QIcon iconBroken_;
    QIcon iconDeprecated_;

    HelpWidget* helpWidget_;

    std::unordered_map<std::string, size_t> useCounts_;
    std::unordered_map<std::string, std::time_t> useTimes_;

    // Called after modules have been registered
    std::shared_ptr<std::function<void()>> onModulesDidRegister_;
    // Called before modules have been unregistered
    std::shared_ptr<std::function<void()>> onModulesWillUnregister_;
};

class IVW_QTEDITOR_API ProcessorDragObject : public QDrag {
public:
    ProcessorDragObject(QWidget* source, std::unique_ptr<Processor> processor);
};

}  // namespace inviwo

#endif  // IVW_PROCESSORLISTWIDGET_H
