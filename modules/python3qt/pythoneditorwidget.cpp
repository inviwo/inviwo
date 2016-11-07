/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2016 Inviwo Foundation
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

#include <modules/python3/pythonincluder.h>
#include <modules/python3qt/pythoneditorwidget.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/util/logcentral.h>
#include <inviwo/core/util/filesystem.h>
#include <modules/python3/python3module.h>
#include <inviwo/core/util/clock.h>
#include <inviwo/qt/widgets/properties/syntaxhighlighter.h>
#include <inviwo/qt/editor/inviwomainwindow.h>

#include <inviwo/qt/widgets/inviwofiledialog.h>
#include <inviwo/core/util/settings/systemsettings.h>
#include <modules/python3/pyinviwo.h>

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
#include <warn/pop>

namespace inviwo {


const static std::string defaultSource =
"#Inviwo Python script \nimport inviwo \nimport inviwoqt \n\nhelp('inviwo') \nhelp('inviwoqt') "
    "\n";

PythonEditorWidget::PythonEditorWidget(InviwoMainWindow* ivwwin, InviwoApplication* app)
    : InviwoDockWidget(tr("Python Editor"), ivwwin)
    , settings_("Inviwo", "Inviwo")
    , infoTextColor_(153, 153, 153)
    , errorTextColor_(255, 107, 107)
    , runAction_(nullptr)
    , script_()
    , unsavedChanges_(false)
    , app_(app)
    , appendLog_(true)
{

    setObjectName("PythonEditor");
    settings_.beginGroup("PythonEditor");
    QString lastFile = settings_.value("lastScript", "").toString();
    appendLog_ = settings_.value("appendLog", appendLog_).toBool();
    settings_.endGroup();
    setVisible(false);
    setWindowIcon(QIcon(":/icons/python.png"));

    QMainWindow* mainWindow = new QMainWindow();
    mainWindow->setContextMenuPolicy(Qt::NoContextMenu);
    QToolBar* toolBar = new QToolBar();
    mainWindow->addToolBar(toolBar);
    toolBar->setFloatable(false);
    toolBar->setMovable(false);
    setWidget(mainWindow);

    {
        runAction_ = toolBar->addAction(QIcon(":/icons/python.png"), "Compile and Run");
        runAction_->setShortcut(QKeySequence(tr("F5")));
        runAction_->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        runAction_->setToolTip("Compile and Run Script");
        mainWindow->addAction(runAction_);
        connect(runAction_, &QAction::triggered, [this]() {run(); });
    }
    {
        auto action = toolBar->addAction(QIcon(":/icons/new.png"), tr("&New Script"));
        action->setShortcut(QKeySequence::New);
        action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        action->setToolTip("New Script");
        mainWindow->addAction(action);
        connect(action, &QAction::triggered, [this](){setDefaultText();});
    }
    {
        auto action = toolBar->addAction(QIcon(":/icons/open.png"), tr("&Open Script"));
        action->setShortcut(QKeySequence::Open);
        action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        action->setToolTip("Open Script");
        mainWindow->addAction(action);
        connect(action, &QAction::triggered, [this](){open();});
    }

    {
        auto action = toolBar->addAction(QIcon(":/icons/save.png"), tr("&Save Script"));
        action->setShortcut(QKeySequence::Save);
        action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        action->setToolTip("Save Script");
        mainWindow->addAction(action);
        connect(action, &QAction::triggered, [this](){save();});
    }
    {
        auto action = toolBar->addAction(QIcon(":/icons/saveas.png"), tr("&Save Script As..."));
        action->setShortcut(QKeySequence::SaveAs);
        action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        action->setToolTip("Save Script As...");
        mainWindow->addAction(action);
        connect(action, &QAction::triggered, [this](){saveAs();});
    }
    {
        QIcon icon;
        icon.addFile(":/icons/log-append.png", QSize(), QIcon::Normal, QIcon::On);
        icon.addFile(":/icons/log-clearonrun.png", QSize(), QIcon::Normal, QIcon::Off);

        QString str = (appendLog_ ? "Append Log" : "Clear Log on Run");
        auto action = toolBar->addAction(icon, str);
        action->setShortcut(Qt::ControlModifier + Qt::Key_E);
        action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        action->setCheckable(true);
        action->setChecked(appendLog_);
        action->setToolTip(appendLog_ ? "Append Log" : "Clear Log on Run");
        mainWindow->addAction(action);
        connect(action, &QAction::toggled, [this, action](bool toggle) { 
            appendLog_ = toggle; 
            // update tooltip and menu entry
            QString tglstr = (toggle ? "Append Log" : "Clear Log on Run");
            action->setText(tglstr);
            action->setToolTip(tglstr);
            // update settings
            settings_.beginGroup("PythonEditor");
            settings_.setValue("appendLog", appendLog_);
            settings_.endGroup();
        });
    }
    {
        auto action = toolBar->addAction(QIcon(":/icons/log-clear.png"), "Clear Log Output");
        action->setShortcut(Qt::ControlModifier + Qt::Key_E);
        action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        action->setToolTip("Clear Log Output");
        mainWindow->addAction(action);
        connect(action, &QAction::triggered, [this](){clearOutput();});
    }

    // Done creating buttons
    QSplitter* splitter = new QSplitter(nullptr);
    splitter->setOrientation(Qt::Vertical);
    pythonCode_ = new PythonTextEditor(nullptr);
    pythonCode_->setObjectName("pythonEditor");
    pythonCode_->setUndoRedoEnabled(true);
    setDefaultText();
    pythonOutput_ = new QTextEdit(nullptr);
    pythonOutput_->setObjectName("pythonConsole");
    pythonOutput_->setReadOnly(true);
    pythonOutput_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    syntaxHighligther_ =
        SyntaxHighligther::createSyntaxHighligther<Python>(pythonCode_->document());

    splitter->addWidget(pythonCode_);
    splitter->addWidget(pythonOutput_);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 0);
    splitter->setHandleWidth(2);
    // enable QSplitter:hover stylesheet
    // QTBUG-13768 https://bugreports.qt.io/browse/QTBUG-13768
    splitter->handle(1)->setAttribute(Qt::WA_Hover);
    mainWindow->setCentralWidget(splitter);

    QObject::connect(pythonCode_, SIGNAL(textChanged()), this, SLOT(onTextChange()));
    // close this window before the main window is closed
    QObject::connect(ivwwin, &InviwoMainWindow::closingMainWindow, [this]() { delete this; });

    this->updateStyle();

    
    this->resize(500, 700);

    if (app_) {
        app_->getSettingsByType<SystemSettings>()->pythonSyntax_.onChange(
            this, &PythonEditorWidget::updateStyle);
        app_->getSettingsByType<SystemSettings>()->pyFontSize_.onChange(
            this, &PythonEditorWidget::updateStyle);
        app_->registerFileObserver(this);
    }
    unsavedChanges_ = false;

    if (lastFile.size() != 0) loadFile(lastFile.toLocal8Bit().constData(), false);

    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    setFloating(true);
}


void PythonEditorWidget::updateStyle() {
    auto color = InviwoApplication::getPtr()->getSettingsByType<SystemSettings>()->pyBGColor_.get();
    auto size = InviwoApplication::getPtr()->getSettingsByType<SystemSettings>()->pyFontSize_.get();
    std::stringstream ss;
    ss << "background-color: rgb(" << color.r << ", " << color.g << ", " << color.b << ");"
       << std::endl;
    ss << "font-size: " << size << "px;";
    pythonCode_->setStyleSheet(ss.str().c_str());
    syntaxHighligther_->rehighlight();
}

PythonEditorWidget::~PythonEditorWidget() {
    if (unsavedChanges_) {
        int ret =
            QMessageBox::question(this, "Python Editor", "Do you want to save unsaved changes?",
                                     "Save", "Discard");
        if (ret == 0) save();
    }
}

void PythonEditorWidget::appendToOutput(const std::string& msg, bool error) {
    pythonOutput_->setTextColor(error ? errorTextColor_ : infoTextColor_);
    pythonOutput_->append(msg.c_str());
}

void PythonEditorWidget::fileChanged(const std::string& fileName) {
    std::string msg = "The file " + filesystem::getFileNameWithExtension(scriptFileName_) +
                      " has been modified outside of Inwivo, do you want to reload its contents";
    int ret = QMessageBox::question(this, "Python Editor", msg.c_str(), "Yes", "No");

    if (ret == 0) {  // yes
        readFile();
    } else {
        unsavedChanges_ = true;
        // set code change to true so that we can quick save without making more changes
    }
}

void PythonEditorWidget::loadFile(std::string fileName, bool askForSave) {
    if (askForSave && unsavedChanges_) {
        int ret =
            QMessageBox::question(this, "Python Editor", "Do you want to save unsaved changes?",
                                     "Save", "Discard", "Cancel");

        if (ret == 0) save();

        if (ret == 2)  // Cancel
            return;
    }
    setFileName(fileName);    
    settings_.beginGroup("PythonEditor");
    settings_.setValue("lastScript", scriptFileName_.c_str());
    settings_.endGroup();
    readFile();
}

void PythonEditorWidget::onPyhonExecutionOutput(const std::string& msg,
                                                const PythonExecutionOutputStream& outputType) {
    appendToOutput(msg, outputType != sysstdout);
}

void PythonEditorWidget::save() {
    if (script_.getSource() == defaultSource) return;  // nothing to be saved

    if (scriptFileName_.size() == 0) {
        saveAs();
    } else if (unsavedChanges_) {
        stopFileObservation(scriptFileName_);
        std::ofstream file(scriptFileName_.c_str());
        file << pythonCode_->toPlainText().toLocal8Bit().constData();
        file.close();
        startFileObservation(scriptFileName_);
        LogInfo("Python Script saved(" << scriptFileName_ << ")");
        settings_.beginGroup("PythonEditor");
        settings_.setValue("lastScript", scriptFileName_.c_str());
        settings_.endGroup();
        unsavedChanges_ = false;
        updateTitleBar();
    }
}

void PythonEditorWidget::readFile() {
    std::ifstream file(scriptFileName_.c_str());
    std::string text((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    replaceInString(text, "\t", "    ");
    pythonCode_->setPlainText(text.c_str());
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

void PythonTextEditor::keyPressEvent(QKeyEvent* keyEvent) {
    if (keyEvent->key() == Qt::Key_Tab) {
        keyEvent->accept();
        insertPlainText("    ");
    } else {
        QPlainTextEdit::keyPressEvent(keyEvent);
    }
}

void PythonEditorWidget::saveAs() {
    InviwoFileDialog saveFileDialog(this, "Save Python Script ...", "script");

    saveFileDialog.setFileMode(FileMode::AnyFile);
    saveFileDialog.setAcceptMode(AcceptMode::Save);
    saveFileDialog.setConfirmOverwrite(true);

    saveFileDialog.addSidebarPath(PathType::Scripts);

    saveFileDialog.addExtension("py", "Python Script");

    if (saveFileDialog.exec()) {
        stopFileObservation(scriptFileName_);
        QString path = saveFileDialog.selectedFiles().at(0);

        if (!path.endsWith(".py")) path.append(".py");

        setFileName(path.toLocal8Bit().constData());
        unsavedChanges_ = true;
        save();
    }
}

void PythonEditorWidget::open() {
    if (unsavedChanges_) {
        int ret =
            QMessageBox::information(this, "Python Editor", "Do you want to save unsaved changes?",
                                     "Save", "Discard", "Cancel");

        if (ret == 0) save();

        if (ret == 2)  // Cancel
            return;
    }

    InviwoFileDialog openFileDialog(this, "Open Python Script ...", "script");

    openFileDialog.setFileMode(FileMode::AnyFile);

    openFileDialog.addSidebarPath(PathType::Scripts);

    openFileDialog.addExtension("py", "Python Script");

    if (openFileDialog.exec()) {
        unsavedChanges_ = false;
        stopFileObservation(scriptFileName_);
        setFileName(openFileDialog.selectedFiles().at(0).toLocal8Bit().constData());
        settings_.beginGroup("PythonEditor");
        settings_.setValue("lastScript", scriptFileName_.c_str());
        settings_.endGroup();
        startFileObservation(scriptFileName_);
        readFile();
    }
}

void PythonEditorWidget::run() {
    if (unsavedChanges_ && scriptFileName_.size() != 0)  // save if needed
        save();

    runAction_->setDisabled(true);
    util::OnScopeExit reenable([&]() {
        runAction_->setEnabled(true);
    });
    PyInviwo::getPtr()->addObserver(this);
    if (!appendLog_) {
        clearOutput();
    }

    app_->getInteractionStateManager().beginInteraction();
    Clock c;
    c.start();
    bool ok = script_.run();
    c.tick();

    if (ok) {
        LogInfo("Python Script Executed successfully");
    }

    LogInfo("Execution time: " << c.getElapsedMiliseconds() << " ms");
    app_->getInteractionStateManager().endInteraction();
    PyInviwo::getPtr()->removeObserver(this);
}

void PythonEditorWidget::show() {
    InviwoDockWidget::show();
    raise();
    updateStyle();
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

    pythonCode_->setPlainText(defaultSource.c_str());
    unsavedChanges_ = false;
    script_.setFilename("");
    script_.setSource(defaultSource);
    stopFileObservation(scriptFileName_);
    setFileName("");
    settings_.beginGroup("PythonEditor");
    settings_.setValue("lastScript", scriptFileName_.c_str());
    settings_.endGroup();
}

void PythonEditorWidget::clearOutput() { pythonOutput_->setText(""); }

void PythonEditorWidget::onTextChange() {
    std::string source = pythonCode_->toPlainText().toLocal8Bit().constData();

    script_.setSource(source);
    unsavedChanges_ = true;
    updateTitleBar();
}

void PythonEditorWidget::setFileName(const std::string filename) {
    scriptFileName_ = filename;
    updateTitleBar();
}

void PythonEditorWidget::updateTitleBar() {
    QString str;
    if (scriptFileName_.empty()) {
        str = "(unnamed file)";
    }
    else {
        str = QString::fromStdString(scriptFileName_);
    }

    updateWindowTitle(QString("Python Editor - %1%2").arg(str).arg(unsavedChanges_ ? "*" : ""));
}

}  // namespace
