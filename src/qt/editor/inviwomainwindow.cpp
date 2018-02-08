/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2018 Inviwo Foundation
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

#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/common/inviwocore.h>
#include <inviwo/core/util/commandlineparser.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/settings/systemsettings.h>
#include <inviwo/core/util/utilities.h>
#include <inviwo/core/util/systemcapabilities.h>
#include <inviwo/core/util/licenseinfo.h>
#include <inviwo/core/util/vectoroperations.h>
#include <inviwo/core/network/workspacemanager.h>
#include <inviwo/qt/editor/consolewidget.h>
#include <inviwo/qt/editor/helpwidget.h>
#include <inviwo/qt/editor/inviwoaboutwindow.h>
#include <inviwo/qt/editor/processorpreview.h>
#include <inviwo/qt/editor/networkeditor.h>
#include <inviwo/qt/editor/networkeditorview.h>
#include <inviwo/qt/editor/processorlistwidget.h>
#include <inviwo/qt/editor/settingswidget.h>
#include <inviwo/qt/editor/networksearch.h>
#include <inviwo/qt/editor/processorgraphicsitem.h>
#include <inviwo/qt/editor/inviwoeditmenu.h>
#include <inviwo/qt/applicationbase/inviwoapplicationqt.h>
#include <modules/qtwidgets/inviwofiledialog.h>
#include <modules/qtwidgets/propertylistwidget.h>
#include <inviwo/core/metadata/processormetadata.h>
#include <inviwo/core/common/inviwomodulefactoryobject.h>
#include <inviwo/core/network/workspaceutils.h>
#include <inviwo/core/network/networklock.h>
#include <inviwo/core/processors/compositeprocessor.h>
#include <inviwo/core/processors/compositeprocessorutils.h>

#include <inviwomodulespaths.h>

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

#include <warn/pop>

#include <algorithm>

namespace inviwo {

InviwoMainWindow::InviwoMainWindow(InviwoApplicationQt* app)
    : QMainWindow()
    , app_(app)
    , networkEditor_(nullptr)
    , maximized_(false)
    , untitledWorkspaceName_("untitled")
    , snapshotArg_("s", "snapshot",
                   "Specify base name of each snapshot, or \"UPN\" string for processor name.",
                   false, "", "file name")
    , screenGrabArg_("g", "screengrab", "Specify default name of each screen grab.", false, "",
                     "file name")
    , saveProcessorPreviews_("", "save-previews", "Save processor previews to the supplied path",
                             false, "", "path")
    , updateWorkspaces_("", "update-workspaces",
                        "Go through and update all workspaces the the latest versions")
    , undoManager_(this) {

    setObjectName("InviwoMainWindow");

    app_->setMainWindow(this);

    setAcceptDrops(true);

    // make sure, tooltips are always shown (this includes port inspectors as well)
    this->setAttribute(Qt::WA_AlwaysShowToolTips, true);
    editMenu_ = new InviwoEditMenu(this);
    networkEditor_ = util::make_unique<NetworkEditor>(this);
    // initialize console widget first to receive log messages
    consoleWidget_ = std::make_shared<ConsoleWidget>(this);
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

    app->getCommandLineParser().add(&saveProcessorPreviews_,
                                    [this]() {
                                        utilqt::saveProcessorPreviews(
                                            app_, saveProcessorPreviews_.getValue());

                                    },
                                    1200);

    app->getCommandLineParser().add(&updateWorkspaces_, [this]() { util::updateWorkspaces(app_); },
                                    1250);

    networkEditorView_ = new NetworkEditorView(networkEditor_.get(), this);
    NetworkEditorObserver::addObservation(networkEditor_.get());
    auto grid = new QGridLayout(networkEditorView_);
    grid->setContentsMargins(7, 7, 7, 7);
    networkSearch_ = new NetworkSearch(this);
    grid->addWidget(networkSearch_, 0, 0, Qt::AlignTop | Qt::AlignRight);

    setCentralWidget(networkEditorView_);

    settingsWidget_ = new SettingsWidget(this);
    addDockWidget(Qt::RightDockWidgetArea, settingsWidget_);
    settingsWidget_->setVisible(false);
    settingsWidget_->loadState();

    helpWidget_ = new HelpWidget(this);
    tabifyDockWidget(settingsWidget_, helpWidget_);
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
        auto newAction = new QAction(QIcon(":/icons/newfile.png"), tr("&New Workspace"), this);
        newAction->setShortcut(QKeySequence::New);
        newAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        this->addAction(newAction);
        connect(newAction, &QAction::triggered, this, &InviwoMainWindow::newWorkspace);
        fileMenuItem->addAction(newAction);
        workspaceToolBar->addAction(newAction);
    }

    {
        auto openAction = new QAction(QIcon(":/icons/open.png"), tr("&Open Workspace"), this);
        openAction->setShortcut(QKeySequence::Open);
        openAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        this->addAction(openAction);
        connect(openAction, &QAction::triggered, this,
                static_cast<void (InviwoMainWindow::*)()>(&InviwoMainWindow::openWorkspace));
        fileMenuItem->addAction(openAction);
        workspaceToolBar->addAction(openAction);
    }

    {
        auto saveAction = new QAction(QIcon(":/icons/save.png"), tr("&Save Workspace"), this);
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
            new QAction(QIcon(":/icons/saveas.png"), tr("&Save Workspace As"), this);
        saveAsAction->setShortcut(QKeySequence::SaveAs);
        saveAsAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        this->addAction(saveAsAction);
        connect(saveAsAction, &QAction::triggered, this, &InviwoMainWindow::saveWorkspaceAs);
        fileMenuItem->addAction(saveAsAction);
        workspaceToolBar->addAction(saveAsAction);
    }

    {
        auto workspaceActionSaveAsCopy =
            new QAction(QIcon(":/icons/savecopy.png"), tr("&Save Workspace As Copy"), this);
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
                saveFileDialog.setConfirmOverwrite(true);

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
                if (askToSaveWorkspaceChanges()) openWorkspace(action->data().toString());
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
            auto menu = util::make_unique<QMenu>(QString::fromStdString(module->getIdentifier()));
            for (auto item : filesystem::getDirectoryContents(moduleWorkspacePath)) {
                // only accept inviwo workspace files
                if (filesystem::getFileExtension(item) != "inv") continue;
                auto action = menu->addAction(QString::fromStdString(item));
                auto path = QString::fromStdString(moduleWorkspacePath + "/" + item);
                connect(action, &QAction::triggered, this, [this, path]() {
                    // open as regular workspace with proper filename if control is pressed
                    bool controlPressed = (app_->keyboardModifiers() == Qt::ControlModifier);
                    if (askToSaveWorkspaceChanges()) {
                        openWorkspace(path, !controlPressed);
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
            auto menu = util::make_unique<QMenu>(QString::fromStdString(module->getIdentifier()));
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
                        if (askToSaveWorkspaceChanges()) openWorkspace(path);
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

    {
        fileMenuItem->addSeparator();
        auto exitAction = new QAction(QIcon(":/icons/exit.png"), tr("&Exit"), this);
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
            editMenu_->addAction(QIcon(":/icons/searchnetwork.png"), tr("&Search Network"));
        searchNetwork->setShortcut(Qt::ShiftModifier + Qt::ControlModifier + Qt::Key_F);
        connect(searchNetwork, &QAction::triggered, [this]() {
            networkSearch_->setVisible(true);
            networkSearch_->setFocus();
        });

        auto findAction =
            editMenu_->addAction(QIcon(":/icons/findprocessor.png"), tr("&Find Processor"));
        findAction->setShortcut(QKeySequence::Find);
        connect(findAction, &QAction::triggered, this, [this]() {
            processorTreeWidget_->show();
            processorTreeWidget_->focusSearch();
        });

        auto addProcessorAction =
            editMenu_->addAction(QIcon(":/icons/processor-add.png"), tr("&Add Processor"));
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
        consoleWidget_->toggleViewAction()->setText(tr("&Output Console"));
        viewMenuItem->addAction(consoleWidget_->toggleViewAction());
        helpWidget_->toggleViewAction()->setText(tr("&Help"));
        viewMenuItem->addAction(helpWidget_->toggleViewAction());
    }

    {
        // application/developer mode menu entries
        QIcon visibilityModeIcon;
        visibilityModeIcon.addFile(":/icons/usermode.png", QSize(), QIcon::Normal, QIcon::Off);
        visibilityModeIcon.addFile(":/icons/developermode.png", QSize(), QIcon::Normal, QIcon::On);
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
        enableDisableIcon.addFile(":/icons/unlocked.png", QSize(), QIcon::Active, QIcon::Off);
        enableDisableIcon.addFile(":/icons/locked.png", QSize(), QIcon::Active, QIcon::On);
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

        auto compAction = networkMenuItem->addAction(QIcon(":/icons/composite-create.png"),
                                                     tr("&Create Composite"));
        compAction->setEnabled(false);
        networkToolBar->addAction(compAction);
        compAction->setShortcut(Qt::ControlModifier + Qt::Key_G);
        connect(compAction, &QAction::triggered, this, [this]() {
            util::replaceSelectionWithCompositeProcessor(*(app_->getProcessorNetwork()));
        });

        auto expandAction = networkMenuItem->addAction(QIcon(":/icons/composite-expand.png"),
                                                       tr("&Expand Composite"));
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

#if IVW_PROFILING
    {
        auto resetTimeMeasurementsAction =
            new QAction(QIcon(":/icons/timer.png"), tr("Reset All Time Measurements"), this);
        resetTimeMeasurementsAction->setCheckable(false);

        connect(resetTimeMeasurementsAction, &QAction::triggered,
                [&]() { networkEditor_->resetAllTimeMeasurements(); });

        networkToolBar->addAction(resetTimeMeasurementsAction);
        networkMenuItem->addAction(resetTimeMeasurementsAction);
    }
#endif

    // Help
    {
        helpMenuItem->addAction(helpWidget_->toggleViewAction());

        auto aboutBoxAction = new QAction(QIcon(":/icons/about.png"), tr("&About"), this);
        connect(aboutBoxAction, &QAction::triggered, this, &InviwoMainWindow::showAboutBox);
        helpMenuItem->addAction(aboutBoxAction);
    }
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

void InviwoMainWindow::addToRecentWorkspaces(QString workspaceFileName) {
    QStringList recentFiles{getRecentWorkspaceList()};

    recentFiles.removeAll(workspaceFileName);
    recentFiles.prepend(workspaceFileName);

    if (recentFiles.size() > static_cast<int>(maxNumRecentFiles_)) recentFiles.removeLast();
    saveRecentWorkspaceList(recentFiles);
}

QStringList InviwoMainWindow::getRecentWorkspaceList() const {
    QSettings settings;
    settings.beginGroup(objectName());
    QStringList list{settings.value("recentFileList").toStringList()};
    settings.endGroup();

    return list;
}

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

void InviwoMainWindow::newWorkspace() {
    if (currentWorkspaceFileName_ != "")
        if (!askToSaveWorkspaceChanges()) return;

    app_->getWorkspaceManager()->clear();

    setCurrentWorkspace(untitledWorkspaceName_);
    getNetworkEditor()->setModified(false);
}

void InviwoMainWindow::openWorkspace(QString workspaceFileName) {
    openWorkspace(workspaceFileName, false);
}

void InviwoMainWindow::openWorkspace(QString workspaceFileName, bool exampleWorkspace) {
    std::string fileName{utilqt::fromQString(workspaceFileName)};
    fileName = filesystem::cleanupPath(fileName);
    workspaceFileName = utilqt::toQString(fileName);

    if (!filesystem::fileExists(fileName)) {
        LogError("Could not find workspace file: " << fileName);
        return;
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

            if (exampleWorkspace) {
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
}

void InviwoMainWindow::openLastWorkspace(std::string workspace) {
    workspace = filesystem::cleanupPath(workspace);
    if (!workspace.empty()) {
        openWorkspace(utilqt::toQString(workspace));
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
        openFileDialog.setFileMode(FileMode::AnyFile);

        if (openFileDialog.exec()) {
            QString path = openFileDialog.selectedFiles().at(0);
            openWorkspace(path);
        }
    }
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
    saveFileDialog.setConfirmOverwrite(true);

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
    saveFileDialog.setConfirmOverwrite(true);

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

HelpWidget* InviwoMainWindow::getHelpWidget() const { return helpWidget_; }

InviwoApplication* InviwoMainWindow::getInviwoApplication() const { return app_; }
InviwoApplicationQt* InviwoMainWindow::getInviwoApplicationQt() const { return app_; }

InviwoEditMenu* InviwoMainWindow::getInviwoEditMenu() const { return editMenu_; }

void InviwoMainWindow::dropEvent(QDropEvent* event) {
    const QMimeData* mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        QStringList pathList;
        QList<QUrl> urlList = mimeData->urls();

        // pick first url
        auto filename = urlList.front().toLocalFile();
        openWorkspace(filename);
        event->acceptProposedAction();
    }
}

void InviwoMainWindow::dragEnterEvent(QDragEnterEvent* event) { event->acceptProposedAction(); }

}  // namespace inviwo
