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

#include <inviwo/qt/editor/inviwoqteditordefine.h>
#include <inviwo/qt/editor/networkeditorobserver.h>
#include <inviwo/core/properties/optionproperty.h>
#include <warn/push>
#include <warn/ignore/all>
#include <QMainWindow>
#include <QDockWidget>
#include <QListWidget>
#include <QToolBar>
#include <QToolButton>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <warn/pop>

namespace inviwo {

class NetworkEditorView;
class NetworkEditor;
class PropertyListWidget;
class ProcessorTreeWidget;
class ResourceManagerWidget;
class ConsoleWidget;
class SettingsWidget;
class HelpWidget;
class InviwoApplication;

class IVW_QTEDITOR_API InviwoMainWindow : public QMainWindow, public NetworkEditorObserver {
#include <warn/push>
#include <warn/ignore/all>
    Q_OBJECT
#include <warn/pop>
public:
    static const unsigned int maxNumRecentFiles_ = 10;

    InviwoMainWindow(InviwoApplication* app);
    virtual ~InviwoMainWindow();

    void initialize();
    void showWindow();

    void openLastWorkspace();
    void openWorkspace(QString workspaceFileName);
    std::string getCurrentWorkspace();

    bool processCommandLineArgs();

    virtual void onNetworkEditorFileChanged(const std::string& filename) override;
    virtual void onModifiedStatusChanged(const bool& newStatus) override;

    void visibilityModeChangedInSettings();

    NetworkEditor* getNetworkEditor() const;
    SettingsWidget*  getSettingsWidget() const;
    ProcessorTreeWidget*  getProcessorTreeWidget() const;
    PropertyListWidget*  getPropertyListWidget() const;
    ConsoleWidget*  getConsoleWidget() const;
    ResourceManagerWidget*  getResourceManagerWidget() const;
    HelpWidget*  getHelpWidget() const;
    InviwoApplication* getInviwoApplication() const;
    const std::unordered_map<std::string, QAction*>& getActions() const;

public slots:
    void newWorkspace();
    void openWorkspace();
    void openRecentWorkspace();
    /**
    * \brief clear the recent file list in the menu
    */
    void clearRecentWorkspaceMenu();
    void openExampleWorkspace();
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
    void setVisibilityMode(bool value);  // True = Application, False = Developer

    void reloadStyleSheet();

private:
    void addMenus();
    void addMenuActions();
    void addToolBars();
    void closeEvent(QCloseEvent* event) override;

    void saveWindowState();
    void loadWindowState();

    bool askToSaveWorkspaceChanges();

    void addToRecentWorkspaces(QString workspaceFileName);
    /**
    * \brief update the recent file list in the menu based on the internal status
    */
    void updateRecentWorkspaceMenu();
    /**
    * \brief query the Qt settings for recent workspaces and update internal status
    */
    QStringList getRecentWorkspaceList() const;
    /**
     * \brief update Qt settings for recent workspaces with internal status
     */
    void saveRecentWorkspaceList(const QStringList& list);
    void setCurrentWorkspace(QString workspaceFileName);

    void updateWindowTitle();

    /**
     * \brief compile a list of example workspaces and update the menu
     */
    void fillExampleWorkspaceMenu();
    /**
    * \brief compile a list of test workspaces from inviwo-dev and external
    * modules and update the menu
    */
    void fillTestWorkspaceMenu();

    InviwoApplication* app_;
    NetworkEditor* networkEditor_;
    NetworkEditorView* networkEditorView_;
    OptionPropertyInt* appUsageModeProp_;

    // toolbar
    QToolBar* basicToolbar_;

    // dock widgets
    SettingsWidget* settingsWidget_;
    ProcessorTreeWidget* processorTreeWidget_;
    PropertyListWidget* propertyListWidget_;
    ConsoleWidget* consoleWidget_;
    ResourceManagerWidget* resourceManagerWidget_;
    HelpWidget* helpWidget_;

    // menus
    QMenu* fileMenuItem_;
    QMenu* editMenuItem_;
    QMenu* viewMenuItem_;
    QMenu* helpMenuItem_;

    QMenu* recentWorkspaceMenu_;
    QMenu* testWorkspaceMenu_;
    QMenu* exampleWorkspaceMenu_;

    // menu actions
    std::unordered_map<std::string, QAction*> actions_;
    
    QAction* workspaceActionNew_;
    QAction* workspaceActionOpen_;
    QAction* workspaceActionSave_;
    QAction* workspaceActionSaveAs_;
    QAction* workspaceActionSaveAsCopy_;
    QAction* exitAction_;
    std::vector<QAction*> workspaceActionRecent_;
    QAction* clearRecentWorkspaces_;
    QAction* visibilityModeAction_;
    QAction* aboutBoxAction_;
    QToolButton* enableDisableEvaluationButton_;
#if IVW_PROFILING
    QToolButton* resetTimeMeasurementsButton_;
#endif
    // toolbars
    QToolBar* workspaceToolBar_;
    QToolBar* viewModeToolBar_;

    // settings
    bool maximized_;
    bool exampleWorkspaceOpen_;

    // paths
    QString rootDir_;
    QString workspaceFileDir_;
    QString currentWorkspaceFileName_;
    QString workspaceOnLastSuccessfulExit_;
};

}  // namespace

#endif  // IVW_INVIWOMAINWINDOW_H
