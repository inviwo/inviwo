/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2023 Inviwo Foundation
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

#include <inviwo/core/common/inviwoapplication.h>             // for InviwoApplication
#include <inviwo/core/util/chronoutils.h>                     // for durationToString
#include <inviwo/core/util/clock.h>                           // for Clock
#include <inviwo/core/util/filedialogstate.h>                 // for FileMode, FileMode::AnyFile
#include <inviwo/core/util/filesystem.h>                      // for ifstream, ofstream
#include <inviwo/core/util/pathtype.h>                        // for PathType, PathType::Scripts
#include <inviwo/core/util/raiiutils.h>                       // for OnScopeExit, OnScopeExit::E...
#include <inviwo/core/util/stringconversion.h>                // for replaceInString
#include <modules/python3/python3module.h>                    // for Python3Module
#include <modules/python3/pythonexecutionoutputobservable.h>  // for PythonOutputType, PythonOut...
#include <modules/python3/pythoninterpreter.h>                // for PythonInterpreter
#include <modules/python3/pythonscript.h>                     // for PythonScript
#include <modules/python3qt/python3qtmodule.h>                // for Python3QtModule
#include <modules/python3qt/pythoneditorwidget.h>             // for PythonEditorWidget
#include <modules/python3qt/pythonsyntaxhighlight.h>          // for setPythonOutputSyntaxHighlight
#include <modules/qtwidgets/codeedit.h>                       // for CodeEdit
#include <modules/qtwidgets/editorfileobserver.h>             // for EditorFileObserver
#include <modules/qtwidgets/inviwodockwidget.h>               // for InviwoDockWidget
#include <modules/qtwidgets/inviwofiledialog.h>               // for InviwoFileDialog
#include <modules/qtwidgets/inviwoqtutils.h>                  // for fromQString, toQString, emToPx

#include <fstream>     // for operator<<, basic_ostream
#include <functional>  // for __base
#include <iterator>    // for istreambuf_iterator
#include <memory>      // for shared_ptr
#include <string>      // for char_traits, string, operator+
#include <vector>      // for vector

#include <QAction>          // for QAction
#include <QFileDialog>      // for QFileDialog, QFileDialog::D...
#include <QFlags>           // for QFlags
#include <QIcon>            // for QIcon, QIcon::Normal, QIcon...
#include <QKeySequence>     // for QKeySequence, QKeySequence:...
#include <QList>            // for QList
#include <QMainWindow>      // for QMainWindow
#include <QMessageBox>      // for QMessageBox, operator|, QMe...
#include <QObject>          // for QObject
#include <QPlainTextEdit>   // for QPlainTextEdit
#include <QSettings>        // for QSettings
#include <QSize>            // for QSize
#include <QSizeF>           // for QSizeF
#include <QSplitter>        // for QSplitter
#include <QSplitterHandle>  // for QSplitterHandle
#include <QStatusBar>       // for QStatusBar
#include <QString>          // for QString
#include <QStringList>      // for QStringList
#include <QTextDocument>    // for QTextDocument
#include <QToolBar>         // for QToolBar
#include <QVariant>         // for QVariant
#include <Qt>               // for operator|, WidgetWithChildr...

#include <fmt/format.h>
#include <fmt/std.h>

class QCloseEvent;
class QWidget;

namespace inviwo {

const static std::string defaultSource =
    "#Inviwo Python script \nimport inviwopy\n\n\napp = inviwopy.app\nnetwork = app.network\n";

PythonEditorWidget::PythonEditorWidget(QWidget* parent, InviwoApplication* app)
    : InviwoDockWidget(tr("Python Editor"), parent, "PythonEditorWidget")
    , infoTextColor_(153, 153, 153)
    , errorTextColor_(255, 107, 107)
    , runAction_(nullptr)
    , script_()
    , app_(app)
    , appendLog_(nullptr)
    , fileObserver_(this, "Python Editor") {
    setWindowIcon(QIcon(":/svgicons/python.svg"));

    mainWindow_ = new QMainWindow();
    mainWindow_->setContextMenuPolicy(Qt::NoContextMenu);
    QToolBar* toolBar = new QToolBar();
    mainWindow_->addToolBar(toolBar);
    toolBar->setFloatable(false);
    toolBar->setMovable(false);
    setWidget(mainWindow_);

    QSplitter* splitter = new QSplitter(nullptr);
    splitter->setOrientation(Qt::Vertical);

    auto settings = app->getSettingsByType<PythonSyntaxHighlight>();
    {

        pythonCode_ = new CodeEdit{this};
        codeCallbacks_ =
            utilqt::setPythonSyntaxHighlight(pythonCode_->syntaxHighlighter(), *settings);
        setDefaultText();
    }
    {
        pythonOutput_ = new CodeEdit{this};
        outputCallbacks_ =
            utilqt::setPythonOutputSyntaxHighlight(pythonOutput_->syntaxHighlighter(), *settings);
        pythonOutput_->setReadOnly(true);
        pythonOutput_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    }

    splitter->addWidget(pythonCode_);
    splitter->addWidget(pythonOutput_);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 0);
    splitter->setHandleWidth(2);
    // enable QSplitter:hover stylesheet
    // QTBUG-13768 https://bugreports.qt.io/browse/QTBUG-13768
    splitter->handle(1)->setAttribute(Qt::WA_Hover);
    mainWindow_->setCentralWidget(splitter);

    QObject::connect(pythonCode_, &CodeEdit::textChanged, this, &PythonEditorWidget::onTextChange);
    QObject::connect(pythonCode_, &CodeEdit::modificationChanged, this,
                     [this](bool b) { setWindowModified(b); });
    pythonCode_->installEventFilter(&fileObserver_);

    fileObserver_.setReloadFileCallback([this]() { readFile(); });
    fileObserver_.setModifiedCallback([this](bool m) { pythonCode_->document()->setModified(m); });

    {

        QIcon icon;
        icon.addFile(":/svgicons/run-script.svg", QSize(), QIcon::Normal, QIcon::Off);
        icon.addFile(":/svgicons/stop-script.svg", QSize(), QIcon::Normal, QIcon::On);

        runAction_ = toolBar->addAction(icon, "Compile and Run");
        runAction_->setCheckable(true);
        runAction_->setShortcut(QKeySequence(tr("F5")));
        runAction_->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        runAction_->setToolTip(
            "Compile and Run Script. To abort evaluation 'inviwopy.qt.update()' needs to called "
            "periodically.");
        mainWindow_->addAction(runAction_);
        connect(runAction_, &QAction::triggered, this, [this](bool run) { runOrStop(run); });
    }
    {
        auto action = toolBar->addAction(QIcon(":/svgicons/newfile.svg"), tr("&New Script"));
        action->setShortcut(QKeySequence::New);
        action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        action->setToolTip("New Script");
        mainWindow_->addAction(action);
        connect(action, &QAction::triggered, this, [this]() { setDefaultText(); });
    }
    {
        auto action = toolBar->addAction(QIcon(":/svgicons/open.svg"), tr("&Open Script"));
        action->setShortcut(QKeySequence::Open);
        action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        action->setToolTip("Open Script");
        mainWindow_->addAction(action);
        connect(action, &QAction::triggered, this, [this]() { open(); });
    }

    {
        auto action = toolBar->addAction(QIcon(":/svgicons/save.svg"), tr("&Save Script"));
        action->setShortcut(QKeySequence::Save);
        action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        action->setToolTip("Save Script");
        mainWindow_->addAction(action);
        connect(action, &QAction::triggered, this, [this]() { save(); });
    }
    {
        auto action = toolBar->addAction(QIcon(":/svgicons/save-as.svg"), tr("&Save Script As..."));
        action->setShortcut(QKeySequence::SaveAs);
        action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        action->setToolTip("Save Script As...");
        mainWindow_->addAction(action);
        connect(action, &QAction::triggered, this, [this]() { saveAs(); });
    }
    {
        toolBar->addSeparator();
        auto undo = toolBar->addAction(QIcon(":/svgicons/undo.svg"), "&Undo");
        undo->setShortcut(QKeySequence::Undo);
        undo->setEnabled(false);
        QObject::connect(undo, &QAction::triggered, this, [this]() { pythonCode_->undo(); });
        QObject::connect(pythonCode_, &QPlainTextEdit::undoAvailable, undo, &QAction::setEnabled);

        auto redo = toolBar->addAction(QIcon(":/svgicons/redo.svg"), "&Redo");
        redo->setShortcut(QKeySequence::Redo);
        redo->setEnabled(false);
        QObject::connect(redo, &QAction::triggered, this, [this]() { pythonCode_->redo(); });
        QObject::connect(pythonCode_, &QPlainTextEdit::redoAvailable, redo, &QAction::setEnabled);
    }
    {
        toolBar->addSeparator();
        QIcon icon;
        icon.addFile(":/svgicons/log-append.svg", QSize(), QIcon::Normal, QIcon::On);
        icon.addFile(":/svgicons/log-clear-on-run.svg", QSize(), QIcon::Normal, QIcon::Off);

        appendLog_ = toolBar->addAction(icon, "Append Log");
        appendLog_->setShortcut(Qt::CTRL | Qt::Key_A);
        appendLog_->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        appendLog_->setCheckable(true);
        appendLog_->setChecked(true);
        appendLog_->setToolTip("Append Log");
        mainWindow_->addAction(appendLog_);
        connect(appendLog_, &QAction::toggled, this, [this](bool toggle) {
            // update tooltip and menu entry
            QString tglstr = (toggle ? "Append Log" : "Clear Log on Run");
            appendLog_->setText(tglstr);
            appendLog_->setToolTip(tglstr);
        });
    }
    {
        auto action = toolBar->addAction(QIcon(":/svgicons/log-clear.svg"), "Clear Log Output");
        action->setShortcut(Qt::CTRL | Qt::Key_E);
        action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        action->setToolTip("Clear Log Output");
        mainWindow_->addAction(action);
        connect(action, &QAction::triggered, [this]() { clearOutput(); });
    }

    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    resize(utilqt::emToPx(this, QSizeF(50, 70)));  // default size
    updateWindowTitle();
}

PythonEditorWidget::~PythonEditorWidget() = default;

void PythonEditorWidget::closeEvent(QCloseEvent* event) {
    if (pythonCode_->document()->isModified()) {
        QMessageBox msgBox(QMessageBox::Question, "Python Editor",
                           "Do you want to save unsaved changes?",
                           QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, this);

        int retval = msgBox.exec();
        if (retval == static_cast<int>(QMessageBox::Save)) {
            save();
        } else if (retval == static_cast<int>(QMessageBox::Cancel)) {
            event->ignore();
            return;
        } else {
            pythonCode_->document()->setModified(false);
        }
    }
    InviwoDockWidget::closeEvent(event);
}

void PythonEditorWidget::appendToOutput(const std::string& msg, bool) {
    pythonOutput_->appendPlainText(utilqt::toQString(msg));
}

void PythonEditorWidget::loadFile(const std::filesystem::path& fileName, bool askForSave) {
    if (askForSave && pythonCode_->document()->isModified()) {
        QMessageBox msgBox(QMessageBox::Question, "Python Editor",
                           "Do you want to save unsaved changes?",
                           QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, this);

        int retval = msgBox.exec();
        if (retval == static_cast<int>(QMessageBox::Save)) {
            save();
        } else if (retval == static_cast<int>(QMessageBox::Cancel)) {
            return;
        }
    }

    setFileName(fileName);
    readFile();
}

void PythonEditorWidget::onPyhonExecutionOutput(const std::string& msg,
                                                PythonOutputType outputType) {
    appendToOutput(msg, outputType != PythonOutputType::sysstdout);
}

void PythonEditorWidget::save() {
    if (script_.getSource() == defaultSource) return;  // nothing to be saved

    if (scriptFileName_.empty()) {
        saveAs();
    } else if (pythonCode_->document()->isModified()) {
        fileObserver_.ignoreNextUpdate();

        auto file = std::ofstream(scriptFileName_);
        file << utilqt::fromQString(pythonCode_->toPlainText());
        file.close();

        pythonCode_->document()->setModified(false);
        mainWindow_->statusBar()->showMessage(
            utilqt::toQString(fmt::format("Saved {}", scriptFileName_)));
    }
}

void PythonEditorWidget::readFile() {
    auto file = std::ifstream(scriptFileName_);
    std::string text((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    replaceInString(text, "\t", "    ");
    pythonCode_->setPlainText(utilqt::toQString(text));
    script_.setSource(text);
    script_.setFilename(scriptFileName_);
}

void PythonEditorWidget::saveAs() {
    InviwoFileDialog saveFileDialog(this, "Save Python Script ...", "script");

    saveFileDialog.setFileMode(FileMode::AnyFile);
    saveFileDialog.setAcceptMode(AcceptMode::Save);
    saveFileDialog.setOption(QFileDialog::Option::DontConfirmOverwrite, false);

    saveFileDialog.addSidebarPath(PathType::Scripts);
    saveFileDialog.addExtension("py", "Python Script");

    if (saveFileDialog.exec()) {
        QString path = saveFileDialog.selectedFiles().at(0);

        if (!path.endsWith(".py")) path.append(".py");

        setFileName(utilqt::fromQString(path));
        save();
    }
}

void PythonEditorWidget::open() {
    if (!askSaveChanges()) {
        return;
    }

    InviwoFileDialog openFileDialog(this, "Open Python Script ...", "script");
    openFileDialog.setFileMode(FileMode::AnyFile);
    openFileDialog.addSidebarPath(PathType::Scripts);
    openFileDialog.addExtension("py", "Python Script");

    if (openFileDialog.exec()) {
        loadFile(utilqt::fromQString(openFileDialog.selectedFiles().at(0)), false);
    }
}

void PythonEditorWidget::runOrStop(bool runScript) {
    if (runScript) {
        run();
    } else {
        stop();
    }
}

void PythonEditorWidget::run() {
    app_->getModuleByType<Python3Module>()->getPythonInterpreter()->addObserver(this);

    util::OnScopeExit reenable([&]() {
        runAction_->setChecked(false);
        app_->getModuleByType<Python3Module>()->getPythonInterpreter()->removeObserver(this);
    });

    if (!appendLog_->isChecked()) {
        clearOutput();
    }

    // save the current code to QSettings such that it can be recovered in case of a crash or if the
    // user forgets to save it as a file.
    saveState();

    mainWindow_->statusBar()->showMessage("Running ...");
    Clock c;
    c.start();
    bool ok = script_.run();
    c.stop();

    std::stringstream ss;
    ss << (ok ? "Executed Successfully" : "Failed");
    ss << " (" << util::durationToString(c.getElapsedTime()) << ")";
    mainWindow_->statusBar()->showMessage(utilqt::toQString(ss.str()));
}

void PythonEditorWidget::stop() {
    app_->getModuleByType<Python3QtModule>()->abortPythonEvaluation();
}

void PythonEditorWidget::show() {
    InviwoDockWidget::show();
    raise();
    setVisible(true);
}

void PythonEditorWidget::setDefaultText() {
    if (!askSaveChanges()) {
        return;
    }

    pythonCode_->setPlainText(utilqt::toQString(defaultSource));
    script_.setSource(defaultSource);
    setFileName("");
}

void PythonEditorWidget::clearOutput() { pythonOutput_->setPlainText(""); }

void PythonEditorWidget::onTextChange() {
    std::string source = utilqt::fromQString(pythonCode_->toPlainText());
    script_.setSource(source);
}

void PythonEditorWidget::setFileName(const std::filesystem::path& filename) {
    scriptFileName_ = filename;
    script_.setFilename(filename);
    fileObserver_.setFileName(filename);
    updateWindowTitle();
}

void PythonEditorWidget::updateWindowTitle() {
    QString str;
    if (scriptFileName_.empty()) {
        str = "(unnamed file)";
    } else {
        str = QString::fromStdString(scriptFileName_);
    }

    setWindowTitle(QString("Python Editor - %1[*]").arg(str));
}

void PythonEditorWidget::saveState() {
    InviwoDockWidget::saveState();

    QSettings settings;
    settings.beginGroup(objectName());
    settings.setValue("appendLog", appendLog_->isChecked());
    settings.setValue("lastScript", utilqt::toQString(scriptFileName_));
    settings.setValue("source", pythonCode_->toPlainText());
    settings.endGroup();
}

void PythonEditorWidget::loadState() {
    InviwoDockWidget::loadState();

    // Restore state
    QSettings settings;
    settings.beginGroup(objectName());
    QString lastFile = settings.value("lastScript", "").toString();
    // If the script was saved to disk, the load it, else we use the script that was in the
    // editor at last script execution or when inviwo closed (CloseEvent)
    if (!lastFile.isEmpty()) {
        loadFile(utilqt::fromQString(lastFile), false);
    } else {
        QString src = settings.value("source", "").toString();
        if (src.length() != 0) {
            pythonCode_->setPlainText(src);
            script_.setSource(utilqt::fromQString(src));
        }
    }

    auto append = settings.value("appendLog", appendLog_->isCheckable()).toBool();
    appendLog_->setChecked(append);

    settings.endGroup();
}

bool PythonEditorWidget::askSaveChanges() {
    if (pythonCode_->document()->isModified()) {
        QMessageBox msgBox(QMessageBox::Question, "Python Editor",
                           "Do you want to save unsaved changes?",
                           QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, this);

        int retval = msgBox.exec();
        if (retval == static_cast<int>(QMessageBox::Save)) {
            save();
        } else if (retval == static_cast<int>(QMessageBox::Cancel)) {
            return false;
        }
    }
    return true;
}

}  // namespace inviwo
