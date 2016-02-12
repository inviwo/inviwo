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

#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/util/commandlineparser.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/settings/systemsettings.h>
#include <inviwo/core/util/utilities.h>
#include <inviwo/qt/editor/consolewidget.h>
#include <inviwo/qt/editor/helpwidget.h>
#include <inviwo/qt/editor/inviwomainwindow.h>
#include <inviwo/qt/editor/networkeditor.h>
#include <inviwo/qt/editor/networkeditorview.h>
#include <inviwo/qt/editor/processorlistwidget.h>
#include <inviwo/qt/editor/resourcemanagerwidget.h>
#include <inviwo/qt/editor/settingswidget.h>
#include <inviwo/qt/widgets/inviwoapplicationqt.h>
#include <inviwo/qt/widgets/inviwofiledialog.h>
#include <inviwo/qt/widgets/propertylistwidget.h>

#include <inviwomodulespaths.h>

#include <warn/push>
#include <warn/ignore/all>

#include <QScreen>
#include <QStandardPaths>

#include <QActionGroup>
#include <QClipboard>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QList>
#include <QMessageBox>
#include <QMimeData>
#include <QSettings>
#include <QUrl>
#include <QVariant>

#include <algorithm>

#include <warn/pop>


// enable menu entry to reload the application stylesheet
//#define IVW_STYLESHEET_RELOAD

namespace inviwo {

InviwoMainWindow::InviwoMainWindow(InviwoApplicationQt* app)
    : QMainWindow()
    , app_(app)
    , networkEditor_(nullptr)
    , appUsageModeProp_(nullptr)
    , exampleWorkspaceOpen_(false)

    , snapshotArg_("s", "snapshot",
                   "Specify base name of each snapshot, or \"UPN\" string for processor name.",
                   false, "", "file name")
    , screenGrabArg_("g", "screengrab", "Specify default name of each screen grab.", false, "",
                     "file name") {
    networkEditor_ = new NetworkEditor(this);
    // initialize console widget first to receive log messages
    consoleWidget_ = new ConsoleWidget(this);
    // LogCentral takes ownership of logger
    LogCentral::getPtr()->registerLogger(consoleWidget_);
    currentWorkspaceFileName_ = "";

    const QDesktopWidget dw;
    auto screen = dw.screenGeometry(this);
    const float maxRatio = 0.8f;

    QSize size(1920, 1080);
    size.setWidth(std::min(size.width(), static_cast<int>(screen.width() * maxRatio)));
    size.setHeight(std::min(size.height(), static_cast<int>(screen.height() * maxRatio)));

    // Center Window
    QPoint pos{screen.width() / 2 - size.width() / 2, screen.height() / 2 - size.height() / 2};

    resize(size);
    move(pos);

    app->getCommandLineParser().add(&snapshotArg_, [this]() {
        saveCanvases(app_->getCommandLineParser().getOutputPath(), snapshotArg_.getValue());
    }, 1000);

    app->getCommandLineParser().add(&screenGrabArg_, [this]() {
        getScreenGrab(app_->getCommandLineParser().getOutputPath(), screenGrabArg_.getValue());
    }, 1000);
}

InviwoMainWindow::~InviwoMainWindow() {
    LogCentral::getPtr()->unregisterLogger(consoleWidget_);
    delete networkEditor_;
}

void InviwoMainWindow::initialize() {
    networkEditorView_ = new NetworkEditorView(networkEditor_, this);
    NetworkEditorObserver::addObservation(networkEditor_);
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

    addDockWidget(Qt::BottomDockWidgetArea, consoleWidget_);
    // load settings and restore window state
    QSettings settings("Inviwo", "Inviwo");
    settings.beginGroup("mainwindow");
    restoreGeometry(settings.value("geometry", saveGeometry()).toByteArray());
    restoreState(settings.value("state", saveState()).toByteArray());
    maximized_ = settings.value("maximized", false).toBool();

    QString firstWorkspace = filesystem::getPath(PathType::Workspaces, "/boron.inv").c_str();
    workspaceOnLastSuccessfulExit_ =
        settings.value("workspaceOnLastSuccessfulExit", QVariant::fromValue(firstWorkspace))
            .toString();
    settings.setValue("workspaceOnLastSuccessfulExit", "");
    settings.endGroup();
    rootDir_ = QString::fromStdString(filesystem::getPath(PathType::Data));
    workspaceFileDir_ = rootDir_ + "/workspaces";
    settingsWidget_->updateSettingsWidget();

    // initialize menus
    addActions();
    updateRecentWorkspaceMenu();

#ifdef WIN32
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

    app_->setWindowDecorationOffset(offset);
#endif
}

void InviwoMainWindow::showWindow() {
    if (maximized_)
        showMaximized();
    else
        show();
};

void InviwoMainWindow::saveCanvases(std::string path, std::string fileName) {
    if (path.empty()) path = app_->getPath(PathType::Images);

    repaint();
    app_->processEvents();
    util::saveAllCanvases(app_->getProcessorNetwork(), path, fileName);
}

void InviwoMainWindow::getScreenGrab(std::string path, std::string fileName) {
    if (path.empty()) path = filesystem::getPath(PathType::Images);

    repaint();
    app_->processEvents();
    QPixmap screenGrab = QGuiApplication::primaryScreen()->grabWindow(this->winId());
    screenGrab.save(QString::fromStdString(path + "/" + fileName), "png");
}

void InviwoMainWindow::addActions() {
    auto menu = menuBar();

    auto fileMenuItem = new QMenu(tr("&File"), menu);
    auto editMenuItem = new QMenu(tr("&Edit"), menu);
    auto viewMenuItem = new QMenu(tr("&View"), menu);
    auto evalMenuItem = new QMenu(tr("&Evaluation"), menu);
    auto helpMenuItem = new QMenu(tr("&Help"), menu);

    QAction* first = menu->actions().size() > 0 ? menu->actions()[0] : nullptr;
    menu->insertMenu(first, fileMenuItem);
    menu->insertMenu(first, editMenuItem);
    menu->insertMenu(first, viewMenuItem);
    menu->insertMenu(first, evalMenuItem);
    menu->addMenu(helpMenuItem);

    auto workspaceToolBar = addToolBar("File");
    workspaceToolBar->setObjectName("fileToolBar");
    auto viewModeToolBar = addToolBar("View");
    viewModeToolBar->setObjectName("viewModeToolBar");
    auto evalToolBar = addToolBar("Evaluation");
    evalToolBar->setObjectName("evalToolBar");

    // file menu entries
    {
        auto newAction = new QAction(QIcon(":/icons/new.png"), tr("&New Workspace"), this);
        actions_["New"] = newAction;
        newAction->setShortcut(QKeySequence::New);
        newAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        this->addAction(newAction);
        connect(newAction, SIGNAL(triggered()), this, SLOT(newWorkspace()));
        fileMenuItem->addAction(newAction);
        workspaceToolBar->addAction(newAction);
    }

    {
        auto openAction = new QAction(QIcon(":/icons/open.png"), tr("&Open Workspace"), this);
        openAction->setShortcut(QKeySequence::Open);
        openAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        this->addAction(openAction);
        actions_["Open"] = openAction;
        connect(openAction, SIGNAL(triggered()), this, SLOT(openWorkspace()));
        fileMenuItem->addAction(openAction);
        workspaceToolBar->addAction(openAction);
    }

    {
        auto saveAction = new QAction(QIcon(":/icons/save.png"), tr("&Save Workspace"), this);
        saveAction->setShortcut(QKeySequence::Save);
        saveAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        this->addAction(saveAction);
        actions_["Save"] = saveAction;
        connect(saveAction, SIGNAL(triggered()), this, SLOT(saveWorkspace()));
        fileMenuItem->addAction(saveAction);
        workspaceToolBar->addAction(saveAction);
    }

    {
        auto saveAsAction =
            new QAction(QIcon(":/icons/saveas.png"), tr("&Save Workspace As"), this);
        saveAsAction->setShortcut(QKeySequence::SaveAs);
        saveAsAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        this->addAction(saveAsAction);
        actions_["Save As"] = saveAsAction;
        connect(saveAsAction, SIGNAL(triggered()), this, SLOT(saveWorkspaceAs()));
        fileMenuItem->addAction(saveAsAction);
        workspaceToolBar->addAction(saveAsAction);
    }

    {
        auto workspaceActionSaveAsCopy =
            new QAction(QIcon(":/icons/saveas.png"), tr("&Save Workspace As Copy"), this);
        connect(workspaceActionSaveAsCopy, SIGNAL(triggered()), this, SLOT(saveWorkspaceAsCopy()));
        fileMenuItem->addAction(workspaceActionSaveAsCopy);
    }

    {
        fileMenuItem->addSeparator();
        auto recentWorkspaceMenu = fileMenuItem->addMenu(tr("&Recent Workspaces"));
        // create placeholders for recent workspaces
        workspaceActionRecent_.resize(maxNumRecentFiles_);
        for (auto& action : workspaceActionRecent_) {
            action = new QAction(this);
            action->setVisible(false);
            recentWorkspaceMenu->addAction(action);
            QObject::connect(action, SIGNAL(triggered()), this, SLOT(openRecentWorkspace()));
        }
        // action for clearing the recent file menu
        clearRecentWorkspaces_ = recentWorkspaceMenu->addAction("Clear Recent Workspace List");
        clearRecentWorkspaces_->setEnabled(false);
        QObject::connect(clearRecentWorkspaces_, SIGNAL(triggered()), this,
                         SLOT(clearRecentWorkspaceMenu()));
    }

    {
        // create list of all example workspaces
        auto exampleWorkspaceMenu = fileMenuItem->addMenu(tr("&Example Workspaces"));
        fillExampleWorkspaceMenu(exampleWorkspaceMenu);
    }

    {
        // TODO: need a DEVELOPER flag here!
        // create list of all test workspaces, inviwo-dev and other external modules, i.e.
        // "research"
        auto testWorkspaceMenu = fileMenuItem->addMenu(tr("&Test Workspaces"));
        fillTestWorkspaceMenu(testWorkspaceMenu);
    }

    {
        fileMenuItem->addSeparator();
        auto exitAction = new QAction(QIcon(":/icons/button_cancel.png"), tr("&Exit"), this);
        exitAction->setShortcut(QKeySequence::Close);
        connect(exitAction, SIGNAL(triggered()), this, SLOT(close()));
        fileMenuItem->addAction(exitAction);
    }

    // Edit
    {
        auto cutAction = new QAction(tr("&Cut"), this);
        actions_["Cut"] = cutAction;
        cutAction->setShortcut(QKeySequence::Cut);
        editMenuItem->addAction(cutAction);
        cutAction->setEnabled(false);
        
    }

    {
        auto copyAction = new QAction(tr("&Copy"), this);
        actions_["Copy"] = copyAction;
        copyAction->setShortcut(QKeySequence::Copy);
        editMenuItem->addAction(copyAction);
        copyAction->setEnabled(false);
    }

    {
        auto pasteAction = new QAction(tr("&Paste"), this);
        actions_["Paste"] = pasteAction;
        pasteAction->setShortcut(QKeySequence::Paste);
        editMenuItem->addAction(pasteAction);
    }

    {
        auto deleteAction = new QAction(tr("&Delete"), this);
        actions_["Delete"] = deleteAction;
        deleteAction->setShortcuts(QList<QKeySequence>(
            {QKeySequence::Delete, QKeySequence(Qt::ControlModifier + Qt::Key_Backspace)}));
        editMenuItem->addAction(deleteAction);
        deleteAction->setEnabled(false);
    }

    editMenuItem->addSeparator();

    {
        auto selectAlllAction = new QAction(tr("&Select All"), this);
        actions_["Select All"] = selectAlllAction;
        selectAlllAction->setShortcut(QKeySequence::SelectAll);
        editMenuItem->addAction(selectAlllAction);
        connect(selectAlllAction, &QAction::triggered, [&]() { networkEditor_->selectAll(); });
    }

    editMenuItem->addSeparator();

    {
        auto findAction = new QAction(tr("&Find Processor"), this);
        actions_["Find Processor"] = findAction;
        findAction->setShortcut(QKeySequence::Find);
        editMenuItem->addAction(findAction);
        connect(findAction, &QAction::triggered, [&]() { processorTreeWidget_->focusSearch(); });
    }

    {
        auto addProcessorAction = new QAction(tr("&Add Processor"), this);
        actions_["Add Processor"] = addProcessorAction;
        addProcessorAction->setShortcut(Qt::ControlModifier + Qt::Key_D);
        editMenuItem->addAction(addProcessorAction);
        connect(addProcessorAction, &QAction::triggered,
                [&]() { processorTreeWidget_->addSelectedProcessor(); });
    }

    editMenuItem->addSeparator();

    {
        auto clearLogAction = new QAction(tr("&Clear Log"), this);
        actions_["Clear Log"] = clearLogAction;
        clearLogAction->setShortcut(Qt::ControlModifier + Qt::Key_E);
        editMenuItem->addAction(clearLogAction);
        connect(clearLogAction, &QAction::triggered, [&]() { consoleWidget_->clear(); });
    }

    // View
    {
        // dock widget visibility menu entries
        viewMenuItem->addAction(settingsWidget_->toggleViewAction());
        processorTreeWidget_->toggleViewAction()->setText(tr("&Processor List"));
        viewMenuItem->addAction(processorTreeWidget_->toggleViewAction());
        propertyListWidget_->toggleViewAction()->setText(tr("&Property List"));
        viewMenuItem->addAction(propertyListWidget_->toggleViewAction());
        consoleWidget_->toggleViewAction()->setText(tr("&Output Console"));
        viewMenuItem->addAction(consoleWidget_->toggleViewAction());
        helpWidget_->toggleViewAction()->setText(tr("&Help"));
        viewMenuItem->addAction(helpWidget_->toggleViewAction());
        // Disabled until we figure out what we want to use it for //Peter
        // viewMenuItem->addAction(resourceManagerWidget_->toggleViewAction());
    }

    {
        // application/developer mode menu entries
        QIcon visibilityModeIcon;
        visibilityModeIcon.addFile(":/icons/view-developer.png", QSize(), QIcon::Normal,
                                   QIcon::Off);
        visibilityModeIcon.addFile(":/icons/view-application.png", QSize(), QIcon::Normal,
                                   QIcon::On);
        visibilityModeAction_ = new QAction(visibilityModeIcon, tr("&Application Mode"), this);
        visibilityModeAction_->setCheckable(true);
        visibilityModeAction_->setChecked(false);

        viewMenuItem->addAction(visibilityModeAction_);
        viewModeToolBar->addAction(visibilityModeAction_);

        appUsageModeProp_ = &InviwoApplication::getPtr()
                                 ->getSettingsByType<SystemSettings>()
                                 ->applicationUsageModeProperty_;
        appUsageModeProp_->onChange(this, &InviwoMainWindow::visibilityModeChangedInSettings);

        connect(visibilityModeAction_, SIGNAL(triggered(bool)), this,
                SLOT(setVisibilityMode(bool)));

        visibilityModeChangedInSettings();
    }

    // Evaluation
    {
        QIcon enableDisableIcon;
        enableDisableIcon.addFile(":/icons/button_ok.png", QSize(), QIcon::Active, QIcon::Off);
        enableDisableIcon.addFile(":/icons/button_cancel.png", QSize(), QIcon::Active, QIcon::On);
        auto lockNetworkAction = new QAction(enableDisableIcon, tr("&Lock Network"), this);
        lockNetworkAction->setCheckable(true);
        lockNetworkAction->setChecked(false);
        lockNetworkAction->setToolTip("Enable/Disable Network Evaluation");

        lockNetworkAction->setShortcut(Qt::ControlModifier + Qt::Key_L);
        evalMenuItem->addAction(lockNetworkAction);
        evalToolBar->addAction(lockNetworkAction);
        connect(lockNetworkAction, &QAction::triggered, [lockNetworkAction]() {
            if (lockNetworkAction->isChecked()) {
                InviwoApplicationQt::getPtr()->getProcessorNetwork()->lock();
            } else {
                InviwoApplicationQt::getPtr()->getProcessorNetwork()->unlock();
            }
        });
    }

#if IVW_PROFILING
    {
        auto resetTimeMeasurementsAction =
            new QAction(QIcon(":/icons/stopwatch.png"), tr("Reset All Time Measurements"), this);
        resetTimeMeasurementsAction->setCheckable(false);

        connect(resetTimeMeasurementsAction, &QAction::triggered,
                [&]() { networkEditor_->resetAllTimeMeasurements(); });

        evalToolBar->addAction(resetTimeMeasurementsAction);
        evalMenuItem->addAction(resetTimeMeasurementsAction);
    }
#endif

    // Help
    {
        helpMenuItem->addAction(helpWidget_->toggleViewAction());

        auto aboutBoxAction = new QAction(QIcon(":/icons/about.png"), tr("&About"), this);
        connect(aboutBoxAction, SIGNAL(triggered()), this, SLOT(showAboutBox()));
        helpMenuItem->addAction(aboutBoxAction);
    }

#if defined(IVW_STYLESHEET_RELOAD)
    {
        QAction* action = new QAction(tr("&Reload Stylesheet"), this);
        QObject::connect(action, SIGNAL(triggered()), this, SLOT(reloadStyleSheet()));
        helpMenuItem->addAction(action);
    }
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

void InviwoMainWindow::fillExampleWorkspaceMenu(QMenu* menu) {
    for (const auto& module : app_->getModules()) {
        QMenu* moduleMenu = nullptr;
        auto moduleWorkspacePath = module->getPath(ModulePath::Workspaces);
        if (filesystem::directoryExists(moduleWorkspacePath)) {
            for (auto item : filesystem::getDirectoryContents(moduleWorkspacePath)) {
                // only accept inviwo workspace files
                if (filesystem::getFileExtension(item) == "inv") {
                    if(!moduleMenu)
                        moduleMenu = menu->addMenu(QString::fromStdString(module->getIdentifier()));
                
                    QString filename(QString::fromStdString(item));
                    QAction* action = moduleMenu->addAction(filename);
                    QString path(
                        QString("%1/%2").arg(QString::fromStdString(moduleWorkspacePath)).arg(filename));
                    action->setData(path);

                    QObject::connect(action, SIGNAL(triggered()), this,
                                     SLOT(openExampleWorkspace()));
                }
            }
        }
    }
    menu->menuAction()->setVisible(!menu->isEmpty());
}

void InviwoMainWindow::fillTestWorkspaceMenu(QMenu* menu) {
    for (const auto& module : app_->getModules()) {
        auto moduleTestPath = module->getPath(ModulePath::RegressionTests);
        if (filesystem::directoryExists(moduleTestPath)) {
            QMenu* moduleMenu = nullptr;

            for (auto test : filesystem::getDirectoryContents(moduleTestPath,
                                                              filesystem::ListMode::Directories)) {
                std::string testdir = moduleTestPath + "/" + test;
                // only accept inviwo workspace files
                if (filesystem::directoryExists(testdir)) {
                    for (auto item : filesystem::getDirectoryContents(testdir)) {
                        if (filesystem::getFileExtension(item) == "inv") {
                            if (!moduleMenu) {
                                moduleMenu =
                                    menu->addMenu(QString::fromStdString(module->getIdentifier()));
                            }
                            QAction* action = moduleMenu->addAction(QString::fromStdString(item));
                            action->setData(QString::fromStdString(testdir + "/" + item));
                            QObject::connect(action, SIGNAL(triggered()), this,
                                             SLOT(openRecentWorkspace()));
                        }
                    }
                }
            }
        }
    }

    // store path and extracted module name
    std::vector<std::pair<std::string, std::string> > paths;  // we need to keep the order...

    // add default workspace path
    std::string coreWorkspacePath = app_->getPath(PathType::Workspaces) + "/tests";
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

    // add paths of inviwo modules, avoid duplicates
    for (auto directory : inviwoModulePaths_) {
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

    // add menu entries
    for (auto& elem : paths) {
        QMenu* baseMenu = menu;
        // add module name as submenu folder for better organization, if it exists
        if (!elem.second.empty()) {
            baseMenu = menu->addMenu(QString::fromStdString(elem.second));
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
    menu->menuAction()->setVisible(!menu->isEmpty());
}

std::string InviwoMainWindow::getCurrentWorkspace() {
    return currentWorkspaceFileName_.toLocal8Bit().constData();
}

void InviwoMainWindow::newWorkspace() {
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

void InviwoMainWindow::openLastWorkspace(std::string workspace) {
    if (!workspace.empty()) {
        openWorkspace(QString::fromStdString(workspace));
    } else if (!workspaceOnLastSuccessfulExit_.isEmpty()) {
        openWorkspace(workspaceOnLastSuccessfulExit_);
    } else {
        newWorkspace();
    }
}

void InviwoMainWindow::openWorkspace() {
    if (askToSaveWorkspaceChanges()) {
        InviwoFileDialog openFileDialog(this, "Open Workspace ...", "workspace");

        openFileDialog.addSidebarPath(PathType::Workspaces);
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

    saveFileDialog.addSidebarPath(PathType::Workspaces);
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

    saveFileDialog.addSidebarPath(PathType::Workspaces);
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

NetworkEditor* InviwoMainWindow::getNetworkEditor() const { return networkEditor_; }

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

void InviwoMainWindow::exitInviwo(bool saveIfModified) {
    if(!saveIfModified) getNetworkEditor()->setModified(false);
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
    QString resourcePath = app->getPath(PathType::Resources).c_str();
    QFile styleSheetFile(resourcePath + "/stylesheets/inviwo.qss");
    styleSheetFile.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(styleSheetFile.readAll());
    dynamic_cast<InviwoApplicationQt*>(app)->setStyleSheet(styleSheet);
    styleSheetFile.close();
}

SettingsWidget* InviwoMainWindow::getSettingsWidget() const { return settingsWidget_; }

ProcessorTreeWidget* InviwoMainWindow::getProcessorTreeWidget() const {
    return processorTreeWidget_;
}

PropertyListWidget* InviwoMainWindow::getPropertyListWidget() const { return propertyListWidget_; }

ConsoleWidget* InviwoMainWindow::getConsoleWidget() const { return consoleWidget_; }

ResourceManagerWidget* InviwoMainWindow::getResourceManagerWidget() const {
    return resourceManagerWidget_;
}

HelpWidget* InviwoMainWindow::getHelpWidget() const { return helpWidget_; }

InviwoApplication* InviwoMainWindow::getInviwoApplication() const { return app_; }

const std::unordered_map<std::string, QAction*>& InviwoMainWindow::getActions() const {
    return actions_;
}

}  // namespace