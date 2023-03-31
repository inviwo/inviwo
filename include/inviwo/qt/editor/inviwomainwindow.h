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

#pragma once

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
class QStackedWidget;

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
class InviwoApplication;
class InviwoDockWidget;
class InviwoEditMenu;
class InviwoAboutWindow;
class ResourceManagerDockWidget;
class FileAssociations;
class ToolsMenu;
class TextLabelOverlay;
class MenuKeyboardEventFilter;
class Processor;

class IVW_QTEDITOR_API InviwoMainWindow : public QMainWindow, public NetworkEditorObserver {
public:
    InviwoMainWindow(InviwoApplication* app);
    virtual ~InviwoMainWindow();

    void showWindow();

    /**
     * loads the given example workspace.
     *
     * @return true if the example was opened, otherwise false.
     */
    bool openExample(const std::filesystem::path& exampleFileName);

    void openLastWorkspace(const std::filesystem::path& workspace = {});
    /**
     * loads the given workspace.
     *
     * @return true if the workspace was opened, otherwise false.
     */
    bool openWorkspace(const std::filesystem::path& workspaceFileName);

    /**
     * loads the given workspace. In case there are unsaved changes, the user will be asked to save
     * or discard them, or cancel the loading.
     * @return true if the workspace was opened, otherwise false.
     */
    bool openWorkspaceAskToSave(const std::filesystem::path& workspaceFileName);
    const std::filesystem::path& getCurrentWorkspace();

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

    /**
     * shows a file dialog for appending a workspace.
     *
     * @return true if the workspace was appended, otherwise false.
     * @see askToSaveWorkspaceChanges
     */
    bool appendWorkspace();

    /**
     * saves the current workspace. If the workspace does not have a name yet, a file dialog will be
     * shown.
     * @return true if the workspace was saved, otherwise false.
     * @see saveWorkspaceAs
     */
    bool saveWorkspace();
    /**
     * saves the current workspace using a file dialog
     * @return true if the workspace was saved, otherwise false.
     * @see saveWorkspaceAs
     */
    bool saveWorkspaceAs();

    /*
     * Save the current workspace into a new workspace file,
     * leaves the current workspace file as current workspace
     */
    void saveWorkspaceAsCopy();
    bool askToSaveWorkspaceChanges();
    void exitInviwo(bool saveIfModified = true);
    void showAboutBox();

    void toggleWelcomeScreen();
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

    /*
     * Access the WelcomeWidget using this function as it does delayed initialization, i.e., creates
     * it if non-existing.
     */
    WelcomeWidget* getWelcomeWidget();

    /**
     * loads the workspace \p workspaceFileName. In case there are unsaved changes, the user will
     * be asked to save or discard them, or cancel the loading.
     *
     * @param workspaceFileName
     * @param isExample    if true, the workspace file name will not be set. Thereby preventing
     *                     the user from accidentally overwriting the original file. In addition,
     *                     the workspace is _not_ added to the recent file list.
     * @return true if the workspace was opened, otherwise false.
     * @see askToSaveWorkspaceChanges
     */
    bool openWorkspace(const std::filesystem::path& workspaceFileName, bool isExample);

    /**
     * saves the current workspace to the given \p workspaceFileName.
     * @return true if the workspace was saved, otherwise false.
     */
    bool saveWorkspace(const std::filesystem::path& workspaceFileName);
    void appendWorkspace(const std::filesystem::path& workspaceFileName);

    std::optional<std::filesystem::path> askForWorkspaceToOpen();

    void addActions();

    void closeEvent(QCloseEvent* event) override;

    void saveWindowState();
    void loadWindowState();

    void saveSnapshots(const std::filesystem::path& path, std::string_view fileName);
    void getScreenGrab(const std::filesystem::path& path, std::string_view fileName);

    void addToRecentWorkspaces(const std::filesystem::path& workspaceFileName);

    /**
     * \brief update Qt settings for recent workspaces with internal status
     */
    void saveRecentWorkspaceList(const QStringList& list);
    void setCurrentWorkspace(const std::filesystem::path& workspaceFileName);

    void updateWindowTitle();

    InviwoApplication* app_;
    MenuKeyboardEventFilter* menuEventFilter_;
    InviwoEditMenu* editMenu_ = nullptr;
    ToolsMenu* toolsMenu_ = nullptr;
    QMenu* exampleMenu_ = nullptr;
    QMenu* testMenu_ = nullptr;
    std::shared_ptr<ConsoleWidget> consoleWidget_;
    std::unique_ptr<NetworkEditor> networkEditor_;
    QStackedWidget* centralWidget_;
    NetworkEditorView* networkEditorView_;

    SettingsWidget* settings_;
    ProcessorTreeWidget* processorTreeWidget_;
    ResourceManagerDockWidget* resourceManagerDockWidget_;
    PropertyListWidget* propertyListWidget_;
    HelpWidget* helpWidget_;

    ///< Use delayed initialization as it can be expensive.
    WelcomeWidget* welcomeWidget_ = nullptr;
    AnnotationsWidget* annotationsWidget_ = nullptr;
    InviwoAboutWindow* inviwoAboutWindow_ = nullptr;

    std::vector<QAction*> workspaceActionRecent_;
    QAction* clearRecentWorkspaces_;

    std::unique_ptr<FileAssociations> fileAssociations_;

    WorkspaceManager::SerializationHandle annotationSerializationHandle_;
    WorkspaceManager::DeserializationHandle annotationDeserializationHandle_;
    WorkspaceManager::ClearHandle annotationClearHandle_;

    // settings
    bool maximized_;

    // paths
    std::filesystem::path untitledWorkspaceName_;
    std::filesystem::path workspaceFileDir_;
    std::filesystem::path currentWorkspaceFileName_;
    std::filesystem::path workspaceOnLastSuccessfulExit_;

    // command line switches
    TCLAP::ValueArg<std::string> snapshotArg_;
    TCLAP::ValueArg<std::string> screenGrabArg_;
    TCLAP::ValueArg<std::string> saveProcessorPreviews_;
    TCLAP::ValueArg<std::string> openData_;
    TCLAP::SwitchArg updateExampleWorkspaces_;
    TCLAP::SwitchArg updateRegressionWorkspaces_;
    TCLAP::ValueArg<std::string> updateWorkspacesInPath_;

    UndoManager undoManager_;

    struct VisibleWidgets {
        /**
         * store all visible processor and dock widgets before hiding them
         */
        void hide(InviwoMainWindow* win);
        /**
         * show previously hidden processor and dock widgets
         */
        void show();

        std::vector<Processor*> processors;
        std::vector<QDockWidget*> dockwidgets;
    };
    VisibleWidgets visibleWidgetState_;  //!< holds all processor and dock widgets that were visible
                                         //!< before showing the welcome widget
    WorkspaceManager::ClearHandle visibleWidgetsClearHandle_;
};

}  // namespace inviwo
