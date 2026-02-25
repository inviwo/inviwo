/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2026 Inviwo Foundation
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
#include <modules/qtwidgets/inviwodockwidget.h>
#include <inviwo/core/processors/processorfactoryobject.h>
#include <inviwo/core/processors/processorfactory.h>

#include <inviwo/qt/editor/processorlistmodel.h>
#include <inviwo/qt/editor/processorlistfilter.h>
#include <inviwo/qt/editor/processorlistview.h>

#include <optional>

#include <warn/push>
#include <warn/ignore/all>
#include <QComboBox>
#include <QLineEdit>
#include <QListWidget>
#include <QMouseEvent>
#include <QTreeView>
#include <QDrag>
#include <QString>
#include <warn/pop>

namespace inviwo {

class HelpWidget;
class InviwoMainWindow;
class InviwoApplication;
class ProcessorListWidget;
class Processor;

class IVW_QTEDITOR_API ProcessorListWidget : public InviwoDockWidget,
                                             public FactoryObserver<ProcessorFactoryObject> {
public:
    using Grouping = ProcessorListModel::Grouping;
    using Item = ProcessorListModel::Item;
    using Role = ProcessorListModel::Role;
    using Type = ProcessorListModel::Node::Type;

    ProcessorListWidget(InviwoMainWindow* parent, HelpWidget* helpWidget);
    ProcessorListWidget(const ProcessorListWidget&) = delete;
    ProcessorListWidget& operator=(const ProcessorListWidget&) = delete;
    ProcessorListWidget(ProcessorListWidget&&) = delete;
    ProcessorListWidget& operator=(ProcessorListWidget&&) = delete;
    virtual ~ProcessorListWidget();

    void focusSearch(bool selectAll = true);
    void addSelectedProcessor();
    void addProcessor(const QString& className);
    void recordProcessorUse(const std::string& id);

    std::shared_ptr<Processor> createProcessor(const QString& cid);

    Grouping getGrouping() const;

    void setPredecessorProcessor(std::string_view identifier);

    void buildList();

protected:
    const QIcon* getCodeStateIcon(CodeState) const;

private:
    virtual void onRegister(ProcessorFactoryObject* pfo) override;
    virtual void onUnRegister(ProcessorFactoryObject*) override;

    virtual void closeEvent(QCloseEvent* event) override;

    InviwoApplication* app_;
    InviwoMainWindow* win_;

    ProcessorListModel* model_;
    ProcessorListFilter* filter_;
    ProcessorListView* view_;

    QComboBox* listView_;
    QLineEdit* lineEdit_;

    QPoint dragStartPosition_;

    HelpWidget* helpWidget_;

    std::unordered_map<std::string, size_t> useCounts_;
    std::unordered_map<std::string, std::time_t> useTimes_;

    // Called after modules have been registered
    std::shared_ptr<std::function<void()>> onModulesDidRegister_;
    // Called before modules have been unregistered
    std::shared_ptr<std::function<void()>> onModulesWillUnregister_;
};

}  // namespace inviwo
