/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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

#include <inviwo/qt/editor/inviwomainwindow.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/common/inviwocore.h>
#include <inviwo/core/util/commandlineparser.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/settings/systemsettings.h>
#include <inviwo/core/util/utilities.h>
#include <inviwo/core/util/systemcapabilities.h>
#include <inviwo/core/util/licenseinfo.h>
#include <inviwo/core/util/vectoroperations.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/rendercontext.h>
#include <inviwo/core/network/workspacemanager.h>
#include <inviwo/qt/editor/consolewidget.h>
#include <inviwo/qt/editor/helpwidget.h>
#include <inviwo/qt/editor/inviwoaboutwindow.h>
#include <inviwo/qt/editor/toolsmenu.h>
#include <inviwo/qt/editor/processorpreview.h>
#include <inviwo/qt/editor/networkeditor.h>
#include <inviwo/qt/editor/networkeditorview.h>
#include <inviwo/qt/editor/processorlistwidget.h>
#include <inviwo/qt/editor/settingswidget.h>
#include <inviwo/qt/editor/annotationswidget.h>
#include <inviwo/qt/editor/networksearch.h>
#include <inviwo/qt/editor/processorgraphicsitem.h>
#include <inviwo/qt/editor/inviwoeditmenu.h>
#include <inviwo/qt/editor/welcomewidget.h>
#include <inviwo/qt/editor/resourcemanager/resourcemanagerdockwidget.h>
#include <inviwo/qt/applicationbase/inviwoapplicationqt.h>
#include <inviwo/qt/editor/workspaceannotationsqt.h>
#include <modules/qtwidgets/inviwofiledialog.h>
#include <modules/qtwidgets/propertylistwidget.h>
#include <inviwo/core/metadata/processormetadata.h>
#include <inviwo/core/common/inviwomodulefactoryobject.h>
#include <inviwo/core/network/workspaceutils.h>
#include <inviwo/core/network/networklock.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/processorwidget.h>
#include <inviwo/core/processors/compositeprocessor.h>
#include <inviwo/core/processors/compositeprocessorutils.h>

#include <inviwo/qt/editor/fileassociations.h>
#include <inviwo/qt/editor/dataopener.h>
#include <inviwo/core/rendering/datavisualizermanager.h>

#include <warn/push>
#include <warn/ignore/all>

#include <QScreen>
#include <QStandardPaths>
#include <QGridLayout>
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
#include <QToolBar>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QBuffer>
#include <QTabWidget>
#include <QToolButton>

#include <warn/pop>

#include <algorithm>

namespace inviwo {

InviwoMainWindow::InviwoMainWindow(InviwoApplicationQt* app)
    : QMainWindow()
    , app_(app)
    , editMenu_{new InviwoEditMenu(this)}  // needed in ConsoleWidget
    , toolsMenu_{new ToolsMenu(this)}
    , consoleWidget_{[this]() {
        // initialize console widget first to receive log messages
        auto cw = std::make_shared<ConsoleWidget>(this);
        LogCentral::getPtr()->registerLogger(cw);
        return cw;
    }()}
    , networkEditor_(nullptr)
    , fileAssociations_{std::make_unique<FileAssociations>(this)}
    , maximized_(false)
    , untitledWorkspaceName_{"untitled"}
    , snapshotArg_{"s",
                   "snapshot",
                   "Specify base name of each snapshot, or \"UPN\" string for processor name.",
                   false,
                   "",
                   "file name"}
    , screenGrabArg_{"g",   "screengrab", "Specify default name of each screen grab.",
                     false, "",           "file name"}
    , saveProcessorPreviews_{"",
                             "save-previews",
                             "Save processor previews to the supplied path",
                             false,
                             "",
                             "path"}
    , openData_{"d", "data", "Try and open a data file", false, "", "file name"}
    , updateWorkspaces_{"", "update-workspaces",
                        "Update workspaces of all modules to the latest version "
                        "(located in '<module>/data/workspaces/*')"}
    , updateRegressionWorkspaces_{"", "update-regression-workspaces",
                                  "Update regression workspaces of all modules to the latest "
                                  "version "
                                  "(located in '<module>/tests/regression/*')"}
    , updateWorkspacesInPath_{"",
                              "update-workspaces-in-path",
                              "Update all workspaces in path to the latest version",
                              false,
                              "",
                              "path"}
    , undoManager_(this) {

    setObjectName("InviwoMainWindow");

    app_->setMainWindow(this);

    setAcceptDrops(true);

    // make sure, tooltips are always shown (this includes port inspectors as well)
    this->setAttribute(Qt::WA_AlwaysShowToolTips, true);

    networkEditor_ = std::make_unique<NetworkEditor>(this);

    currentWorkspaceFileName_ = "";

    app_->installNativeEventFilter(fileAssociations_.get());

    fileAssociations_->registerFileType(
        "Inviwo.workspace", "Inviwo Workspace", ".inv", 0,
        {{"Open", "-w \"%1\"", "open",
          [this](const std::string& file) { openWorkspaceAskToSave(utilqt::toQString(file)); }},
         {"Append", "-w \"%1\"", "append",
          [this](const std::string& file) { appendWorkspace(file); }}});

    fileAssociations_->registerFileType(
        "Inviwo.volume", "Inviwo Volume", ".dat", 0,
        {{"Open", "-d \"%1\"", "data", [this](const std::string& file) {
              auto net = app_->getProcessorNetwork();
              util::insertNetworkForData(file, net);
          }}});

    const QDesktopWidget dw;
    auto screen = dw.screenGeometry(this);
    const float maxRatio = 0.8f;

    QSize size = utilqt::emToPx(this, QSizeF(192, 108));
    size.setWidth(std::min(size.width(), static_cast<int>(screen.width() * maxRatio)));
    size.setHeight(std::min(size.height(), static_cast<int>(screen.height() * maxRatio)));

    // Center Window
    QPoint pos{screen.width() / 2 - size.width() / 2, screen.height() / 2 - size.height() / 2};

    resize(size);
    move(pos);

    app->getCommandLineParser().add(&openData_,
                                    [this]() {
                                        auto net = app_->getProcessorNetwork();
                                        util::insertNetworkForData(openData_.getValue(), net, true);
                                    },
                                    900);

    app->getCommandLineParser().add(&snapshotArg_,
                                    [this]() {
                                        saveCanvases(app_->getCommandLineParser().getOutputPath(),
                                                     snapshotArg_.getValue());
                                    },
                                    1000);

    app->getCommandLineParser().add(&screenGrabArg_,
                                    [this]() {
                                        getScreenGrab(app_->getCommandLineParser().getOutputPath(),
                                                      screenGrabArg_.getValue());
                                    },
                                    1000);

    app->getCommandLineParser().add(
        &saveProcessorPreviews_,
        [this]() { utilqt::saveProcessorPreviews(app_, saveProcessorPreviews_.getValue()); }, 1200);

    app->getCommandLineParser().add(&updateWorkspaces_, [this]() { util::updateWorkspaces(app_); },
                                    1250);

    app->getCommandLineParser().add(&updateRegressionWorkspaces_,
                                    [this]() { util::updateRegressionWorkspaces(app_); }, 1250);

    app->getCommandLineParser().add(
        &updateWorkspacesInPath_,
        [this]() { util::updateWorkspaces(app_, updateWorkspacesInPath_.getValue()); }, 1250);

    networkEditorView_ = new NetworkEditorView(networkEditor_.get(), this);
    NetworkEditorObserver::addObservation(networkEditor_.get());

    centralWidget_ = new QTabWidget(this);
    centralWidget_->setObjectName("CentralTabWidget");
    centralWidget_->setTabPosition(QTabWidget::North);
    centralWidget_->setMovable(true);
    centralWidget_->setTabBarAutoHide(true);

    centralWidget_->addTab(networkEditorView_, "Network Editor");
    setCentralWidget(centralWidget_);

    settingsWidget_ = new SettingsWidget(this);
    addDockWidget(Qt::RightDockWidgetArea, settingsWidget_);
    settingsWidget_->setVisible(false);
    settingsWidget_->loadState();

    annotationsWidget_ = new AnnotationsWidget(this);
    tabifyDockWidget(settingsWidget_, annotationsWidget_);
    annotationsWidget_->setVisible(true);
    annotationsWidget_->loadState();

    helpWidget_ = new HelpWidget(this);
    tabifyDockWidget(annotationsWidget_, helpWidget_);
    helpWidget_->setVisible(true);
    helpWidget_->loadState();

    processorTreeWidget_ = new ProcessorTreeWidget(this, helpWidget_);
    addDockWidget(Qt::LeftDockWidgetArea, processorTreeWidget_);
    processorTreeWidget_->setVisible(true);
    processorTreeWidget_->loadState();

    propertyListWidget_ = new PropertyListWidget(this, app_);
    tabifyDockWidget(helpWidget_, propertyListWidget_);
    propertyListWidget_->setVisible(true);
    propertyListWidget_->loadState();

    addDockWidget(Qt::BottomDockWidgetArea, consoleWidget_.get());
    consoleWidget_->setVisible(true);
    consoleWidget_->loadState();

    resourceManagerDockWidget_ = new ResourceManagerDockWidget(this, *app->getResourceManager());
    addDockWidget(Qt::LeftDockWidgetArea, resourceManagerDockWidget_);
    resourceManagerDockWidget_->setVisible(false);
    resourceManagerDockWidget_->loadState();

    // register workspace annotation serialization and deserialization as well as clear callback
    annotationSerializationHandle_ = app_->getWorkspaceManager()->onSave(
        [&](Serializer& s) {
            const int fixedHeight = 256;

            try {
                auto canvases = utilqt::getCanvasImages(app_->getProcessorNetwork(), false);
                for (auto& img : canvases) {
                    img.second = img.second.scaledToHeight(fixedHeight);
                }
                annotationsWidget_->getAnnotations().setCanvasImages(canvases);

                annotationsWidget_->getAnnotations().setNetworkImage(
                    networkEditorView_->exportViewToImage(true, true,
                                                          QSize(fixedHeight, fixedHeight)));
            } catch (...) {
                // something went wrong fetching the canvas images,
                // continue saving workspace file without any images
            }

            s.serialize("WorkspaceAnnotations", annotationsWidget_->getAnnotations());
        },
        WorkspaceSaveMode::Disk);

    annotationDeserializationHandle_ = app_->getWorkspaceManager()->onLoad([&](Deserializer& d) {
        d.deserialize("WorkspaceAnnotations", annotationsWidget_->getAnnotations());
    });

    annotationClearHandle_ = app_->getWorkspaceManager()->onClear([&]() {
        annotationsWidget_->getAnnotations().resetAllPoperties();
        annotationsWidget_->getAnnotations().setAuthor(app_->getSystemSettings().workspaceAuthor_);
    });

    // load settings and restore window state
    loadWindowState();

    QSettings settings;
    settings.beginGroup(objectName());
    QString firstWorkspace = filesystem::getPath(PathType::Workspaces, "/boron.inv").c_str();
    workspaceOnLastSuccessfulExit_ =
        settings.value("workspaceOnLastSuccessfulExit", firstWorkspace).toString();
    settings.setValue("workspaceOnLastSuccessfulExit", "");
    settings.endGroup();

    rootDir_ = QString::fromStdString(filesystem::getPath(PathType::Data));
    workspaceFileDir_ = rootDir_ + "/workspaces";

    // initialize menus
    addActions();
    networkEditorView_->setFocus();
}

InviwoMainWindow::~InviwoMainWindow() = default;

void InviwoMainWindow::showWindow() {
    if (maximized_)
        showMaximized();
    else
        show();
}

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

    auto fileMenuItem = menu->addMenu(tr("&File"));
    menu->addMenu(editMenu_);
    auto viewMenuItem = menu->addMenu(tr("&View"));
    auto networkMenuItem = menu->addMenu(tr("&Network"));
    menu->addMenu(toolsMenu_);
    auto windowMenuItem = menu->addMenu("&Windows");
    auto helpMenuItem = menu->addMenu(tr("&Help"));

    auto workspaceToolBar = addToolBar("File");
    workspaceToolBar->setObjectName("fileToolBar");
    auto editToolBar = addToolBar("Edit");
    editToolBar->setObjectName("fileToolBar");
    auto viewModeToolBar = addToolBar("View");
    viewModeToolBar->setObjectName("viewModeToolBar");
    auto networkToolBar = addToolBar("Network");
    networkToolBar->setObjectName("networkToolBar");

    // file menu entries

    {
        auto welcomeAction =
            new QAction(QIcon(":/svgicons/about-enabled.svg"), tr("&Get Started"), this);
        this->addAction(welcomeAction);
        connect(welcomeAction, &QAction::triggered, this, &InviwoMainWindow::showWelcomeScreen);
        fileMenuItem->addAction(welcomeAction);
        fileMenuItem->addSeparator();
    }

    {
        auto newAction = new QAction(QIcon(":/svgicons/newfile.svg"), tr("&New Workspace"), this);
        newAction->setShortcut(QKeySequence::New);
        newAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        this->addAction(newAction);
        connect(newAction, &QAction::triggered, this, [this]() {
            if (newWorkspace()) {
                hideWelcomeScreen();
            }
        });
        fileMenuItem->addAction(newAction);
        workspaceToolBar->addAction(newAction);
    }

    {
        auto openAction = new QAction(QIcon(":/svgicons/open.svg"), tr("&Open Workspace"), this);
        openAction->setShortcut(QKeySequence::Open);
        openAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        this->addAction(openAction);
        connect(openAction, &QAction::triggered, this, [this]() {
            if (openWorkspace()) {
                hideWelcomeScreen();
            }
        });
        fileMenuItem->addAction(openAction);
        workspaceToolBar->addAction(openAction);
    }

    {
        auto saveAction = new QAction(QIcon(":/svgicons/save.svg"), tr("&Save Workspace"), this);
        saveAction->setShortcut(QKeySequence::Save);
        saveAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        this->addAction(saveAction);
        connect(saveAction, &QAction::triggered, this,
                static_cast<void (InviwoMainWindow::*)()>(&InviwoMainWindow::saveWorkspace));
        fileMenuItem->addAction(saveAction);
        workspaceToolBar->addAction(saveAction);
    }

    {
        auto saveAsAction =
            new QAction(QIcon(":/svgicons/save-as.svg"), tr("&Save Workspace As"), this);
        saveAsAction->setShortcut(QKeySequence::SaveAs);
        saveAsAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        this->addAction(saveAsAction);
        connect(saveAsAction, &QAction::triggered, this, &InviwoMainWindow::saveWorkspaceAs);
        fileMenuItem->addAction(saveAsAction);
        workspaceToolBar->addAction(saveAsAction);
    }

    {
        auto workspaceActionSaveAsCopy =
            new QAction(QIcon(":/svgicons/save-as-copy.svg"), tr("&Save Workspace As Copy"), this);
        connect(workspaceActionSaveAsCopy, &QAction::triggered, this,
                &InviwoMainWindow::saveWorkspaceAsCopy);
        fileMenuItem->addAction(workspaceActionSaveAsCopy);
    }

    {
        auto exportNetworkMenu = fileMenuItem->addMenu("&Export Network");

        auto backgroundVisibleAction = exportNetworkMenu->addAction("Background Visible");
        backgroundVisibleAction->setCheckable(true);
        backgroundVisibleAction->setChecked(true);
        exportNetworkMenu->addSeparator();

        auto exportNetworkImageFunc = [this, backgroundVisibleAction](bool entireScene) {
            return [this, backgroundVisibleAction, entireScene](bool /*state*/) {
                InviwoFileDialog saveFileDialog(this, "Export Network ...", "image");
                saveFileDialog.setFileMode(FileMode::AnyFile);
                saveFileDialog.setAcceptMode(AcceptMode::Save);
                saveFileDialog.setOption(QFileDialog::Option::DontConfirmOverwrite, false);

                saveFileDialog.addSidebarPath(PathType::Workspaces);
                saveFileDialog.addSidebarPath(workspaceFileDir_);

                saveFileDialog.addExtension("png", "PNG");
                saveFileDialog.addExtension("jpg", "JPEG");
                saveFileDialog.addExtension("bmp", "BMP");
                saveFileDialog.addExtension("pdf", "PDF");

                if (saveFileDialog.exec()) {
                    QString path = saveFileDialog.selectedFiles().at(0);
                    networkEditorView_->exportViewToFile(path, entireScene,
                                                         backgroundVisibleAction->isChecked());
                    LogInfo("Exported network to \"" << utilqt::fromQString(path) << "\"");
                }
            };
        };

        connect(exportNetworkMenu->addAction("Entire Network ..."), &QAction::triggered,
                exportNetworkImageFunc(true));

        connect(exportNetworkMenu->addAction("Current View ..."), &QAction::triggered,
                exportNetworkImageFunc(false));
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
            connect(action, &QAction::triggered, this, [this, action]() {
                if (askToSaveWorkspaceChanges()) {
                    if (openWorkspace(action->data().toString())) {
                        hideWelcomeScreen();
                    }
                }
            });
        }
        // action for clearing the recent file menu
        clearRecentWorkspaces_ = recentWorkspaceMenu->addAction("Clear Recent Workspace List");
        clearRecentWorkspaces_->setEnabled(false);
        connect(clearRecentWorkspaces_, &QAction::triggered, this, [this]() {
            for (auto elem : workspaceActionRecent_) {
                elem->setVisible(false);
            }
            // save empty list
            saveRecentWorkspaceList(QStringList());
            clearRecentWorkspaces_->setEnabled(false);

            if (welcomeWidget_) {
                welcomeWidget_->updateRecentWorkspaces();
            }
        });

        connect(recentWorkspaceMenu, &QMenu::aboutToShow, this, [this]() {
            for (auto elem : workspaceActionRecent_) {
                elem->setVisible(false);
            }

            QStringList recentFiles{getRecentWorkspaceList()};
            for (int i = 0; i < recentFiles.size(); ++i) {
                if (!recentFiles[i].isEmpty()) {
                    const bool exists = QFileInfo(recentFiles[i]).exists();
                    const auto menuEntry = QString("&%1 %2%3")
                                               .arg(i + 1)
                                               .arg(recentFiles[i])
                                               .arg(exists ? "" : " (missing)");
                    workspaceActionRecent_[i]->setVisible(true);
                    workspaceActionRecent_[i]->setText(menuEntry);
                    workspaceActionRecent_[i]->setEnabled(exists);
                    workspaceActionRecent_[i]->setData(recentFiles[i]);
                }
            }
            clearRecentWorkspaces_->setEnabled(!recentFiles.isEmpty());
        });
    }

    // create list of all example workspaces
    exampleMenu_ = fileMenuItem->addMenu(tr("&Example Workspaces"));
    connect(exampleMenu_, &QMenu::aboutToShow, this, [this]() {
        exampleMenu_->clear();
        for (const auto& module : app_->getModules()) {
            auto moduleWorkspacePath = module->getPath(ModulePath::Workspaces);
            if (!filesystem::directoryExists(moduleWorkspacePath)) continue;
            auto menu = std::make_unique<QMenu>(QString::fromStdString(module->getIdentifier()));
            for (auto item : filesystem::getDirectoryContents(moduleWorkspacePath)) {
                // only accept inviwo workspace files
                if (filesystem::getFileExtension(item) != "inv") continue;
                auto action = menu->addAction(QString::fromStdString(item));
                auto path = QString::fromStdString(moduleWorkspacePath + "/" + item);
                connect(action, &QAction::triggered, this, [this, path]() {
                    // open as regular workspace with proper filename if control is pressed
                    bool controlPressed = (app_->keyboardModifiers() == Qt::ControlModifier);
                    if (askToSaveWorkspaceChanges()) {
                        if (openWorkspace(path, !controlPressed)) {
                            hideWelcomeScreen();
                        }
                    }
                });
            }
            if (!menu->isEmpty()) exampleMenu_->addMenu(menu.release());
        }
        if (exampleMenu_->isEmpty()) {
            exampleMenu_->addAction("No example workspaces found")->setEnabled(false);
        }
    });

    // create list of all test workspaces
    testMenu_ = fileMenuItem->addMenu(tr("&Test Workspaces"));
    connect(testMenu_, &QMenu::aboutToShow, this, [this]() {
        testMenu_->clear();
        for (const auto& module : app_->getModules()) {
            auto moduleTestPath = module->getPath(ModulePath::RegressionTests);
            if (!filesystem::directoryExists(moduleTestPath)) continue;
            auto menu = std::make_unique<QMenu>(QString::fromStdString(module->getIdentifier()));
            for (auto test : filesystem::getDirectoryContents(moduleTestPath,
                                                              filesystem::ListMode::Directories)) {
                std::string testdir = moduleTestPath + "/" + test;
                if (!filesystem::directoryExists(testdir)) continue;
                for (auto item : filesystem::getDirectoryContents(testdir)) {
                    // only accept inviwo workspace files
                    if (filesystem::getFileExtension(item) != "inv") continue;
                    auto action = menu->addAction(QString::fromStdString(item));
                    auto path = QString::fromStdString(testdir + "/" + item);
                    connect(action, &QAction::triggered, this, [this, path]() {
                        if (askToSaveWorkspaceChanges()) {
                            if (openWorkspace(path)) {
                                hideWelcomeScreen();
                            }
                        }
                    });
                }
            }
            if (!menu->isEmpty()) testMenu_->addMenu(menu.release());
        }
        if (testMenu_->isEmpty()) {
            testMenu_->addAction("No test workspaces found")->setEnabled(false);
        }
    });

    if (app_->getModuleManager().isRuntimeModuleReloadingEnabled()) {
        fileMenuItem->addSeparator();
        auto reloadAction = new QAction(tr("&Reload modules"), this);
        connect(reloadAction, &QAction::triggered, this,
                [&]() { app_->getModuleManager().reloadModules(); });
        fileMenuItem->addAction(reloadAction);
    }

#ifdef IVW_DEBUG
    {
        fileMenuItem->addSeparator();
        auto reloadStyle = fileMenuItem->addAction("Reload Style sheet");
        connect(reloadStyle, &QAction::triggered, [this](bool /*state*/) {
            // The following code snippet allows to reload the Qt style sheets during
            // runtime, which is handy while we change them.
            app_->setStyleSheetFile(QString::fromStdString(app_->getPath(PathType::Resources) +
                                                           "/stylesheets/inviwo.qss"));
        });
    }
#endif

    {
        fileMenuItem->addSeparator();
        auto exitAction = new QAction(QIcon(":/svgicons/exit.svg"), tr("&Exit"), this);
        exitAction->setShortcut(QKeySequence::Close);
        connect(exitAction, &QAction::triggered, this, &InviwoMainWindow::close);
        fileMenuItem->addAction(exitAction);
    }

    // Edit
    {
        auto front = editMenu_->actions().front();
        editMenu_->insertAction(front, undoManager_.getUndoAction());
        editMenu_->insertAction(front, undoManager_.getRedoAction());
        editMenu_->insertSeparator(front);

        // here will the cut/copy/paste/del/select already in the menu be.

        editMenu_->addSeparator();
        auto searchNetwork =
            editMenu_->addAction(QIcon(":/svgicons/find-network.svg"), tr("&Search Network"));
        searchNetwork->setShortcut(Qt::ShiftModifier + Qt::ControlModifier + Qt::Key_F);
        connect(searchNetwork, &QAction::triggered, [this]() {
            networkEditorView_->getNetworkSearch().setVisible(true);
            networkEditorView_->getNetworkSearch().setFocus();
        });

        auto findAction =
            editMenu_->addAction(QIcon(":/svgicons/find-processor.svg"), tr("&Find Processor"));
        findAction->setShortcut(QKeySequence::Find);
        connect(findAction, &QAction::triggered, this, [this]() {
            processorTreeWidget_->show();
            processorTreeWidget_->focusSearch();
        });

        auto addProcessorAction =
            editMenu_->addAction(QIcon(":/svgicons/processor-add.svg"), tr("&Add Processor"));
        addProcessorAction->setShortcut(Qt::ControlModifier + Qt::Key_D);
        connect(addProcessorAction, &QAction::triggered, this,
                [this]() { processorTreeWidget_->addSelectedProcessor(); });

        editMenu_->addSeparator();

        editMenu_->addAction(consoleWidget_->getClearAction());

        // add actions to tool bar
        editToolBar->addAction(undoManager_.getUndoAction());
        editToolBar->addAction(undoManager_.getRedoAction());
        editToolBar->addSeparator();
        editToolBar->addAction(searchNetwork);
        editToolBar->addAction(findAction);
        editToolBar->addAction(addProcessorAction);
    }

    // View
    {
        // dock widget visibility menu entries
        viewMenuItem->addAction(settingsWidget_->toggleViewAction());
        processorTreeWidget_->toggleViewAction()->setText(tr("&Processor List"));
        viewMenuItem->addAction(processorTreeWidget_->toggleViewAction());
        propertyListWidget_->toggleViewAction()->setText(tr("&Property List"));
        viewMenuItem->addAction(propertyListWidget_->toggleViewAction());
        annotationsWidget_->toggleViewAction()->setText(tr("&Workspace Annotations"));
        viewMenuItem->addAction(annotationsWidget_->toggleViewAction());
        resourceManagerDockWidget_->toggleViewAction()->setText(tr("&Resource Manager"));
        viewMenuItem->addAction(resourceManagerDockWidget_->toggleViewAction());
        consoleWidget_->toggleViewAction()->setText(tr("&Output Console"));
        viewMenuItem->addAction(consoleWidget_->toggleViewAction());
        helpWidget_->toggleViewAction()->setText(tr("&Help"));
        viewMenuItem->addAction(helpWidget_->toggleViewAction());
    }

    {
        // application/developer mode menu entries
        QIcon visibilityModeIcon;
        visibilityModeIcon.addFile(":/svgicons/usermode.svg", QSize(), QIcon::Normal, QIcon::Off);
        visibilityModeIcon.addFile(":/svgicons/developermode.svg", QSize(), QIcon::Normal,
                                   QIcon::On);
        visibilityModeAction_ = new QAction(visibilityModeIcon, tr("&Application Mode"), this);
        visibilityModeAction_->setToolTip("Switch to Application Mode");
        visibilityModeAction_->setCheckable(true);
        visibilityModeAction_->setChecked(false);
        viewMenuItem->addAction(visibilityModeAction_);
        viewModeToolBar->addAction(visibilityModeAction_);
        connect(visibilityModeAction_, &QAction::triggered, [this](bool appView) {
            if (appView) {
                app_->setApplicationUsageMode(UsageMode::Application);
                visibilityModeAction_->setToolTip("Switch to Developer Mode");
                visibilityModeAction_->setText("&Developer Mode");
            } else {
                app_->setApplicationUsageMode(UsageMode::Development);
                visibilityModeAction_->setToolTip("Switch to Application Mode");
                visibilityModeAction_->setText("&Application Mode");
            }
        });

        auto& appUsageModeProp_ = app_->getSystemSettings().applicationUsageMode_;
        appUsageModeProp_.onChange([this]() { visibilityModeChangedInSettings(); });
        visibilityModeChangedInSettings();
    }

    // Network
    {
        QIcon enableDisableIcon;
        enableDisableIcon.addFile(":/svgicons/unlocked.svg", QSize(), QIcon::Normal, QIcon::Off);
        enableDisableIcon.addFile(":/svgicons/locked.svg", QSize(), QIcon::Normal, QIcon::On);
        auto disableEvalAction =
            new QAction(enableDisableIcon, tr("Disable &Network Evaluation"), this);
        disableEvalAction->setCheckable(true);
        disableEvalAction->setChecked(false);

        disableEvalAction->setShortcut(Qt::ControlModifier + Qt::Key_L);
        networkMenuItem->addAction(disableEvalAction);
        networkToolBar->addAction(disableEvalAction);
        connect(disableEvalAction, &QAction::triggered, this, [disableEvalAction, this]() {
            if (disableEvalAction->isChecked()) {
                app_->getProcessorNetwork()->lock();
                disableEvalAction->setToolTip("Enable Network Evaluation");
                disableEvalAction->setText("Enable &Network Evaluation");
            } else {
                app_->getProcessorNetwork()->unlock();
                disableEvalAction->setToolTip("Disable Network Evaluation");
                disableEvalAction->setText("Disable &Network Evaluation");
            }
        });

        auto compAction = networkMenuItem->addAction(
            QIcon(":/svgicons/composite-create-enabled.svg"), tr("&Create Composite"));
        compAction->setEnabled(false);
        networkToolBar->addAction(compAction);
        compAction->setShortcut(Qt::ControlModifier + Qt::Key_G);
        connect(compAction, &QAction::triggered, this, [this]() {
            util::replaceSelectionWithCompositeProcessor(*(app_->getProcessorNetwork()));
        });

        auto expandAction = networkMenuItem->addAction(
            QIcon(":/svgicons/composite-expand-enabled.svg"), tr("&Expand Composite"));
        networkToolBar->addAction(expandAction);
        expandAction->setShortcut(Qt::ControlModifier + Qt::ShiftModifier + Qt::Key_G);
        expandAction->setEnabled(false);
        connect(expandAction, &QAction::triggered, this, [this]() {
            std::unordered_set<CompositeProcessor*> selectedComposites;
            for (auto item : networkEditor_->selectedItems()) {
                if (auto pgi = qgraphicsitem_cast<ProcessorGraphicsItem*>(item)) {
                    if (auto comp = dynamic_cast<CompositeProcessor*>(pgi->getProcessor())) {
                        util::expandCompositeProcessorIntoNetwork(*comp);
                    }
                }
            }
        });

        auto updateButtonState = [this, expandAction, compAction] {
            std::unordered_set<CompositeProcessor*> selectedComposites;
            std::unordered_set<Processor*> selectedProcessors;
            for (auto item : networkEditor_->selectedItems()) {
                if (auto pgi = qgraphicsitem_cast<ProcessorGraphicsItem*>(item)) {
                    selectedProcessors.insert(pgi->getProcessor());
                    if (auto comp = dynamic_cast<CompositeProcessor*>(pgi->getProcessor())) {
                        selectedComposites.insert(comp);
                    }
                }
            }
            expandAction->setDisabled(selectedComposites.empty());
            compAction->setDisabled(selectedProcessors.empty());
        };
        connect(networkEditor_.get(), &QGraphicsScene::selectionChanged, this, updateButtonState);
        connect(networkMenuItem, &QMenu::aboutToShow, this, updateButtonState);
    }
    {
        networkMenuItem->addSeparator();
        auto invalidateNetwork = networkMenuItem->addAction("Invalidate All Output");
        connect(invalidateNetwork, &QAction::triggered, [this](bool /*state*/) {
            NetworkLock lock(app_->getProcessorNetwork());
            auto processors = app_->getProcessorNetwork()->getProcessors();
            for (const auto& p : processors) {
                p->invalidate(InvalidationLevel::InvalidOutput);
            }
        });

        auto invalidateResourcesNetwork = networkMenuItem->addAction("Invalidate All Resources");
        connect(invalidateResourcesNetwork, &QAction::triggered, [this](bool /*state*/) {
            NetworkLock lock(app_->getProcessorNetwork());
            auto processors = app_->getProcessorNetwork()->getProcessors();
            for (const auto& p : processors) {
                p->invalidate(InvalidationLevel::InvalidResources);
            }
        });
    }

#if IVW_PROFILING
    {
        networkMenuItem->addSeparator();
        auto resetTimeMeasurementsAction =
            new QAction(QIcon(":/svgicons/timer.svg"), tr("Reset All Time Measurements"), this);
        resetTimeMeasurementsAction->setCheckable(false);

        connect(resetTimeMeasurementsAction, &QAction::triggered,
                [&]() { networkEditor_->resetAllTimeMeasurements(); });

        networkToolBar->addAction(resetTimeMeasurementsAction);
        networkMenuItem->addAction(resetTimeMeasurementsAction);
    }
#endif

    // Windows
    {
        QObject::connect(windowMenuItem, &QMenu::aboutToShow, this, [this, windowMenuItem]() {
            windowMenuItem->clear();
            auto showAllAction = windowMenuItem->addAction("&Show All");
            auto hideAllAction = windowMenuItem->addAction("&Hide All");

            QObject::connect(showAllAction, &QAction::triggered, this, [this]() {
                auto widgetProcessors =
                    util::copy_if(app_->getProcessorNetwork()->getProcessors(),
                                  [](const auto p) { return p->hasProcessorWidget(); });
                for (const auto p : widgetProcessors) {
                    p->getProcessorWidget()->show();
                }
            });
            QObject::connect(hideAllAction, &QAction::triggered, this, [this]() {
                auto widgetProcessors =
                    util::copy_if(app_->getProcessorNetwork()->getProcessors(),
                                  [](const auto p) { return p->hasProcessorWidget(); });
                for (const auto p : widgetProcessors) {
                    p->getProcessorWidget()->hide();
                }
            });

            auto widgetProcessors =
                util::copy_if(app_->getProcessorNetwork()->getProcessors(),
                              [](const auto p) { return p->hasProcessorWidget(); });
            std::sort(widgetProcessors.begin(), widgetProcessors.end(), [](auto a, auto b) {
                return iCaseLess(a->getDisplayName(), b->getDisplayName());
            });

            if (!widgetProcessors.empty()) {
                windowMenuItem->addSeparator();
            }
            for (const auto p : widgetProcessors) {
                auto action =
                    windowMenuItem->addAction(QString("%1 (%2)")
                                                  .arg(utilqt::toQString(p->getDisplayName()))
                                                  .arg(utilqt::toQString(p->getIdentifier())));
                action->setCheckable(true);
                action->setChecked(p->getProcessorWidget()->isVisible());
                QObject::connect(action, &QAction::toggled, this,
                                 [p](bool toggle) { p->getProcessorWidget()->setVisible(toggle); });
            }
        });
    }

    // Help
    {
        helpMenuItem->addAction(helpWidget_->toggleViewAction());

        auto aboutBoxAction =
            new QAction(QIcon(":/svgicons/about-enabled.svg"), tr("&About"), this);
        connect(aboutBoxAction, &QAction::triggered, this, &InviwoMainWindow::showAboutBox);
        helpMenuItem->addAction(aboutBoxAction);
    }
}

void InviwoMainWindow::updateWindowTitle() {
    static const QString format{"Inviwo - Interactive Visualization Workshop - %1%2 (%3)"};

    setWindowTitle(
        format.arg(currentWorkspaceFileName_)
            .arg(getNetworkEditor()->isModified() ? "*" : "")
            .arg(visibilityModeAction_->isChecked() ? "Application mode" : "Developer mode"));
}

void InviwoMainWindow::addToRecentWorkspaces(QString workspaceFileName) {
    QStringList recentFiles{getRecentWorkspaceList()};

    recentFiles.removeAll(workspaceFileName);
    recentFiles.prepend(workspaceFileName);

    if (recentFiles.size() > static_cast<int>(maxNumRecentFiles_)) recentFiles.removeLast();
    saveRecentWorkspaceList(recentFiles);

    if (welcomeWidget_) {
        welcomeWidget_->updateRecentWorkspaces();
    }
}

QStringList InviwoMainWindow::getRecentWorkspaceList() const {
    QSettings settings;
    settings.beginGroup(objectName());
    QStringList list{settings.value("recentFileList").toStringList()};
    settings.endGroup();

    return list;
}

bool InviwoMainWindow::hasRestoreWorkspace() const { return undoManager_.hasRestore(); }

void InviwoMainWindow::restoreWorkspace() { undoManager_.restore(); }

void InviwoMainWindow::saveRecentWorkspaceList(const QStringList& list) {
    QSettings settings;
    settings.beginGroup(objectName());
    settings.setValue("recentFileList", list);
    settings.endGroup();
}

void InviwoMainWindow::setCurrentWorkspace(QString workspaceFileName) {
    workspaceFileDir_ = QFileInfo(workspaceFileName).absolutePath();
    currentWorkspaceFileName_ = workspaceFileName;
    updateWindowTitle();
}

std::string InviwoMainWindow::getCurrentWorkspace() {
    return currentWorkspaceFileName_.toLocal8Bit().constData();
}

bool InviwoMainWindow::newWorkspace() {
    if (currentWorkspaceFileName_ != "")
        if (!askToSaveWorkspaceChanges()) return false;

    app_->getWorkspaceManager()->clear();

    setCurrentWorkspace(untitledWorkspaceName_);
    getNetworkEditor()->setModified(false);
    return true;
}

bool InviwoMainWindow::openWorkspace(QString workspaceFileName) {
    return openWorkspace(workspaceFileName, false);
}

bool InviwoMainWindow::openWorkspaceAskToSave(QString workspaceFileName) {
    if (askToSaveWorkspaceChanges()) {
        return openWorkspace(workspaceFileName, false);
    } else {
        return false;
    }
}

bool InviwoMainWindow::openExample(QString exampleFileName) {
    return openWorkspace(exampleFileName, true);
}

void InviwoMainWindow::openLastWorkspace(std::string workspace) {
    QSettings settings;
    settings.beginGroup(objectName());
    const bool loadlastWorkspace = settings.value("autoloadLastWorkspace", true).toBool();
    const bool showWelcomePage = settings.value("showWelcomePage", true).toBool();

    const auto loadSuccessful = [&]() {
        workspace = filesystem::cleanupPath(workspace);
        if (!workspace.empty()) {
            return openWorkspace(utilqt::toQString(workspace));
        } else if (loadlastWorkspace && !workspaceOnLastSuccessfulExit_.isEmpty()) {
            return openWorkspace(workspaceOnLastSuccessfulExit_);
        } else {
            newWorkspace();
        }
        return false;
    }();

    if (showWelcomePage && !loadSuccessful) {
        showWelcomeScreen();
    }
}

bool InviwoMainWindow::openWorkspace() {
    if (askToSaveWorkspaceChanges()) {
        InviwoFileDialog openFileDialog(this, "Open Workspace ...", "workspace");
        openFileDialog.addSidebarPath(PathType::Workspaces);
        openFileDialog.addSidebarPath(workspaceFileDir_);
        openFileDialog.addExtension("inv", "Inviwo File");
        openFileDialog.setFileMode(FileMode::AnyFile);

        if (openFileDialog.exec()) {
            QString path = openFileDialog.selectedFiles().at(0);
            return openWorkspace(path);
        }
    }
    return false;
}

bool InviwoMainWindow::openWorkspace(QString workspaceFileName, bool isExample) {
    std::string fileName{utilqt::fromQString(workspaceFileName)};
    fileName = filesystem::cleanupPath(fileName);
    workspaceFileName = utilqt::toQString(fileName);

    if (!filesystem::fileExists(fileName)) {
        LogError("Could not find workspace file: " << fileName);
        return false;
    }

    {
        NetworkLock lock(app_->getProcessorNetwork());
        app_->getWorkspaceManager()->clear();
        try {
            app_->getWorkspaceManager()->load(fileName, [&](ExceptionContext ec) {
                try {
                    throw;
                } catch (const IgnoreException& e) {
                    util::log(
                        e.getContext(),
                        "Incomplete network loading " + fileName + " due to " + e.getMessage(),
                        LogLevel::Error);
                }
            });

            if (isExample) {
                setCurrentWorkspace(untitledWorkspaceName_);
            } else {
                setCurrentWorkspace(workspaceFileName);
                addToRecentWorkspaces(workspaceFileName);
            }
        } catch (const Exception& e) {
            util::log(e.getContext(),
                      "Unable to load network " + fileName + " due to " + e.getMessage(),
                      LogLevel::Error);
            app_->getWorkspaceManager()->clear();
            setCurrentWorkspace(untitledWorkspaceName_);
        }
        app_->processEvents();  // make sure the gui is ready before we unlock.
    }
    saveWindowState();
    getNetworkEditor()->setModified(false);
    return true;
}

void InviwoMainWindow::appendWorkspace(const std::string& file) {
    NetworkLock lock(app_->getProcessorNetwork());
    std::ifstream fs(file);
    if (!fs) {
        LogError("Could not open workspace file: " << file);
        return;
    }
    networkEditor_->append(fs, file);
    app_->processEvents();  // make sure the gui is ready before we unlock.
}

void InviwoMainWindow::saveWorkspace(QString workspaceFileName) {
    std::string fileName{utilqt::fromQString(workspaceFileName)};
    fileName = filesystem::cleanupPath(fileName);

    try {
        app_->getWorkspaceManager()->save(fileName, [&](ExceptionContext ec) {
            try {
                throw;
            } catch (const IgnoreException& e) {
                util::log(e.getContext(),
                          "Incomplete network save " + fileName + " due to " + e.getMessage(),
                          LogLevel::Error);
            }
        });
        getNetworkEditor()->setModified(false);
        updateWindowTitle();
        LogInfo("Workspace saved to: " << fileName);
    } catch (const Exception& e) {
        util::log(e.getContext(),
                  "Unable to save network " + fileName + " due to " + e.getMessage(),
                  LogLevel::Error);
    }
}

void InviwoMainWindow::saveWorkspace() {
    if (currentWorkspaceFileName_ == untitledWorkspaceName_)
        saveWorkspaceAs();
    else {
        saveWorkspace(currentWorkspaceFileName_);
    }
}

void InviwoMainWindow::saveWorkspaceAs() {
    InviwoFileDialog saveFileDialog(this, "Save Workspace ...", "workspace");
    saveFileDialog.setFileMode(FileMode::AnyFile);
    saveFileDialog.setAcceptMode(AcceptMode::Save);
    saveFileDialog.setOption(QFileDialog::Option::DontConfirmOverwrite, false);

    saveFileDialog.addSidebarPath(PathType::Workspaces);
    saveFileDialog.addSidebarPath(workspaceFileDir_);

    saveFileDialog.addExtension("inv", "Inviwo File");

    if (saveFileDialog.exec()) {
        QString path = saveFileDialog.selectedFiles().at(0);
        if (!path.endsWith(".inv")) path.append(".inv");

        saveWorkspace(path);
        setCurrentWorkspace(path);
        addToRecentWorkspaces(path);
    }
    saveWindowState();
}

void InviwoMainWindow::saveWorkspaceAsCopy() {
    InviwoFileDialog saveFileDialog(this, "Save Workspace ...", "workspace");
    saveFileDialog.setFileMode(FileMode::AnyFile);
    saveFileDialog.setAcceptMode(AcceptMode::Save);
    saveFileDialog.setOption(QFileDialog::Option::DontConfirmOverwrite, false);

    saveFileDialog.addSidebarPath(PathType::Workspaces);
    saveFileDialog.addSidebarPath(workspaceFileDir_);

    saveFileDialog.addExtension("inv", "Inviwo File");

    if (saveFileDialog.exec()) {
        QString path = saveFileDialog.selectedFiles().at(0);

        if (!path.endsWith(".inv")) path.append(".inv");

        saveWorkspace(path);
        addToRecentWorkspaces(path);
    }
    saveWindowState();
}

void InviwoMainWindow::showWelcomeScreen() {
    auto createTabCloseButton = [&](int tabIndex) {
        const auto iconsize = utilqt::emToPx(this, QSizeF(1.2, 1.2));
        auto closeBtn = new QToolButton();
        QIcon icon;
        icon.addFile(":/svgicons/close.svg", iconsize);
        closeBtn->setIcon(icon);
        closeBtn->setIconSize(iconsize);

        QObject::connect(closeBtn, &QToolButton::clicked, this, [&]() { hideWelcomeScreen(); });

        centralWidget_->tabBar()->setTabButton(tabIndex, QTabBar::RightSide, closeBtn);
    };

    if (!welcomeWidget_) {
        welcomeWidget_ = std::make_unique<WelcomeWidget>(this, centralWidget_);

        centralWidget_->setUpdatesEnabled(false);
        centralWidget_->insertTab(0, welcomeWidget_.get(), "Get Started");
        createTabCloseButton(0);
        centralWidget_->setUpdatesEnabled(true);
    }
    if (centralWidget_->indexOf(welcomeWidget_.get()) < 0) {
        centralWidget_->insertTab(0, welcomeWidget_.get(), "Get Started");
        createTabCloseButton(0);
    }

    centralWidget_->setCurrentWidget(welcomeWidget_.get());
    welcomeWidget_->setFilterFocus();
}

void InviwoMainWindow::hideWelcomeScreen() {
    if (welcomeWidget_) {
        centralWidget_->removeTab(centralWidget_->indexOf(welcomeWidget_.get()));
    }
}

void InviwoMainWindow::onModifiedStatusChanged(const bool& /*newStatus*/) { updateWindowTitle(); }

void InviwoMainWindow::showAboutBox() {
    if (!inviwoAboutWindow_) {
        inviwoAboutWindow_ = new InviwoAboutWindow(this);
        inviwoAboutWindow_->setVisible(false);
        inviwoAboutWindow_->loadState();
    }
    inviwoAboutWindow_->show();
}

void InviwoMainWindow::visibilityModeChangedInSettings() {
    auto network = app_->getProcessorNetwork();
    switch (app_->getApplicationUsageMode()) {
        case UsageMode::Development: {
            for (auto& p : network->getProcessors()) {
                auto md = p->getMetaData<ProcessorMetaData>(ProcessorMetaData::CLASS_IDENTIFIER);
                if (md->isSelected()) {
                    propertyListWidget_->addProcessorProperties(p);
                } else {
                    propertyListWidget_->removeProcessorProperties(p);
                }
            }

            if (visibilityModeAction_->isChecked()) {
                visibilityModeAction_->setChecked(false);
            }
            networkEditorView_->hideNetwork(false);
            break;
        }
        case UsageMode::Application: {
            if (!visibilityModeAction_->isChecked()) {
                visibilityModeAction_->setChecked(true);
            }
            networkEditorView_->hideNetwork(true);

            for (auto& p : network->getProcessors()) {
                propertyListWidget_->addProcessorProperties(p);
            }
            break;
        }
    }

    updateWindowTitle();
}

NetworkEditor* InviwoMainWindow::getNetworkEditor() const { return networkEditor_.get(); }

void InviwoMainWindow::exitInviwo(bool saveIfModified) {
    if (!saveIfModified) getNetworkEditor()->setModified(false);
    QMainWindow::close();
    app_->closeInviwoApplication();
}

void InviwoMainWindow::saveWindowState() {
    QSettings settings;
    settings.beginGroup(objectName());
    settings.setValue("geometry", saveGeometry());
    settings.setValue("state", saveState());
    settings.setValue("maximized", isMaximized());
    settings.endGroup();
}

void InviwoMainWindow::loadWindowState() {
    QSettings settings;
    settings.beginGroup(objectName());
    restoreGeometry(settings.value("geometry", saveGeometry()).toByteArray());
    restoreState(settings.value("state", saveState()).toByteArray());
    maximized_ = settings.value("maximized", false).toBool();
    settings.endGroup();
}

void InviwoMainWindow::closeEvent(QCloseEvent* event) {
    if (!askToSaveWorkspaceChanges()) {
        event->ignore();
        return;
    }

    app_->getWorkspaceManager()->clear();

    saveWindowState();

    QSettings settings;
    settings.beginGroup(objectName());
    if (currentWorkspaceFileName_ == untitledWorkspaceName_) {
        settings.setValue("workspaceOnLastSuccessfulExit", "");
    } else {
        settings.setValue("workspaceOnLastSuccessfulExit", currentWorkspaceFileName_);
    }
    settings.endGroup();

    // pass a close event to all children to let the same state etc.
    for (auto& child : children()) {
        QCloseEvent closeEvent;
        QApplication::sendEvent(child, &closeEvent);
    }

    QMainWindow::closeEvent(event);
}

bool InviwoMainWindow::askToSaveWorkspaceChanges() {
    bool continueOperation = true;

    if (getNetworkEditor()->isModified() && !app_->getProcessorNetwork()->isEmpty()) {
        QMessageBox msgBox(this);
        msgBox.setText("Workspace Modified");
        msgBox.setInformativeText("Do you want to save your changes?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Yes);
        // set minimum size to suppress Qt warning
        // see bug report https://bugreports.qt.io/browse/QTBUG-63661
        msgBox.setMinimumSize(msgBox.minimumSizeHint());
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

SettingsWidget* InviwoMainWindow::getSettingsWidget() const { return settingsWidget_; }

ProcessorTreeWidget* InviwoMainWindow::getProcessorTreeWidget() const {
    return processorTreeWidget_;
}

PropertyListWidget* InviwoMainWindow::getPropertyListWidget() const { return propertyListWidget_; }

ConsoleWidget* InviwoMainWindow::getConsoleWidget() const { return consoleWidget_.get(); }

AnnotationsWidget* InviwoMainWindow::getAnnotationsWidget() const { return annotationsWidget_; }

HelpWidget* InviwoMainWindow::getHelpWidget() const { return helpWidget_; }

NetworkEditorView& InviwoMainWindow::getNetworkEditorView() const { return *networkEditorView_; }
TextLabelOverlay& InviwoMainWindow::getNetworkEditorOverlay() const {
    return networkEditorView_->getOverlay();
}

InviwoApplication* InviwoMainWindow::getInviwoApplication() const { return app_; }
InviwoApplicationQt* InviwoMainWindow::getInviwoApplicationQt() const { return app_; }

InviwoEditMenu* InviwoMainWindow::getInviwoEditMenu() const { return editMenu_; }
ToolsMenu* InviwoMainWindow::getToolsMenu() const { return toolsMenu_; }

void InviwoMainWindow::dragEnterEvent(QDragEnterEvent* event) { dragMoveEvent(event); }

void InviwoMainWindow::dragMoveEvent(QDragMoveEvent* event) {
    auto mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        QList<QUrl> urlList = mimeData->urls();
        auto filename = urlList.front().toLocalFile();
        auto ext = toLower(filesystem::getFileExtension(utilqt::fromQString(filename)));

        if (ext == "inv" ||
            !app_->getDataVisualizerManager()->getDataVisualizersForExtension(ext).empty()) {

            if (event->keyboardModifiers() & Qt::ControlModifier) {
                event->setDropAction(Qt::CopyAction);
            } else {
                event->setDropAction(Qt::MoveAction);
            }
            event->accept();
        } else {
            event->setDropAction(Qt::IgnoreAction);
            event->ignore();
        }
    } else {
        event->ignore();
    }
}

void InviwoMainWindow::dropEvent(QDropEvent* event) {
    const QMimeData* mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        // use dispatch front here to avoid blocking the drag&drop source, e.g. Windows Explorer,
        // while the drop operation is performed
        auto action = [this, keyModifiers = event->keyboardModifiers(),
                       urlList = mimeData->urls()]() {
            RenderContext::getPtr()->activateDefaultRenderContext();

            bool first = true;
            for (auto& file : urlList) {
                auto filename = file.toLocalFile();

                if (toLower(filesystem::getFileExtension(utilqt::fromQString(filename))) == "inv") {
                    if (!first || keyModifiers & Qt::ControlModifier) {
                        appendWorkspace(utilqt::fromQString(filename));
                    } else {
                        openWorkspaceAskToSave(filename);
                    }
                } else {
                    util::insertNetworkForData(
                        utilqt::fromQString(filename), app_->getProcessorNetwork(),
                        static_cast<bool>(keyModifiers & Qt::ControlModifier),
                        static_cast<bool>(keyModifiers & Qt::AltModifier), this);
                }
                first = false;
            }
            undoManager_.pushStateIfDirty();
        };
        app_->dispatchFrontAndForget(action);

        event->accept();
    } else {
        event->ignore();
    }
}

}  // namespace inviwo
