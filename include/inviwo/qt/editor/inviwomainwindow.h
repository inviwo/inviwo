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

#ifndef IVW_INVIWOMAINWINDOW_H
#define IVW_INVIWOMAINWINDOW_H

#include <inviwo/qt/editor/inviwoqteditordefine.h>
#include <inviwo/qt/editor/networkeditorobserver.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/network/workspacemanager.h>
#include <inviwo/qt/editor/undomanager.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <warn/pop>

#include <tclap/CmdLine.h>

class QDropEvent;
class QDragEnterEvent;
class QTabWidget;

namespace inviwo {

class NetworkEditorView;
class NetworkEditor;
class PropertyListWidget;
class ProcessorTreeWidget;
class ConsoleWidget;
class SettingsWidget;
class HelpWidget;
class WelcomeWidget;
class AnnotationsWidget;
class InviwoApplicationQt;
class InviwoEditMenu;
class InviwoAboutWindow;
class ResourceManagerDockWidget;
class FileAssociations;
class ToolsMenu;
class TextLabelOverlay;

class IVW_QTEDITOR_API InviwoMainWindow : public QMainWindow, public NetworkEditorObserver {
public:
    static const unsigned int maxNumRecentFiles_ = 10;

    InviwoMainWindow(InviwoApplicationQt* app);
    virtual ~InviwoMainWindow();

    void showWindow();

    /**
     * loads the given example workspace.
     *
     * @return true if the example was opened, otherwise false.
     */
    bool openExample(QString exampleFileName);

    void openLastWorkspace(std::string workspace = "");
    /**
     * loads the given workspace.
     *
     * @return true if the workspace was opened, otherwise false.
     */
    bool openWorkspace(QString workspaceFileName);

    /**
     * loads the given workspace. In case there are unsaved changes, the user will be asked to save
     * or discard them, or cancel the loading.
     * @return true if the workspace was opened, otherwise false.
     */
    bool openWorkspaceAskToSave(QString workspaceFileName);
    std::string getCurrentWorkspace();

    NetworkEditor* getNetworkEditor() const;
    NetworkEditorView& getNetworkEditorView() const;
    TextLabelOverlay& getNetworkEditorOverlay() const;
    SettingsWidget* getSettingsWidget() const;
    ProcessorTreeWidget* getProcessorTreeWidget() const;
    PropertyListWidget* getPropertyListWidget() const;
    ConsoleWidget* getConsoleWidget() const;
    AnnotationsWidget* getAnnotationsWidget() const;
    HelpWidget* getHelpWidget() const;
    InviwoApplication* getInviwoApplication() const;
    InviwoApplicationQt* getInviwoApplicationQt() const;

    InviwoEditMenu* getInviwoEditMenu() const;
    ToolsMenu* getToolsMenu() const;

    /**
     * sets up an empty workspace. In case there are unsaved changes, the user will be asked to save
     * or discard them, or cancel the task.
     *
     * @return true if the workspace was opened, otherwise false.
     * @see askToSaveWorkspaceChanges
     */
    bool newWorkspace();
    /**
     * shows a file dialog for loading a workspace. In case there are unsaved changes, the user will
     * be asked to save or discard them, or cancel the loading.
     *
     * @return true if the workspace was opened, otherwise false.
     * @see askToSaveWorkspaceChanges
     */
    bool openWorkspace();

    void saveWorkspace();
    void saveWorkspaceAs();

    /*
     * Save the current workspace into a new workspace file,
     * leaves the current workspace file as current workspace
     */
    void saveWorkspaceAsCopy();
    bool askToSaveWorkspaceChanges();
    void exitInviwo(bool saveIfModified = true);
    void showAboutBox();

    void showWelcomeScreen();
    void hideWelcomeScreen();

    /**
     * \brief query the Qt settings for recent workspaces
     */
    QStringList getRecentWorkspaceList() const;

    bool hasRestoreWorkspace() const;
    void restoreWorkspace();

protected:
    virtual void dragEnterEvent(QDragEnterEvent* event) override;
    virtual void dragMoveEvent(QDragMoveEvent* event) override;
    virtual void dropEvent(QDropEvent* event) override;

private:
    virtual void onModifiedStatusChanged(const bool& newStatus) override;

    void visibilityModeChangedInSettings();

    /**
     * loads the workspace \p workspaceFileName. In case there are unsaved changes, the user will
     * be asked to save or discard them, or cancel the loading.
     *
     * @param isExample    if true, the workspace file name will not be set. Thereby preventing
     *                     the user from accidentally overwriting the original file. In addition,
     *                     the workspace is _not_ added to the recent file list.
     * @return true if the workspace was opened, otherwise false.
     * @see askToSaveWorkspaceChanges
     */
    bool openWorkspace(QString workspaceFileName, bool isExample);
    void saveWorkspace(QString workspaceFileName);
    void appendWorkspace(const std::string& workspaceFileName);

    void addActions();

    void closeEvent(QCloseEvent* event) override;

    void saveWindowState();
    void loadWindowState();

    void saveCanvases(std::string path, std::string fileName);
    void getScreenGrab(std::string path, std::string fileName);

    void addToRecentWorkspaces(QString workspaceFileName);

    /**
     * \brief update Qt settings for recent workspaces with internal status
     */
    void saveRecentWorkspaceList(const QStringList& list);
    void setCurrentWorkspace(QString workspaceFileName);

    void updateWindowTitle();

    InviwoApplicationQt* app_;
    InviwoEditMenu* editMenu_ = nullptr;
    ToolsMenu* toolsMenu_ = nullptr;
    QMenu* exampleMenu_ = nullptr;
    QMenu* testMenu_ = nullptr;
    std::shared_ptr<ConsoleWidget> consoleWidget_;
    std::unique_ptr<NetworkEditor> networkEditor_;
    QTabWidget* centralWidget_;
    NetworkEditorView* networkEditorView_;

    SettingsWidget* settingsWidget_;
    ProcessorTreeWidget* processorTreeWidget_;
    ResourceManagerDockWidget* resourceManagerDockWidget_;
    PropertyListWidget* propertyListWidget_;
    HelpWidget* helpWidget_;
    std::unique_ptr<WelcomeWidget> welcomeWidget_;
    AnnotationsWidget* annotationsWidget_ = nullptr;
    InviwoAboutWindow* inviwoAboutWindow_ = nullptr;

    std::vector<QAction*> workspaceActionRecent_;
    QAction* clearRecentWorkspaces_;
    QAction* visibilityModeAction_;

    std::unique_ptr<FileAssociations> fileAssociations_;

    WorkspaceManager::SerializationHandle annotationSerializationHandle_;
    WorkspaceManager::DeserializationHandle annotationDeserializationHandle_;
    WorkspaceManager::ClearHandle annotationClearHandle_;

    // settings
    bool maximized_;

    // paths
    const QString untitledWorkspaceName_;
    QString rootDir_;
    QString workspaceFileDir_;
    QString currentWorkspaceFileName_;
    QString workspaceOnLastSuccessfulExit_;

    // command line switches
    TCLAP::ValueArg<std::string> snapshotArg_;
    TCLAP::ValueArg<std::string> screenGrabArg_;
    TCLAP::ValueArg<std::string> saveProcessorPreviews_;
    TCLAP::ValueArg<std::string> openData_;
    TCLAP::SwitchArg updateWorkspaces_;
    TCLAP::SwitchArg updateRegressionWorkspaces_;
    TCLAP::ValueArg<std::string> updateWorkspacesInPath_;

    UndoManager undoManager_;
};

}  // namespace inviwo

#endif  // IVW_INVIWOMAINWINDOW_H
