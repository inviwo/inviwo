/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2025 Inviwo Foundation
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
#include <QPlainTextEdit>
#include <warn/pop>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/qt/editor/consolewidget.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/qt/editor/inviwomainwindow.h>
#include <inviwo/core/util/ostreamjoiner.h>
#include <inviwo/qt/editor/inviwoeditmenu.h>

#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/network/processornetworkobserver.h>

namespace inviwo {

namespace detail {

enum Roles { Fulltext = Qt::UserRole + 1 };

}  // namespace detail

TextSelectionDelegate::TextSelectionDelegate(QWidget* parent) : QItemDelegate(parent) {}

QWidget* TextSelectionDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                                             const QModelIndex& index) const {
    if (index.column() == static_cast<int>(LogTableModelEntry::ColumnID::Message)) {
        auto value = index.model()->data(index, Qt::EditRole).toString();
        auto widget = new QPlainTextEdit(value, parent);
        widget->setReadOnly(true);
        return widget;
    } else {
        return QItemDelegate::createEditor(parent, option, index);
    }
}

void TextSelectionDelegate::setModelData([[maybe_unused]] QWidget* editor,
                                         [[maybe_unused]] QAbstractItemModel* model,
                                         [[maybe_unused]] const QModelIndex& index) const {
    // dummy function to prevent changing the model
}

struct BackgroundJobs : QLabel, ProcessorNetworkObserver {
    BackgroundJobs(QWidget* parent, ProcessorNetwork* net) : QLabel(parent) {
        net->addObserver(this);
        update(0);
    }

    void update(int jobs) { setText(QString("Background Jobs: %1").arg(jobs)); }

    virtual void onProcessorBackgroundJobsChanged(Processor*, int, int total) override {
        update(total);
    }
};

ConsoleWidget::ConsoleWidget(InviwoMainWindow* parent)
    : InviwoDockWidget(tr("Console"), parent, "ConsoleWidget")
    , tableView_(new QTableView(this))
    , model_()
    , filter_(new QSortFilterProxyModel(this))
    , levelFilter_(new QSortFilterProxyModel(this))
    , textSelectionDelegate_(new TextSelectionDelegate(this))
    , filterPattern_(new QLineEdit(this))
    , mainWindow_(parent)
    , editActionsHandle_{} {

    setAllowedAreas(Qt::BottomDockWidgetArea);
    resize(utilqt::emToPx(this, QSizeF(60, 60)));  // default size

    qRegisterMetaType<LogTableModelEntry>("LogTableModelEntry");

    filter_->setSourceModel(&model_);
    filter_->setFilterKeyColumn(static_cast<int>(LogTableModelEntry::ColumnID::Message));

    levelFilter_->setSourceModel(filter_);
    levelFilter_->setFilterKeyColumn(static_cast<int>(LogTableModelEntry::ColumnID::Level));

    filterPattern_->setClearButtonEnabled(true);

    tableView_->setModel(levelFilter_);
    tableView_->setGridStyle(Qt::NoPen);
    tableView_->setCornerButtonEnabled(false);

    tableView_->setContextMenuPolicy(Qt::ActionsContextMenu);
    clearAction_ = new QAction(QIcon(":/svgicons/log-clear.svg"), tr("&Clear Log"), this);
    clearAction_->setShortcut(Qt::CTRL | Qt::Key_E);
    connect(clearAction_, &QAction::triggered, [&]() { clear(); });

    tableView_->hideColumn(static_cast<int>(LogTableModelEntry::ColumnID::Date));
    tableView_->hideColumn(static_cast<int>(LogTableModelEntry::ColumnID::Level));
    tableView_->hideColumn(static_cast<int>(LogTableModelEntry::ColumnID::Audience));
    tableView_->hideColumn(static_cast<int>(LogTableModelEntry::ColumnID::Path));
    tableView_->hideColumn(static_cast<int>(LogTableModelEntry::ColumnID::File));
    tableView_->hideColumn(static_cast<int>(LogTableModelEntry::ColumnID::Line));
    tableView_->hideColumn(static_cast<int>(LogTableModelEntry::ColumnID::Function));

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

    tableView_->horizontalHeader()->setContextMenuPolicy(Qt::ActionsContextMenu);
    tableView_->horizontalHeader()->setResizeContentsPrecision(0);
    tableView_->horizontalHeader()->setSectionResizeMode(cols - 1, QHeaderView::Stretch);

    tableView_->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    tableView_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    tableView_->verticalHeader()->setVisible(false);
    tableView_->verticalHeader()->setResizeContentsPrecision(0);
    tableView_->verticalHeader()->setMinimumSectionSize(1);
    tableView_->verticalHeader()->setDefaultSectionSize(1);
    tableView_->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

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

    auto levelCallback = [this](bool /*checked*/) {
        if (util::all_of(levels, [](const auto& level) { return level.action->isChecked(); })) {
            levelFilter_->setFilterRegularExpression("");
        } else {
            std::stringstream ss;
            auto joiner = util::make_ostream_joiner(ss, "|");
            joiner = "None";
            for (const auto& level : levels) {
                if (level.action->isChecked()) joiner = level.level;
            }
            levelFilter_->setFilterRegularExpression(QString::fromStdString(ss.str()));
        }
        applyRowHeights(0, tableView_->verticalHeader()->count());
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
        const auto threads = mainWindow_->getInviwoApplication()->getThreadPool().getSize();
        const auto queueSize = mainWindow_->getInviwoApplication()->getThreadPool().getQueueSize();
        threadPoolInfo_->setText(
            QString("Pool: %1 Queued Jobs / %2 Threads").arg(queueSize, 3).arg(threads, 2));
    });
    timer->start(1000);

    statusBar->addWidget(
        new BackgroundJobs(this, mainWindow_->getInviwoApplication()->getProcessorNetwork()));

    statusBar->addSpacing(20);
    statusBar->addWidget(new QLabel("Filter", this));
    filterPattern_->setMinimumWidth(200);
    statusBar->addWidget(filterPattern_, 1);
    statusBar->addSpacing(5);

    auto clearFilter = new QAction(makeIcon("find-clear"), "C&lear Filter", this);
    clearFilter->setEnabled(false);

    connect(filterPattern_, &QLineEdit::textChanged, [this, clearFilter](const QString& text) {
        filter_->setFilterRegularExpression(text);
        clearFilter->setEnabled(!text.isEmpty());

        applyRowHeights(0, tableView_->verticalHeader()->count());
    });

    connect(clearFilter, &QAction::triggered, [this]() {
        filterPattern_->setText("");
        applyRowHeights(0, tableView_->verticalHeader()->count());
    });

    auto filterAction = new QAction(makeIcon("find"), "&Filter", this);
    filterAction->setShortcut(Qt::CTRL | Qt::ALT | Qt::Key_F);
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
    connect(copyAction, &QAction::triggered, this,
            [this]() { copy(tableView_->selectionModel()); });

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

    connect(this, &ConsoleWidget::hasNewEntries, this, &ConsoleWidget::onNewEntries,
            Qt::QueuedConnection);

    connect(this, &ConsoleWidget::scrollToBottom, tableView_, &QTableView::scrollToBottom,
            Qt::QueuedConnection);

    connect(this, &ConsoleWidget::clearSignal, this, &ConsoleWidget::clear);

    restoreState(viewColGroup);

    editActionsHandle_ =
        setupCopyPaste(mainWindow_->getInviwoEditMenu(), tableView_->selectionModel(), this);
}

ConsoleWidget::~ConsoleWidget() = default;

void ConsoleWidget::restoreState(QMenu* viewColGroup) {
    QSettings settings;
    settings.beginGroup(objectName());

    auto colVisible = settings.value("columnsVisible", QVariantList()).toList();
    auto colWidths = settings.value("columnsWidth", QVariantList()).toList();
    auto count = std::min(colVisible.size(), colWidths.size());

    for (int i = 0; i < count; ++i) {
        const bool visible = colVisible[i].toBool();
        viewColGroup->actions()[i]->setChecked(visible);
        tableView_->horizontalHeader()->setSectionHidden(i, !visible);
        if (visible) tableView_->horizontalHeader()->resizeSection(i, colWidths[i].toInt());
    }

    auto levelsActive = settings.value("levelsActive", QVariantList());
    size_t i = 0;
    for (const auto& level : levelsActive.toList()) {
        if (i < levels.size()) {
            levels[i++].action->setChecked(level.toBool());
        }
    }

    auto filterText = settings.value("filterText", "");
    filterPattern_->setText(filterText.toString());

    settings.endGroup();
}

std::shared_ptr<MenuItem> ConsoleWidget::setupCopyPaste(InviwoEditMenu* editMenu,
                                                        QItemSelectionModel* selectionModel,
                                                        QObject* owner) {
    return editMenu->registerItem(std::make_shared<MenuItem>(
        owner,
        [selectionModel](MenuItemType t) -> bool {
            switch (t) {
                case MenuItemType::copy:
                    return selectionModel->hasSelection();
                case MenuItemType::cut:
                case MenuItemType::paste:
                case MenuItemType::del:
                case MenuItemType::select:
                default:
                    return false;
            }
        },
        [selectionModel](MenuItemType t) -> void {
            switch (t) {
                case MenuItemType::copy: {
                    if (selectionModel->hasSelection()) {
                        copy(selectionModel);
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

void ConsoleWidget::log(std::string_view source, LogLevel level, LogAudience audience,
                        std::string_view file, std::string_view function, int line,
                        std::string_view msg) {
    LogTableModelEntry e{
        std::chrono::system_clock::now(), source, level, audience, file, line, function, msg};
    logEntry(std::move(e));
}

void ConsoleWidget::logEntry(LogTableModelEntry e) {
    {
        std::unique_lock lock{entriesMutex_};
        newEntries_.push_back(std::move(e));
    }
    emit hasNewEntries();
}

void ConsoleWidget::applyRowHeights(int start, int stop) {
    for (int i = start; i < stop; ++i) {
        const auto hd = tableView_->model()->headerData(i, Qt::Vertical, Qt::SizeHintRole);
        const auto height = hd.toSize().height();
        tableView_->setRowHeight(i, height);
    }
}

void ConsoleWidget::onNewEntries() {
    {
        std::unique_lock lock{entriesMutex_};
        if (newEntries_.empty()) return;

        for (auto& e : newEntries_) {
            updateIndicators(e.level);
        }

        const auto oldCount = tableView_->verticalHeader()->count();

        model_.log(newEntries_);
        newEntries_.clear();

        const auto newCount = tableView_->verticalHeader()->count();
        applyRowHeights(oldCount, newCount);
    }

    emit scrollToBottom();
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
    auto mind = model_.index(row, col);
    auto lind = filter_->mapFromSource(mind);
    return levelFilter_->mapFromSource(lind);
}

void ConsoleWidget::copy(QItemSelectionModel* selectionModel) {
    const auto& inds = selectionModel->selectedIndexes();
    if (inds.isEmpty()) return;

    int prevRow = inds.first().row();
    bool first = true;
    QString text;
    for (const auto& ind : inds) {
        // if (!tableView_->isColumnHidden(ind.column())) {
        if (!first && ind.row() == prevRow) {
            text.append('\t');
        } else if (!first) {
            text.append('\n');
        }
        if (auto v = ind.data(detail::Roles::Fulltext); !v.isNull()) {
            text.append(v.toString());
        } else {
            text.append(ind.data(Qt::DisplayRole).toString());
        }
        first = false;
        //}
        prevRow = ind.row();
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

namespace {
std::pair<int, int> getLineHeightAndMargin(const QFont& font) {
    QStyleOptionViewItem opt;
    opt.font = font;
    opt.fontMetrics = QFontMetrics{font};
    opt.features |= QStyleOptionViewItem::HasDisplay;
    opt.styleObject = nullptr;
    opt.text = "One line text";
    auto* style = qApp->style();
    auto size1 = style->sizeFromContents(QStyle::CT_ItemViewItem, &opt, QSize(), nullptr);
    opt.text = "One line text\nAnother line";
    opt.text.replace(QLatin1Char('\n'), QChar::LineSeparator);
    auto size2 = style->sizeFromContents(QStyle::CT_ItemViewItem, &opt, QSize(), nullptr);

    int lineHeight = size2.height() - size1.height();
    int margin = size1.height() - lineHeight;

    return {lineHeight, margin};
}
}  // namespace

LogTableModel::LogTableModel() : QAbstractTableModel{} {}

void LogTableModel::log(std::vector<LogTableModelEntry>& entries) {
    if (entries.empty()) return;

    beginInsertRows(QModelIndex{}, static_cast<int>(entries_.size()),
                    static_cast<int>(entries_.size() + entries.size()) - 1);

    entries_.insert(entries_.end(), std::make_move_iterator(entries.begin()),
                    std::make_move_iterator(entries.end()));

    endInsertRows();
}

void LogTableModel::clear() {
    beginResetModel();
    entries_.clear();
    endResetModel();
}

QVariant LogTableModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) return {};
    if (index.row() >= std::ssize(entries_)) return {};

    static const QColor infoTextColor = {153, 153, 153};
    static const QColor warnTextColor = {221, 165, 8};
    static const QColor errorTextColor = {255, 107, 107};

    const auto& entry = entries_[index.row()];
    const auto id = static_cast<LogTableModelEntry::ColumnID>(index.column());

    switch (role) {
        case Qt::DisplayRole: {
            switch (id) {
                case LogTableModelEntry::ColumnID::Date:
                    return entry.date;
                case LogTableModelEntry::ColumnID::Time:
                    return entry.time;
                case LogTableModelEntry::ColumnID::Source:
                    return entry.source;
                case LogTableModelEntry::ColumnID::Level:
                    return entry.levelStr;
                case LogTableModelEntry::ColumnID::Audience:
                    return entry.audience;
                case LogTableModelEntry::ColumnID::Path:
                    return entry.path;
                case LogTableModelEntry::ColumnID::File:
                    return entry.file;
                case LogTableModelEntry::ColumnID::Line:
                    return entry.line;
                case LogTableModelEntry::ColumnID::Function:
                    return entry.function;
                case LogTableModelEntry::ColumnID::Message:
                    return entry.message;
                default:
                    return {};
            }
        }
        case Qt::TextAlignmentRole:
            return Qt::AlignLeft;
        case Qt::ForegroundRole: {
            switch (entry.level) {
                case LogLevel::Info:
                    return QBrush(infoTextColor);
                case LogLevel::Warn:
                    return QBrush(warnTextColor);
                case LogLevel::Error:
                    return QBrush(errorTextColor);
                default:
                    return QBrush(infoTextColor);
            }
        }
        case Qt::FontRole:
            return LogTableModelEntry::logFont();

        case Qt::EditRole:
            [[fallthrough]];
        case detail::Roles::Fulltext:
            if (id == LogTableModelEntry::ColumnID::Message) {
                return entry.fullMessage;
            } else {
                return {};
            }

        default:
            return {};
    }
}

QVariant LogTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    switch (orientation) {
        case Qt::Horizontal: {
            switch (role) {
                case Qt::DisplayRole:
                    return section < static_cast<int>(LogTableModelEntry::size())
                               ? getName(static_cast<LogTableModelEntry::ColumnID>(section))
                               : QVariant{};
                case Qt::TextAlignmentRole:
                    return Qt::AlignLeft;
                default:
                    return {};
            }
        }
        case Qt::Vertical: {
            switch (role) {
                case Qt::SizeHintRole: {
                    if (section >= std::ssize(entries_)) return {};
                    const auto& entry = entries_[section];
                    return QSize{1, entry.height};
                }
                default:
                    return {};
            }
        }
    }
    return {};
}

Qt::ItemFlags LogTableModel::flags(const QModelIndex& index) const {
    auto flags = QAbstractTableModel::flags(index);
    // make only the message column editable
    const auto col = static_cast<LogTableModelEntry::ColumnID>(index.column());
    if (col == LogTableModelEntry::ColumnID::Message) {
        flags |= Qt::ItemIsEditable;
    }
    return flags;
}

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

const QFont& LogTableModelEntry::logFont() {
    static QFont font{QFontDatabase::systemFont(QFontDatabase::FixedFont)};
    return font;
}

const std::pair<int, int>& LogTableModelEntry::lineHeightAndMargin() {
    static std::pair<int, int> lineAndMargin{getLineHeightAndMargin(logFont())};
    return lineAndMargin;
}

LogTableModelEntry::LogTableModelEntry(std::chrono::system_clock::time_point time,
                                       std::string_view source, LogLevel level,
                                       LogAudience audience, const std::filesystem::path& file,
                                       int line, std::string_view function, std::string_view msg)

    : level{level}
    , date{utilqt::toQString(getDate(time))}
    , time{utilqt::toQString(getTime(time))}
    , source{utilqt::toQString(source)}
    , levelStr{utilqt::toQString(toString(level))}
    , audience{utilqt::toQString(toString(audience))}
    , path{utilqt::toQString(file.parent_path())}
    , file{utilqt::toQString(file.filename())}
    , line{utilqt::toQString(toString(line))}
    , function{utilqt::toQString(function)}
    , message{utilqt::toQString(util::elideLines(util::rtrim(msg)))}
    , fullMessage{utilqt::toQString(util::rtrim(msg))}
    , height{0} {

    const auto lines = std::count(msg.begin(), msg.end(), '\n') + 1;
    const auto& [lineHeight, margin] = lineHeightAndMargin();
    height = static_cast<int>(margin + lines * lineHeight);
}

std::string LogTableModelEntry::getDate(std::chrono::system_clock::time_point time) {
    auto in_time_t = std::chrono::system_clock::to_time_t(time);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%F");
    return std::move(ss).str();
}
std::string LogTableModelEntry::getTime(std::chrono::system_clock::time_point time) {
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(time.time_since_epoch()) % 1000;

    auto in_time_t = std::chrono::system_clock::to_time_t(time);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%T");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return std::move(ss).str();
}

}  // namespace inviwo
