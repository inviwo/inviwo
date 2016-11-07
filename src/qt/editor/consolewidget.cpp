/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2016 Inviwo Foundation
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
#include <warn/pop>

#include <inviwo/core/common/inviwo.h>
#include <inviwo/qt/editor/consolewidget.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/qt/editor/inviwomainwindow.h>
#include <inviwo/core/util/ostreamjoiner.h>

namespace inviwo {

ConsoleWidget::ConsoleWidget(InviwoMainWindow* parent)
    : InviwoDockWidget(tr("Console"), parent)
    , tableView_(new QTableView(this))
    , model_(tableView_)
    , filter_(new QSortFilterProxyModel(this))
    , levelFilter_(new QSortFilterProxyModel(this))
    , filterPattern_(new QLineEdit(this))
    , mainwindow_(parent) {

    setObjectName("ConsoleWidget");
    setAllowedAreas(Qt::BottomDockWidgetArea);

    filter_->setSourceModel(model_.model());
    filter_->setFilterKeyColumn(static_cast<int>(LogTableModel::Columns::Message));

    levelFilter_->setSourceModel(filter_);
    levelFilter_->setFilterKeyColumn(static_cast<int>(LogTableModel::Columns::Level));

    tableView_->setModel(levelFilter_);
    tableView_->setGridStyle(Qt::NoPen);
    tableView_->setCornerButtonEnabled(false);
    
    tableView_->setContextMenuPolicy(Qt::ActionsContextMenu);
    clearAction_ = new QAction(tr("&Clear Log"), this);
    clearAction_->setShortcut(Qt::ControlModifier + Qt::Key_E);
    connect(clearAction_, &QAction::triggered, [&]() { clear(); });
    tableView_->addAction(clearAction_);
   
    tableView_->hideColumn(static_cast<int>(LogTableModel::Columns::Date));
    tableView_->hideColumn(static_cast<int>(LogTableModel::Columns::Level));
    tableView_->hideColumn(static_cast<int>(LogTableModel::Columns::Audience));
    tableView_->hideColumn(static_cast<int>(LogTableModel::Columns::Path));
    tableView_->hideColumn(static_cast<int>(LogTableModel::Columns::File));
    tableView_->hideColumn(static_cast<int>(LogTableModel::Columns::Line));
    tableView_->hideColumn(static_cast<int>(LogTableModel::Columns::Function));

    tableView_->horizontalHeader()->setContextMenuPolicy(Qt::ActionsContextMenu);
    
    const auto cols = tableView_->horizontalHeader()->count();

    auto viewColGroup = new QMenu(this);
    for (int i = 0; i < cols; ++i) {
        auto viewCol = new QAction(
            QString("View ") + model_.getName(static_cast<LogTableModel::Columns>(i)), this);
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
    auto viewColAction = new QAction("View", this);
    viewColAction->setMenu(viewColGroup);
    tableView_->addAction(viewColAction);

    tableView_->horizontalHeader()->setResizeContentsPrecision(0);
    tableView_->horizontalHeader()->setSectionResizeMode(cols - 1, QHeaderView::Stretch);
    tableView_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    tableView_->verticalHeader()->setVisible(false);
    tableView_->verticalHeader()->setResizeContentsPrecision(0);
    tableView_->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    auto height = 2 + QFontMetrics(QFontDatabase::systemFont(QFontDatabase::FixedFont)).height();
    tableView_->verticalHeader()->setMinimumSectionSize(height);
    tableView_->verticalHeader()->setDefaultSectionSize(height);
    
    QHBoxLayout *statusBar = new QHBoxLayout();
    statusBar->setObjectName("StatusBar");

    auto makeToolButton = [this, statusBar](const QString& label, const QString& file,
                                            bool checkable = true) {
        auto button = new QToolButton(this);
        auto icon = QIcon();
        if(checkable) {
            icon.addPixmap(QPixmap(":/icons/" + file + ".png"), QIcon::Normal, QIcon::On);
            icon.addPixmap(QPixmap(":/icons/" + file + "-bw.png"), QIcon::Normal, QIcon::Off);
        } else {
            icon.addPixmap(QPixmap(":/icons/" + file + ".png"), QIcon::Normal, QIcon::Off);
            icon.addPixmap(QPixmap(":/icons/" + file + "-bw.png"), QIcon::Disabled, QIcon::Off);
        }

        auto action = new QAction(icon, label, this);
        action->setCheckable(checkable);
        if (checkable) action->setChecked(true);

        button->setDefaultAction(action);
        statusBar->addWidget(button);
        return action;
    };

    auto updateRowsHeights = [this]() {
        tableView_->setUpdatesEnabled(false);
        auto height = QFontMetrics(QFontDatabase::systemFont(QFontDatabase::FixedFont)).height();

        auto vrows = tableView_->verticalHeader()->count();
        for (int i = 0; i < vrows; ++i) {
            auto mind = mapToSource(i, static_cast<int>(LogTableModel::Columns::Message));
            auto message = mind.data(Qt::DisplayRole).toString();
            auto lines = std::count(message.begin(), message.end(), '\n') + 1;
            tableView_->verticalHeader()->resizeSection(i, 2 + lines * height);
        }
        tableView_->setUpdatesEnabled(true);
    };

    auto levelCallback = [this, updateRowsHeights](bool checked) {
        if (util::all_of(levels, [](const auto& level){return level.action->isChecked();})) {
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
        connect(level.action, &QAction::triggered, levelCallback);
    }
    auto viewAction = new QAction("Log Level", this);
    viewAction->setMenu(levelGroup);
    tableView_->addAction(viewAction);

    statusBar->addStretch(3);
    statusBar->addWidget(new QLabel("Filter", this));

    filterPattern_->setMinimumWidth(200);
    statusBar->addWidget(filterPattern_, 1);
    auto clearFilter = makeToolButton("Clear Filter", "button_cancel", false);
    clearFilter->setEnabled(false);

    tableView_->addAction(clearFilter);

    statusBar->addSpacing(5);

    auto clearButton = new QToolButton(this);
    clearButton->setDefaultAction(clearAction_);
    statusBar->addWidget(clearButton);
    statusBar->addSpacing(5);

    connect(filterPattern_, &QLineEdit::textChanged,
            [this, updateRowsHeights, clearFilter](const QString& text) {
                filter_->setFilterRegExp(text);
                updateRowsHeights();
                clearFilter->setEnabled(!text.isEmpty());
            });

    connect(clearFilter, &QAction::triggered, [this]() {
        filterPattern_->setText("");
    });

    auto filterAction = new QAction("Filter", this);
    filterAction->setShortcut(Qt::ControlModifier + Qt::AltModifier + Qt::Key_F);
    connect(filterAction, &QAction::triggered, [this](){
        raise();
        filterPattern_->setFocus();
        filterPattern_->selectAll();
    });
    tableView_->addAction(filterAction);


    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(tableView_);
    layout->addLayout(statusBar);

    layout->setContentsMargins(3, 0, 0, 3);

    QWidget* w = new QWidget();
    w->setLayout(layout);
    setWidget(w);
    tableView_->installEventFilter(this);
    tableView_->setAttribute(Qt::WA_Hover);

    connect(this, &ConsoleWidget::logSignal, [&](LogTableModel::Entry e) { log(e); });
    connect(this, &ConsoleWidget::clearSignal, [&]() { clear(); });

    // Restore State
    QSettings settings("Inviwo", "Inviwo");
    settings.beginGroup("console");

    auto columnsActive = settings.value("columnsActive", QVariant(QList<QVariant>()));
    auto columnsWidth = settings.value("columnsWidth", QVariant(QList<QVariant>()));

    int i = 0;
    for (const auto& col : columnsActive.toList()) {
        tableView_->horizontalHeader()->setSectionHidden(i++, col.toBool());
    }

    i = 0;
    for (const auto& col : columnsWidth.toList()) {
        if (!tableView_->horizontalHeader()->isHidden()) {
            tableView_->horizontalHeader()->resizeSection(i++, col.toInt());
        }
    }

    auto levelsActive = settings.value("levelsActive", QVariant(QList<QVariant>()));
    i = 0;
    for (const auto& level : levelsActive.toList()) {
        levels[i++].action->setChecked(level.toBool());
    }
    auto filterText = settings.value("filterText", "");
    filterPattern_->setText(filterText.toString());

    settings.endGroup();

}

ConsoleWidget::~ConsoleWidget() = default;

QAction* ConsoleWidget::getClearAction() {
    return clearAction_;
}

void ConsoleWidget::clear(){
    if (QThread::currentThread() != QCoreApplication::instance()->thread()){
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
    auto it = util::find_if(levels, [&](const auto& l){return l.level == level;});
    if (it != levels.end()) {
        it->label->setText(toString(++(it->count)).c_str());
    }
}

void ConsoleWidget::log(std::string source, LogLevel level, LogAudience audience,
                        const char* file, const char* function, int line, std::string msg) {
    LogTableModel::Entry e = {
        std::chrono::system_clock::now(),
        source,
        level,
        audience,
        file ? file : "",
        line,
        function ? function : "",
        msg
    };
    log(std::move(e));
}


void ConsoleWidget::logProcessor(Processor* processor, LogLevel level, LogAudience audience,
                                 std::string msg, const char* file, const char* function,
                                 int line) {
    LogTableModel::Entry e = {
        std::chrono::system_clock::now(),
        processor->getIdentifier(),
        level,
        audience,
        file ? file : "",
        line,
        function ? function : "",
        msg
    };
    log(std::move(e));
}

void ConsoleWidget::logNetwork(LogLevel level, LogAudience audience,
                              std::string msg, const char* file, const char* function,
                              int line) {
    LogTableModel::Entry e = {
        std::chrono::system_clock::now(),
        "ProcessorNetwork",
        level,
        audience,
        file ? file : "",
        line,
        function ? function : "",
        msg
    };
    log(std::move(e));
}

void ConsoleWidget::log(LogTableModel::Entry e) {
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
        auto vind = mapFromSource(model_.model()->rowCount()-1,0);
        if (vind.isValid()) {
            tableView_->verticalHeader()->resizeSection(vind.row(), 2 + lines*height);
        }
    }

    tableView_->scrollToBottom();
    tableView_->setUpdatesEnabled(true);
}



void ConsoleWidget::keyPressEvent(QKeyEvent* keyEvent) {
    if (keyEvent->key() == Qt::Key_E && keyEvent->modifiers() == Qt::ControlModifier){
        clear();
    }
}

bool ConsoleWidget::eventFilter(QObject* object, QEvent* event) {
    if (event->type() == QEvent::FocusIn) {
        focus_ = true;

        auto enable = tableView_->selectionModel()->hasSelection();
        auto action = mainwindow_->getActions().find("Copy")->second;

        if (!connections_["Copy"]) {
            connections_["Copy"] = connect(action, &QAction::triggered, [&]() {
                const auto& inds = tableView_->selectionModel()->selectedIndexes();
                int prevrow = inds.first().row();
                bool first = true;
                QString text;
                for (const auto& ind : inds) {
                    if (!tableView_->isColumnHidden(ind.column())) {
                        if (!first && ind.row() == prevrow) {
                            text.append('\t');
                        } else if(!first) {
                            text.append('\n');
                        }
                        text.append(ind.data(Qt::DisplayRole).toString());
                        first = false;
                    }
                    prevrow = ind.row();
                }
                auto mimedata = util::make_unique<QMimeData>();
                mimedata->setData(QString("text/plain"), text.toUtf8());
                QApplication::clipboard()->setMimeData(mimedata.release());
            });
        }
        action->setEnabled(enable);

    } else if (event->type() == QEvent::FocusOut) {
        focus_ = false;
        auto action = mainwindow_->getActions().find("Copy")->second;
        disconnect(connections_["Copy"]);
        action->setEnabled(focus_ || hover_);
    } else if (event->type() == QEvent::HoverEnter) {
        hover_ = true;
        auto enable = tableView_->selectionModel()->hasSelection();
        auto action = mainwindow_->getActions().find("Copy")->second;
        action->setEnabled(enable);
        mainwindow_->getActions().find("Paste")->second->setEnabled(false);
        mainwindow_->getActions().find("Cut")->second->setEnabled(false);
        mainwindow_->getActions().find("Delete")->second->setEnabled(false);

    } else if (event->type() == QEvent::HoverLeave) {
        hover_ = false;
        auto action = mainwindow_->getActions().find("Copy")->second;
        action->setEnabled(focus_ || hover_);
    }
    return false;
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

void ConsoleWidget::closeEvent(QCloseEvent *event) {
    QSettings settings("Inviwo", "Inviwo");
    settings.beginGroup("console");
    settings.setValue("geometry", saveGeometry());

    const auto cols = tableView_->horizontalHeader()->count();
    QList<QVariant> columnsActive;
    QList<QVariant> columnsWidth;
    columnsActive.reserve(cols);
    columnsWidth.reserve(cols);
    for (int i = 0; i < cols; ++i) {
        columnsActive.append(tableView_->horizontalHeader()->isSectionHidden(i));
        columnsWidth.append(tableView_->horizontalHeader()->sectionSize(i));
    }
    QList<QVariant> levelsActive;
    for (const auto& level : levels) {
        levelsActive.append(level.action->isChecked());
    }
    
    settings.setValue("columnsActive", QVariant(columnsActive));
    settings.setValue("columnsWidth", QVariant(columnsWidth));
    settings.setValue("levelsActive", QVariant(levelsActive));
    settings.setValue("filterText", QVariant(filterPattern_->text()));
    settings.endGroup();
}

LogTableModel::LogTableModel(QTableView* view)
    : view_(view), model_(0, static_cast<int>(Entry::size())) {
    for (size_t i = 0; i < Entry::size(); ++i) {
        auto item = new QStandardItem(getName(static_cast<Columns>(i)));
        item->setTextAlignment(Qt::AlignLeft);
        model_.setHorizontalHeaderItem(static_cast<int>(i), item);
    }
}


void LogTableModel::log(Entry entry) {
    QList<QStandardItem*> items;
    items.reserve(static_cast<int>(Entry::size()));
    for (size_t i = 0; i < Entry::size(); ++i) {
        items.append(entry.get(static_cast<Columns>(i)));
        items.last()->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
        items.last()->setTextAlignment(Qt::AlignLeft);
        items.last()->setEditable(false);    
        //items.last()->setSizeHint(QSize(1, lines * height));
        
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

QStandardItemModel* LogTableModel::model() {
    return &model_;
}

void LogTableModel::clear() {
    model_.removeRows(0, model_.rowCount());
}

QString LogTableModel::getName(Columns ind) const {
    switch (ind) {
        case Columns::Date: return QString("Date");
        case Columns::Time: return QString("Time");
        case Columns::Source: return QString("Source");
        case Columns::Level: return QString("Level");
        case Columns::Audience: return QString("Audience");
        case Columns::Path: return QString("Path");
        case Columns::File: return QString("File");
        case Columns::Line: return QString("Line");
        case Columns::Function: return QString("Function");
        case Columns::Message: return QString("Message");
        default: return QString();
    }
}

std::string LogTableModel::Entry::getDate() const {
    auto in_time_t = std::chrono::system_clock::to_time_t(time);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%F");
    return ss.str();
}
std::string LogTableModel::Entry::getTime() const {
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(time.time_since_epoch()) % 1000;

    auto in_time_t = std::chrono::system_clock::to_time_t(time);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%T");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

QStandardItem* LogTableModel::Entry::get(Columns ind) const {
    switch (ind) {
        case Columns::Date:
            return new QStandardItem(utilqt::toQString(getDate()));
        case Columns::Time:
            return new QStandardItem(utilqt::toQString(getTime()));
        case Columns::Source:
            return new QStandardItem(utilqt::toQString(source));
        case Columns::Level:
            return new QStandardItem(utilqt::toQString(toString(level)));
        case Columns::Audience:
            return new QStandardItem(utilqt::toQString(toString(audience)));
        case Columns::Path:
            return new QStandardItem(utilqt::toQString(filesystem::getFileDirectory(fileName)));
        case Columns::File:
            return new QStandardItem(
                utilqt::toQString(filesystem::getFileNameWithExtension(fileName)));
        case Columns::Line:
            return new QStandardItem(utilqt::toQString(toString(lineNumber)));
        case Columns::Function:
            return new QStandardItem(utilqt::toQString(funcionName));
        case Columns::Message:
            return new QStandardItem(utilqt::toQString(message));
        default:
            return new QStandardItem();
    }
}

} // namespace
