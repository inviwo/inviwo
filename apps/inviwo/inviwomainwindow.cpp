/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#include "inviwomainwindow.h"
#include <inviwo/core/network/processornetworkevaluator.h>
#include <inviwo/qt/editor/networkeditorview.h>
#include <inviwo/qt/widgets/inviwoapplicationqt.h>
#include <inviwo/qt/widgets/propertylistwidget.h>
#include <inviwo/qt/editor/processorlistwidget.h>
#include <inviwo/core/util/commandlineparser.h>
#include <inviwo/core/util/settings/systemsettings.h>
#include <inviwo/qt/editor/resourcemanagerwidget.h>
#include <inviwo/qt/editor/consolewidget.h>
#include <inviwo/qt/editor/settingswidget.h>
#include <inviwo/qt/editor/helpwidget.h>

#include <inviwo/qt/widgets/inviwofiledialog.h>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QStandardPaths>
#include <QScreen>
#else
#include <QDesktopServices>
#endif
#include <QActionGroup>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QList>
#include <QMessageBox>
#include <QSettings>
#include <QUrl>
#include <QVariant>

#ifdef IVW_PYTHON2_QT
#define IVW_PYTHON_QT
//#include <modules/python2qt/pythoneditorwidget.h>
#include <modules/pythonqt/pythoneditorwidget.h>
#elif IVW_PYTHON3_QT
#define IVW_PYTHON_QT
#include <modules/python3qt/pythoneditorwidget.h>
#endif

namespace inviwo {

InviwoMainWindow::InviwoMainWindow()
    : QMainWindow()
    , ProcessorNetworkObserver()
    , visibilityModeProperty_(NULL) {
    NetworkEditor::init();
    networkEditor_ = NetworkEditor::getPtr();
    // initialize console widget first to receive log messages
    consoleWidget_ = new ConsoleWidget(this);
    // LogCentral takes ownership of logger
    LogCentral::getPtr()->registerLogger(consoleWidget_);
    currentWorkspaceFileName_ = "";
}

InviwoMainWindow::~InviwoMainWindow() {
    deinitialize();
    NetworkEditor::deleteInstance();
    LogCentral::getPtr()->unregisterLogger(consoleWidget_);
}

void InviwoMainWindow::initialize() {
    networkEditorView_ = new NetworkEditorView(networkEditor_, this);
    NetworkEditorObserver::addObservation(getNetworkEditor());
    setCentralWidget(networkEditorView_);

    resourceManagerWidget_ = new ResourceManagerWidget(this);
    addDockWidget(Qt::LeftDockWidgetArea, resourceManagerWidget_);
    resourceManagerWidget_->hide();

    settingsWidget_ = new SettingsWidget(this);
    addDockWidget(Qt::LeftDockWidgetArea, settingsWidget_);
    settingsWidget_->hide();

    helpWidget_ = new HelpWidget(this);
    processorTreeWidget_ = new ProcessorTreeWidget(this, helpWidget_);
    addDockWidget(Qt::LeftDockWidgetArea, processorTreeWidget_);
    addDockWidget(Qt::LeftDockWidgetArea, helpWidget_, Qt::Vertical);

    propertyListWidget_ = new PropertyListWidget(this);
    addDockWidget(Qt::RightDockWidgetArea, propertyListWidget_);
    networkEditor_->setPropertyListWidget(propertyListWidget_);

    addDockWidget(Qt::BottomDockWidgetArea, consoleWidget_);
    // load settings and restore window state
    QSettings settings("Inviwo", "Inviwo");
    settings.beginGroup("mainwindow");
    restoreGeometry(settings.value("geometry", saveGeometry()).toByteArray());
    restoreState(settings.value("state", saveState()).toByteArray());
    QPoint newPos = settings.value("pos", pos()).toPoint();
    QSize newSize = settings.value("size", size()).toSize();
    maximized_ = settings.value("maximized", true).toBool();
    QDesktopWidget* desktop = QApplication::desktop();
    QRect wholeScreenGeometry = desktop->screenGeometry(0);

    for (int i = 1; i < desktop->screenCount(); i++)
        wholeScreenGeometry = wholeScreenGeometry.united(desktop->screenGeometry(i));

    wholeScreenGeometry.setRect(wholeScreenGeometry.x() - 10, wholeScreenGeometry.y() - 10,
                                wholeScreenGeometry.width() + 20,
                                wholeScreenGeometry.height() + 20);
    QPoint bottomRight = QPoint(newPos.x() + newSize.width(), newPos.y() + newSize.height());

    if (!wholeScreenGeometry.contains(newPos) || !wholeScreenGeometry.contains(bottomRight)) {
        move(QPoint(0, 0));
        resize(wholeScreenGeometry.width() - 20, wholeScreenGeometry.height() - 20);
    } else {
        move(newPos);
        resize(newSize);
    }

    recentFileList_ = settings.value("recentFileList").toStringList();
    QString firstWorkspace = InviwoApplication::getPtr()
                                 ->getPath(InviwoApplication::PATH_WORKSPACES, "boron.inv")
                                 .c_str();
    workspaceOnLastSucessfullExit_ = settings.value("workspaceOnLastSucessfullExit",
                                                    QVariant::fromValue(firstWorkspace)).toString();
    settings.setValue("workspaceOnLastSucessfullExit", "");
    settings.endGroup();
    rootDir_ =
        QString::fromStdString(InviwoApplication::getPtr()->getPath(InviwoApplication::PATH_DATA));
    workspaceFileDir_ = rootDir_ + "workspaces/";
    settingsWidget_->updateSettingsWidget();

    // initialize menus
    addMenus();
    addMenuActions();
    addToolBars();
    updateRecentWorkspaces();

#ifdef WIN32
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    // Fix window offset when restoring old position for correct positioning
    // The frame size should be determined only once before starting up the 
    // main application and stored in InviwoApplicationQt
    // determine size of window border (frame size) 
    // as long as widget is not shown, no border exists, i.e. this->pos() == this->geometry().topLeft()

    QWidget* w = new QWidget();
    w->move(-5000, -5000);
    w->show();
    QPoint widgetPos = w->pos();
    QRect widgetGeo = w->geometry();
    QPoint offset(widgetGeo.left() - widgetPos.x(), widgetGeo.top() - widgetPos.y());
    w->hide();
    delete w;

    static_cast<InviwoApplicationQt*>(InviwoApplicationQt::getPtr())
        ->setWindowDecorationOffset(offset);
#endif
#endif
}

void InviwoMainWindow::showWindow() {
    if (maximized_)
        showMaximized();
    else
        show();
};

void InviwoMainWindow::deinitialize() {}

void InviwoMainWindow::initializeWorkspace() {
    ProcessorNetwork* processorNetwork =
        inviwo::InviwoApplicationQt::getPtr()->getProcessorNetwork();
    ProcessorNetworkObserver::addObservation(processorNetwork);
    processorNetwork->addObserver(this);
}

void InviwoMainWindow::onProcessorNetworkChange() { updateWindowTitle(); }

bool InviwoMainWindow::processCommandLineArgs() {
    const CommandLineParser* cmdparser =
        inviwo::InviwoApplicationQt::getPtr()->getCommandLineParser();
#ifdef IVW_PYTHON_QT

    if (cmdparser->getRunPythonScriptAfterStartup()) {
        PythonEditorWidget::getPtr()->show();
        PythonEditorWidget::getPtr()->loadFile(cmdparser->getPythonScriptName(), false);
        PythonEditorWidget::getPtr()->run();
    }

#endif

    if (cmdparser->getScreenGrabAfterStartup()) {
        std::string path = cmdparser->getOutputPath();

        if (path.empty())
            path = InviwoApplication::getPtr()->getPath(InviwoApplication::PATH_IMAGES);

        repaint();
        int curScreen = QApplication::desktop()->screenNumber(this);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
        QPixmap screenGrab = QGuiApplication::primaryScreen()->grabWindow(
            QApplication::desktop()->screen(curScreen)->winId());
#else
        QPixmap screenGrab =
            QPixmap::grabWindow(QApplication::desktop()->screen(curScreen)->winId());
#endif
        // QPixmap screenGrab = QPixmap::grabWindow(winId());
        std::string fileName = cmdparser->getScreenGrabName();
        screenGrab.save(QString::fromStdString(path + "/" + fileName), "png");
    }

    if (cmdparser->getCaptureAfterStartup()) {
        ProcessorNetworkEvaluator* networkEvaluator =
            inviwo::InviwoApplicationQt::getPtr()->getProcessorNetworkEvaluator();
        networkEvaluator->requestEvaluate();
        std::string path = cmdparser->getOutputPath();
        
        if (path.empty())
            path = InviwoApplication::getPtr()->getPath(InviwoApplication::PATH_IMAGES);
        
        repaint();
        networkEvaluator->saveSnapshotAllCanvases(path, cmdparser->getSnapshotName());
    }

    if (cmdparser->getQuitApplicationAfterStartup()) {
        getNetworkEditor()->setModified(false);
        return false;
    }

    return true;
}

void InviwoMainWindow::addMenus() {
    menuBar_ = menuBar();
    QAction* first = 0;

    if (menuBar_->actions().size() > 0) first = menuBar_->actions()[0];

    fileMenuItem_ = new QMenu(tr("&File"), menuBar_);
    viewMenuItem_ = new QMenu(tr("&View"), menuBar_);
    menuBar_->insertMenu(first, fileMenuItem_);
    menuBar_->insertMenu(first, viewMenuItem_);
    helpMenuItem_ = menuBar_->addMenu(tr("&Help"));
}

void InviwoMainWindow::addMenuActions() {
    // file menu entries
    workspaceActionNew_ = new QAction(QIcon(":/icons/new.png"), tr("&New Workspace"), this);
    workspaceActionNew_->setShortcut(QKeySequence::New);
    connect(workspaceActionNew_, SIGNAL(triggered()), this, SLOT(newWorkspace()));
    fileMenuItem_->addAction(workspaceActionNew_);
    workspaceActionOpen_ = new QAction(QIcon(":/icons/open.png"), tr("&Open Workspace"), this);
    workspaceActionOpen_->setShortcut(QKeySequence::Open);
    connect(workspaceActionOpen_, SIGNAL(triggered()), this, SLOT(openWorkspace()));
    fileMenuItem_->addAction(workspaceActionOpen_);
    workspaceActionSave_ = new QAction(QIcon(":/icons/save.png"), tr("&Save Workspace"), this);
    workspaceActionSave_->setShortcut(QKeySequence::Save);
    connect(workspaceActionSave_, SIGNAL(triggered()), this, SLOT(saveWorkspace()));
    fileMenuItem_->addAction(workspaceActionSave_);
    workspaceActionSaveAs_ =
        new QAction(QIcon(":/icons/saveas.png"), tr("&Save Workspace As"), this);
    workspaceActionSaveAs_->setShortcut(QKeySequence::SaveAs);
    connect(workspaceActionSaveAs_, SIGNAL(triggered()), this, SLOT(saveWorkspaceAs()));
    fileMenuItem_->addAction(workspaceActionSaveAs_);

    workspaceActionSaveAsCopy_ =
        new QAction(QIcon(":/icons/saveas.png"), tr("&Save Workspace As Copy"), this);
    connect(workspaceActionSaveAsCopy_, SIGNAL(triggered()), this, SLOT(saveWorkspaceAsCopy()));
    fileMenuItem_->addAction(workspaceActionSaveAsCopy_);

    recentFileSeparator_ = fileMenuItem_->addSeparator();

    for (int i = 0; i < maxNumRecentFiles_; i++) {
        workspaceActionRecent_[i] = new QAction(this);
        workspaceActionRecent_[i]->setVisible(false);
        connect(workspaceActionRecent_[i], SIGNAL(triggered()), this, SLOT(openRecentWorkspace()));
        fileMenuItem_->addAction(workspaceActionRecent_[i]);
    }

    recentFileSeparator_ = fileMenuItem_->addSeparator();
    exitAction_ = new QAction(QIcon(":/icons/button_cancel.png"), tr("&Exit"), this);
    exitAction_->setShortcut(QKeySequence::Close);
    connect(exitAction_, SIGNAL(triggered()), this, SLOT(close()));
    fileMenuItem_->addAction(exitAction_);
    // dock widget visibility menu entries
    viewMenuItem_->addAction(settingsWidget_->toggleViewAction());
    processorTreeWidget_->toggleViewAction()->setText(tr("&Processor List"));
    viewMenuItem_->addAction(processorTreeWidget_->toggleViewAction());
    propertyListWidget_->toggleViewAction()->setText(tr("&Property List"));
    viewMenuItem_->addAction(propertyListWidget_->toggleViewAction());
    consoleWidget_->toggleViewAction()->setText(tr("&Output Console"));
    viewMenuItem_->addAction(consoleWidget_->toggleViewAction());
    helpWidget_->toggleViewAction()->setText(tr("&Help"));
    viewMenuItem_->addAction(helpWidget_->toggleViewAction());

    // Disabled until we figure out what we want to use it for //Peter
    //viewMenuItem_->addAction(resourceManagerWidget_->toggleViewAction());

    // application/developer mode menu entries
    visibilityModeAction_ = new QAction(tr("&Application Mode"), this);
    visibilityModeAction_->setCheckable(true);
    visibilityModeAction_->setChecked(false);

    QIcon visibilityModeIcon;
    visibilityModeIcon.addFile(":/icons/view-developer.png", QSize(), QIcon::Active, QIcon::Off);
    visibilityModeIcon.addFile(":/icons/view-application.png", QSize(), QIcon::Active, QIcon::On);
    visibilityModeAction_->setIcon(visibilityModeIcon);
    viewMenuItem_->addAction(visibilityModeAction_);

    visibilityModeProperty_ = &InviwoApplication::getPtr()
                                   ->getSettingsByType<SystemSettings>()
                                   ->applicationUsageModeProperty_;
    visibilityModeProperty_->onChange(this, &InviwoMainWindow::visibilityModeChangedInSettings);

    connect(visibilityModeAction_, SIGNAL(triggered(bool)), this, SLOT(setVisibilityMode(bool)));

    visibilityModeChangedInSettings();

    enableDisableEvaluationButton_ = new QToolButton(this);
    enableDisableEvaluationButton_->setToolTip(tr("Enable/Disable Evaluation"));
    enableDisableEvaluationButton_->setCheckable(true);
    enableDisableEvaluationButton_->setChecked(false);
    QIcon enableDisableIcon;
    enableDisableIcon.addFile(":/icons/button_ok.png", QSize(), QIcon::Active, QIcon::Off);
    enableDisableIcon.addFile(":/icons/button_cancel.png", QSize(), QIcon::Active, QIcon::On);
    enableDisableEvaluationButton_->setIcon(enableDisableIcon);
    connect(enableDisableEvaluationButton_, SIGNAL(toggled(bool)), this,
            SLOT(disableEvaluation(bool)));

#if IVW_PROFILING
    resetTimeMeasurementsButton_ = new QToolButton(this);
    resetTimeMeasurementsButton_->setToolTip(tr("Reset All Time Measurements"));
    resetTimeMeasurementsButton_->setCheckable(false);
    resetTimeMeasurementsButton_->setIcon(QIcon(":/icons/stopwatch.png"));
    connect(resetTimeMeasurementsButton_, SIGNAL(clicked()), networkEditor_, SLOT(resetAllTimeMeasurements()));
#endif

    aboutBoxAction_ = new QAction(QIcon(":/icons/about.png"), tr("&About"), this);
    connect(aboutBoxAction_, SIGNAL(triggered()), this, SLOT(showAboutBox()));
    helpMenuItem_->addAction(aboutBoxAction_);
}

void InviwoMainWindow::addToolBars() {
    workspaceToolBar_ = addToolBar("File");
    workspaceToolBar_->setObjectName("fileToolBar");
    workspaceToolBar_->addAction(workspaceActionNew_);
    workspaceToolBar_->addAction(workspaceActionOpen_);
    workspaceToolBar_->addAction(workspaceActionSave_);
    workspaceToolBar_->addAction(workspaceActionSaveAs_);
    viewModeToolBar_ = addToolBar("View");
    viewModeToolBar_->setObjectName("viewModeToolBar");
    viewModeToolBar_->addAction(visibilityModeAction_);
    viewModeToolBar_->addWidget(enableDisableEvaluationButton_);
#if IVW_PROFILING
    viewModeToolBar_->addWidget(resetTimeMeasurementsButton_);
#endif
}

void InviwoMainWindow::updateWindowTitle() {
    QString windowTitle = QString("Inviwo - Interactive Visualization Workshop - ");
    windowTitle.append(currentWorkspaceFileName_);

    if (getNetworkEditor()->isModified()) windowTitle.append("*");

    if (visibilityModeAction_->isChecked()) {
        windowTitle.append(" (Application mode)");
    } else {
        windowTitle.append(" (Developer mode)");
    }

    setWindowTitle(windowTitle);
}

void InviwoMainWindow::updateRecentWorkspaces() {
    for (int i = 0; i < recentFileList_.size(); i++) {
        if (!recentFileList_[i].isEmpty()) {
            QString menuEntry =
                tr("&%1 %2").arg(i + 1).arg(QFileInfo(recentFileList_[i]).fileName());
            workspaceActionRecent_[i]->setText(menuEntry);
            workspaceActionRecent_[i]->setData(recentFileList_[i]);
            workspaceActionRecent_[i]->setVisible(true);
        } else
            workspaceActionRecent_[i]->setVisible(false);
    }

    recentFileSeparator_->setVisible(recentFileList_.size() > 0);
}

void InviwoMainWindow::addToRecentWorkspaces(QString workspaceFileName) {
    recentFileList_.removeAll(workspaceFileName);
    recentFileList_.prepend(workspaceFileName);

    if (recentFileList_.size() > maxNumRecentFiles_) recentFileList_.removeLast();

    updateRecentWorkspaces();
}

void InviwoMainWindow::setCurrentWorkspace(QString workspaceFileName) {
    workspaceFileDir_ = QFileInfo(workspaceFileName).absolutePath();
    currentWorkspaceFileName_ = workspaceFileName;
    updateWindowTitle();
}

std::string InviwoMainWindow::getCurrentWorkspace() {
    return currentWorkspaceFileName_.toLocal8Bit().constData();
}

void InviwoMainWindow::newWorkspace() {
#ifdef IVW_PYTHON_QT
    if (PythonEditorWidget::getPtr()->isActiveWindow() &&
        PythonEditorWidget::getPtr()->hasFocus()) {
        PythonEditorWidget::getPtr()->setDefaultText();
        return;
    }
#endif

    if (currentWorkspaceFileName_ != "")
        if (!askToSaveWorkspaceChanges()) return;

    getNetworkEditor()->clearNetwork();
    setCurrentWorkspace(rootDir_ + "workspaces/untitled.inv");
    getNetworkEditor()->setModified(false);
    updateWindowTitle();
}

void InviwoMainWindow::openWorkspace(QString workspaceFileName) {
    QFile file(workspaceFileName);

    if (!file.exists()) {
        LogError("Could not find workspace file: " << workspaceFileName.toLocal8Bit().constData());
        return;
    }

    getNetworkEditor()->loadNetwork(workspaceFileName.toLocal8Bit().constData());
    onNetworkEditorFileChanged(workspaceFileName.toLocal8Bit().constData());
    addToRecentWorkspaces(workspaceFileName);
    saveWindowState();
}

void InviwoMainWindow::onNetworkEditorFileChanged(const std::string& filename) {
    setCurrentWorkspace(filename.c_str());
    addToRecentWorkspaces(filename.c_str());
}

void InviwoMainWindow::onModifiedStatusChanged(const bool& newStatus) { updateWindowTitle(); }

void InviwoMainWindow::openLastWorkspace() {
    // if a workspace is defined by an argument, that workspace is opened, otherwise, the last
    // opened workspace is used
    const CommandLineParser* cmdparser =
        inviwo::InviwoApplicationQt::getPtr()->getCommandLineParser();

    if (cmdparser->getLoadWorkspaceFromArg())
        openWorkspace(static_cast<const QString>(cmdparser->getWorkspacePath().c_str()));
    else if (!workspaceOnLastSucessfullExit_.isEmpty())
        openWorkspace(workspaceOnLastSucessfullExit_);
    else
        newWorkspace();
}

void InviwoMainWindow::openWorkspace() {
    if (askToSaveWorkspaceChanges()) {
        InviwoFileDialog openFileDialog(this, "Open Workspace ...", "workspace");

        openFileDialog.addSidebarPath(InviwoApplication::PATH_WORKSPACES);
        openFileDialog.addSidebarPath(InviwoApplication::PATH_WORKSPACES);
        openFileDialog.addSidebarPath(workspaceFileDir_);

        openFileDialog.addExtension("inv", "Inviwo File");

        openFileDialog.setFileMode(QFileDialog::AnyFile);

        if (openFileDialog.exec()) {
            QString path = openFileDialog.selectedFiles().at(0);
            openWorkspace(path);
        }
    }
}

void InviwoMainWindow::openRecentWorkspace() {
    QAction* action = qobject_cast<QAction*>(sender());

    if (action) {
        if (askToSaveWorkspaceChanges()) openWorkspace(action->data().toString());
    }
}

void InviwoMainWindow::saveWorkspace() {
#ifdef IVW_PYTHON_QT

    if (PythonEditorWidget::getPtr()->isActiveWindow() &&
        PythonEditorWidget::getPtr()->hasFocus()) {
        PythonEditorWidget::getPtr()->save();
        return;
    }  // only save workspace if python editor does not have focus

#endif

    if (currentWorkspaceFileName_.contains("untitled.inv"))
        saveWorkspaceAs();
    else {
        getNetworkEditor()->saveNetwork(currentWorkspaceFileName_.toLocal8Bit().constData());
        updateWindowTitle();
    }

    /*
    // The following code snippet allows to reload the Qt style sheets during runtime,
    // which is handy while we change them. once the style sheets have been finalized,
    // this code should be removed.
    QFile styleSheetFile("C:/inviwo/resources/stylesheets/inviwo.qss");
    styleSheetFile.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(styleSheetFile.readAll());
    dynamic_cast<InviwoApplicationQt*>(InviwoApplication::getPtr())->setStyleSheet(styleSheet);
    styleSheetFile.close();
    */
}

void InviwoMainWindow::saveWorkspaceAs() {
    InviwoFileDialog saveFileDialog(this, "Save Workspace ...", "workspace");
    saveFileDialog.setFileMode(QFileDialog::AnyFile);
    saveFileDialog.setAcceptMode(QFileDialog::AcceptSave);
    saveFileDialog.setConfirmOverwrite(true);

    saveFileDialog.addSidebarPath(InviwoApplication::PATH_WORKSPACES);
    saveFileDialog.addSidebarPath(InviwoApplication::PATH_WORKSPACES);
    saveFileDialog.addSidebarPath(workspaceFileDir_);

    saveFileDialog.addExtension("inv", "Inviwo File");

    if (saveFileDialog.exec()) {
        QString path = saveFileDialog.selectedFiles().at(0);

        if (!path.endsWith(".inv")) path.append(".inv");

        getNetworkEditor()->saveNetwork(path.toLocal8Bit().constData());
        setCurrentWorkspace(path);
        addToRecentWorkspaces(path);
    }
    saveWindowState();
}

void InviwoMainWindow::saveWorkspaceAsCopy() {
    InviwoFileDialog saveFileDialog(this, "Save Workspace ...", "workspace");
    saveFileDialog.setFileMode(QFileDialog::AnyFile);
    saveFileDialog.setAcceptMode(QFileDialog::AcceptSave);
    saveFileDialog.setConfirmOverwrite(true);

    saveFileDialog.addSidebarPath(InviwoApplication::PATH_WORKSPACES);
    saveFileDialog.addSidebarPath(InviwoApplication::PATH_WORKSPACES);
    saveFileDialog.addSidebarPath(workspaceFileDir_);

    saveFileDialog.addExtension("inv", "Inviwo File");

    if (saveFileDialog.exec()) {
        QString path = saveFileDialog.selectedFiles().at(0);

        if (!path.endsWith(".inv")) path.append(".inv");

        getNetworkEditor()->saveNetwork(path.toLocal8Bit().constData());
        addToRecentWorkspaces(path);
    }
    saveWindowState();
}

void InviwoMainWindow::disableEvaluation(bool disable) {
    if (disable)
        inviwo::InviwoApplicationQt::getPtr()->getProcessorNetworkEvaluator()->disableEvaluation();
    else
        inviwo::InviwoApplicationQt::getPtr()->getProcessorNetworkEvaluator()->enableEvaluation();
}

void InviwoMainWindow::showAboutBox() {
    std::string aboutText;
    aboutText.append("<b>Inviwo v" + IVW_VERSION + "</b><br>");
    aboutText.append("Interactive Visualization Workshop<br>");
    aboutText.append("&copy; 2012-2015 The Inviwo Foundation<br>");
    aboutText.append("<a href='http://www.inviwo.org/'>http://www.inviwo.org/</a>");
    aboutText.append(
        "<p>Inviwo is a rapid prototyping environment for interactive \
                     visualizations.<br>It is licensed under the Simplified BSD license.</p>");
    aboutText.append("<p><b>Core Team:</b><br>");
    aboutText.append(
        "Erik Sund&eacute;n, Daniel J&ouml;nsson, Martin Falk, Peter Steneteg, <br>Rickard Englund,"
        " Sathish Kottravel, Timo Ropinski</p>");
    aboutText.append("<p><b>Former Developers:</b><br>");
    aboutText.append("Alexander Johansson, Andreas Valter, Johan Nor&eacute;n, Emanuel Winblad, "
        "Hans-Christian Helltegen, Viktor Axelsson</p>");
    QMessageBox::about(this, QString::fromStdString("Inviwo v" + IVW_VERSION),
                       QString::fromStdString(aboutText));
}

void InviwoMainWindow::visibilityModeChangedInSettings() {
    if (visibilityModeProperty_) {
        size_t selectedIdx = visibilityModeProperty_->getSelectedIndex();
        if (selectedIdx == DEVELOPMENT) {
            if (visibilityModeAction_->isChecked()) {
                visibilityModeAction_->setChecked(false);
            }
            propertyListWidget_->setUsageMode(DEVELOPMENT);
            setVisibilityMode(false);
        } else if (selectedIdx == APPLICATION) {
            if (!visibilityModeAction_->isChecked()) {
                visibilityModeAction_->setChecked(true);
            }
            propertyListWidget_->setUsageMode(APPLICATION);
            setVisibilityMode(true);
        }
        updateWindowTitle();
    }
}

// False == Development, True = Application
void InviwoMainWindow::setVisibilityMode(bool applicationView) {
    size_t selectedIdx = visibilityModeProperty_->getSelectedIndex();
    if (applicationView) {
        if (selectedIdx != APPLICATION) visibilityModeProperty_->setSelectedIndex(APPLICATION);
    } else {
        if (selectedIdx != DEVELOPMENT) visibilityModeProperty_->setSelectedIndex(DEVELOPMENT);
    }

    networkEditorView_->hideNetwork(applicationView);

    updateWindowTitle();
}

void InviwoMainWindow::exitInviwo() {
    QMainWindow::close();
    InviwoApplication::getPtr()->closeInviwoApplication();
}

void InviwoMainWindow::saveWindowState() {
    QSettings settings("Inviwo", "Inviwo");
    settings.beginGroup("mainwindow");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("state", saveState());
    settings.setValue("maximized", isMaximized());
    settings.setValue("pos", pos());
    settings.setValue("size", size());
    settings.setValue("recentFileList", recentFileList_);
    settings.endGroup();
}
void InviwoMainWindow::loadWindowState() {}

void InviwoMainWindow::closeEvent(QCloseEvent* event) {
    if (!askToSaveWorkspaceChanges()) {
        event->ignore();
        return;
    }
    QString loadedNetwork = currentWorkspaceFileName_;
    getNetworkEditor()->clearNetwork();
    // save window state
    saveWindowState();
    settingsWidget_->saveSettings();

    QSettings settings("Inviwo", "Inviwo");
    settings.beginGroup("mainwindow");
    if (!loadedNetwork.contains("untitled.inv"))
        settings.setValue("workspaceOnLastSucessfullExit", loadedNetwork);
    else
        settings.setValue("workspaceOnLastSucessfullExit", "");
    settings.endGroup();

    QMainWindow::closeEvent(event);
}

bool InviwoMainWindow::askToSaveWorkspaceChanges() {
    bool continueOperation = true;

    if (getNetworkEditor()->isModified()) {
        QMessageBox msgBox;
        msgBox.setText("Workspace Modified");
        msgBox.setInformativeText("Do you want to save your changes?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Yes);
        int answer = msgBox.exec();

        switch (answer) {
            case QMessageBox::Yes:
                saveWorkspace();
                break;

            case QMessageBox::No:
                break;

            case QMessageBox::Cancel:
                continueOperation = false;
                break;
        }
    }

    return continueOperation;
}

}  // namespace