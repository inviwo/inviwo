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

#ifndef IVW_INVIWOMAINWINDOW_H
#define IVW_INVIWOMAINWINDOW_H

#include <inviwo/qt/editor/inviwoqteditordefine.h>
#include <inviwo/qt/editor/networkeditorobserver.h>
#include <inviwo/core/properties/optionproperty.h>
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

namespace inviwo {

class NetworkEditorView;
class NetworkEditor;
class PropertyListWidget;
class ProcessorTreeWidget;
class ConsoleWidget;
class SettingsWidget;
class HelpWidget;
class InviwoApplicationQt;
class NetworkSearch;
class InviwoEditMenu;
class InviwoAboutWindow;
class ResourceManagerDockWidget;
class FileAssociations;

class IVW_QTEDITOR_API InviwoMainWindow : public QMainWindow, public NetworkEditorObserver {
public:
    static const unsigned int maxNumRecentFiles_ = 10;

    InviwoMainWindow(InviwoApplicationQt* app);
    virtual ~InviwoMainWindow();

    void showWindow();

    void openLastWorkspace(std::string workspace = "");
    void openWorkspace(QString workspaceFileName);

    /**
     * Open the given workspaceFileName but only after asking whether to save the
     * current workspace. The use can also abort the open bu pressing cancel.
     * @return true if the workspace was opened, otherwise false.
     */
    bool openWorkspaceAskToSave(QString workspaceFileName);
    std::string getCurrentWorkspace();

    NetworkEditor* getNetworkEditor() const;
    SettingsWidget* getSettingsWidget() const;
    ProcessorTreeWidget* getProcessorTreeWidget() const;
    PropertyListWidget* getPropertyListWidget() const;
    ConsoleWidget* getConsoleWidget() const;
    HelpWidget* getHelpWidget() const;
    InviwoApplication* getInviwoApplication() const;
    InviwoApplicationQt* getInviwoApplicationQt() const;

    InviwoEditMenu* getInviwoEditMenu() const;

    void newWorkspace();
    void openWorkspace();

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

protected:
    virtual void dragEnterEvent(QDragEnterEvent* event) override;
    virtual void dragMoveEvent(QDragMoveEvent* event) override;
    virtual void dropEvent(QDropEvent* event) override;


private:
    virtual void onModifiedStatusChanged(const bool& newStatus) override;

    void visibilityModeChangedInSettings();

    void openWorkspace(QString workspaceFileName, bool exampleWorkspace);
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
    * \brief query the Qt settings for recent workspaces and update internal status
    */
    QStringList getRecentWorkspaceList() const;
    /**
     * \brief update Qt settings for recent workspaces with internal status
     */
    void saveRecentWorkspaceList(const QStringList& list);
    void setCurrentWorkspace(QString workspaceFileName);

    void updateWindowTitle();

    InviwoApplicationQt* app_;
    InviwoEditMenu* editMenu_ = nullptr;
    QMenu* exampleMenu_ = nullptr;
    QMenu* testMenu_ = nullptr;
    std::shared_ptr<ConsoleWidget> consoleWidget_;
    std::unique_ptr<NetworkEditor> networkEditor_;
    NetworkEditorView* networkEditorView_;
 
    SettingsWidget* settingsWidget_;
    ProcessorTreeWidget* processorTreeWidget_;
    ResourceManagerDockWidget* resourceManagerDockWidget_;
    PropertyListWidget* propertyListWidget_;
    HelpWidget* helpWidget_;
    NetworkSearch* networkSearch_;
    InviwoAboutWindow* inviwoAboutWindow_ = nullptr;
    
    std::vector<QAction*> workspaceActionRecent_;
    QAction* clearRecentWorkspaces_;
    QAction* visibilityModeAction_;

    std::unique_ptr<FileAssociations> fileAssociations_;

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
    
    UndoManager undoManager_;
};

}  // namespace

#endif  // IVW_INVIWOMAINWINDOW_H
