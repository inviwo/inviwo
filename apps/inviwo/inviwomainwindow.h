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

#ifndef IVW_INVIWOMAINWINDOW_H
#define IVW_INVIWOMAINWINDOW_H

#include <QMainWindow>
#include <QDockWidget>
#include <QListWidget>
#include <QToolBar>
#include <QToolButton>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <inviwo/core/properties/baseoptionproperty.h>
#include <inviwo/qt/editor/networkeditor.h>

namespace inviwo {

class NetworkEditorView;
class NetworkEditor;
class PropertyListWidget;
class ProcessorTreeWidget;
class ResourceManagerWidget;
class ConsoleWidget;
class SettingsWidget;
class HelpWidget;

class InviwoMainWindow : public QMainWindow,
    public NetworkEditorObserver,
    public ProcessorNetworkObserver {
    Q_OBJECT
public:
    static const unsigned int maxNumRecentFiles_ = 10;

    InviwoMainWindow();
    ~InviwoMainWindow();

    virtual void initialize();
    virtual void showWindow();
    virtual void deinitialize();
    virtual void initializeWorkspace();

    void openLastWorkspace();
    void openWorkspace(QString workspaceFileName);
    std::string getCurrentWorkspace();

    bool processCommandLineArgs();

    virtual void onProcessorNetworkChange();
    virtual void onNetworkEditorFileChanged(const std::string& filename);
    virtual void onModifiedStatusChanged(const bool &newStatus);

    void visibilityModeChangedInSettings();

    NetworkEditor* getNetworkEditor() { return networkEditor_; }

public slots:
    void newWorkspace();
    void openWorkspace();
    void openRecentWorkspace();
    void saveWorkspace();
    void saveWorkspaceAs();

    /*
    * Save the current workspace into a new workspace file, 
    * leaves the current workspace file as current workspace
    */
    void saveWorkspaceAsCopy();
    void exitInviwo();
    void disableEvaluation(bool);
    void showAboutBox();
    void setVisibilityMode(bool value); // True = Application, False = Developer

private:
    void addMenus();
    void addMenuActions();
    void addToolBars();
    void closeEvent(QCloseEvent* event);

    void saveWindowState();
    void loadWindowState();

    bool askToSaveWorkspaceChanges();

    void addToRecentWorkspaces(QString workspaceFileName);
    void updateRecentWorkspaces();
    void setCurrentWorkspace(QString workspaceFileName);

    void updateWindowTitle();

    NetworkEditor* networkEditor_;
    NetworkEditorView* networkEditorView_;
    OptionPropertyInt* visibilityModeProperty_;

    // mainwindow toolbar
    QToolBar* basicToolbar_;

    // dock widgets
    SettingsWidget* settingsWidget_;
    ProcessorTreeWidget* processorTreeWidget_;
    PropertyListWidget* propertyListWidget_;
    ConsoleWidget* consoleWidget_;
    ResourceManagerWidget* resourceManagerWidget_;
    HelpWidget* helpWidget_;

    // mainwindow menus
    QMenuBar* menuBar_;
    QMenu* fileMenuItem_;
    QMenu* viewMenuItem_;
    QMenu* viewModeItem_;
    QMenu* helpMenuItem_;

    // mainwindow menuactions
    QAction* workspaceActionNew_;
    QAction* workspaceActionOpen_;
    QAction* workspaceActionSave_;
    QAction* workspaceActionSaveAs_;
    QAction* workspaceActionSaveAsCopy_;
    QAction* exitAction_;
    QAction* recentFileSeparator_;
    QAction* workspaceActionRecent_[maxNumRecentFiles_];
    QAction* visibilityModeAction_;
    QAction* aboutBoxAction_;
    QToolButton* enableDisableEvaluationButton_;
#if IVW_PROFILING
    QToolButton* resetTimeMeasurementsButton_;
#endif
    // mainwindow toolbars
    QToolBar* workspaceToolBar_;
    QToolBar* viewModeToolBar_;

    // settings
    bool maximized_;

    // paths
    QString rootDir_;
    QString workspaceFileDir_;
    QString currentWorkspaceFileName_;
    QString workspaceOnLastSucessfullExit_;
    QStringList recentFileList_;
};

} // namespace

#endif // IVW_INVIWOMAINWINDOW_H
