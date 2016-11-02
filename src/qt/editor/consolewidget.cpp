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
#include <warn/pop>

#include <inviwo/core/common/inviwo.h>
#include <inviwo/qt/editor/consolewidget.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/qt/editor/inviwomainwindow.h>

namespace inviwo {

ConsoleWidget::ConsoleWidget(InviwoMainWindow* parent)
    : InviwoDockWidget(tr("Console"), parent)
    , errorsLabel_(nullptr)
    , warningsLabel_(nullptr)
    , infoLabel_(nullptr)
    , numErrors_(0)
    , numWarnings_(0)
    , numInfos_(0)
    , mainwindow_(parent) {

    setObjectName("ConsoleWidget");
    setAllowedAreas(Qt::BottomDockWidgetArea);

    tableView_ = new QTableView(this);
    tableView_->setModel(model_.model());
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
    }

    tableView_->horizontalHeader()->setResizeContentsPrecision(0);
    tableView_->horizontalHeader()->setSectionResizeMode(cols - 1, QHeaderView::Stretch);
    tableView_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    tableView_->verticalHeader()->setVisible(false);
    tableView_->verticalHeader()->setResizeContentsPrecision(0);
    tableView_->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    tableView_->verticalHeader()->setMinimumSectionSize(5);
    tableView_->verticalHeader()->setDefaultSectionSize(13);
    
    QHBoxLayout *statusBar = new QHBoxLayout();

    errorsLabel_ = new QLabel("0", this);
    warningsLabel_ = new QLabel("0", this);
    infoLabel_ = new QLabel("0", this);

    QFrame* line1 = new QFrame();
    QFrame* line2 = new QFrame();
    QFrame* line3 = new QFrame();
    line1->setFrameShape(QFrame::VLine);
    line2->setFrameShape(QFrame::VLine);
    line3->setFrameShape(QFrame::VLine);
    
    line1->setFrameShadow(QFrame::Raised);
    line2->setFrameShadow(QFrame::Raised);
    line3->setFrameShadow(QFrame::Raised);

    statusBar->addWidget(new QLabel( "<img width='16' height='16' src=':/icons/error.png'>", this));
    statusBar->addWidget(errorsLabel_);
    statusBar->addWidget(line1);
    statusBar->addWidget(new QLabel( "<img width='16' height='16' src=':/icons/warning.png'>", this));
    statusBar->addWidget(warningsLabel_);
    statusBar->addWidget(line2);
    statusBar->addWidget(new QLabel( "<img width='16' height='16' src=':/icons/info.png'>", this));
    statusBar->addWidget(infoLabel_);
    statusBar->addWidget(line3);
    

    statusBar->addItem(new QSpacerItem(40, 1, QSizePolicy::Expanding, QSizePolicy::Minimum));
    
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(tableView_);
    layout->addLayout(statusBar);

    layout->setContentsMargins(3, 0, 0, 3);

    QWidget* w = new QWidget();
    w->setLayout(layout);
    setWidget(w);
    tableView_->installEventFilter(this);
    tableView_->setAttribute(Qt::WA_Hover);

    connect(this, &ConsoleWidget::updateIndicatorsSignal, [&](LogLevel l){updateIndicators(l);});
    connect(this, SIGNAL(clearSignal()), this, SLOT(clear()));
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
    errorsLabel_->setText("0");
    warningsLabel_->setText("0");
    infoLabel_->setText("0");
    numErrors_ = numWarnings_ = numInfos_ = 0;
}

void ConsoleWidget::updateIndicators(LogLevel level) {
    if (QThread::currentThread() != QCoreApplication::instance()->thread()) {
        emit updateIndicatorsSignal(level);
    } else {
        switch (level) {
            case LogLevel::Error: {
                errorsLabel_->setText(toString(++numErrors_).c_str());
                break;
            }
            case LogLevel::Warn: {
                warningsLabel_->setText(toString(++numWarnings_).c_str());
                break;
            }
            case LogLevel::Info: {
                infoLabel_->setText(toString(++numInfos_).c_str());
                break;
            }
        }
    }
}

void ConsoleWidget::log(std::string logSource, LogLevel level, LogAudience audience,
                        const char* fileName, const char* function, int line, std::string msg) {

    model_.log(logSource, level, audience, fileName, function, line, msg);
    updateIndicators(level);
    tableView_->scrollToBottom(); //crazy slow...
}

void ConsoleWidget::logProcessor(Processor* processor, LogLevel level, LogAudience audience,
                                 std::string msg, const char* file, const char* function,
                                 int line) {
    model_.log(processor->getIdentifier(), level, audience, file, function, line,
               msg);
    updateIndicators(level);
    tableView_->scrollToBottom();
}

void ConsoleWidget::logNetwork(LogLevel level, LogAudience audience,
                              std::string msg, const char* file, const char* function,
                              int line) {
    
    model_.log("ProcessorNetwork", level, audience, file, function, line, msg);
    updateIndicators(level);
    tableView_->scrollToBottom();
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
                QString text;
                for (const auto& ind : inds) {
                    auto data = model_.model()->data(ind);
                    if (tableView_->isColumnHidden(ind.column())) continue;
                    text.append(data.toString());
                    if (ind.row() == prevrow) {
                        text.append('\t');
                    } else {
                        text.append('\n');
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

LogTableModel::LogTableModel() : QObject(), model_(0, Entry::size()) {
    connect(this, &LogTableModel::logSignal, [&](Entry e) { log(e); });

    for (size_t i = 0; i < Entry::size(); ++i) {
        auto item = new QStandardItem(getName(static_cast<Columns>(i)));
        item->setTextAlignment(Qt::AlignLeft);
        model_.setHorizontalHeaderItem(i, item);
    }
}

void LogTableModel::log(std::string logSource, LogLevel logLevel, LogAudience audience,
                        const char* fileName, const char* functionName, int lineNumber,
                        std::string logMsg) {

    Entry e = {std::chrono::system_clock::now(),
               logSource,
               logLevel,
               audience,
               fileName ? fileName : "",
               lineNumber,
               functionName ? functionName : "",
               logMsg};

    if (QThread::currentThread() != QCoreApplication::instance()->thread()) {
        emit logSignal(e);
    } else {
        log(e);
    }
}

void LogTableModel::log(Entry entry) {
    auto lines = std::count(entry.message.begin(), entry.message.end(), '\n') + 1;

    QList<QStandardItem*> items;
    items.reserve(Entry::size());
    for (size_t i = 0; i < Entry::size(); ++i) {
        items.append(entry.get(static_cast<Columns>(i)));
        items.last()->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
        items.last()->setTextAlignment(Qt::AlignLeft);
        items.last()->setEditable(false);
        auto height = QFontMetrics(items.last()->font()).height();
        items.last()->setSizeHint(QSize(1, lines * height));
        
        
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

std::string LogTableModel::Entry::getTime(const std::string& format) const {
    auto in_time_t = std::chrono::system_clock::to_time_t(time);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), format.c_str());
    return ss.str();
}

QStandardItem* LogTableModel::Entry::get(Columns ind) const {
    switch (ind) {
        case Columns::Date:
            return new QStandardItem(utilqt::toQString(getTime("%F")));
        case Columns::Time:
            return new QStandardItem(utilqt::toQString(getTime("%T")));
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
