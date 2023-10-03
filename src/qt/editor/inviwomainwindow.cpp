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

#include <inviwo/qt/editor/inviwomainwindow.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/common/inviwocore.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/inviwocommondefines.h>
#include <inviwo/core/util/commandlineparser.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/settings/systemsettings.h>
#include <inviwo/core/util/utilities.h>
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
#include <inviwo/qt/applicationbase/qtapptools.h>
#include <inviwo/qt/editor/workspaceannotationsqt.h>
#include <modules/qtwidgets/inviwofiledialog.h>
#include <modules/qtwidgets/propertylistwidget.h>
#include <modules/qtwidgets/keyboardutils.h>
#include <modules/qtwidgets/editorsettings.h>
#include <inviwo/core/metadata/processormetadata.h>
#include <inviwo/core/common/inviwomodulefactoryobject.h>
#include <inviwo/core/network/workspaceutils.h>
#include <inviwo/core/network/networklock.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/processorwidget.h>
#include <inviwo/core/processors/compositeprocessor.h>
#include <inviwo/core/processors/compositeprocessorutils.h>
#include <inviwo/core/processors/exporter.h>

#include <inviwo/qt/editor/fileassociations.h>
#include <inviwo/qt/editor/dataopener.h>
#include <inviwo/core/rendering/datavisualizermanager.h>
#include <inviwo/core/util/timer.h>

#include <QScreen>
#include <QStandardPaths>
#include <QGridLayout>
#include <QActionGroup>
#include <QClipboard>
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
#include <QScreen>
#include <QToolButton>
#include <QStackedWidget>
#include <QApplication>

#include <fmt/std.h>

#include <algorithm>

namespace inviwo {

namespace {
QString addAppendSuffix(const QString& str, bool append) {
    if (append) {
        return QString{str}.append(" (Append)");
    } else {
        return str;
    }
}
}  // namespace

class MenuKeyboardEventFilter : public QObject {
public:
    MenuKeyboardEventFilter(QObject* parent) : QObject(parent) {}

    // This will not trigger on MacOS, so we also trigger update on hovered and aboutToShow.
    virtual bool eventFilter(QObject*, QEvent* ev) override {
        if (ev->type() == QEvent::KeyPress || ev->type() == QEvent::KeyRelease) {
            const auto ctrl =
                QGuiApplication::queryKeyboardModifiers().testFlag(Qt::ControlModifier);
            update(ctrl);
        }
        return false;
    }

    void update(bool ctrl) {
        if (ctrl != ctrlPressed) {
            ctrlPressed = ctrl;
            for (auto& action : actions_) {
                action.action->setText(addAppendSuffix(action.text, ctrl));
            }
        }
    }

    void add(QAction* action, const QString& text) {
        auto sig = connect(action, &QAction::hovered, this, [this]() {
            const auto ctrl =
                QGuiApplication::queryKeyboardModifiers().testFlag(Qt::ControlModifier);
            update(ctrl);
        });
        actions_.push_back({action, text, sig});
    }

    void add(QMenu* menu) {
        auto sig = connect(menu, &QMenu::aboutToShow, this, [this]() {
            const auto ctrl =
                QGuiApplication::queryKeyboardModifiers().testFlag(Qt::ControlModifier);
            update(ctrl);
        });
        menus_.push_back({menu, sig});
    }

    void clear() {
        for (auto& action : actions_) {
            disconnect(action.connection);
        }
        actions_.clear();
        for (auto& menu : menus_) {
            disconnect(menu.connection);
        }
        menus_.clear();
    }

    struct Action {
        QAction* action;
        QString text;
        QMetaObject::Connection connection;
    };

    struct Menu {
        QMenu* menu;
        QMetaObject::Connection connection;
    };

private:
    bool ctrlPressed = false;
    std::vector<Action> actions_;
    std::vector<Menu> menus_;
};

namespace detail {

auto getWidgetProcessors(InviwoApplication* app) {
    auto widgetProcessors = util::copy_if(app->getProcessorNetwork()->getProcessors(),
                                          [](const auto p) { return p->hasProcessorWidget(); });
    std::sort(widgetProcessors.begin(), widgetProcessors.end(),
              [](auto a, auto b) { return iCaseLess(a->getDisplayName(), b->getDisplayName()); });
    return widgetProcessors;
}

auto getFloatingDockWidgets(InviwoMainWindow* win) {
    auto dockWidgets = util::copy_if(win->findChildren<InviwoDockWidget*>(),
                                     [](const auto p) { return p->isFloating(); });
    std::sort(dockWidgets.begin(), dockWidgets.end(), [](auto a, auto b) {
        return QString::compare(a->windowTitle(), b->windowTitle(), Qt::CaseInsensitive);
    });

    return dockWidgets;
}

}  // namespace detail

InviwoMainWindow::InviwoMainWindow(InviwoApplication* app)
    : QMainWindow()
    , app_(app)
    , menuEventFilter_{new MenuKeyboardEventFilter(this)}
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
    , updateExampleWorkspaces_{"", "update-workspaces",
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
    , undoManager_(this)
    , visibleWidgetsClearHandle_{
          app->getWorkspaceManager()->onClear([&]() { visibleWidgetState_.processors.clear(); })} {

    setObjectName("InviwoMainWindow");

    setAcceptDrops(true);

    // make sure, tooltips are always shown (this includes port inspectors as well)
    setAttribute(Qt::WA_AlwaysShowToolTips, true);

    networkEditor_ = std::make_unique<NetworkEditor>(this);

    currentWorkspaceFileName_ = "";

    qApp->installNativeEventFilter(fileAssociations_.get());

    fileAssociations_->registerFileType(
        "Inviwo.workspace", "Inviwo Workspace", ".inv", 0,
        {{"Open", "-w \"%1\"", "open",
          [this](const std::filesystem::path& file) { openWorkspaceAskToSave(file); }},
         {"Append", "-w \"%1\"", "append",
          [this](const std::filesystem::path& file) { appendWorkspace(file); }}});

    fileAssociations_->registerFileType(
        "Inviwo.volume", "Inviwo Volume", ".dat", 0,
        {{"Open", "-d \"%1\"", "data", [this](const std::filesystem::path& file) {
              auto net = app_->getProcessorNetwork();
              util::insertNetworkForData(file, net);
          }}});

    auto screen = QGuiApplication::primaryScreen();
    const float maxRatio = 0.8f;
    const auto ssize = screen->availableSize();

    QSize size = utilqt::emToPx(this, QSizeF(192, 108));
    size.setWidth(std::min(size.width(), static_cast<int>(ssize.width() * maxRatio)));
    size.setHeight(std::min(size.height(), static_cast<int>(ssize.height() * maxRatio)));

    // Center Window
    QPoint pos{ssize.width() / 2 - size.width() / 2, ssize.height() / 2 - size.height() / 2};

    resize(size);
    move(pos);

    app->getCommandLineParser().add(
        &openData_,
        [this]() {
            auto net = app_->getProcessorNetwork();
            util::insertNetworkForData(openData_.getValue(), net, true);
        },
        900);

    app->getCommandLineParser().add(
        &snapshotArg_,
        [this]() {
            saveSnapshots(app_->getCommandLineParser().getOutputPath(), snapshotArg_.getValue());
        },
        1000);

    app->getCommandLineParser().add(
        &screenGrabArg_,
        [this]() {
            getScreenGrab(app_->getCommandLineParser().getOutputPath(), screenGrabArg_.getValue());
        },
        1000);

    app->getCommandLineParser().add(
        &saveProcessorPreviews_,
        [this]() { utilqt::saveProcessorPreviews(app_, saveProcessorPreviews_.getValue()); }, 1200);

    app->getCommandLineParser().add(
        &updateExampleWorkspaces_,
        [this]() {
            util::updateExampleWorkspaces(app_, util::DryRun::No, []() {
                qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
            });
        },
        1250);

    app->getCommandLineParser().add(
        &updateRegressionWorkspaces_,
        [this]() {
            util::updateRegressionWorkspaces(app_, util::DryRun::No, []() {
                qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
            });
        },
        1250);

    app->getCommandLineParser().add(
        &updateWorkspacesInPath_,
        [this]() {
            util::updateWorkspaces(app_, updateWorkspacesInPath_.getValue(), util::DryRun::No);
        },
        1250);

    networkEditorView_ = new NetworkEditorView(networkEditor_.get(), this);
    NetworkEditorObserver::addObservation(networkEditor_.get());

    settings_ = new SettingsWidget(this);
    addDockWidget(Qt::RightDockWidgetArea, settings_);
    settings_->setVisible(false);
    settings_->loadState();

    annotationsWidget_ = new AnnotationsWidget(this);
    tabifyDockWidget(settings_, annotationsWidget_);
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

    centralWidget_ = new QStackedWidget(this);
    centralWidget_->setObjectName("CentralTabWidget");

    centralWidget_->addWidget(networkEditorView_);
    setCentralWidget(centralWidget_);

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
        annotationsWidget_->getAnnotations().setAuthor(
            app_->getSettingsByType<EditorSettings>()->workspaceAuthor);
    });

    // load settings and restore window state
    loadWindowState();

    QSettings settings;
    settings.beginGroup(objectName());
    QString firstWorkspace =
        utilqt::toQString(filesystem::getPath(PathType::Workspaces) / "boron.inv");
    workspaceOnLastSuccessfulExit_ =
        utilqt::toPath(settings.value("workspaceOnLastSuccessfulExit", firstWorkspace).toString());
    settings.setValue("workspaceOnLastSuccessfulExit", "");
    settings.endGroup();

    workspaceFileDir_ = filesystem::getPath(PathType::Workspaces);

    // initialize menus
    addActions();
    networkEditorView_->setFocus();

    utilqt::configurePoolResizeWait(*app_, this);
}

InviwoMainWindow::~InviwoMainWindow() { app_->setPoolResizeWaitCallback(nullptr); }

void InviwoMainWindow::showWindow() {
    if (maximized_) {
        showMaximized();
    } else {
        show();
    }
}

void InviwoMainWindow::saveSnapshots(const std::filesystem::path& path, std::string_view fileName) {
    repaint();
    qApp->processEvents();
    app_->waitForPool();

    while (app_->getProcessorNetwork()->runningBackgroundJobs() > 0) {
        qApp->processEvents();
        app_->processFront();
    }

    rendercontext::activateDefault();
    util::exportAllFiles(
        *app_->getProcessorNetwork(), path, fileName,
        {FileExtension{"png", ""}, FileExtension{"csv", ""}, FileExtension{"txt", ""}},
        Overwrite::Yes);
}

void InviwoMainWindow::getScreenGrab(const std::filesystem::path& path, std::string_view fileName) {
    repaint();
    qApp->processEvents();
    app_->waitForPool();
    QPixmap screenGrab = QGuiApplication::primaryScreen()->grabWindow(winId());
    screenGrab.save(utilqt::toQString(path / fileName), "png");
}

void InviwoMainWindow::addActions() {
    auto menu = menuBar();

    auto fileMenuItem = menu->addMenu(tr("&File"));
    menu->addMenu(editMenu_);
    auto viewMenuItem = menu->addMenu(tr("&View"));
    auto networkMenuItem = menu->addMenu(tr("&Network"));
    menu->addMenu(toolsMenu_);
    auto windowMenuItem = menu->addMenu("&Window");
    auto helpMenuItem = menu->addMenu(tr("&Help"));

    auto workspaceToolBar = addToolBar("File");
    workspaceToolBar->setObjectName("fileToolBar");
    workspaceToolBar->setMovable(false);
    workspaceToolBar->setFloatable(false);

    auto editToolBar = addToolBar("Edit");
    editToolBar->setObjectName("editToolBar");
    editToolBar->setMovable(false);
    editToolBar->setFloatable(false);

    auto findToolBar = addToolBar("Edit");
    findToolBar->setObjectName("findToolBar");
    findToolBar->setMovable(false);
    findToolBar->setFloatable(false);

    auto networkToolBar = addToolBar("Network");
    networkToolBar->setObjectName("networkToolBar");
    networkToolBar->setMovable(false);
    networkToolBar->setFloatable(false);

    // file menu entries

    {
        auto welcomeAction =
            new QAction(QIcon(":/svgicons/about-enabled.svg"), tr("&Get Started"), this);
        welcomeAction->setShortcut(Qt::SHIFT | Qt::CTRL | Qt::Key_N);
        welcomeAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        addAction(welcomeAction);
        connect(welcomeAction, &QAction::triggered, this, [this]() {
            toggleWelcomeScreen();
            // need to manually repaint the central widget after triggering the action from the file
            // menu. Otherwise the welcome widget is active but not redrawn.
            centralWidget_->repaint();
        });
        fileMenuItem->addAction(welcomeAction);
        fileMenuItem->addSeparator();
        workspaceToolBar->addAction(welcomeAction);
    }

    {
        auto newAction = new QAction(QIcon(":/svgicons/newfile.svg"), tr("&New Workspace"), this);
        newAction->setShortcut(QKeySequence::New);
        newAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        addAction(newAction);
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
        addAction(openAction);
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
        addAction(saveAction);
        connect(saveAction, &QAction::triggered, this,
                static_cast<bool (InviwoMainWindow::*)()>(&InviwoMainWindow::saveWorkspace));
        fileMenuItem->addAction(saveAction);
        workspaceToolBar->addAction(saveAction);
    }

    {
        auto saveAsAction =
            new QAction(QIcon(":/svgicons/save-as.svg"), tr("&Save Workspace As"), this);
        saveAsAction->setShortcut(QKeySequence::SaveAs);
        saveAsAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        addAction(saveAsAction);
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
        auto appendAction =
            new QAction(QIcon(":/svgicons/open-append.svg"), tr("&Append Workspace"), this);
        addAction(appendAction);
        connect(appendAction, &QAction::triggered, this, [this]() {
            if (appendWorkspace()) {
                hideWelcomeScreen();
            }
        });
        fileMenuItem->addAction(appendAction);
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
                    const std::filesystem::path path =
                        utilqt::toPath(saveFileDialog.selectedFiles().at(0));
                    networkEditorView_->exportViewToFile(path, entireScene,
                                                         backgroundVisibleAction->isChecked());
                    LogInfo("Exported network to " << path);
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

        // action for clearing the recent file menu
        auto clearRecentWorkspaces = recentWorkspaceMenu->addAction("Clear Recent Workspace List");
        clearRecentWorkspaces->setEnabled(false);
        connect(clearRecentWorkspaces, &QAction::triggered, this,
                [this, recentWorkspaceMenu, clearRecentWorkspaces]() {
                    auto actions = recentWorkspaceMenu->actions();
                    actions.pop_back();  // ignore the clear Item.
                    for (auto elem : actions) {
                        elem->setVisible(false);
                    }
                    // save empty list
                    saveRecentWorkspaceList(QStringList());
                    clearRecentWorkspaces->setEnabled(false);
                });

        connect(recentWorkspaceMenu, &QMenu::aboutToShow, this,
                [recentWorkspaceMenu, clearRecentWorkspaces, this]() {
                    qApp->installEventFilter(menuEventFilter_);
                    menuEventFilter_->add(recentWorkspaceMenu);

                    auto size = app_->getSettingsByType<EditorSettings>()->numRecentFiles;

                    while (recentWorkspaceMenu->actions().size() < size + 1) {
                        auto action = new QAction(this);
                        action->setVisible(false);
                        recentWorkspaceMenu->insertAction(recentWorkspaceMenu->actions().front(),
                                                          action);
                        connect(action, &QAction::triggered, this, [this, action]() {
                            if (qApp->keyboardModifiers() == Qt::ControlModifier) {
                                appendWorkspace(utilqt::toPath(action->data().toString()));
                                hideWelcomeScreen();
                            } else if (askToSaveWorkspaceChanges()) {
                                if (openWorkspace(utilqt::toPath(action->data().toString()))) {
                                    hideWelcomeScreen();
                                }
                            }
                        });
                    }
                    while (recentWorkspaceMenu->actions().size() > size + 1) {
                        recentWorkspaceMenu->removeAction(recentWorkspaceMenu->actions().front());
                    }

                    auto actions = recentWorkspaceMenu->actions();
                    actions.pop_back();  // ignore the clear Item.

                    for (auto elem : actions) {
                        elem->setVisible(false);
                    }

                    auto recentFiles = getRecentWorkspaceList();
                    const int maxRecentFiles = std::min(static_cast<int>(recentFiles.size()),
                                                        static_cast<int>(actions.size()));
                    for (int i = 0; i < maxRecentFiles; ++i) {
                        if (!recentFiles[i].isEmpty()) {
                            const bool exists = QFileInfo(recentFiles[i]).exists();
                            const auto menuEntry = QString("&%1 %2%3")
                                                       .arg(i + 1)
                                                       .arg(recentFiles[i])
                                                       .arg(exists ? "" : " (missing)");
                            actions[i]->setVisible(true);
                            actions[i]->setText(menuEntry);
                            actions[i]->setEnabled(exists);
                            actions[i]->setData(recentFiles[i]);
                            menuEventFilter_->add(actions[i], menuEntry);
                        }
                    }
                    clearRecentWorkspaces->setEnabled(!recentFiles.isEmpty());
                });
        connect(recentWorkspaceMenu, &QMenu::aboutToHide, this, [this]() {
            qApp->removeEventFilter(menuEventFilter_);
            menuEventFilter_->clear();
        });
    }

    // create list of all example workspaces
    exampleMenu_ = fileMenuItem->addMenu(tr("&Example Workspaces"));

    connect(exampleMenu_, &QMenu::aboutToShow, this, [this]() {
        qApp->installEventFilter(menuEventFilter_);
        menuEventFilter_->add(exampleMenu_);

        exampleMenu_->clear();
        for (const auto& inviwoModule : app_->getModuleManager().getInviwoModules()) {
            const auto moduleWorkspacePath = inviwoModule.getPath(ModulePath::Workspaces);
            if (!std::filesystem::is_directory(moduleWorkspacePath)) continue;
            auto menu =
                std::make_unique<QMenu>(QString::fromStdString(inviwoModule.getIdentifier()));
            for (auto file : filesystem::getDirectoryContents(moduleWorkspacePath)) {
                // only accept inviwo workspace files
                if (file.extension() != ".inv") continue;
                const auto qFile = utilqt::toQString(file);
                auto action = menu->addAction(qFile);
                menuEventFilter_->add(action, qFile);
                const auto path = moduleWorkspacePath / file;
                connect(action, &QAction::triggered, this, [this, path]() {
                    auto action = util::getModifierAction(QApplication::keyboardModifiers());
                    if (action == ModifierAction::AppendWorkspace) {
                        appendWorkspace(path);
                        hideWelcomeScreen();
                    } else if (askToSaveWorkspaceChanges()) {
                        // open examples as regular workspace with filename and full path if
                        // modifier key is pressed
                        if (openWorkspace(path, action != ModifierAction::OpenWithPath)) {
                            hideWelcomeScreen();
                        }
                    }
                });
            }
            if (!menu->isEmpty()) {
                menuEventFilter_->add(menu.get());
                exampleMenu_->addMenu(menu.release());
            }
        }
        if (exampleMenu_->isEmpty()) {
            exampleMenu_->addAction("No example workspaces found")->setEnabled(false);
        }
    });
    connect(exampleMenu_, &QMenu::aboutToHide, this, [this]() {
        qApp->removeEventFilter(menuEventFilter_);
        menuEventFilter_->clear();
    });

    // create list of all test workspaces
    testMenu_ = fileMenuItem->addMenu(tr("&Test Workspaces"));
    connect(testMenu_, &QMenu::aboutToShow, this, [this]() {
        qApp->installEventFilter(menuEventFilter_);
        menuEventFilter_->add(testMenu_);

        testMenu_->clear();
        for (const auto& inviwoModule : app_->getModuleManager().getInviwoModules()) {
            auto moduleTestPath = inviwoModule.getPath(ModulePath::RegressionTests);
            if (!std::filesystem::is_directory(moduleTestPath)) continue;
            auto menu =
                std::make_unique<QMenu>(QString::fromStdString(inviwoModule.getIdentifier()));
            for (auto test : filesystem::getDirectoryContents(moduleTestPath,
                                                              filesystem::ListMode::Directories)) {
                const auto testDir = moduleTestPath / test;
                if (!std::filesystem::is_directory(testDir)) continue;
                for (auto file : filesystem::getDirectoryContents(testDir)) {
                    // only accept inviwo workspace files
                    if (file.extension() != ".inv") continue;
                    auto qFile = utilqt::toQString(file);
                    auto action = menu->addAction(qFile);
                    menuEventFilter_->add(action, qFile);
                    auto path = testDir / file;
                    connect(action, &QAction::triggered, this, [this, path]() {
                        auto action = util::getModifierAction(QApplication::keyboardModifiers());
                        if (action == ModifierAction::AppendWorkspace) {
                            appendWorkspace(path);
                            hideWelcomeScreen();
                        } else if (askToSaveWorkspaceChanges()) {
                            if (openWorkspace(path)) {
                                hideWelcomeScreen();
                            }
                        }
                    });
                }
            }
            if (!menu->isEmpty()) {
                menuEventFilter_->add(menu.get());
                testMenu_->addMenu(menu.release());
            }
        }
        if (testMenu_->isEmpty()) {
            testMenu_->addAction("No test workspaces found")->setEnabled(false);
        }
    });

    connect(testMenu_, &QMenu::aboutToHide, this, [this]() {
        qApp->removeEventFilter(menuEventFilter_);
        menuEventFilter_->clear();
    });

    if (app_->getModuleManager().isRuntimeModuleReloadingEnabled()) {
        fileMenuItem->addSeparator();
        auto reloadAction = new QAction(tr("&Reload modules"), this);
        connect(reloadAction, &QAction::triggered, this,
                [&]() { app_->getModuleManager().reloadModules(); });
        fileMenuItem->addAction(reloadAction);
    }

#ifndef IVW_RELEASE
    {
        fileMenuItem->addSeparator();
        auto reloadStyle = fileMenuItem->addAction("Reload Style sheet");
        connect(reloadStyle, &QAction::triggered, [this](bool /*state*/) {
            // The following code snippet allows to reload the Qt style sheets during
            // runtime, which is handy while we change them.
            utilqt::setStyleSheetFile(app_->getPath(PathType::Resources) /
                                      "stylesheets/inviwo.qss");
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
        searchNetwork->setShortcut(Qt::SHIFT | Qt::CTRL | Qt::Key_F);
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
        addProcessorAction->setShortcut(Qt::CTRL | Qt::Key_D);
        connect(addProcessorAction, &QAction::triggered, this,
                [this]() { processorTreeWidget_->addSelectedProcessor(); });

        editMenu_->addSeparator();

        editMenu_->addAction(consoleWidget_->getClearAction());

        // add actions to tool bar
        editToolBar->addAction(undoManager_.getUndoAction());
        editToolBar->addAction(undoManager_.getRedoAction());

        findToolBar->addAction(searchNetwork);
        findToolBar->addAction(findAction);
        findToolBar->addAction(addProcessorAction);
    }

    // View
    {
        // dock widget visibility menu entries
        viewMenuItem->addAction(settings_->toggleViewAction());
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

    // Network
    {
        QIcon enableDisableIcon;
        enableDisableIcon.addFile(":/svgicons/unlocked.svg", QSize(), QIcon::Normal, QIcon::Off);
        enableDisableIcon.addFile(":/svgicons/locked.svg", QSize(), QIcon::Normal, QIcon::On);
        auto disableEvalAction =
            new QAction(enableDisableIcon, tr("Disable &Network Evaluation"), this);
        disableEvalAction->setCheckable(true);
        disableEvalAction->setChecked(false);

        disableEvalAction->setShortcut(Qt::CTRL | Qt::Key_L);
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
        compAction->setShortcut(Qt::CTRL | Qt::Key_G);
        connect(compAction, &QAction::triggered, this, [this]() {
            util::replaceSelectionWithCompositeProcessor(*(app_->getProcessorNetwork()));
        });

        auto expandAction = networkMenuItem->addAction(
            QIcon(":/svgicons/composite-expand-enabled.svg"), tr("&Expand Composite"));
        networkToolBar->addAction(expandAction);
        expandAction->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_G);
        expandAction->setEnabled(false);
        connect(expandAction, &QAction::triggered, this, [this]() {
            for (auto item : networkEditor_->selectedItems()) {
                if (auto pgi = qgraphicsitem_cast<ProcessorGraphicsItem*>(item)) {
                    if (auto comp = dynamic_cast<CompositeProcessor*>(pgi->getProcessor())) {
                        util::expandCompositeProcessorIntoNetwork(*comp);
                    }
                }
            }
        });

        auto updateButtonState = [this, expandAction, compAction] {
            bool selectedProcessor = false;
            bool selectedComposite = false;

            for (auto item : networkEditor_->selectedItems()) {
                if (auto pgi = qgraphicsitem_cast<ProcessorGraphicsItem*>(item)) {
                    selectedProcessor = true;
                    if (dynamic_cast<CompositeProcessor*>(pgi->getProcessor())) {
                        selectedComposite = true;
                    }
                }
            }
            expandAction->setEnabled(selectedComposite);
            compAction->setEnabled(selectedProcessor);
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
        auto showAllAction = new QAction("&Show All", this);
        auto hideAllAction = new QAction("&Hide All", this);
        windowMenuItem->addAction(showAllAction);
        windowMenuItem->addAction(hideAllAction);
        addAction(showAllAction);
        addAction(hideAllAction);

        showAllAction->setShortcut(Qt::SHIFT | Qt::CTRL | Qt::Key_H);
        showAllAction->setShortcutContext(Qt::ApplicationShortcut);

        hideAllAction->setShortcut(Qt::ALT | Qt::CTRL | Qt::Key_H);
        hideAllAction->setShortcutContext(Qt::ApplicationShortcut);

        QObject::connect(showAllAction, &QAction::triggered, this, [this]() {
            for (const auto p : detail::getWidgetProcessors(app_)) {
                p->getProcessorWidget()->setVisible(true);
            }
            for (const auto dockWidget : detail::getFloatingDockWidgets(this)) {
                dockWidget->show();
            }
        });
        QObject::connect(hideAllAction, &QAction::triggered, this, [this]() {
            for (const auto p : detail::getWidgetProcessors(app_)) {
                p->getProcessorWidget()->setVisible(false);
            }
            for (const auto dockWidget : detail::getFloatingDockWidgets(this)) {
                dockWidget->hide();
            }
        });

        QObject::connect(
            windowMenuItem, &QMenu::aboutToShow, this,
            [this, windowMenuItem, showAllAction, hideAllAction]() {
                windowMenuItem->clear();
                windowMenuItem->addAction(showAllAction);
                windowMenuItem->addAction(hideAllAction);

                const auto widgetProcessors = detail::getWidgetProcessors(app_);
                if (!widgetProcessors.empty()) windowMenuItem->addSeparator();

                for (const auto p : widgetProcessors) {
                    auto item =
                        windowMenuItem->addMenu(QString("%1 (%2)")
                                                    .arg(utilqt::toQString(p->getDisplayName()))
                                                    .arg(utilqt::toQString(p->getIdentifier())));
                    auto visible = item->addAction("Visible");
                    visible->setCheckable(true);
                    visible->setChecked(p->getProcessorWidget()->isVisible());
                    QObject::connect(visible, &QAction::toggled, this, [p](bool toggle) {
                        p->getProcessorWidget()->setVisible(toggle);
                    });

                    auto opTop = item->addAction("On Top");
                    opTop->setCheckable(true);
                    opTop->setChecked(p->getProcessorWidget()->isOnTop());
                    QObject::connect(opTop, &QAction::toggled, this, [p](bool toggle) {
                        p->getProcessorWidget()->setOnTop(toggle);
                    });

                    auto fullScreen = item->addAction("Full Screen");
                    fullScreen->setCheckable(true);
                    fullScreen->setChecked(p->getProcessorWidget()->isFullScreen());
                    QObject::connect(fullScreen, &QAction::toggled, this, [p](bool toggle) {
                        p->getProcessorWidget()->setFullScreen(toggle);
                    });

                    auto raise = item->addAction("Raise");
                    QObject::connect(raise, &QAction::triggered, this, [p]() {
                        if (auto w = dynamic_cast<QWidget*>(p->getProcessorWidget())) {
                            w->raise();
                        }
                    });
                    auto lower = item->addAction("Lower");
                    QObject::connect(lower, &QAction::triggered, this, [p]() {
                        if (auto w = dynamic_cast<QWidget*>(p->getProcessorWidget())) {
                            w->lower();
                        }
                    });
                }

                const auto dockWidgets = detail::getFloatingDockWidgets(this);
                if (!dockWidgets.empty()) windowMenuItem->addSeparator();

                for (const auto dockWidget : dockWidgets) {
                    auto action =
                        windowMenuItem->addAction(QString("%1").arg(dockWidget->windowTitle()));
                    action->setCheckable(true);
                    action->setChecked(dockWidget->isVisible());
                    QObject::connect(action, &QAction::toggled, this,
                                     [dockWidget](bool toggle) { dockWidget->setVisible(toggle); });
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
    setWindowTitle(utilqt::toQString(
        fmt::format("Inviwo v{} - Interactive Visualization Workshop - {}{}", inviwo::build::version, currentWorkspaceFileName_,
                    getNetworkEditor()->isModified() ? "*" : "")));
}

void InviwoMainWindow::addToRecentWorkspaces(const std::filesystem::path& workspaceFileName) {
    QStringList recentFiles{getRecentWorkspaceList()};

    recentFiles.removeAll(utilqt::toQString(workspaceFileName));
    recentFiles.prepend(utilqt::toQString(workspaceFileName));

    if (recentFiles.size() > app_->getSettingsByType<EditorSettings>()->numRecentFiles) {
        recentFiles.removeLast();
    }
    saveRecentWorkspaceList(recentFiles);
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

    if (welcomeWidget_) {
        welcomeWidget_->updateRecentWorkspaces(list);
    }
}

void InviwoMainWindow::setCurrentWorkspace(const std::filesystem::path& workspaceFileName) {
    workspaceFileDir_ = std::filesystem::absolute(workspaceFileName);
    currentWorkspaceFileName_ = workspaceFileName;
    updateWindowTitle();
}

const std::filesystem::path& InviwoMainWindow::getCurrentWorkspace() {
    return currentWorkspaceFileName_;
}

bool InviwoMainWindow::newWorkspace() {
    if (currentWorkspaceFileName_ != "")
        if (!askToSaveWorkspaceChanges()) return false;

    app_->getWorkspaceManager()->clear();

    setCurrentWorkspace(untitledWorkspaceName_);
    getNetworkEditor()->setModified(false);
    return true;
}

bool InviwoMainWindow::openWorkspace(const std::filesystem::path& workspaceFileName) {
    return openWorkspace(workspaceFileName, false);
}

bool InviwoMainWindow::openWorkspaceAskToSave(const std::filesystem::path& workspaceFileName) {
    if (askToSaveWorkspaceChanges()) {
        return openWorkspace(workspaceFileName, false);
    } else {
        return false;
    }
}

bool InviwoMainWindow::openExample(const std::filesystem::path& exampleFileName) {
    return openWorkspace(exampleFileName, true);
}

void InviwoMainWindow::openLastWorkspace(const std::filesystem::path& workspace) {
    QSettings settings;
    const bool showWelcomePage = settings.value("WelcomeWidget/showWelcomePage", true).toBool();
    const bool loadLastWorkspace =
        settings.value("WelcomeWidget/autoloadLastWorkspace", false).toBool();

    const auto loadSuccessful = [&]() {
        if (!workspace.empty()) {
            return openWorkspace(workspace);
        } else if (loadLastWorkspace && !workspaceOnLastSuccessfulExit_.empty()) {
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

std::optional<std::filesystem::path> InviwoMainWindow::askForWorkspaceToOpen() {
    if (askToSaveWorkspaceChanges()) {
        InviwoFileDialog openFileDialog(this, "Open Workspace ...", "workspace");
        openFileDialog.addSidebarPath(PathType::Workspaces);
        openFileDialog.addSidebarPath(workspaceFileDir_);
        openFileDialog.addExtension("inv", "Inviwo File");
        openFileDialog.setFileMode(FileMode::AnyFile);

        if (openFileDialog.exec()) {
            QString path = openFileDialog.selectedFiles().at(0);
            return utilqt::toPath(path);
        }
    }
    return std::nullopt;
}

bool InviwoMainWindow::openWorkspace() {
    if (auto path = askForWorkspaceToOpen()) {
        return openWorkspace(*path);
    }
    return false;
}

bool InviwoMainWindow::appendWorkspace() {
    if (auto path = askForWorkspaceToOpen()) {
        appendWorkspace(*path);
        return true;
    }
    return false;
}

bool InviwoMainWindow::openWorkspace(const std::filesystem::path& fileName, bool isExample) {

    if (!std::filesystem::is_regular_file(fileName)) {
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
                    util::logError(e.getContext(), "Incomplete network loading {} due to {}",
                                   fileName, e.getMessage());
                }
            });

            if (isExample) {
                setCurrentWorkspace(untitledWorkspaceName_);
            } else {
                setCurrentWorkspace(fileName);
                addToRecentWorkspaces(fileName);
            }
        } catch (const Exception& e) {
            util::logError(e.getContext(), "Unable to load network {} due to {}", fileName,
                           e.getMessage());
            app_->getWorkspaceManager()->clear();
            setCurrentWorkspace(untitledWorkspaceName_);
        }
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);  // make sure the gui is ready
                                                                  // before we unlock.
    }
    getNetworkEditor()->setModified(false);
    return true;
}

void InviwoMainWindow::appendWorkspace(const std::filesystem::path& file) {
    NetworkLock lock(app_->getProcessorNetwork());
    networkEditor_->append(file);
    qApp->processEvents();  // make sure the gui is ready before we unlock.
}

bool InviwoMainWindow::saveWorkspace(const std::filesystem::path& fileName) {

    util::OnScopeExit onExit(nullptr);
    if (welcomeWidget_ && centralWidget_->currentWidget() == welcomeWidget_) {
        // When the Welcome widget is visible, we need to restore and show all previously hidden
        // widgets prior saving the workspace, then hide them again afterward
        visibleWidgetState_.show();
        onExit.setAction([this]() { visibleWidgetState_.hide(this); });
    }

    try {
        app_->getWorkspaceManager()->save(fileName, [&](ExceptionContext ec) {
            try {
                throw;
            } catch (const IgnoreException& e) {
                util::logError(e.getContext(), "Incomplete network save {} due to {}", fileName,
                               e.getMessage());
            }
        });
        getNetworkEditor()->setModified(false);
        updateWindowTitle();
        LogInfo(fmt::format("Workspace saved to: {}", fileName));
        return true;
    } catch (const Exception& e) {
        util::logError(e.getContext(), "Unable to save network {} due to {}", fileName,
                       e.getMessage());
    }
    return false;
}

bool InviwoMainWindow::saveWorkspace() {
    if (currentWorkspaceFileName_ == untitledWorkspaceName_)
        return saveWorkspaceAs();
    else {
        return saveWorkspace(currentWorkspaceFileName_);
    }
}

bool InviwoMainWindow::saveWorkspaceAs() {
    InviwoFileDialog saveFileDialog(this, "Save Workspace ...", "workspace");
    saveFileDialog.setFileMode(FileMode::AnyFile);
    saveFileDialog.setAcceptMode(AcceptMode::Save);
    saveFileDialog.setOption(QFileDialog::Option::DontConfirmOverwrite, false);

    saveFileDialog.addSidebarPath(PathType::Workspaces);
    saveFileDialog.addSidebarPath(workspaceFileDir_);

    saveFileDialog.addExtension("inv", "Inviwo File");

    bool savedWorkspace = false;
    if (saveFileDialog.exec()) {
        std::filesystem::path path = utilqt::toPath(saveFileDialog.selectedFiles().at(0));
        if (path.extension() != ".inv") path += ".inv";

        saveWorkspace(path);
        setCurrentWorkspace(path);
        addToRecentWorkspaces(path);
        savedWorkspace = true;
    }
    return savedWorkspace;
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
        std::filesystem::path path = utilqt::toPath(saveFileDialog.selectedFiles().at(0));
        if (path.extension() != ".inv") path += ".inv";

        if (saveWorkspace(path)) {
            addToRecentWorkspaces(path);
        }
    }
}

void InviwoMainWindow::toggleWelcomeScreen() {
    if (getWelcomeWidget()->isVisible()) {
        hideWelcomeScreen();
    } else {
        showWelcomeScreen();
    }
}

void InviwoMainWindow::showWelcomeScreen() {
    setUpdatesEnabled(false);

    // The order of hiding and showing widget here is important. Need to hide the non-floating
    // dockwidgets first or they will end up being included in the size of the welcome app window
    // even if the are not visible.
    // Floating dock widgets and processor widgets are hidden, too. Otherwise they will be on top of
    // the main window.
    visibleWidgetState_.hide(this);

    auto welcomeWidget = getWelcomeWidget();
    centralWidget_->addWidget(welcomeWidget);
    centralWidget_->setCurrentWidget(welcomeWidget);
    welcomeWidget->enableRestoreButton(hasRestoreWorkspace());

    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    setUpdatesEnabled(true);
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    welcomeWidget->setFocus();
}

void InviwoMainWindow::hideWelcomeScreen() {
    if (!welcomeWidget_ || !welcomeWidget_->isVisible()) {
        return;
    }
    setUpdatesEnabled(false);

    centralWidget_->setCurrentWidget(networkEditorView_);
    centralWidget_->removeWidget(getWelcomeWidget());
    // restore previously hidden dock and processor widgets
    visibleWidgetState_.show();

    setUpdatesEnabled(true);
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
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

WelcomeWidget* inviwo::InviwoMainWindow::getWelcomeWidget() {
    // The welcome widget is rather expensive to keep around, so do not create it unnecessarily.
    // Tested on Windows, where 50 MB were released upon deletion.
    if (!welcomeWidget_) {
        welcomeWidget_ = new WelcomeWidget(getInviwoApplication(), centralWidget_);
        welcomeWidget_->updateRecentWorkspaces(getRecentWorkspaceList());

        connect(welcomeWidget_, &WelcomeWidget::loadWorkspace, this,
                [this](const std::filesystem::path& filename, bool isExample) {
                    if (askToSaveWorkspaceChanges()) {
                        hideWelcomeScreen();
                        if (isExample) {
                            openExample(filename);
                        } else {
                            openWorkspace(filename);
                        }
                        saveWindowState();
                    }
                });
        connect(welcomeWidget_, &WelcomeWidget::appendWorkspace, this,
                [this](const std::filesystem::path& filename) {
                    hideWelcomeScreen();
                    appendWorkspace(filename);
                    saveWindowState();
                });
        connect(welcomeWidget_, &WelcomeWidget::openWorkspace, this, [this]() {
            if (auto path = askForWorkspaceToOpen()) {
                hideWelcomeScreen();
                openWorkspace(*path);
                saveWindowState();
            }
        });
        connect(welcomeWidget_, &WelcomeWidget::newWorkspace, this, [this]() {
            hideWelcomeScreen();
            newWorkspace();
            saveWindowState();
        });
        connect(welcomeWidget_, &WelcomeWidget::restoreWorkspace, this, [this]() {
            hideWelcomeScreen();
            if (askToSaveWorkspaceChanges()) {
                restoreWorkspace();
            }
            saveWindowState();
        });
    }

    return welcomeWidget_;
}

NetworkEditor* InviwoMainWindow::getNetworkEditor() const { return networkEditor_.get(); }

void InviwoMainWindow::exitInviwo(bool saveIfModified) {
    if (!saveIfModified) getNetworkEditor()->setModified(false);
    QMainWindow::close();
    qApp->exit();
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
    // clear dangling pointers to processors with processor widgets
    visibleWidgetState_.processors.clear();
    hideWelcomeScreen();

    saveWindowState();

    QSettings settings;
    settings.beginGroup(objectName());
    if (currentWorkspaceFileName_ == untitledWorkspaceName_) {
        settings.setValue("workspaceOnLastSuccessfulExit", "");
    } else {
        settings.setValue("workspaceOnLastSuccessfulExit",
                          utilqt::toQString(currentWorkspaceFileName_));
    }
    settings.endGroup();

    // loop over all children and trigger close events _before_ the main window is closed and
    // destroyed. This way, the main window can stay open if a child widget does not close, i.e.
    // calls event->ignore().
    // @see QWidget::closeEvent()
    for (auto& child : children()) {
        QApplication::sendEvent(child, event);
        if (!event->isAccepted()) {
            return;
        }
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

SettingsWidget* InviwoMainWindow::getSettingsWidget() const { return settings_; }

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

InviwoEditMenu* InviwoMainWindow::getInviwoEditMenu() const { return editMenu_; }
ToolsMenu* InviwoMainWindow::getToolsMenu() const { return toolsMenu_; }

void InviwoMainWindow::dragEnterEvent(QDragEnterEvent* event) { dragMoveEvent(event); }

void InviwoMainWindow::dragMoveEvent(QDragMoveEvent* event) {
    auto mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        QList<QUrl> urlList = mimeData->urls();
        std::filesystem::path filename = utilqt::toPath(urlList.front().toLocalFile());
        auto ext = toLower(filename.extension().string());

        if (ext == ".inv" ||
            !app_->getDataVisualizerManager()->getDataVisualizersForFile(filename).empty()) {

            if (event->modifiers() & Qt::ControlModifier) {
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
        // use dispatch front here to avoid blocking the drag&drop source, e.g. Windows
        // Explorer, while the drop operation is performed
        auto action = [this, keyModifiers = event->modifiers(), urlList = mimeData->urls()]() {
            RenderContext::getPtr()->activateDefaultRenderContext();

            hideWelcomeScreen();

            bool first = true;
            for (auto& file : urlList) {
                std::filesystem::path filename = utilqt::toPath(file.toLocalFile());

                if (toLower(filename.extension().string()) == ".inv") {
                    if (!first || keyModifiers & Qt::ControlModifier) {
                        appendWorkspace(filename);
                    } else {
                        openWorkspaceAskToSave(filename);
                    }
                } else {
                    util::insertNetworkForData(
                        filename, app_->getProcessorNetwork(),
                        static_cast<bool>(keyModifiers & Qt::ControlModifier),
                        static_cast<bool>(keyModifiers & Qt::AltModifier), this);
                }
                first = false;
            }
            saveWindowState();
            undoManager_.pushStateIfDirty();
        };
        app_->dispatchFrontAndForget(action);

        event->accept();
    } else {
        event->ignore();
    }
}

void InviwoMainWindow::VisibleWidgets::hide(InviwoMainWindow* win) {
    dockwidgets = util::copy_if(win->findChildren<QDockWidget*>(),
                                [](const auto w) { return w->isVisible(); });
    processors = util::copy_if(
        win->getInviwoApplication()->getProcessorNetwork()->getProcessors(), [](const auto p) {
            return p->hasProcessorWidget() && p->getProcessorWidget()->isVisible();
        });

    for (const auto p : processors) {
        p->getProcessorWidget()->setVisible(false);
    }
    for (const auto dockWidget : dockwidgets) {
        dockWidget->hide();
    }
}

void InviwoMainWindow::VisibleWidgets::show() {
    for (const auto p : processors) {
        p->getProcessorWidget()->setVisible(true);
    }
    for (const auto dockWidget : dockwidgets) {
        dockWidget->show();
    }
    processors.clear();
    dockwidgets.clear();
}

}  // namespace inviwo
