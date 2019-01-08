/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2018 Inviwo Foundation
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

#include <pybind11/pybind11.h>
#include <modules/python3qt/pythoneditorwidget.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/util/logcentral.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/clock.h>
#include <inviwo/core/util/stringconversion.h>

#include <modules/qtwidgets/inviwoqtutils.h>
#include <modules/qtwidgets/inviwofiledialog.h>
#include <modules/qtwidgets/qtwidgetssettings.h>

#include <modules/python3/python3module.h>
#include <modules/python3/pythoninterpreter.h>

#include <modules/qtwidgets/codeedit.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QCommandLinkButton>
#include <QSplitter>
#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QButtonGroup>
#include <QToolBar>
#include <QMainWindow>
#include <QMenuBar>
#include <QSpacerItem>
#include <QHBoxLayout>
#include <QFrame>
#include <QPalette>
#include <QStatusBar>
#include <warn/pop>

#include <sstream>

namespace inviwo {

const static std::string defaultSource =
    "#Inviwo Python script \nimport inviwopy\n\n\napp = inviwopy.app\nnetwork = app.network\n";

PythonEditorWidget::PythonEditorWidget(QWidget* parent, InviwoApplication* app)
    : InviwoDockWidget(tr("Python Editor"), parent, "PythonEditorWidget")
    , infoTextColor_(153, 153, 153)
    , errorTextColor_(255, 107, 107)
    , runAction_(nullptr)
    , script_()
    , unsavedChanges_(false)
    , app_(app)
    , appendLog_(nullptr) {
    setWindowIcon(QIcon(":/icons/python.png"));

    mainWindow_ = new QMainWindow();
    mainWindow_->setContextMenuPolicy(Qt::NoContextMenu);
    QToolBar* toolBar = new QToolBar();
    mainWindow_->addToolBar(toolBar);
    toolBar->setFloatable(false);
    toolBar->setMovable(false);
    setWidget(mainWindow_);
    mainWindow_->statusBar();

    {
        runAction_ = toolBar->addAction(QIcon(":/icons/python.png"), "Compile and Run");
        runAction_->setShortcut(QKeySequence(tr("F5")));
        runAction_->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        runAction_->setToolTip("Compile and Run Script");
        mainWindow_->addAction(runAction_);
        connect(runAction_, &QAction::triggered, [this]() { run(); });
    }
    {
        auto action = toolBar->addAction(QIcon(":/icons/newfile.png"), tr("&New Script"));
        action->setShortcut(QKeySequence::New);
        action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        action->setToolTip("New Script");
        mainWindow_->addAction(action);
        connect(action, &QAction::triggered, [this]() { setDefaultText(); });
    }
    {
        auto action = toolBar->addAction(QIcon(":/icons/open.png"), tr("&Open Script"));
        action->setShortcut(QKeySequence::Open);
        action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        action->setToolTip("Open Script");
        mainWindow_->addAction(action);
        connect(action, &QAction::triggered, [this]() { open(); });
    }

    {
        auto action = toolBar->addAction(QIcon(":/icons/save.png"), tr("&Save Script"));
        action->setShortcut(QKeySequence::Save);
        action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        action->setToolTip("Save Script");
        mainWindow_->addAction(action);
        connect(action, &QAction::triggered, [this]() { save(); });
    }
    {
        auto action = toolBar->addAction(QIcon(":/icons/saveas.png"), tr("&Save Script As..."));
        action->setShortcut(QKeySequence::SaveAs);
        action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        action->setToolTip("Save Script As...");
        mainWindow_->addAction(action);
        connect(action, &QAction::triggered, [this]() { saveAs(); });
    }
    {
        QIcon icon;
        icon.addFile(":/icons/log-append.png", QSize(), QIcon::Normal, QIcon::On);
        icon.addFile(":/icons/log-clearonrun.png", QSize(), QIcon::Normal, QIcon::Off);

        appendLog_ = toolBar->addAction(icon, "Append Log");
        appendLog_->setShortcut(Qt::ControlModifier + Qt::Key_E);
        appendLog_->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        appendLog_->setCheckable(true);
        appendLog_->setChecked(true);
        appendLog_->setToolTip("Append Log");
        mainWindow_->addAction(appendLog_);
        connect(appendLog_, &QAction::toggled, [this](bool toggle) {
            // update tooltip and menu entry
            QString tglstr = (toggle ? "Append Log" : "Clear Log on Run");
            appendLog_->setText(tglstr);
            appendLog_->setToolTip(tglstr);
        });
    }
    {
        auto action = toolBar->addAction(QIcon(":/icons/log-clear.png"), "Clear Log Output");
        action->setShortcut(Qt::ControlModifier + Qt::Key_E);
        action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        action->setToolTip("Clear Log Output");
        mainWindow_->addAction(action);
        connect(action, &QAction::triggered, [this]() { clearOutput(); });
    }

    // Done creating buttons
    QSplitter* splitter = new QSplitter(nullptr);
    splitter->setOrientation(Qt::Vertical);
    pythonCode_ = new CodeEdit{Python};

    setDefaultText();
    pythonOutput_ = new CodeEdit(None, nullptr);
    pythonOutput_->setReadOnly(true);
    pythonOutput_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

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
    pythonCode_->installEventFilter(this);

    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    resize(QSize(500, 700));  // default size

    unsavedChanges_ = false;
}

PythonEditorWidget::~PythonEditorWidget() = default;

void PythonEditorWidget::closeEvent(QCloseEvent* event) {
    if (unsavedChanges_) {
        int ret = QMessageBox::question(this, "Python Editor",
                                        "Do you want to save unsaved changes?", "Save", "Discard");
        if (ret == 0) save();
    }
    InviwoDockWidget::closeEvent(event);
}

bool PythonEditorWidget::eventFilter(QObject* obj, QEvent* event) {
    if (event->type() == QEvent::FocusIn) {
        if (fileChangedInBackground_) {
            queryReloadFile();
        }
        return false;
    } else {
        // standard event processing
        return QObject::eventFilter(obj, event);
    }
}

void PythonEditorWidget::focusInEvent(QFocusEvent* event) {
    InviwoDockWidget::focusInEvent(event);
    if (fileChangedInBackground_) {
        queryReloadFile();
    }
}

void PythonEditorWidget::appendToOutput(const std::string& msg, bool) {
    pythonOutput_->appendPlainText(utilqt::toQString(msg));
}

void PythonEditorWidget::fileChanged(const std::string& /*fileName*/) {
    if (!fileChangedInBackground_) {
        fileChangedInBackground_ = true;
        queryReloadFile();
    }
}

void PythonEditorWidget::loadFile(std::string fileName, bool askForSave) {
    if (askForSave && unsavedChanges_) {
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

    stopFileObservation(scriptFileName_);
    setFileName(fileName);
    startFileObservation(scriptFileName_);
    readFile();
}

void PythonEditorWidget::queryReloadFile() {
    if (this->hasFocus() && fileChangedInBackground_ && !reloadQueryInProgress_) {
        reloadQueryInProgress_ = true;
        std::string msg =
            "The file " + filesystem::getFileNameWithExtension(scriptFileName_) +
            " has been modified outside of Inwivo, do you want to reload its contents?";

        QMessageBox msgBox(QMessageBox::Question, "Python Editor", utilqt::toQString(msg),
                           QMessageBox::Yes | QMessageBox::No, this);
        msgBox.setWindowModality(Qt::WindowModal);

        if (msgBox.exec() == static_cast<int>(QMessageBox::Yes)) {
            readFile();
        } else {
            unsavedChanges_ = true;
            // set code change to true so that we can quick save without making more changes
        }
        fileChangedInBackground_ = false;
        reloadQueryInProgress_ = false;
    }
}

void PythonEditorWidget::onPyhonExecutionOutput(const std::string& msg,
                                                PythonOutputType outputType) {
    appendToOutput(msg, outputType != PythonOutputType::sysstdout);
}

void PythonEditorWidget::save() {
    if (script_.getSource() == defaultSource) return;  // nothing to be saved

    if (scriptFileName_.empty()) {
        saveAs();
    } else if (unsavedChanges_) {
        stopFileObservation(scriptFileName_);

        auto file = filesystem::ofstream(scriptFileName_);
        file << utilqt::fromQString(pythonCode_->toPlainText());
        file.close();

        startFileObservation(scriptFileName_);
        mainWindow_->statusBar()->showMessage(utilqt::toQString("Saved " + scriptFileName_));
        unsavedChanges_ = false;
        updateTitleBar();
    }
}

void PythonEditorWidget::readFile() {
    auto file = filesystem::ifstream(scriptFileName_);
    std::string text((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    replaceInString(text, "\t", "    ");
    pythonCode_->setPlainText(utilqt::toQString(text));
    script_.setSource(text);
    script_.setFilename(scriptFileName_);
    unsavedChanges_ = false;
    updateTitleBar();
}

bool PythonEditorWidget::hasFocus() const {
    if (InviwoDockWidget::hasFocus()) return true;

    if (pythonCode_->hasFocus()) return true;

    return false;
}

void PythonEditorWidget::saveAs() {
    InviwoFileDialog saveFileDialog(this, "Save Python Script ...", "script");

    saveFileDialog.setFileMode(FileMode::AnyFile);
    saveFileDialog.setAcceptMode(AcceptMode::Save);
    saveFileDialog.setConfirmOverwrite(true);
    saveFileDialog.addSidebarPath(PathType::Scripts);
    saveFileDialog.addExtension("py", "Python Script");

    if (saveFileDialog.exec()) {
        QString path = saveFileDialog.selectedFiles().at(0);

        if (!path.endsWith(".py")) path.append(".py");

        setFileName(utilqt::fromQString(path));
        unsavedChanges_ = true;
        save();
    }
}

void PythonEditorWidget::open() {
    if (unsavedChanges_) {
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

    InviwoFileDialog openFileDialog(this, "Open Python Script ...", "script");
    openFileDialog.setFileMode(FileMode::AnyFile);
    openFileDialog.addSidebarPath(PathType::Scripts);
    openFileDialog.addExtension("py", "Python Script");

    if (openFileDialog.exec()) {
        unsavedChanges_ = false;
        loadFile(utilqt::fromQString(openFileDialog.selectedFiles().at(0)), false);
    }
}

void PythonEditorWidget::run() {
    runAction_->setDisabled(true);
    util::OnScopeExit reenable([&]() { runAction_->setEnabled(true); });
    app_->getModuleByType<Python3Module>()->getPythonInterpreter()->addObserver(this);
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
    ss << " (" << c.getElapsedMilliseconds() << " ms)";
    mainWindow_->statusBar()->showMessage(utilqt::toQString(ss.str()));

    app_->getModuleByType<Python3Module>()->getPythonInterpreter()->removeObserver(this);
}

void PythonEditorWidget::show() {
    InviwoDockWidget::show();
    raise();
    setVisible(true);
}

void PythonEditorWidget::setDefaultText() {
    if (unsavedChanges_) {
        int ret =
            QMessageBox::information(this, "Python Editor", "Do you want to save unsaved changes?",
                                     "Save", "Discard Changes", "Cancel");

        if (ret == 0)
            save();
        else if (ret == 2)  // cancel
            return;
    }

    pythonCode_->setPlainText(utilqt::toQString(defaultSource));
    unsavedChanges_ = false;
    script_.setSource(defaultSource);
    stopFileObservation(scriptFileName_);
    setFileName("");
}

void PythonEditorWidget::clearOutput() { pythonOutput_->setPlainText(""); }

void PythonEditorWidget::onTextChange() {
    std::string source = utilqt::fromQString(pythonCode_->toPlainText());

    script_.setSource(source);
    unsavedChanges_ = true;
    updateTitleBar();
}

void PythonEditorWidget::setFileName(const std::string& filename) {
    scriptFileName_ = filename;
    script_.setFilename(filename);
    updateTitleBar();
}

void PythonEditorWidget::updateTitleBar() {
    QString str;
    if (scriptFileName_.empty()) {
        str = "(unnamed file)";
    } else {
        str = QString::fromStdString(scriptFileName_);
    }

    setWindowTitle(QString("Python Editor - %1%2").arg(str).arg(unsavedChanges_ ? "*" : ""));
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

}  // namespace inviwo
