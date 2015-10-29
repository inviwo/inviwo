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
#include <inviwo/core/util/utilities.h>
#include <inviwo/qt/editor/networkeditorview.h>
#include <inviwo/qt/widgets/inviwoapplicationqt.h>
#include <inviwo/qt/widgets/propertylistwidget.h>
#include <inviwo/qt/editor/processorlistwidget.h>
#include <inviwo/core/util/commandlineparser.h>
#include <inviwo/core/util/settings/systemsettings.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/qt/editor/resourcemanagerwidget.h>
#include <inviwo/qt/editor/consolewidget.h>
#include <inviwo/qt/editor/settingswidget.h>
#include <inviwo/qt/editor/helpwidget.h>
#include <inviwo/qt/widgets/inviwofiledialog.h>

#include <pathsexternalmodules.h>

#include <warn/push>
#include <warn/ignore/all>
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

#include <algorithm>
#include <warn/pop>

#ifdef IVW_PYTHON2_QT
#define IVW_PYTHON_QT
#include <modules/pythonqt/pythoneditorwidget.h>
#elif IVW_PYTHON3_QT
#define IVW_PYTHON_QT
#include <modules/python3qt/pythoneditorwidget.h>
#endif

// enable menu entry to reload the application stylesheet
//#define IVW_STYLESHEET_RELOAD

namespace inviwo {

InviwoMainWindow::InviwoMainWindow()
    : QMainWindow()
    , appUsageModeProp_(nullptr)
    , testWorkspaceMenu_(nullptr)  // this menu item is not always available!
    , exampleWorkspaceOpen_(false) {
    NetworkEditor::init();
    networkEditor_ = NetworkEditor::getPtr();
    // initialize console widget first to receive log messages
    consoleWidget_ = new ConsoleWidget(this);
    // LogCentral takes ownership of logger
    LogCentral::getPtr()->registerLogger(consoleWidget_);
    currentWorkspaceFileName_ = "";

    const QDesktopWidget dw;
    auto screen = dw.screenGeometry(this);
    const float maxRatio = 0.8;

    QSize size(1920, 1080);
    size.setWidth(std::min(size.width(), static_cast<int>(screen.width() * maxRatio)));
    size.setHeight(std::min(size.height(), static_cast<int>(screen.height() * maxRatio)));

    // Center Window
    QPoint pos{screen.width() / 2 - size.width() / 2, screen.height() / 2 - size.height() / 2};

    resize(size);
    move(pos);
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
    addDockWidget(Qt::RightDockWidgetArea, helpWidget_);

    processorTreeWidget_ = new ProcessorTreeWidget(this, helpWidget_);
    addDockWidget(Qt::LeftDockWidgetArea, processorTreeWidget_);

    propertyListWidget_ = new PropertyListWidget(this);
    addDockWidget(Qt::RightDockWidgetArea, propertyListWidget_);
    networkEditor_->setPropertyListWidget(propertyListWidget_);

    addDockWidget(Qt::BottomDockWidgetArea, consoleWidget_);
    // load settings and restore window state
    QSettings settings("Inviwo", "Inviwo");
    settings.beginGroup("mainwindow");
    restoreGeometry(settings.value("geometry", saveGeometry()).toByteArray());
    restoreState(settings.value("state", saveState()).toByteArray());
    maximized_ = settings.value("maximized", false).toBool();

    auto app = InviwoApplication::getPtr();

    QString firstWorkspace = app->getPath(InviwoApplication::PATH_WORKSPACES, "/boron.inv").c_str();
    workspaceOnLastSuccessfulExit_ =
        settings.value("workspaceOnLastSuccessfulExit", QVariant::fromValue(firstWorkspace))
            .toString();
    settings.setValue("workspaceOnLastSuccessfulExit", "");
    settings.endGroup();
    rootDir_ = QString::fromStdString(app->getPath(InviwoApplication::PATH_DATA));
    workspaceFileDir_ = rootDir_ + "/workspaces";
    settingsWidget_->updateSettingsWidget();

    // initialize menus
    addMenus();
    addMenuActions();
    addToolBars();
    updateRecentWorkspaceMenu();

#ifdef WIN32
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    // Fix window offset when restoring old position for correct positioning
    // The frame size should be determined only once before starting up the
    // main application and stored in InviwoApplicationQt
    // determine size of window border (frame size)
    // as long as widget is not shown, no border exists, i.e. this->pos() ==
    // this->geometry().topLeft()

    QWidget* w = new QWidget(nullptr, Qt::Tool);
    w->move(-5000, -5000);
    w->show();
    QPoint widgetPos = w->pos();
    QRect widgetGeo = w->geometry();
    QPoint offset(widgetGeo.left() - widgetPos.x(), widgetGeo.top() - widgetPos.y());
    w->hide();
    delete w;

    static_cast<InviwoApplicationQt*>(app)->setWindowDecorationOffset(offset);
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
void InviwoMainWindow::initializeWorkspace() {}

bool InviwoMainWindow::processCommandLineArgs() {
    auto app = static_cast<InviwoApplicationQt*>(InviwoApplication::getPtr());
    const auto cmdparser = app->getCommandLineParser();
#ifdef IVW_PYTHON_QT

    if (cmdparser->getRunPythonScriptAfterStartup()) {
        PythonEditorWidget::getPtr()->loadFile(cmdparser->getPythonScriptName(), false);
        PythonEditorWidget::getPtr()->run();
    }

#endif

    if (cmdparser->getScreenGrabAfterStartup()) {
        std::string path = cmdparser->getOutputPath();
        if (path.empty()) path = app->getPath(InviwoApplication::PATH_IMAGES);

        repaint();
        app->processEvents();
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
        QPixmap screenGrab = QGuiApplication::primaryScreen()->grabWindow(this->winId());
#else
        QPixmap screenGrab = QPixmap::grabWindow(this->winId());
#endif
        std::string fileName = cmdparser->getScreenGrabName();
        screenGrab.save(QString::fromStdString(path + "/" + fileName), "png");
    }

    if (cmdparser->getCaptureAfterStartup()) {
        std::string path = cmdparser->getOutputPath();
        if (path.empty()) path = app->getPath(InviwoApplication::PATH_IMAGES);

        repaint();
        app->processEvents();
        util::saveAllCanvases(app->getProcessorNetwork(), path, cmdparser->getSnapshotName());
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

    fileMenuItem_->addSeparator();
    recentWorkspaceMenu_ = fileMenuItem_->addMenu(tr("&Recent Workspaces"));
    fileMenuItem_->addSeparator();
    exampleWorkspaceMenu_ = fileMenuItem_->addMenu(tr("&Example Workspaces"));
    // TODO: need a DEVELOPER flag here!
    testWorkspaceMenu_ = fileMenuItem_->addMenu(tr("&Test Workspaces"));
    // -------
    fileMenuItem_->addSeparator();

    // create placeholders for recent workspaces
    workspaceActionRecent_.resize(maxNumRecentFiles_);
    for (auto& action : workspaceActionRecent_) {
        action = new QAction(this);
        action->setVisible(false);
        recentWorkspaceMenu_->addAction(action);
        QObject::connect(action, SIGNAL(triggered()), this, SLOT(openRecentWorkspace()));
    }
    // action for clearing the recent file menu
    clearRecentWorkspaces_ = recentWorkspaceMenu_->addAction("Clear Recent Workspace List");
    clearRecentWorkspaces_->setEnabled(false);
    QObject::connect(clearRecentWorkspaces_, SIGNAL(triggered()), this,
                     SLOT(clearRecentWorkspaceMenu()));

    // create list of all example workspaces
    fillExampleWorkspaceMenu();

    // TODO: need a DEVELOPER flag here!
    // create list of all test workspaces, inviwo-dev and other external modules, i.e. "research"
    fillTestWorkspaceMenu();
    // ---------

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
    // viewMenuItem_->addAction(resourceManagerWidget_->toggleViewAction());

    // application/developer mode menu entries
    visibilityModeAction_ = new QAction(tr("&Application Mode"), this);
    visibilityModeAction_->setCheckable(true);
    visibilityModeAction_->setChecked(false);

    QIcon visibilityModeIcon;
    visibilityModeIcon.addFile(":/icons/view-developer.png", QSize(), QIcon::Normal, QIcon::Off);
    visibilityModeIcon.addFile(":/icons/view-application.png", QSize(), QIcon::Normal, QIcon::On);
    visibilityModeAction_->setIcon(visibilityModeIcon);
    viewMenuItem_->addAction(visibilityModeAction_);

    appUsageModeProp_ = &InviwoApplication::getPtr()
                             ->getSettingsByType<SystemSettings>()
                             ->applicationUsageModeProperty_;
    appUsageModeProp_->onChange(this, &InviwoMainWindow::visibilityModeChangedInSettings);

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
    connect(resetTimeMeasurementsButton_, SIGNAL(clicked()), networkEditor_,
            SLOT(resetAllTimeMeasurements()));
#endif

    helpMenuItem_->addAction(helpWidget_->toggleViewAction());

    aboutBoxAction_ = new QAction(QIcon(":/icons/about.png"), tr("&About"), this);
    connect(aboutBoxAction_, SIGNAL(triggered()), this, SLOT(showAboutBox()));
    helpMenuItem_->addAction(aboutBoxAction_);

#if defined(IVW_STYLESHEET_RELOAD)
    QAction* action = new QAction(tr("&Reload Stylesheet"), this);
    QObject::connect(action, SIGNAL(triggered()), this, SLOT(reloadStyleSheet()));
    helpMenuItem_->addAction(action);
#endif
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

void InviwoMainWindow::updateRecentWorkspaceMenu() {
    QStringList recentFiles{getRecentWorkspaceList()};

    for (auto elem : workspaceActionRecent_) {
        elem->setVisible(false);
    }

    for (int i = 0; i < recentFiles.size(); ++i) {
        workspaceActionRecent_[i]->setVisible(!recentFiles[i].isEmpty());
        if (!recentFiles[i].isEmpty()) {
            QString menuEntry = tr("&%1 %2").arg(i + 1).arg(recentFiles[i]);
            // cannonical path will check whether the path exists and returns an empty string if not
            //.arg(QFileInfo(recentFiles[i]).canonicalFilePath());
            workspaceActionRecent_[i]->setText(menuEntry);
            workspaceActionRecent_[i]->setData(recentFiles[i]);
        }
    }
    clearRecentWorkspaces_->setEnabled(!recentFiles.isEmpty());
}

void InviwoMainWindow::clearRecentWorkspaceMenu() {
    for (auto elem : workspaceActionRecent_) {
        elem->setVisible(false);
    }
    // save empty list
    saveRecentWorkspaceList(QStringList());
    clearRecentWorkspaces_->setEnabled(false);
    updateRecentWorkspaceMenu();
}

void InviwoMainWindow::addToRecentWorkspaces(QString workspaceFileName) {
    QStringList recentFiles{getRecentWorkspaceList()};

    recentFiles.removeAll(workspaceFileName);
    recentFiles.prepend(workspaceFileName);

    if (recentFiles.size() > static_cast<int>(maxNumRecentFiles_)) recentFiles.removeLast();
    saveRecentWorkspaceList(recentFiles);

    updateRecentWorkspaceMenu();
}

QStringList InviwoMainWindow::getRecentWorkspaceList() const {
    QSettings settings("Inviwo", "Inviwo");
    settings.beginGroup("mainwindow");
    QStringList list{settings.value("recentFileList").toStringList()};
    settings.endGroup();

    return list;
}

void InviwoMainWindow::saveRecentWorkspaceList(const QStringList& list) {
    QSettings settings("Inviwo", "Inviwo");
    settings.beginGroup("mainwindow");
    settings.setValue("recentFileList", list);
    settings.endGroup();
}

void InviwoMainWindow::setCurrentWorkspace(QString workspaceFileName) {
    workspaceFileDir_ = QFileInfo(workspaceFileName).absolutePath();
    currentWorkspaceFileName_ = workspaceFileName;
    updateWindowTitle();
}

void InviwoMainWindow::fillExampleWorkspaceMenu() {
    auto app = InviwoApplication::getPtr();

    std::string workspacePath{app->getPath(InviwoApplication::PATH_WORKSPACES)};
    // get non-recursive list of contents
    auto fileList = filesystem::getDirectoryContents(workspacePath);
    for (auto item : fileList) {
        // only accept inviwo workspace files
        if (filesystem::getFileExtension(item) == "inv") {
            QString filename(QString::fromStdString(item));
            QAction* action = exampleWorkspaceMenu_->addAction(filename);
            QString path(QString("%1/%2").arg(QString::fromStdString(workspacePath)).arg(filename));
            action->setData(path);

            QObject::connect(action, SIGNAL(triggered()), this, SLOT(openExampleWorkspace()));
        }
    }
    exampleWorkspaceMenu_->menuAction()->setVisible(!exampleWorkspaceMenu_->isEmpty());
}

void InviwoMainWindow::fillTestWorkspaceMenu() {
    // store path and extracted module name
    std::vector<std::pair<std::string, std::string> > paths;  // we need to keep the order...

    // add default workspace path
    auto app = InviwoApplication::getPtr();
    std::string coreWorkspacePath = app->getPath(InviwoApplication::PATH_WORKSPACES) + "/tests";
    if (filesystem::directoryExists(coreWorkspacePath)) {
        // check whether path contains at least one workspace
        bool workspaceExists = false;
        for (auto item : filesystem::getDirectoryContents(coreWorkspacePath)) {
            // only accept inviwo workspace files
            workspaceExists = (filesystem::getFileExtension(item) == "inv");
            if (workspaceExists) {
                break;
            }
        }
        if (workspaceExists) {
            paths.push_back({coreWorkspacePath, "core"});
        }
    }

// add paths of external modules, avoid duplicates
#ifdef IVW_EXTERNAL_MODULES_PATH_COUNT
    for (auto directory : externalModulePaths_) {
        std::string moduleName;
        // remove "/modules" from given path
        std::size_t endpos = directory.rfind('/');
        if (endpos != std::string::npos) {
            directory = directory.substr(0, endpos);
            std::size_t pos = directory.rfind('/', endpos - 1);
            // extract module name from, e.g., "e:/projects/inviwo/inviwo-dev/modules" ->
            // "inviwo-dev"
            if (pos != std::string::npos) {
                moduleName = directory.substr(pos + 1, endpos - pos - 1);
            }
        }
        // TODO: remove hard-coded path to test workspaces
        directory += "/data/workspaces/tests";
        if (!filesystem::directoryExists(directory)) {
            continue;
        }

        // TODO: use cannoncial/absolute paths for comparison
        bool duplicate = false;
        for (auto item : paths) {
            if (item.first == directory) {
                duplicate = true;
                break;
            }
        }
        if (duplicate) {
            continue;
        }

        // check whether path contains at least one workspace
        bool workspaceExists = false;
        for (auto item : filesystem::getDirectoryContents(directory)) {
            // only accept inviwo workspace files
            workspaceExists = (filesystem::getFileExtension(item) == "inv");
            if (workspaceExists) {
                break;
            }
        }

        if (workspaceExists) {
            // at least one workspace could be found
            paths.push_back({directory, moduleName});
        }
    }
#endif

    // add menu entries
    for (auto& elem : paths) {
        QMenu* baseMenu = testWorkspaceMenu_;
        // add module name as submenu folder for better organization, if it exists
        if (!elem.second.empty()) {
            baseMenu = testWorkspaceMenu_->addMenu(QString::fromStdString(elem.second));
        }

        // add test workspaces to submenu
        auto fileList = filesystem::getDirectoryContents(elem.first);
        for (auto item : fileList) {
            // only accept inviwo workspace files
            if (filesystem::getFileExtension(item) == "inv") {
                QString filename(QString::fromStdString(item));
                QAction* action = baseMenu->addAction(filename);
                QString path(
                    QString("%1/%2").arg(QString::fromStdString(elem.first)).arg(filename));
                action->setData(path);

                QObject::connect(action, SIGNAL(triggered()), this, SLOT(openRecentWorkspace()));
            }
        }
    }
    testWorkspaceMenu_->menuAction()->setVisible(!testWorkspaceMenu_->isEmpty());
}

void InviwoMainWindow::keyPressEvent(QKeyEvent *e) {
    switch(e->modifiers()){
        case Qt::ControlModifier: {
            switch(e->key()) {
                case Qt::Key_F: {
                    processorTreeWidget_->focusSearch();
                    e->accept();
                    break;
                }
                case Qt::Key_E: {
                    consoleWidget_->clear();
                    e->accept();
                    break;
                }
            }
            break;
        }
    }
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

    exampleWorkspaceOpen_ = false;
    getNetworkEditor()->clearNetwork();
    setCurrentWorkspace(rootDir_ + "/workspaces/untitled.inv");
    getNetworkEditor()->setModified(false);
    updateWindowTitle();
}

void InviwoMainWindow::openWorkspace(QString workspaceFileName) {
    std::string fileName{workspaceFileName.toStdString()};

    if (!filesystem::fileExists(fileName)) {
        LogError("Could not find workspace file: " << fileName);
        return;
    }

    exampleWorkspaceOpen_ = false;
    bool loaded = getNetworkEditor()->loadNetwork(fileName);

    if (loaded) {
        onNetworkEditorFileChanged(fileName);
        saveWindowState();
    } else {
        setCurrentWorkspace(rootDir_ + "workspaces/untitled.inv");
        getNetworkEditor()->setModified(false);
        updateWindowTitle();
    }
}

void InviwoMainWindow::onNetworkEditorFileChanged(const std::string& filename) {
    if (exampleWorkspaceOpen_) {
        // ignore file change events for example workspaces
        return;
    }

    QString str{QString::fromStdString(filename)};
    setCurrentWorkspace(str);
    addToRecentWorkspaces(str);
}

void InviwoMainWindow::onModifiedStatusChanged(const bool& newStatus) { updateWindowTitle(); }

void InviwoMainWindow::openLastWorkspace() {
    // if a workspace is defined by an argument, that workspace is opened, otherwise, the last
    // opened workspace is used
    const auto cmdparser = InviwoApplicationQt::getPtr()->getCommandLineParser();

    if (cmdparser->getLoadWorkspaceFromArg()) {
        openWorkspace(static_cast<const QString>(cmdparser->getWorkspacePath().c_str()));
    } else if (!workspaceOnLastSuccessfulExit_.isEmpty()) {
        openWorkspace(workspaceOnLastSuccessfulExit_);
    } else {
        newWorkspace();
    }
}

void InviwoMainWindow::openWorkspace() {
    if (askToSaveWorkspaceChanges()) {
        InviwoFileDialog openFileDialog(this, "Open Workspace ...", "workspace");

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
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        if (askToSaveWorkspaceChanges()) openWorkspace(action->data().toString());
    }
}

void InviwoMainWindow::openExampleWorkspace() {
    QAction* action = qobject_cast<QAction*>(sender());
    if (action && askToSaveWorkspaceChanges()) {
        std::string fileName = action->data().toString().toStdString();
        if (!filesystem::fileExists(fileName)) {
            LogError("Could not find example workspace: " << fileName);
            return;
        }

        exampleWorkspaceOpen_ = true;
        bool loaded = getNetworkEditor()->loadNetwork(fileName);
        if (loaded) {
            // FIXME: example data sets are no longer added to recent files
            // addToRecentWorkspaces(action->data().toString());
            saveWindowState();
        }
        // reset workspace title in every case, we don't want to overwrite the
        // existing example workspace
        setCurrentWorkspace(rootDir_ + "workspaces/untitled.inv");
        getNetworkEditor()->setModified(false);
        updateWindowTitle();
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
    saveFileDialog.addSidebarPath(workspaceFileDir_);

    saveFileDialog.addExtension("inv", "Inviwo File");

    if (saveFileDialog.exec()) {
        QString path = saveFileDialog.selectedFiles().at(0);

        if (!path.endsWith(".inv")) path.append(".inv");

        getNetworkEditor()->saveNetwork(path.toStdString());
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
    saveFileDialog.addSidebarPath(workspaceFileDir_);

    saveFileDialog.addExtension("inv", "Inviwo File");

    if (saveFileDialog.exec()) {
        QString path = saveFileDialog.selectedFiles().at(0);

        if (!path.endsWith(".inv")) path.append(".inv");

        getNetworkEditor()->saveNetwork(path.toStdString());
        addToRecentWorkspaces(path);
    }
    saveWindowState();
}

void InviwoMainWindow::disableEvaluation(bool disable) {
    if (disable) {
        InviwoApplicationQt::getPtr()->getProcessorNetworkEvaluator()->disableEvaluation();
    } else {
        InviwoApplicationQt::getPtr()->getProcessorNetworkEvaluator()->enableEvaluation();
    }
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
    aboutText.append(
        "Alexander Johansson, Andreas Valter, Johan Nor&eacute;n, Emanuel Winblad, "
        "Hans-Christian Helltegen, Viktor Axelsson</p>");
    QMessageBox::about(this, QString::fromStdString("Inviwo v" + IVW_VERSION),
                       QString::fromStdString(aboutText));
}

void InviwoMainWindow::visibilityModeChangedInSettings() {
    if (appUsageModeProp_) {
        auto selectedIdx = static_cast<UsageMode>(appUsageModeProp_->getSelectedIndex());
        if (selectedIdx == UsageMode::Development) {
            if (visibilityModeAction_->isChecked()) {
                visibilityModeAction_->setChecked(false);
            }
            networkEditorView_->hideNetwork(false);
        } else if (selectedIdx == UsageMode::Application) {
            if (!visibilityModeAction_->isChecked()) {
                visibilityModeAction_->setChecked(true);
            }
            networkEditorView_->hideNetwork(true);
        }
        updateWindowTitle();
    }
}

// False == Development, True = Application
void InviwoMainWindow::setVisibilityMode(bool applicationView) {
    auto selectedIdx = static_cast<UsageMode>(appUsageModeProp_->getSelectedIndex());
    if (applicationView) {
        if (selectedIdx != UsageMode::Application)
            appUsageModeProp_->setSelectedIndex(static_cast<int>(UsageMode::Application));
    } else {
        if (selectedIdx != UsageMode::Development)
            appUsageModeProp_->setSelectedIndex(static_cast<int>(UsageMode::Development));
    }
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
    // settings.setValue("recentFileList", recentFileList_);
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
    if (!loadedNetwork.contains("untitled.inv")) {
        settings.setValue("workspaceOnLastSuccessfulExit", loadedNetwork);
    } else {
        settings.setValue("workspaceOnLastSuccessfulExit", "");
    }
    settings.endGroup();

    QMainWindow::closeEvent(event);
}

bool InviwoMainWindow::askToSaveWorkspaceChanges() {
    bool continueOperation = true;

    if (getNetworkEditor()->isModified()) {
        QMessageBox msgBox(this);
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

void InviwoMainWindow::reloadStyleSheet() {
    // The following code snippet allows to reload the Qt style sheets during runtime,
    // which is handy while we change them. once the style sheets have been finalized,
    // this code should be removed.

    auto app = InviwoApplication::getPtr();
    QString resourcePath = app->getPath(InviwoApplication::PATH_RESOURCES).c_str();
    QFile styleSheetFile(resourcePath + "/stylesheets/inviwo.qss");
    styleSheetFile.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(styleSheetFile.readAll());
    dynamic_cast<InviwoApplicationQt*>(app)->setStyleSheet(styleSheet);
    styleSheetFile.close();
}

}  // namespace