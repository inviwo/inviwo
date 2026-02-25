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

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/qt/editor/processorlistwidget.h>
#include <inviwo/qt/editor/helpwidget.h>
#include <inviwo/core/processors/processorstate.h>
#include <inviwo/core/processors/processortags.h>
#include <inviwo/core/processors/processordocs.h>
#include <inviwo/core/processors/processorutils.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/network/networkutils.h>
#include <inviwo/core/common/inviwomodule.h>
#include <inviwo/core/common/modulemanager.h>
#include <inviwo/core/util/document.h>
#include <inviwo/core/util/rendercontext.h>
#include <inviwo/core/util/docbuilder.h>
#include <inviwo/core/util/transparentmaps.h>
#include <inviwo/core/network/autolinker.h>
#include <modules/qtwidgets/inviwoqtutils.h>
#include <inviwo/core/metadata/processormetadata.h>
#include <inviwo/qt/editor/lineediteventfilter.h>
#include <inviwo/qt/editor/editorutils.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QApplication>
#include <QLayout>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWidget>
#include <QMimeData>
#include <QHeaderView>
#include <QSettings>
#include <QVariant>
#include <QString>
#include <chrono>
#include <ctime>
#include <QSortFilterProxyModel>
#include <warn/pop>

namespace inviwo {

ProcessorListWidget::ProcessorListWidget(InviwoMainWindow* parent, HelpWidget* helpWidget)
    : InviwoDockWidget(tr("Processors"), parent, "ProcessorTreeWidget")
    , app_{parent->getInviwoApplication()}
    , win_{parent}
    , model_{new ProcessorListModel(this)}
    , filter_{new ProcessorListFilter(model_, app_->getProcessorNetwork(), this)}
    , view_{new ProcessorListView(filter_, this)}
    , helpWidget_{helpWidget} {

    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    resize(utilqt::emToPx(this, QSizeF(50, 80)));  // default size

    QWidget* centralWidget = new QWidget();
    QVBoxLayout* vLayout = new QVBoxLayout(centralWidget);
    const auto space = utilqt::refSpacePx(this);
    vLayout->setSpacing(space);
    vLayout->setContentsMargins(space, space, space, space);
    lineEdit_ = new QLineEdit(centralWidget);
    {
        lineEdit_->setPlaceholderText("Filter processor list...");

        QIcon clearIcon;
        clearIcon.addFile(":/svgicons/lineedit-clear.svg", utilqt::emToPx(this, QSizeF(0.3, 0.3)),
                          QIcon::Normal);
        clearIcon.addFile(":/svgicons/lineedit-clear-active.svg",
                          utilqt::emToPx(this, QSizeF(0.3, 0.3)), QIcon::Active);
        clearIcon.addFile(":/svgicons/lineedit-clear-active.svg",
                          utilqt::emToPx(this, QSizeF(0.3, 0.3)), QIcon::Selected);
        auto clearAction = lineEdit_->addAction(clearIcon, QLineEdit::TrailingPosition);
        clearAction->setVisible(false);
        connect(clearAction, &QAction::triggered, lineEdit_, &QLineEdit::clear);
        connect(lineEdit_, &QLineEdit::textChanged, this, [this, clearAction](const QString& str) {
            clearAction->setVisible(!str.isEmpty());
            QSettings settings;
            settings.beginGroup(objectName());
            settings.setValue("filterText", QVariant(lineEdit_->text()));
            settings.endGroup();
        });
        vLayout->addWidget(lineEdit_);
    }
    QHBoxLayout* listViewLayout = new QHBoxLayout();
    listViewLayout->addWidget(new QLabel("Group by", centralWidget));
    listView_ = new QComboBox(centralWidget);
    listView_->addItem("Alphabet", QVariant::fromValue(Grouping::Alphabetical));
    listView_->addItem("Category", QVariant::fromValue(Grouping::Categorical));
    listView_->addItem("Code State", QVariant::fromValue(Grouping::CodeState));
    listView_->addItem("Module", QVariant::fromValue(Grouping::Module));
    listView_->addItem("Last Used", QVariant::fromValue(Grouping::LastUsed));
    listView_->addItem("Most Used", QVariant::fromValue(Grouping::MostUsed));
    listView_->addItem("Inports", QVariant::fromValue(Grouping::Inports));
    listView_->addItem("Outports", QVariant::fromValue(Grouping::Outports));
    listView_->setCurrentIndex(1);
    connect(listView_, &QComboBox::currentIndexChanged, this, [this](int index) {
        QSettings settings;
        settings.beginGroup(objectName());
        settings.setValue("currentView", index);
        settings.endGroup();
    });
    listView_->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    listViewLayout->addWidget(listView_);
    vLayout->addLayout(listViewLayout);

    lineEdit_->setToolTip(utilqt::toQString(filter_->description()));
    connect(lineEdit_, &QLineEdit::textChanged, filter_,
            [this](const QString& str) { filter_->setCustomFilter(str); });

    connect(listView_, &QComboBox::currentIndexChanged, this, [this](int) {
        auto grouping = listView_->currentData().value<Grouping>();
        filter_->setGrouping(grouping);
        model_->setGrouping(grouping);
        filter_->sort(0);
    });

    QObject::connect(filter_, &QAbstractItemModel::modelReset, view_,
                     &ProcessorListView::expandAll);
    QObject::connect(filter_, &QAbstractItemModel::rowsInserted, view_,
                     &ProcessorListView::expandAll);

    lineEdit_->installEventFilter(new LineEditEventFilter(view_, lineEdit_, false));

    connect(view_->selectionModel(), &QItemSelectionModel::currentRowChanged, this,
            [this](const QModelIndex& current, const QModelIndex&) {
                if (const auto* item = utilqt::getData(current, Role::Item).value<const Item*>()) {
                    helpWidget_->showDocForClassName(item->info.classIdentifier);
                    if (!win_->tabifiedDockWidgets(this).contains(helpWidget_)) {
                        helpWidget_->raise();
                    }
                }
            });

    vLayout->addWidget(view_);
    centralWidget->setLayout(vLayout);
    setWidget(centralWidget);

    onModulesDidRegister_ =
        app_->getModuleManager().onModulesDidRegister([this]() { buildList(); });
    onModulesWillUnregister_ = app_->getModuleManager().onModulesWillUnregister([this]() {
        model_->setItems({});
        app_->getProcessorFactory()->removeObserver(this);
    });

    QSettings settings;
    settings.beginGroup(objectName());
    lineEdit_->setText(settings.value("filterText", "").toString());
    listView_->setCurrentIndex(settings.value("currentView", 1).toInt());

    auto useCounts = settings.value("useCounts", QVariant(QMap<QString, QVariant>{})).toMap();
    for (auto it = useCounts.constBegin(); it != useCounts.constEnd(); ++it) {
        useCounts_[utilqt::fromQString(it.key())] = it.value().toLongLong();
    }
    auto useTimes = settings.value("useTimes", QVariant(QMap<QString, QVariant>{})).toMap();
    for (auto it = useTimes.constBegin(); it != useTimes.constEnd(); ++it) {
        useTimes_[utilqt::fromQString(it.key())] =
            static_cast<std::time_t>(it.value().toLongLong());
    }
    settings.endGroup();

    {  // ensure that the grouping is applied to the model and filter
        const auto grouping = listView_->currentData().value<Grouping>();
        filter_->setGrouping(grouping);
        model_->setGrouping(grouping);
        filter_->sort(0);
    }
}

ProcessorListWidget::~ProcessorListWidget() = default;

void ProcessorListWidget::focusSearch(bool selectAll) {
    raise();
    lineEdit_->setFocus();
    if (selectAll) lineEdit_->selectAll();
}

void ProcessorListWidget::addSelectedProcessor() {
    QString id;
    auto indices = view_->selectionModel()->selectedIndexes();
    if (!indices.empty()) {
        id = utilqt::getData(indices[0], Role::ClassIdentifier).toString();
    }

    if (!id.isEmpty()) {
        addProcessor(id);
        view_->clearSelection();
    } else {
        view_->setFocus(Qt::ShortcutFocusReason);
    }
}

std::shared_ptr<Processor> ProcessorListWidget::createProcessor(const QString& cid) {
    // Make sure the default render context is active to make sure any FBOs etc created in
    // the processor belong to the default context.
    rendercontext::activateDefault();
    const auto className = utilqt::fromQString(cid);
    try {
        if (auto p = app_->getProcessorFactory()->createShared(className)) {
            recordProcessorUse(className);
            return p;
        }
    } catch (Exception& exception) {
        log::exception(exception, "Unable to create processor '{}' due to:\n{}", className,
                       exception.getMessage());
    }
    return nullptr;
}

auto ProcessorListWidget::getGrouping() const -> Grouping {
    return listView_->currentData().value<Grouping>();
}

void ProcessorListWidget::setPredecessorProcessor(std::string_view identifier) {
    lineEdit_->setText(utilqt::toQString(fmt::format("pre:{} ", identifier)));
}

void ProcessorListWidget::addProcessor(const QString& className) {
    // create processor, add it to processor network
    auto* network = app_->getProcessorNetwork();
    if (auto processor = createProcessor(className)) {
        if (auto* target = filter_->currentStr("pre")
                               .transform([&](std::string_view id) {
                                   return network->getProcessorByIdentifier(id);
                               })
                               .value_or(nullptr)) {
            utilqt::addProcessorAndConnect(processor, network, target);
            util::setSelected(processor.get(), true);
            setPredecessorProcessor(processor->getIdentifier());
            focusSearch(false);
        } else {
            const auto bb = util::getBoundingBox(network->getProcessors());
            util::setPosition(processor.get(), ivec2{bb.first.x, bb.second.y} + ivec2{0, 75});
            network->addProcessor(processor);
        }

        AutoLinker::addLinks(*network, *processor);
    }
}

void ProcessorListWidget::buildList() {
    std::vector<ProcessorListModel::Item> items;
    const auto docs = win_->getDocs();
    for (auto& inviwoModule : app_->getModuleManager().getInviwoModules()) {
        for (const auto& processor : inviwoModule.getProcessors()) {
            const auto* help = docs->get(processor->getClassIdentifier());
            items.emplace_back(ProcessorListModel::Item{
                .info = processor->getProcessorInfo(),
                .help = help ? *help : help::HelpProcessor{},
                .moduleId = inviwoModule.getIdentifier(),
                .metaInfo = processor->getMetaInformation(),
                .lastUsed =
                    [&]() {
                        auto it = useTimes_.find(processor->getClassIdentifier());
                        return it == useTimes_.end() ? 0 : it->second;
                    }(),
                .useCount =
                    [&]() {
                        auto it = useCounts_.find(processor->getClassIdentifier());
                        return it == useCounts_.end() ? 0 : it->second;
                    }()});
        }
    }
    model_->setItems(std::move(items));
    app_->getProcessorFactory()->addObserver(this);
}

void ProcessorListWidget::onRegister(ProcessorFactoryObject* pfo) {
    const auto docs = win_->getDocs();
    const auto* help = docs->get(pfo->getClassIdentifier());

    const auto moduleId =
        util::getProcessorModuleIdentifier(pfo->getClassIdentifier(), *app_).value_or("Unknown");

    model_->addItem(Item{.info = pfo->getProcessorInfo(),
                         .help = help ? *help : help::HelpProcessor{},
                         .moduleId = std::string{moduleId},
                         .metaInfo = pfo->getMetaInformation(),
                         .lastUsed =
                             [&]() {
                                 auto it = useTimes_.find(pfo->getClassIdentifier());
                                 return it == useTimes_.end() ? 0 : it->second;
                             }(),
                         .useCount =
                             [&]() {
                                 auto it = useCounts_.find(pfo->getClassIdentifier());
                                 return it == useCounts_.end() ? 0 : it->second;
                             }()});
}
void ProcessorListWidget::onUnRegister(ProcessorFactoryObject* pfo) {
    model_->removeItem(pfo->getClassIdentifier());
}

void ProcessorListWidget::closeEvent(QCloseEvent* event) {
    QSettings settings;
    settings.beginGroup(objectName());
    settings.setValue("filterText", QVariant(lineEdit_->text()));
    settings.setValue("currentView", QVariant(listView_->currentIndex()));

    {
        QMap<QString, QVariant> useCounts;
        for (const auto& i : useCounts_) {
            useCounts[utilqt::toQString(i.first)] = QVariant::fromValue<qint64>(i.second);
        }
        settings.setValue("useCounts", QVariant(useCounts));
    }
    {
        QMap<QString, QVariant> useTimes;
        for (const auto& i : useTimes_) {
            useTimes[utilqt::toQString(i.first)] = QVariant::fromValue<qint64>(i.second);
        }
        settings.setValue("useTimes", QVariant(useTimes));
    }
    settings.endGroup();

    InviwoDockWidget::closeEvent(event);
}

void ProcessorListWidget::recordProcessorUse(const std::string& id) {
    const auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    ++useCounts_[id];
    useTimes_[id] = now;

    model_->updateItem(id, [&](Item& item) {
        item.lastUsed = now;
        ++item.useCount;
        return true;
    });
}

}  // namespace inviwo
