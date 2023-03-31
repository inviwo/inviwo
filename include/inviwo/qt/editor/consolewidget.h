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

#pragma once

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
#include <QItemDelegate>
#include <warn/pop>

#include <chrono>
#include <filesystem>

class QLabel;
class QKeyEvent;
class QAction;
class QSortFilterProxyModel;
class QLineEdit;

class QResizeEvent;
class QWheelEvent;

namespace inviwo {

class MenuItem;
class InviwoMainWindow;

class LogTableModelEntry {
public:
    enum class ColumnID {
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

    LogTableModelEntry(std::chrono::system_clock::time_point time, std::string_view source,
                       LogLevel level, LogAudience audience, const std::filesystem::path& file,
                       int line, std::string_view function, std::string_view msg);

    static constexpr size_t size() { return 10; }
    QList<QStandardItem*> items();
    QStandardItem* header();
    LogLevel level;

private:
    QStandardItem* header_;
    QStandardItem* date_;
    QStandardItem* time_;
    QStandardItem* source_;
    QStandardItem* level_;
    QStandardItem* audience_;
    QStandardItem* path_;
    QStandardItem* file_;
    QStandardItem* line_;
    QStandardItem* function_;
    QStandardItem* message_;

    static std::string getDate(std::chrono::system_clock::time_point time);
    static std::string getTime(std::chrono::system_clock::time_point time);

    static const QFont& logFont();
    static const std::pair<int, int>& lineHeightAndMargin();
};

class IVW_QTEDITOR_API TextSelectionDelegate : public QItemDelegate {
public:
    TextSelectionDelegate(QWidget* parent = nullptr);
    virtual ~TextSelectionDelegate() = default;

    virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                                  const QModelIndex& index) const override;

    // dummy function doing nothing to prevent writing stuff from the editor back to the model
    virtual void setModelData(QWidget* editor, QAbstractItemModel* model,
                              const QModelIndex& index) const override;
};

class IVW_QTEDITOR_API LogModel : public QStandardItemModel {
public:
    LogModel(int rows, int columns, QObject* parent = nullptr);
    virtual ~LogModel() = default;

    virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
};

class IVW_QTEDITOR_API LogTableModel {
public:
    LogTableModel();

    QString getName(LogTableModelEntry::ColumnID ind) const;
    LogModel* model();

    void clear();
    void log(LogTableModelEntry entry);

private:
    LogModel model_;
};

class IVW_QTEDITOR_API ConsoleWidget : public InviwoDockWidget, public Logger {
#include <warn/push>
#include <warn/ignore/all>
    Q_OBJECT
#include <warn/pop>
public:
    ConsoleWidget(InviwoMainWindow* parent);
    ~ConsoleWidget();

    virtual void log(std::string_view logSource, LogLevel logLevel, LogAudience audience,
                     std::string_view file, std::string_view function, int line,
                     std::string_view) override;

    virtual void logProcessor(Processor* processor, LogLevel level, LogAudience audience,
                              std::string_view msg, std::string_view file,
                              std::string_view function, int line) override;

    virtual void logNetwork(LogLevel level, LogAudience audience, std::string_view msg,
                            std::string_view file, std::string_view function, int line) override;

    virtual void logAssertion(std::string_view file, std::string_view function, int line,
                              std::string_view msg) override;

    QAction* getClearAction();
    QTableView* view() { return tableView_; }

public slots:
    void logEntry(LogTableModelEntry);
    void updateIndicators(LogLevel level);
    void clear();

signals:
    void logSignal(LogTableModelEntry level);
    void clearSignal();

protected:
    virtual void keyPressEvent(QKeyEvent* keyEvent) override;
    virtual void closeEvent(QCloseEvent* event) override;

private:
    QModelIndex mapToSource(int row, int col);
    QModelIndex mapFromSource(int row, int col);
    void copy();

    QTableView* tableView_;
    LogTableModel model_;
    QSortFilterProxyModel* filter_;
    QSortFilterProxyModel* levelFilter_;
    TextSelectionDelegate* textSelectionDelegate_;

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

    QLabel* threadPoolInfo_;
    QLineEdit* filterPattern_;
    QAction* clearAction_;
    InviwoMainWindow* mainwindow_;
    std::shared_ptr<MenuItem> editActionsHandle_;
};

}  // namespace inviwo

Q_DECLARE_METATYPE(inviwo::LogTableModelEntry)
