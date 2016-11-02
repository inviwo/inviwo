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
#include <warn/pop>

#include <inviwo/core/common/inviwo.h>
#include <inviwo/qt/editor/consolewidget.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/processors/processor.h>


namespace inviwo {

ConsoleWidget::ConsoleWidget(QWidget* parent)
    : InviwoDockWidget(tr("Console"), parent)
    , errorsLabel_(nullptr)
    , warningsLabel_(nullptr)
    , infoLabel_(nullptr)
    , numErrors_(0)
    , numWarnings_(0)
    , numInfos_(0) {

    setObjectName("ConsoleWidget");
    setAllowedAreas(Qt::BottomDockWidgetArea);

    //textField_->setReadOnly(true);
    //textField_->setContextMenuPolicy(Qt::CustomContextMenu);
    //connect(textField_, SIGNAL(customContextMenuRequested(const QPoint&)), this,
    //        SLOT(showContextMenu(const QPoint&)));

    model2_ = new QStandardItemModel(0, 8, this);

    tableView_ = new QTableView(this);
    tableView_->setModel(model2_);
    tableView_->setGridStyle(Qt::NoPen);
    tableView_->setCornerButtonEnabled(false);
    tableView_->hideColumn(2);
    tableView_->hideColumn(3);
    tableView_->hideColumn(4);
    tableView_->hideColumn(5);
    tableView_->hideColumn(6);


    const auto cols = tableView_->horizontalHeader()->count();
    for (int i = 0; i <cols - 1; ++i) {
        //tableView_->horizontalHeader()->setSectionResizeMode(i, QHeaderView::ResizeToContents);
    }
    tableView_->horizontalHeader()->setSectionResizeMode(cols - 1, QHeaderView::Stretch);

    tableView_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    //tableView_->resizeRowsToContents();

    //tableView_->verticalHeader()->setVisible(false);
    tableView_->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    tableView_->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

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

    connect(this, &ConsoleWidget::updateIndicatorsSignal, [&](LogLevel l){updateIndicators(l);});
    connect(this, SIGNAL(clearSignal()), this, SLOT(clear()));
}

ConsoleWidget::~ConsoleWidget() = default;

void ConsoleWidget::showContextMenu(const QPoint& pos) {
    /*QMenu* menu = textField_->createStandardContextMenu();
    QAction* clearAction = menu->addAction("Clear console");
    clearAction->setShortcut(Qt::ControlModifier + Qt::Key_E);
    menu->addAction(clearAction);
    QAction* result = menu->exec(QCursor::pos());

    if (result == clearAction) {
        clear();
    }

    delete menu;*/
}

void ConsoleWidget::clear(){
    if (QThread::currentThread() != QCoreApplication::instance()->thread()){
        emit clearSignal();
        return;
    }

    //textField_->clear();
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

/*
void ConsoleWidget::logMessage(LogLevel level, QString message) {

    textField_->append(message);
    QTextCursor c =  textField_->textCursor();
    c.movePosition(QTextCursor::End);
    textField_->setTextCursor(c);
}
*/

void ConsoleWidget::log(std::string logSource, LogLevel level, LogAudience audience,
                        const char* fileName, const char* function, int line, std::string msg) {

    model_.log(logSource, level, audience, fileName, function, line, msg);
    updateIndicators(level);
    //if(!tableView_->selectionModel()->hasSelection()) 
    tableView_->scrollTo(model_.index(model_.rowCount() - 1, 0));
}

void ConsoleWidget::logProcessor(Processor* processor, LogLevel level, LogAudience audience,
                                 std::string msg, const char* file, const char* function,
                                 int line) {
    model_.log("Processor " + processor->getIdentifier(), level, audience, file, function, line,
               msg);
    updateIndicators(level);
    //if(!tableView_->selectionModel()->hasSelection()) 
    tableView_->scrollTo(model_.index(model_.rowCount()-1, 0));
}

void ConsoleWidget::logNetwork(LogLevel level, LogAudience audience,
                              std::string msg, const char* file, const char* function,
                              int line) {
    
    model_.log("ProcessorNetwork", level, audience, file, function, line, msg);
    updateIndicators(level);
    //if(!tableView_->selectionModel()->hasSelection()) 
    tableView_->scrollTo(model_.index(model_.rowCount() - 1, 0));
}


void ConsoleWidget::keyPressEvent(QKeyEvent* keyEvent) {
    if (keyEvent->key() == Qt::Key_E && keyEvent->modifiers() == Qt::ControlModifier){
        clear();
    }
}


LogTableModel::LogTableModel() : QAbstractTableModel() {
    connect(this, &LogTableModel::logSignal, [&](Entry e){log(e);});
}

int LogTableModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;
    return static_cast<int>(log_.size());
}

int LogTableModel::columnCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;
    return static_cast<int>(Entry::size());
}

QVariant LogTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Vertical) return QAbstractTableModel::headerData(section, orientation, role);

    if (section >= 0 && section < Entry::size()) {
        switch (role) {
            case Qt::DisplayRole:
                return getName(static_cast<size_t>(section));
            case Qt::TextAlignmentRole:
                return QVariant(Qt::AlignLeft | Qt::AlignCenter);
        }
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}

QVariant LogTableModel::data(const QModelIndex& index, int role) const {
    if (index.row() >= 0 && index.row() < log_.size()) {
        if (index.column() >= 0 && index.column() < Entry::size()) {
            switch (role) {
                case Qt::DisplayRole: {
                    return log_[static_cast<size_t>(index.row())].get(
                        static_cast<size_t>(index.column()));
                }
                case Qt::DecorationRole:
                    return QVariant();
                case Qt::FontRole:
                    return QVariant(QFontDatabase::systemFont(QFontDatabase::FixedFont));
                case Qt::TextAlignmentRole:
                    return QVariant(Qt::AlignLeft | Qt::AlignTop);
                case Qt::ForegroundRole: {
                    switch (log_[static_cast<size_t>(index.row())].level) {
                        case LogLevel::Info: return QVariant(QBrush(infoTextColor_));
                        case LogLevel::Warn: return QVariant(QBrush(warnTextColor_));
                        case LogLevel::Error: return QVariant(QBrush(errorTextColor_));
                        default: return QVariant(QBrush(infoTextColor_));
                    }
                }
            }
        }
    }
    return QVariant();
}

void LogTableModel::log(std::string logSource, LogLevel logLevel, LogAudience audience,
                        const char* fileName, const char* functionName, int lineNumber,
                        std::string logMsg) {

    Entry e = {std::chrono::system_clock::now(),
               logSource,
               logLevel,
               audience,
               fileName,
               lineNumber,
               functionName,
               logMsg};

    if (QThread::currentThread() != QCoreApplication::instance()->thread()) {
        //emit logSignal(e);
    } else {
        log(e);
    }
}

void LogTableModel::log(Entry entry) {
      beginInsertRows(QModelIndex(), log_.size(), log_.size() + 1);
      log_.push_back(std::move(entry));
      endInsertRows();
}

QVariant LogTableModel::getName(size_t ind) const {
    switch (ind) {
        case 0: return QVariant(QString("Time"));
        case 1: return QVariant(QString("Source"));
        case 2: return QVariant(QString("Level"));
        case 3: return QVariant(QString("Audience"));
        case 4: return QVariant(QString("File"));
        case 5: return QVariant(QString("Line"));
        case 6: return QVariant(QString("Function"));
        case 7: return QVariant(QString("Message"));
        default: return QVariant();
    }
}

std::string LogTableModel::Entry::getTime() const {
    auto in_time_t = std::chrono::system_clock::to_time_t(time);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%T");
    return ss.str();
}

QVariant LogTableModel::Entry::get(size_t ind) const {
    switch (ind) {
        case 0: return QVariant(utilqt::toQString(getTime()));
        case 1: return QVariant(utilqt::toQString(source));
        case 2: return QVariant(utilqt::toQString(toString(level)));
        case 3: return QVariant(utilqt::toQString(toString(audience)));
        case 4: return QVariant(utilqt::toQString(fileName));
        case 5: return QVariant(lineNumber);
        case 6: return QVariant(utilqt::toQString(funcionName));
        case 7: return QVariant(utilqt::toQString(message));
        default: return QVariant();
    }
}

} // namespace