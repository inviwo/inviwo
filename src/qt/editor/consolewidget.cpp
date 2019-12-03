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

#include <warn/push>
#include <warn/ignore/all>
#include <QMenu>
#include <QTextCursor>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableView>
#include <QKeyEvent>
#include <QLabel>
#include <QThread>
#include <QCoreApplication>
#include <QFontDatabase>
#include <QHeaderView>
#include <QStandardItem>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QSortFilterProxyModel>
#include <QLineEdit>
#include <QToolButton>
#include <QPixmap>
#include <QSettings>
#include <QScrollBar>
#include <QResizeEvent>
#include <QWheelEvent>
#include <QMessageBox>
#include <QTimer>
#include <warn/pop>

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/qt/editor/consolewidget.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/qt/editor/inviwomainwindow.h>
#include <inviwo/core/util/ostreamjoiner.h>
#include <inviwo/qt/editor/inviwoeditmenu.h>

namespace inviwo {

TextSelectionDelegate::TextSelectionDelegate(QWidget* parent) : QItemDelegate(parent) {}

QWidget* TextSelectionDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                                             const QModelIndex& index) const {
    if (index.column() == static_cast<int>(LogTableModelEntry::ColumnID::Message)) {
        auto value = index.model()->data(index, Qt::EditRole).toString();
        auto widget = new QLineEdit(parent);
        widget->setReadOnly(true);
        widget->setText(value);
        return widget;
    } else {
        return QItemDelegate::createEditor(parent, option, index);
    }
}

void TextSelectionDelegate::setModelData(QWidget* editor, QAbstractItemModel* model,
                                         const QModelIndex& index) const {
    IVW_UNUSED_PARAM(editor);
    IVW_UNUSED_PARAM(model);
    IVW_UNUSED_PARAM(index);
    // dummy function to prevent changing the model
}

ConsoleWidget::ConsoleWidget(InviwoMainWindow* parent)
    : InviwoDockWidget(tr("Console"), parent, "ConsoleWidget")
    , tableView_(new QTableView(this))
    , model_()
    , filter_(new QSortFilterProxyModel(this))
    , levelFilter_(new QSortFilterProxyModel(this))
    , textSelectionDelegate_(new TextSelectionDelegate(this))
    , filterPattern_(new QLineEdit(this))
    , mainwindow_(parent) {

    setAllowedAreas(Qt::BottomDockWidgetArea);
    resize(utilqt::emToPx(this, QSizeF(60, 60)));  // default size

    qRegisterMetaType<LogTableModelEntry>("LogTableModelEntry");

    filter_->setSourceModel(model_.model());
    filter_->setFilterKeyColumn(static_cast<int>(LogTableModelEntry::ColumnID::Message));

    levelFilter_->setSourceModel(filter_);
    levelFilter_->setFilterKeyColumn(static_cast<int>(LogTableModelEntry::ColumnID::Level));

    filterPattern_->setClearButtonEnabled(true);

    tableView_->setModel(levelFilter_);
    tableView_->setGridStyle(Qt::NoPen);
    tableView_->setCornerButtonEnabled(false);

    tableView_->setContextMenuPolicy(Qt::ActionsContextMenu);
    clearAction_ = new QAction(QIcon(":/svgicons/log-clear.svg"), tr("&Clear Log"), this);
    clearAction_->setShortcut(Qt::ControlModifier + Qt::Key_E);
    connect(clearAction_, &QAction::triggered, [&]() { clear(); });

    tableView_->hideColumn(static_cast<int>(LogTableModelEntry::ColumnID::Date));
    tableView_->hideColumn(static_cast<int>(LogTableModelEntry::ColumnID::Level));
    tableView_->hideColumn(static_cast<int>(LogTableModelEntry::ColumnID::Audience));
    tableView_->hideColumn(static_cast<int>(LogTableModelEntry::ColumnID::Path));
    tableView_->hideColumn(static_cast<int>(LogTableModelEntry::ColumnID::File));
    tableView_->hideColumn(static_cast<int>(LogTableModelEntry::ColumnID::Line));
    tableView_->hideColumn(static_cast<int>(LogTableModelEntry::ColumnID::Function));

    tableView_->horizontalHeader()->setContextMenuPolicy(Qt::ActionsContextMenu);
    tableView_->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    const auto cols = tableView_->horizontalHeader()->count();

    auto viewColGroup = new QMenu(this);
    for (int i = 0; i < cols; ++i) {
        auto viewCol =
            new QAction(model_.getName(static_cast<LogTableModelEntry::ColumnID>(i)), this);
        viewCol->setCheckable(true);
        viewCol->setChecked(!tableView_->isColumnHidden(i));
        connect(viewCol, &QAction::triggered, [this, i](bool state) {
            if (!state) {
                tableView_->hideColumn(i);
            } else {
                tableView_->showColumn(i);
            }
        });
        tableView_->horizontalHeader()->addAction(viewCol);
        viewColGroup->addAction(viewCol);
    }
    auto visibleColumnsAction = new QAction("Visible Columns", this);
    visibleColumnsAction->setMenu(viewColGroup);

    tableView_->horizontalHeader()->setResizeContentsPrecision(0);
    tableView_->horizontalHeader()->setSectionResizeMode(cols - 1, QHeaderView::Stretch);
    tableView_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    tableView_->verticalHeader()->setVisible(false);
    tableView_->verticalHeader()->setResizeContentsPrecision(0);
    tableView_->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    const auto height = QFontMetrics(QFontDatabase::systemFont(QFontDatabase::FixedFont)).height();
    constexpr int margin = 2;
    tableView_->verticalHeader()->setMinimumSectionSize(height + margin);
    tableView_->verticalHeader()->setDefaultSectionSize(height + margin);

    QHBoxLayout* statusBar = new QHBoxLayout();
    statusBar->setObjectName("StatusBar");

    auto makeIcon = [](const QString& file, bool checkable = false) {
        auto icon = QIcon();
        if (checkable) {
            icon.addPixmap(QPixmap(":/svgicons/" + file + "-enabled.svg"), QIcon::Normal,
                           QIcon::On);
            icon.addPixmap(QPixmap(":/svgicons/" + file + "-disabled.svg"), QIcon::Normal,
                           QIcon::Off);
        } else {
            icon.addPixmap(QPixmap(":/svgicons/" + file + ".svg"));
        }
        return icon;
    };

    auto makeToolButton = [this, statusBar, makeIcon](const QString& label, const QString& file,
                                                      bool checkable = true) {
        auto button = new QToolButton(this);
        auto action = new QAction(makeIcon(file, checkable), label, this);
        action->setCheckable(checkable);
        if (checkable) action->setChecked(true);

        button->setDefaultAction(action);
        statusBar->addWidget(button);
        return action;
    };

    auto updateRowsHeights = [=]() {
        tableView_->setUpdatesEnabled(false);

        auto vrows = tableView_->verticalHeader()->count();
        for (int i = 0; i < vrows; ++i) {
            auto mind = mapToSource(i, static_cast<int>(LogTableModelEntry::ColumnID::Message));
            const auto message = mind.data(Qt::DisplayRole).toString();
            const auto lines = std::count(message.begin(), message.end(), '\n') + 1;
            tableView_->verticalHeader()->resizeSection(i,
                                                        margin + static_cast<int>(lines) * height);
        }
        tableView_->setUpdatesEnabled(true);
    };

    auto levelCallback = [this, updateRowsHeights](bool /*checked*/) {
        if (util::all_of(levels, [](const auto& level) { return level.action->isChecked(); })) {
            levelFilter_->setFilterRegExp("");
        } else {
            std::stringstream ss;
            auto joiner = util::make_ostream_joiner(ss, "|");
            joiner = "None";
            for (const auto& level : levels) {
                if (level.action->isChecked()) joiner = level.level;
            }
            levelFilter_->setFilterRegExp(QString::fromStdString(ss.str()));
        }
        updateRowsHeights();
    };

    auto levelGroup = new QMenu(this);
    for (auto& level : levels) {
        level.action =
            makeToolButton(QString::fromStdString(level.name), QString::fromStdString(level.icon));
        level.label = new QLabel("0", this);
        statusBar->addWidget(level.label);
        statusBar->addSpacing(5);
        levelGroup->addAction(level.action);
        connect(level.action, &QAction::toggled, levelCallback);
    }
    auto viewAction = new QAction("Log Level", this);
    viewAction->setMenu(levelGroup);

    auto clearButton = new QToolButton(this);
    clearButton->setDefaultAction(clearAction_);
    statusBar->addWidget(clearButton);
    statusBar->addSpacing(5);

    statusBar->addStretch(3);

    threadPoolInfo_ = new QLabel("Pool: 0 Queued Jobs / 0 Threads", this);
    statusBar->addWidget(threadPoolInfo_);
    auto timer = new QTimer(this);
    connect(timer, &QTimer::timeout, threadPoolInfo_, [this]() {
        const auto threads = mainwindow_->getInviwoApplication()->getThreadPool().getSize();
        const auto queueSize = mainwindow_->getInviwoApplication()->getThreadPool().getQueueSize();
        threadPoolInfo_->setText(
            QString("Pool: %1 Queued Jobs / %2 Threads").arg(queueSize, 3).arg(threads, 2));
    });
    timer->start(1000);

    statusBar->addSpacing(20);
    statusBar->addWidget(new QLabel("Filter", this));
    filterPattern_->setMinimumWidth(200);
    statusBar->addWidget(filterPattern_, 1);
    statusBar->addSpacing(5);

    auto clearFilter = new QAction(makeIcon("find-clear"), "C&lear Filter", this);
    clearFilter->setEnabled(false);

    connect(filterPattern_, &QLineEdit::textChanged,
            [this, updateRowsHeights, clearFilter](const QString& text) {
                filter_->setFilterRegExp(text);
                updateRowsHeights();
                clearFilter->setEnabled(!text.isEmpty());
            });

    connect(clearFilter, &QAction::triggered, [this]() { filterPattern_->setText(""); });

    auto filterAction = new QAction(makeIcon("find"), "&Filter", this);
    filterAction->setShortcut(Qt::ControlModifier + Qt::AltModifier + Qt::Key_F);
    connect(filterAction, &QAction::triggered, [this]() {
        raise();
        filterPattern_->setFocus();
        filterPattern_->selectAll();
    });

    // add actions for context menu
    auto createSeparator = [this]() {
        auto separator = new QAction(this);
        separator->setSeparator(true);
        return separator;
    };

    auto copyAction = new QAction(QIcon(":/svgicons/edit-copy.svg"), tr("&Copy"), this);
    copyAction->setEnabled(true);
    connect(copyAction, &QAction::triggered, this, &ConsoleWidget::copy);

    tableView_->addAction(copyAction);
    tableView_->addAction(createSeparator());
    tableView_->addAction(visibleColumnsAction);
    tableView_->addAction(viewAction);
    tableView_->addAction(createSeparator());
    tableView_->addAction(clearAction_);
    tableView_->addAction(createSeparator());
    tableView_->addAction(filterAction);
    tableView_->addAction(clearFilter);

    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(tableView_);
    layout->addLayout(statusBar);

    const auto space = utilqt::emToPx(this, 3 / 9.0);
    layout->setContentsMargins(space, 0, 0, space);

    QWidget* w = new QWidget();
    w->setLayout(layout);
    setWidget(w);

    tableView_->setAttribute(Qt::WA_Hover);
    tableView_->setItemDelegateForColumn(static_cast<int>(LogTableModelEntry::ColumnID::Message),
                                         textSelectionDelegate_);

    connect(this, &ConsoleWidget::logSignal, this, &ConsoleWidget::logEntry);
    connect(this, &ConsoleWidget::clearSignal, this, &ConsoleWidget::clear);

    // Restore State
    QSettings settings;
    settings.beginGroup(objectName());

    {
        auto colVisible = settings.value("columnsVisible", QVariantList()).toList();
        auto colWidths = settings.value("columnsWidth", QVariantList()).toList();
        auto count = std::min(colVisible.size(), colWidths.size());

        for (int i = 0; i < count; ++i) {
            const bool visible = colVisible[i].toBool();
            viewColGroup->actions()[i]->setChecked(visible);
            tableView_->horizontalHeader()->setSectionHidden(i, !visible);
            if (visible) tableView_->horizontalHeader()->resizeSection(i, colWidths[i].toInt());
        }
    }

    {
        auto levelsActive = settings.value("levelsActive", QVariantList());
        int i = 0;
        for (const auto& level : levelsActive.toList()) {
            levels[i++].action->setChecked(level.toBool());
        }
    }

    auto filterText = settings.value("filterText", "");
    filterPattern_->setText(filterText.toString());

    settings.endGroup();

    auto editmenu = mainwindow_->getInviwoEditMenu();
    editActionsHandle_ = editmenu->registerItem(
        std::make_shared<MenuItem>(this,
                                   [this](MenuItemType t) -> bool {
                                       switch (t) {
                                           case MenuItemType::copy:
                                               return tableView_->selectionModel()->hasSelection();
                                           case MenuItemType::cut:
                                           case MenuItemType::paste:
                                           case MenuItemType::del:
                                           case MenuItemType::select:
                                           default:
                                               return false;
                                       }
                                   },
                                   [this](MenuItemType t) -> void {
                                       switch (t) {
                                           case MenuItemType::copy: {
                                               if (tableView_->selectionModel()->hasSelection()) {
                                                   copy();
                                               }
                                               break;
                                           }
                                           case MenuItemType::cut:
                                           case MenuItemType::paste:
                                           case MenuItemType::del:
                                           case MenuItemType::select:
                                           default:
                                               break;
                                       }
                                   }));
}

ConsoleWidget::~ConsoleWidget() = default;

QAction* ConsoleWidget::getClearAction() { return clearAction_; }

void ConsoleWidget::clear() {
    if (QThread::currentThread() != QCoreApplication::instance()->thread()) {
        emit clearSignal();
        return;
    }

    model_.clear();
    for (auto& level : levels) {
        level.label->setText("0");
        level.count = 0;
    }
}

void ConsoleWidget::updateIndicators(LogLevel level) {
    auto it = util::find_if(levels, [&](const auto& l) { return l.level == level; });
    if (it != levels.end()) {
        it->label->setText(toString(++(it->count)).c_str());
    }
}

void ConsoleWidget::log(std::string source, LogLevel level, LogAudience audience, const char* file,
                        const char* function, int line, std::string msg) {
    LogTableModelEntry e = {
        std::chrono::system_clock::now(), source, level, audience, file ? file : "", line,
        function ? function : "",         msg};
    logEntry(std::move(e));
}

void ConsoleWidget::logProcessor(Processor* processor, LogLevel level, LogAudience audience,
                                 std::string msg, const char* file, const char* function,
                                 int line) {
    LogTableModelEntry e = {std::chrono::system_clock::now(),
                            processor->getIdentifier(),
                            level,
                            audience,
                            file ? file : "",
                            line,
                            function ? function : "",
                            msg};
    logEntry(std::move(e));
}

void ConsoleWidget::logNetwork(LogLevel level, LogAudience audience, std::string msg,
                               const char* file, const char* function, int line) {
    LogTableModelEntry e = {std::chrono::system_clock::now(),
                            "ProcessorNetwork",
                            level,
                            audience,
                            file ? file : "",
                            line,
                            function ? function : "",
                            msg};
    logEntry(std::move(e));
}

void ConsoleWidget::logAssertion(const char* file, const char* function, int line,
                                 std::string msg) {
    LogTableModelEntry e = {std::chrono::system_clock::now(),
                            "Assertion",
                            LogLevel::Error,
                            LogAudience::Developer,
                            file ? file : "",
                            line,
                            function ? function : "",
                            msg};
    logEntry(std::move(e));

    auto error = QString{"<b>Assertion Failed</b><br>File: %1:%2<br>Function: %3<p>%4"}
                     .arg(file)
                     .arg(line)
                     .arg(function)
                     .arg(utilqt::toQString(msg));
    QMessageBox::critical(nullptr, "Assertion Failed", error);
}

void ConsoleWidget::logEntry(LogTableModelEntry e) {
    if (QThread::currentThread() != QCoreApplication::instance()->thread()) {
        emit logSignal(e);
        return;
    }

    auto lines = std::count(e.message.begin(), e.message.end(), '\n') + 1;
    auto height = QFontMetrics(QFontDatabase::systemFont(QFontDatabase::FixedFont)).height();

    tableView_->setUpdatesEnabled(false);

    model_.log(e);
    updateIndicators(e.level);

    // Faster but messes with filters.
    if (lines != 1) {
        auto vind = mapFromSource(model_.model()->rowCount() - 1, 0);
        if (vind.isValid()) {
            tableView_->verticalHeader()->resizeSection(vind.row(),
                                                        2 + static_cast<int>(lines) * height);
        }
    }

    tableView_->scrollToBottom();
    tableView_->setUpdatesEnabled(true);
}

void ConsoleWidget::keyPressEvent(QKeyEvent* keyEvent) {
    if (keyEvent->key() == Qt::Key_E && keyEvent->modifiers() == Qt::ControlModifier) {
        clear();
    }
}

QModelIndex ConsoleWidget::mapToSource(int row, int col) {
    auto ind = levelFilter_->index(row, col);
    auto lind = levelFilter_->mapToSource(ind);
    return filter_->mapToSource(lind);
}

QModelIndex ConsoleWidget::mapFromSource(int row, int col) {
    auto mind = model_.model()->index(row, col);
    auto lind = filter_->mapFromSource(mind);
    return levelFilter_->mapFromSource(lind);
}

void ConsoleWidget::copy() {
    const auto& inds = tableView_->selectionModel()->selectedIndexes();
    if (inds.isEmpty()) return;

    int prevrow = inds.first().row();
    bool first = true;
    QString text;
    for (const auto& ind : inds) {
        if (!tableView_->isColumnHidden(ind.column())) {
            if (!first && ind.row() == prevrow) {
                text.append('\t');
            } else if (!first) {
                text.append('\n');
            }
            text.append(ind.data(Qt::DisplayRole).toString());
            first = false;
        }
        prevrow = ind.row();
    }
    auto mimedata = std::make_unique<QMimeData>();
    mimedata->setData(QString("text/plain"), text.toUtf8());
    QApplication::clipboard()->setMimeData(mimedata.release());
}

void ConsoleWidget::closeEvent(QCloseEvent* event) {
    QSettings settings;
    settings.beginGroup(objectName());

    const auto cols = tableView_->horizontalHeader()->count();
    QList<QVariant> columnsVisible;
    QList<QVariant> columnsWidth;
    columnsVisible.reserve(cols);
    columnsWidth.reserve(cols);
    for (int i = 0; i < cols; ++i) {
        columnsVisible.append(!tableView_->horizontalHeader()->isSectionHidden(i));
        columnsWidth.append(tableView_->horizontalHeader()->sectionSize(i));
    }
    QList<QVariant> levelsActive;
    for (const auto& level : levels) {
        levelsActive.append(level.action->isChecked());
    }

    settings.setValue("columnsVisible", QVariant(columnsVisible));
    settings.setValue("columnsWidth", QVariant(columnsWidth));
    settings.setValue("levelsActive", QVariant(levelsActive));
    settings.setValue("filterText", QVariant(filterPattern_->text()));
    settings.endGroup();

    InviwoDockWidget::closeEvent(event);
}

LogTableModel::LogTableModel() : model_(0, static_cast<int>(LogTableModelEntry::size())) {
    for (size_t i = 0; i < LogTableModelEntry::size(); ++i) {
        auto item = new QStandardItem(getName(static_cast<LogTableModelEntry::ColumnID>(i)));
        item->setTextAlignment(Qt::AlignLeft);
        model_.setHorizontalHeaderItem(static_cast<int>(i), item);
    }
}

void LogTableModel::log(LogTableModelEntry entry) {
    QList<QStandardItem*> items;
    items.reserve(static_cast<int>(LogTableModelEntry::size()));
    for (size_t i = 0; i < LogTableModelEntry::size(); ++i) {
        items.append(entry.get(static_cast<LogTableModelEntry::ColumnID>(i)));
        items.last()->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
        items.last()->setTextAlignment(Qt::AlignLeft);
        items.last()->setEditable(false);
        // items.last()->setSizeHint(QSize(1, lines * height));

        switch (entry.level) {
            case LogLevel::Info:
                items.last()->setForeground(QBrush(infoTextColor_));
                break;
            case LogLevel::Warn:
                items.last()->setForeground(QBrush(warnTextColor_));
                break;
            case LogLevel::Error:
                items.last()->setForeground(QBrush(errorTextColor_));
                break;
            default:
                items.last()->setForeground(QBrush(infoTextColor_));
                break;
        }
    }

    model_.appendRow(items);
}

LogModel* LogTableModel::model() { return &model_; }

void LogTableModel::clear() { model_.removeRows(0, model_.rowCount()); }

QString LogTableModel::getName(LogTableModelEntry::ColumnID ind) const {
    switch (ind) {
        case LogTableModelEntry::ColumnID::Date:
            return QString("Date");
        case LogTableModelEntry::ColumnID::Time:
            return QString("Time");
        case LogTableModelEntry::ColumnID::Source:
            return QString("Source");
        case LogTableModelEntry::ColumnID::Level:
            return QString("Level");
        case LogTableModelEntry::ColumnID::Audience:
            return QString("Audience");
        case LogTableModelEntry::ColumnID::Path:
            return QString("Path");
        case LogTableModelEntry::ColumnID::File:
            return QString("File");
        case LogTableModelEntry::ColumnID::Line:
            return QString("Line");
        case LogTableModelEntry::ColumnID::Function:
            return QString("Function");
        case LogTableModelEntry::ColumnID::Message:
            return QString("Message");
        default:
            return QString();
    }
}

std::string LogTableModelEntry::getDate() const {
    auto in_time_t = std::chrono::system_clock::to_time_t(time);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%F");
    return ss.str();
}
std::string LogTableModelEntry::getTime() const {
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(time.time_since_epoch()) % 1000;

    auto in_time_t = std::chrono::system_clock::to_time_t(time);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%T");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

QStandardItem* LogTableModelEntry::get(ColumnID ind) const {
    switch (ind) {
        case ColumnID::Date:
            return new QStandardItem(utilqt::toQString(getDate()));
        case ColumnID::Time:
            return new QStandardItem(utilqt::toQString(getTime()));
        case ColumnID::Source:
            return new QStandardItem(utilqt::toQString(source));
        case ColumnID::Level:
            return new QStandardItem(utilqt::toQString(toString(level)));
        case ColumnID::Audience:
            return new QStandardItem(utilqt::toQString(toString(audience)));
        case ColumnID::Path:
            return new QStandardItem(utilqt::toQString(filesystem::getFileDirectory(fileName)));
        case ColumnID::File:
            return new QStandardItem(
                utilqt::toQString(filesystem::getFileNameWithExtension(fileName)));
        case ColumnID::Line:
            return new QStandardItem(utilqt::toQString(toString(lineNumber)));
        case ColumnID::Function:
            return new QStandardItem(utilqt::toQString(funcionName));
        case ColumnID::Message:
            return new QStandardItem(utilqt::toQString(message));
        default:
            return new QStandardItem();
    }
}

LogModel::LogModel(int rows, int columns, QObject* parent)
    : QStandardItemModel(rows, columns, parent) {}

Qt::ItemFlags LogModel::flags(const QModelIndex& index) const {
    auto flags = QStandardItemModel::flags(index);
    // make only the message column editable
    const auto col = static_cast<LogTableModelEntry::ColumnID>(index.column());
    if (col == LogTableModelEntry::ColumnID::Message) {
        flags |= Qt::ItemIsEditable;
    }
    return flags;
}

}  // namespace inviwo
