/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2024 Inviwo Foundation
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
#include <mutex>

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

    LogLevel level;
    QString date;
    QString time;
    QString source;
    QString levelStr;
    QString audience;
    QString path;
    QString file;
    QString line;
    QString function;
    QString message;
    QString fullMessage;
    int height;

    static const QFont& logFont();

private:
    static std::string getDate(std::chrono::system_clock::time_point time);
    static std::string getTime(std::chrono::system_clock::time_point time);

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

class IVW_QTEDITOR_API LogTableModel : public QAbstractTableModel {
public:
    LogTableModel();

    QString getName(LogTableModelEntry::ColumnID ind) const;

    void clear();
    void log(std::vector<LogTableModelEntry>& entries);

    virtual int rowCount(const QModelIndex& = QModelIndex()) const override {
        return static_cast<int>(entries_.size());
    }
    virtual int columnCount(const QModelIndex& = QModelIndex()) const override {
        return static_cast<int>(LogTableModelEntry::size());
    }
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role = Qt::DisplayRole) const override;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const override;

private:
    std::vector<LogTableModelEntry> entries_;
};

class IVW_QTEDITOR_API ConsoleWidget : public InviwoDockWidget, public Logger {
    Q_OBJECT
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
    void updateIndicators(LogLevel level);
    void clear();
    void onNewEntries();

signals:
    void clearSignal();
    void hasNewEntries();
    void scrollToBottom();

protected:
    virtual void keyPressEvent(QKeyEvent* keyEvent) override;
    virtual void closeEvent(QCloseEvent* event) override;

private:
    void logEntry(LogTableModelEntry);

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
    InviwoMainWindow* mainWindow_;
    std::shared_ptr<MenuItem> editActionsHandle_;

    std::mutex entriesMutex_;
    std::vector<LogTableModelEntry> newEntries_;
};

}  // namespace inviwo

Q_DECLARE_METATYPE(inviwo::LogTableModelEntry)
