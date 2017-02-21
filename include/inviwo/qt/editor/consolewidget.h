/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2017 Inviwo Foundation
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

#ifndef IVW_CONSOLEWIDGET_H
#define IVW_CONSOLEWIDGET_H

#include <inviwo/qt/editor/inviwoqteditordefine.h>
#include <inviwo/core/util/logcentral.h>
#include <modules/qtwidgets/inviwodockwidget.h>
#include <modules/qtwidgets/inviwoqtutils.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QAbstractTableModel>
#include <QTableView>
#include <QVariant>
#include <QStandardItemModel>
#include <QMetaType>

#include <warn/pop>

#include <chrono>

class QTableView;
class QLabel;
class QKeyEvent;
class QAction;
class QSortFilterProxyModel;
class QLineEdit;

namespace inviwo {

class InviwoMainWindow;

struct LogTableModelEntry {
    enum class Columns {
        Date = 0,
        Time,
        Source,
        Level,
        Audience,
        Path,
        File,
        Line,
        Function,
        Message
    };

    LogTableModelEntry() = default;
    LogTableModelEntry(const LogTableModelEntry &other) = default;
    ~LogTableModelEntry() = default;

    std::chrono::system_clock::time_point time;
    std::string source;
    LogLevel level;
    LogAudience audience;
    std::string fileName;
    int lineNumber;
    std::string funcionName;
    std::string message;

    std::string getDate() const;
    std::string getTime() const;
    static constexpr size_t size() { return 10; }
    QStandardItem*  get(Columns ind) const;
};

class IVW_QTEDITOR_API LogTableModel {
public:
    LogTableModel();

    QString getName(LogTableModelEntry::Columns ind) const;
    QStandardItemModel* model();
    
    void clear();    
    void log(LogTableModelEntry entry);


private:
    QColor infoTextColor_ = {153, 153, 153};
    QColor warnTextColor_ = {221, 165, 8};
    QColor errorTextColor_ = {255, 107, 107};

    QStandardItemModel model_;
};

class IVW_QTEDITOR_API ConsoleWidget : public InviwoDockWidget, public Logger {
    #include <warn/push>
    #include <warn/ignore/all>
    Q_OBJECT
    #include <warn/pop>
public:
    ConsoleWidget(InviwoMainWindow* parent);
    ~ConsoleWidget();

    virtual void log(std::string logSource, LogLevel logLevel, LogAudience audience, const char* fileName,
             const char* functionName, int lineNumber, std::string logMsg) override;

    virtual void logProcessor(Processor* processor, LogLevel level, LogAudience audience,
                              std::string msg, const char* file, const char* function,
                              int line) override;

    virtual void logNetwork(LogLevel level, LogAudience audience, std::string msg, const char* file,
                            const char* function, int line) override;

    QAction* getClearAction();
    QTableView* view() {return tableView_;}

public slots:
    void logEntry(LogTableModelEntry);
    void updateIndicators(LogLevel level);
    void clear();

signals:
    void logSignal(LogTableModelEntry level);
    void clearSignal();

protected:
    virtual void keyPressEvent(QKeyEvent* keyEvent) override;
    virtual bool eventFilter(QObject *object, QEvent *event) override;
    virtual void closeEvent(QCloseEvent *event) override;

private:
    QModelIndex mapToSource(int row, int col);
    QModelIndex mapFromSource(int row, int col);


    QTableView* tableView_;
    LogTableModel model_;
    QSortFilterProxyModel* filter_;
    QSortFilterProxyModel* levelFilter_;

    struct Level {
        LogLevel level;
        std::string name;
        std::string icon;
        int count;
        QAction* action;
        QLabel* label;
    };

    std::array<Level, 3> levels = {{{LogLevel::Error, "Errors", "error", 0, nullptr, nullptr},
                                    {LogLevel::Warn, "Warnings", "warning", 0, nullptr, nullptr},
                                    {LogLevel::Info, "Info", "info", 0, nullptr, nullptr}}};

    QLineEdit* filterPattern_;
    QAction* clearAction_;
    InviwoMainWindow* mainwindow_;
    std::unordered_map<std::string, QMetaObject::Connection> connections_;
    
    bool hover_ = false;
    bool focus_ = false;
};

}  // namespace

Q_DECLARE_METATYPE(inviwo::LogTableModelEntry)

#endif  // IVW_CONSOLELISTWIDGET_H
