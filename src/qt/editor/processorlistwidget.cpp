/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2023 Inviwo Foundation
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
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/network/networkutils.h>
#include <inviwo/core/common/inviwomodule.h>
#include <inviwo/core/util/document.h>
#include <inviwo/core/util/rendercontext.h>
#include <inviwo/core/network/autolinker.h>
#include <modules/qtwidgets/inviwoqtutils.h>
#include <inviwo/core/metadata/processormetadata.h>
#include <inviwo/qt/editor/processorpreview.h>
#include <inviwo/qt/editor/processormimedata.h>
#include <inviwo/qt/editor/lineediteventfilter.h>

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
#include <warn/pop>

namespace inviwo {

const int ProcessorTree::identifierRole = Qt::UserRole + 1;
const int ProcessorTree::sortRole = Qt::UserRole + 2;
const int ProcessorTree::viewRole = Qt::UserRole + 3;
const int ProcessorTree::typeRole = Qt::UserRole + 4;

void ProcessorTree::mousePressEvent(QMouseEvent* e) {
    if (e->buttons() & Qt::LeftButton) dragStartPosition_ = e->pos();

    QTreeWidget::mousePressEvent(e);
}

void ProcessorTree::mouseMoveEvent(QMouseEvent* e) {
    if ((e->buttons() & Qt::LeftButton) && dragStartPosition_) {
        if ((e->pos() - *dragStartPosition_).manhattanLength() < QApplication::startDragDistance())
            return;

        auto item = itemAt(*dragStartPosition_);
        if (item &&
            item->data(0, ProcessorTree::typeRole).toInt() == ProcessorTree::ProcessoorType) {
            dragStartPosition_.reset();
            auto id = item->data(0, identifierRole).toString();
            try {
                if (auto p = processorTreeWidget_->createProcessor(id)) {
                    auto drag = new ProcessorDragObject(this, std::move(p));
                    drag->exec(Qt::MoveAction);
                }
            } catch (const std::exception& e) {
                LogError("Error trying to create processor: " << utilqt::fromQString(id)
                                                              << " Message:\n"
                                                              << e.what());
            }
        }
    }
}

void ProcessorTree::mouseReleaseEvent(QMouseEvent* e) {
    dragStartPosition_.reset();
    QTreeWidget::mouseReleaseEvent(e);
}

ProcessorTree::ProcessorTree(ProcessorTreeWidget* parent)
    : QTreeWidget(parent), processorTreeWidget_{parent} {
    setContextMenuPolicy(Qt::CustomContextMenu);

    QObject::connect(this, &QTreeWidget::customContextMenuRequested, this,
                     &ProcessorTree::showContextMenu);
}

void ProcessorTree::showContextMenu(const QPoint& p) {
    auto item = itemAt(p);
    const auto id =
        (item == nullptr) ? QString() : item->data(0, ProcessorTree::identifierRole).toString();
    const bool enableExpandCollapse =
        ((processorTreeWidget_->getGrouping() != ProcessorTreeWidget::Grouping::LastUsed) &&
         (processorTreeWidget_->getGrouping() != ProcessorTreeWidget::Grouping::MostUsed));

    QMenu menu(this);
    auto addItem = menu.addAction("&Add Processor");
    addItem->setEnabled(!id.isEmpty());
    QObject::connect(addItem, &QAction::triggered, this,
                     [&, id]() { processorTreeWidget_->addProcessor(id); });

    menu.addSeparator();

    auto expand = menu.addAction("&Expand All Categories");
    expand->setEnabled(enableExpandCollapse);
    QObject::connect(expand, &QAction::triggered, this, [&]() { expandAll(); });

    auto collapseExceptItem = menu.addAction("C&ollapse Others");
    collapseExceptItem->setEnabled((item != nullptr) && enableExpandCollapse);
    // figure out top-level tree item, i.e. category
    while (item && item->parent()) {
        item = item->parent();
    }
    QObject::connect(collapseExceptItem, &QAction::triggered, this, [&, item]() {
        collapseAll();
        expandItem(item);
    });

    auto collapse = menu.addAction("&Collapse All Categories");
    collapse->setEnabled(enableExpandCollapse);
    QObject::connect(collapse, &QAction::triggered, this, [&]() { collapseAll(); });

    menu.exec(mapToGlobal(p));
}

auto strMatch = [](std::string_view cont, std::string_view s) {
    auto icomp = [](std::string::value_type l1, std::string::value_type r1) {
        return std::tolower(l1) == std::tolower(r1);
    };
    return std::search(cont.begin(), cont.end(), s.begin(), s.end(), icomp) != cont.end();
};

auto matcher = [](auto mptr) {
    return [mptr](std::string_view str, const ProcessorFactoryObject& p,
                  const InviwoModule&) -> bool { return strMatch(std::invoke(mptr, p), str); };
};

ProcessorTreeWidget::ProcessorTreeWidget(InviwoMainWindow* parent, HelpWidget* helpWidget)
    : InviwoDockWidget(tr("Processors"), parent, "ProcessorTreeWidget")
    , app_{parent->getInviwoApplication()}
    , dsl_{{{"identifier", "i", "processor class identifier", true,
             matcher(&ProcessorFactoryObject::getClassIdentifier)},
            {"name", "n", "processor displayname", true,
             matcher(&ProcessorFactoryObject::getDisplayName)},
            {"category", "c", "processor category", true,
             matcher(&ProcessorFactoryObject::getCategory)},
            {"tags", "#", "processor tags", true,
             [](std::string_view str, const ProcessorFactoryObject& p, const InviwoModule&) {
                 for (const auto& tag : p.getTags().tags_) {
                     if (strMatch(tag.getString(), str)) return true;
                 }
                 return false;
             }},
            {"state", "", "processor category", false,
             [](std::string_view str, const ProcessorFactoryObject& p, const InviwoModule&) {
                 return strMatch(toString(p.getCodeState()), str);
             }},
            {"module", "m", "processor module", false,
             [](std::string_view str, const ProcessorFactoryObject&, const InviwoModule& m) {
                 return strMatch(m.getIdentifier(), str);
             }}}}
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

        using P = Document::PathComponent;
        auto doc = dsl_.description();
        auto b = doc.get({P{"html"}, P{"body"}});
        auto desc = b.insert(P::first(), "div");
        desc.append("b", "Search Processors", {{"style", "color:white;"}});
        desc.append("p", "Example: name:raycaster state:stable");
        lineEdit_->setToolTip(utilqt::toQString(doc));

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
            addProcessorsToTree();
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
    listView_->setCurrentIndex(1);
    connect(listView_, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            [this]() {
                addProcessorsToTree();

                QSettings settings;
                settings.beginGroup(objectName());
                settings.setValue("currentView", QVariant(listView_->currentIndex()));
                settings.endGroup();
            });
    listView_->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    listViewLayout->addWidget(listView_);
    vLayout->addLayout(listViewLayout);

    iconStable_ = QIcon(":/svgicons/processor-stable.svg");
    iconExperimental_ = QIcon(":/svgicons/processor-experimental.svg");
    iconBroken_ = QIcon(":/svgicons/processor-broken.svg");
    iconDeprecated_ = QIcon(":/svgicons/processor-deprecated.svg");

    processorTree_ = new ProcessorTree(this);
    processorTree_->setHeaderHidden(true);
    processorTree_->setColumnCount(2);
    processorTree_->setIndentation(utilqt::emToPx(this, 1.0));
    processorTree_->setAnimated(true);
    processorTree_->header()->setStretchLastSection(false);
    processorTree_->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    processorTree_->header()->setSectionResizeMode(1, QHeaderView::Fixed);
    processorTree_->header()->setDefaultSectionSize(utilqt::emToPx(this, 4.0));

    lineEdit_->installEventFilter(new LineEditEventFilter(processorTree_, lineEdit_, false));

    connect(processorTree_, &ProcessorTree::currentItemChanged, this,
            &ProcessorTreeWidget::currentItemChanged);

    vLayout->addWidget(processorTree_);
    centralWidget->setLayout(vLayout);
    setWidget(centralWidget);

    onModulesDidRegister_ = app_->getModuleManager().onModulesDidRegister([this]() {
        addProcessorsToTree();
        app_->getProcessorFactory()->addObserver(this);
    });
    onModulesWillUnregister_ = app_->getModuleManager().onModulesWillUnregister([this]() {
        processorTree_->clear();
        app_->getProcessorFactory()->removeObserver(this);
    });

    QSettings settings;
    settings.beginGroup(objectName());
    lineEdit_->setText(settings.value("filterText", "").toString());
    listView_->setCurrentIndex(settings.value("currentView", 1).toInt());

    {
        auto useCounts = settings.value("useCounts", QVariant(QMap<QString, QVariant>{})).toMap();
        for (auto it = useCounts.constBegin(); it != useCounts.constEnd(); ++it) {
            useCounts_[utilqt::fromQString(it.key())] = it.value().toLongLong();
        }
    }
    {
        auto useTimes = settings.value("useTimes", QVariant(QMap<QString, QVariant>{})).toMap();
        for (auto it = useTimes.constBegin(); it != useTimes.constEnd(); ++it) {
            useTimes_[utilqt::fromQString(it.key())] = std::time_t(it.value().toLongLong());
        }
    }

    settings.endGroup();

    addProcessorsToTree();
}

ProcessorTreeWidget::~ProcessorTreeWidget() = default;

void ProcessorTreeWidget::focusSearch() {
    raise();
    lineEdit_->setFocus();
    lineEdit_->selectAll();
}

void ProcessorTreeWidget::addSelectedProcessor() {
    QString id;
    auto items = processorTree_->selectedItems();
    if (items.size() > 0) {
        id = items[0]->data(0, ProcessorTree::identifierRole).toString();
    } else {
        auto count = processorTree_->topLevelItemCount();
        if (count == 1) {
            auto item = processorTree_->topLevelItem(0);
            if (item->childCount() == 0) {
                id = item->data(0, ProcessorTree::identifierRole).toString();
            } else if (item->childCount() == 1) {
                id = item->child(0)->data(0, ProcessorTree::identifierRole).toString();
            }
        }
    }
    if (!id.isEmpty()) {
        addProcessor(id);
        processorTree_->clearSelection();
    } else {
        processorTree_->setFocus();
        if (processorTree_->topLevelItem(0)->childCount() == 0) {
            processorTree_->topLevelItem(0)->setSelected(true);
        } else {
            processorTree_->topLevelItem(0)->child(0)->setSelected(true);
        }
    }
}

std::shared_ptr<Processor> ProcessorTreeWidget::createProcessor(QString cid) {
    // Make sure the default render context is active to make sure any FBOs etc created in
    // the processor belong to the default context.
    RenderContext::getPtr()->activateDefaultRenderContext();
    const auto className = utilqt::fromQString(cid);
    try {
        if (auto p = app_->getProcessorFactory()->create(className)) {
            recordProcessorUse(className);
            return p;
        }
    } catch (Exception& exception) {
        util::log(
            exception.getContext(),
            "Unable to create processor \"" + className + "\" due to:\n" + exception.getMessage(),
            LogLevel::Error);
    }
    return nullptr;
}

auto ProcessorTreeWidget::getGrouping() const -> Grouping {
    return listView_->currentData().value<Grouping>();
}

void ProcessorTreeWidget::addProcessor(QString className) {
    // create processor, add it to processor network, and generate it's widgets
    auto network = app_->getProcessorNetwork();
    if (auto processor = createProcessor(className)) {
        auto meta = processor->getMetaData<ProcessorMetaData>(ProcessorMetaData::CLASS_IDENTIFIER);
        const auto bb = util::getBoundingBox(network->getProcessors());
        meta->setPosition(ivec2{bb.first.x, bb.second.y} + ivec2(0, 75));

        auto p = network->addProcessor(std::move(processor));
        AutoLinker::addLinks(network, p);
    }
}

const QIcon* ProcessorTreeWidget::getCodeStateIcon(CodeState state) const {
    switch (state) {
        case CodeState::Stable:
            return &iconStable_;

        case CodeState::Broken:
            return &iconBroken_;

        case CodeState::Deprecated:
            return &iconDeprecated_;

        case CodeState::Experimental:
        default:
            return &iconExperimental_;
    }
}
QTreeWidgetItem* ProcessorTreeWidget::addToplevelItemTo(QString title, const std::string& desc) {
    QTreeWidgetItem* newItem = new ProcessorTreeItem(QStringList(title));
    QList<QVariant> sortVal;
    sortVal.append(title);
    newItem->setData(0, ProcessorTree::sortRole, sortVal);
    newItem->setData(0, ProcessorTree::viewRole, listView_->currentIndex());
    newItem->setData(0, ProcessorTree::typeRole, ProcessorTree::GroupType);
    if (!desc.empty()) {
        newItem->setToolTip(0, utilqt::toLocalQString(desc));
    }
    processorTree_->addTopLevelItem(newItem);
    newItem->setFirstColumnSpanned(true);

    return newItem;
}

void ProcessorTreeWidget::onRegister(ProcessorFactoryObject*) { addProcessorsToTree(); }

void ProcessorTreeWidget::onUnRegister(ProcessorFactoryObject*) { addProcessorsToTree(); }

void ProcessorTreeWidget::closeEvent(QCloseEvent* event) {
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

void ProcessorTreeWidget::recordProcessorUse(const std::string& id) {
    ++useCounts_[id];
    useTimes_[id] = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
}

void ProcessorTreeWidget::addProcessorsToTree() {
    processorTree_->clear();
    // add processors from all modules to the list

    if (listView_->currentData().value<Grouping>() == Grouping::CodeState) {
        addToplevelItemTo("Stable Processors", "");
        addToplevelItemTo("Experimental Processors", "");
        addToplevelItemTo("Broken Processors", "");
    }

    dsl_.setSearchString(utilqt::fromQString(lineEdit_->text()));

    for (auto& elem : app_->getModuleManager().getInviwoModules()) {
        for (auto& processor : elem.getProcessors()) {
            if (processor->isVisible() &&
                (lineEdit_->text().isEmpty() || dsl_.match(*processor, elem))) {
                extractInfoAndAddProcessor(processor, &elem);
            }
        }
    }

    // Apply sorting
    switch (listView_->currentData().value<Grouping>()) {
        case Grouping::CodeState: {  // By Code State
            int i = 0;
            while (i < processorTree_->topLevelItemCount()) {
                auto widget = processorTree_->topLevelItem(i);
                if (widget->childCount() == 0) {
                    delete processorTree_->takeTopLevelItem(i);
                } else {
                    widget->sortChildren(0, Qt::AscendingOrder);
                    i++;
                }
            }
            break;
        }
        default:
            processorTree_->sortItems(0, Qt::AscendingOrder);
            break;
    }

    processorTree_->expandAll();
    processorTree_->resizeColumnToContents(1);
}

void ProcessorTreeWidget::extractInfoAndAddProcessor(ProcessorFactoryObject* processor,
                                                     InviwoModule* elem) {
    std::string categoryName;
    std::string categoryDesc;
    QList<QVariant> sortVal;
    sortVal.append(utilqt::toQString(processor->getDisplayName()));

    switch (listView_->currentData().value<Grouping>()) {
        case Grouping::Alphabetical:
            categoryName = processor->getDisplayName().substr(0, 1);
            categoryDesc = "";
            break;
        case Grouping::Categorical:
            categoryName = processor->getCategory();
            categoryDesc = "";
            break;
        case Grouping::CodeState:
            categoryName = toString(processor->getCodeState());
            categoryDesc = "";
            break;
        case Grouping::Module:
            categoryName = elem ? elem->getIdentifier() : "Unkonwn";
            categoryDesc = elem ? elem->getDescription() : "";
            break;
        case Grouping::LastUsed: {
            auto it = useTimes_.find(processor->getClassIdentifier());
            if (it != useTimes_.end()) {
                sortVal.prepend(QVariant::fromValue<qint64>(-it->second));
            } else {
                sortVal.prepend(0);
            }
            categoryName = "";
            categoryDesc = "";
            break;
        }
        case Grouping::MostUsed: {
            auto it = useCounts_.find(processor->getClassIdentifier());
            if (it != useCounts_.end()) {
                sortVal.prepend(QVariant::fromValue<qint64>(-static_cast<qint64>(it->second)));
            } else {
                sortVal.prepend(0);
            }
            categoryName = "";
            categoryDesc = "";
            break;
        }
        default:
            categoryName = "Unkonwn";
            categoryDesc = "";
    }

    QTreeWidgetItem* item = nullptr;
    if (!categoryName.empty()) {
        QString category = utilqt::toQString(categoryName);
        auto items = processorTree_->findItems(category, Qt::MatchFixedString, 0);

        if (items.empty()) items.push_back(addToplevelItemTo(category, categoryDesc));
        item = items[0];
    }
    std::string moduleId = elem ? elem->getIdentifier() : "Unknown";

    auto newItem = new ProcessorTreeItem();
    newItem->setIcon(0, *getCodeStateIcon(processor->getCodeState()));
    newItem->setText(0, utilqt::toQString(processor->getDisplayName()));

    newItem->setTextAlignment(1, Qt::AlignRight);
    newItem->setData(0, ProcessorTree::identifierRole,
                     utilqt::toQString(processor->getClassIdentifier()));
    newItem->setData(0, ProcessorTree::sortRole, sortVal);
    newItem->setData(0, ProcessorTree::viewRole, listView_->currentIndex());
    newItem->setData(0, ProcessorTree::typeRole, ProcessorTree::ProcessoorType);

    // add only platform tags to second column
    auto platformTags = util::getPlatformTags(processor->getTags());
    const bool hasTags = !platformTags.empty();

    if (hasTags) {
        newItem->setText(1, utilqt::toQString(platformTags.getString() + " "));

        QFont font = newItem->font(1);
        font.setWeight(QFont::Bold);
        newItem->setFont(1, font);
    }

    {
        Document doc;
        using P = Document::PathComponent;
        auto b = doc.append("html").append("body");
        b.append("b", processor->getDisplayName(), {{"style", "color:white;"}});
        using H = utildoc::TableBuilder::Header;
        utildoc::TableBuilder tb(b, P::end(), {{"class", "propertyInfo"}});

        tb(H("Module"), moduleId);
        tb(H("Identifier"), processor->getClassIdentifier());
        tb(H("Category"), processor->getCategory());
        tb(H("Code"), processor->getCodeState());
        tb(H("Tags"), processor->getTags().getString());

        {
            auto it = useCounts_.find(processor->getClassIdentifier());
            if (it != useCounts_.end()) {
                tb(H("Uses"), it->second);
            } else {
                tb(H("Uses"), 0);
            }
        }

        {
            auto it = useTimes_.find(processor->getClassIdentifier());
            if (it != useTimes_.end()) {
                tb(H("Last"), std::string(std::ctime(&(it->second))));
            } else {
                tb(H("Last"), "Never");
            }
        }

        b.append(processor->getMetaInformation());

        newItem->setToolTip(0, utilqt::toQString(doc));
        if (hasTags) {
            newItem->setToolTip(1, utilqt::toQString(doc));
        }
    }
    if (item) {
        item->addChild(newItem);
    } else {
        processorTree_->addTopLevelItem(newItem);
    }
    if (!hasTags) {
        newItem->setFirstColumnSpanned(true);
    }
}

void ProcessorTreeWidget::currentItemChanged(QTreeWidgetItem* current,
                                             QTreeWidgetItem* /*previous*/) {
    if (!current) return;
    auto classname =
        utilqt::fromQString(current->data(0, ProcessorTree::identifierRole).toString());
    if (!classname.empty()) {
        helpWidget_->raise();
        helpWidget_->showDocForClassName(classname);
    }
}

static QString mimeType = "inviwo/ProcessorDragObject";

ProcessorDragObject::ProcessorDragObject(QWidget* source, std::shared_ptr<Processor> processor)
    : QDrag(source) {
    auto img = QPixmap::fromImage(utilqt::generateProcessorPreview(*processor, 1.0));
    setPixmap(img);
    auto mime = new ProcessorMimeData(std::move(processor));
    setMimeData(mime);
    setHotSpot(QPoint(img.width() / 2, img.height() / 2));
}

bool ProcessorTreeItem::operator<(const QTreeWidgetItem& other) const {
    auto i = data(0, ProcessorTree::viewRole).toInt();
    auto a = data(0, ProcessorTree::sortRole);
    auto b = other.data(0, ProcessorTree::sortRole);

    switch (i) {
        case 0:  // By Alphabet
        case 1:  // By Category
        case 2:  // By Code State
        case 3:  // By Module
            return QString::compare(a.toList().front().toString(), b.toList().front().toString(),
                                    Qt::CaseInsensitive) < 0;

        case 4:  // Last used
        case 5:  // Most Used
            if (a.toList().front().toLongLong() == b.toList().front().toLongLong()) {
                return QString::compare(a.toList()[1].toString(), b.toList()[1].toString(),
                                        Qt::CaseInsensitive) < 0;
            } else {
                return a.toList().front().toLongLong() < b.toList().front().toLongLong();
            }
        default:
            return QString::compare(a.toList().front().toString(), b.toList().front().toString(),
                                    Qt::CaseInsensitive) < 0;
    }
}

}  // namespace inviwo
