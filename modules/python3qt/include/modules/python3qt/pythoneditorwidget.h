/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2025 Inviwo Foundation
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

#include <modules/python3qt/python3qtmoduledefine.h>
#include <modules/python3/pythonscript.h>
#include <modules/python3/python3module.h>
#include <modules/python3/pythonexecutionoutputobservable.h>
#include <inviwo/core/network/processornetworkobserver.h>
#include <inviwo/core/util/fileobserver.h>
#include <modules/qtwidgets/inviwodockwidget.h>
#include <modules/qtwidgets/editorfileobserver.h>

#include <QTextEdit>
#include <QColor>
#include <QToolButton>
#include <QSettings>
#include <QPlainTextEdit>

#include <filesystem>

class QPlainTextEdit;
class QFocusEvent;
class QMainWindow;

namespace inviwo {

class InviwoApplication;
class SyntaxHighligther;
class CodeEdit;

class IVW_MODULE_PYTHON3QT_API PythonEditorWidget : public InviwoDockWidget,
                                                    public PythonExecutionOutputObserver {
public:
    PythonEditorWidget(QWidget* parent, InviwoApplication* app,
                       std::function<void(const std::string&)> onTextChange = nullptr);
    virtual ~PythonEditorWidget();

    void appendToOutput(const std::string& msg, bool error = false);
    void loadFile(const std::filesystem::path& fileName, bool askForSave = true);

    virtual void onPythonExecutionOutput(const std::string& msg,
                                         PythonOutputType outputType) override;

    void save();
    void saveAs();
    void open();
    void runOrStop(bool run);
    void run();
    void stop();
    void show();
    void setDefaultText();
    void clearOutput();

    void onTextChange();

    virtual void loadState() override;

    void restore();

    void setName(std::string_view name);
    const std::string& getName() const;
    void setSource(std::string_view source);
    const std::string& getSource() const;

protected:
    virtual void closeEvent(QCloseEvent* event) override;

private:
    void setFileName(const std::filesystem::path& filename);
    void updateWindowTitle();
    void readFile();
    bool askSaveChanges();

    virtual void saveState() override;

    QMainWindow* mainWindow_;
    CodeEdit* pythonCode_;
    std::vector<std::shared_ptr<std::function<void()>>> codeCallbacks_;
    CodeEdit* pythonOutput_;
    std::vector<std::shared_ptr<std::function<void()>>> outputCallbacks_;

    QColor infoTextColor_;
    QColor errorTextColor_;

    QAction* runAction_;

    PythonScript script_;
    std::filesystem::path scriptFileName_;

    std::shared_ptr<std::function<void()>> syntaxCallback_;

    InviwoApplication* app_;
    QAction* appendLog_;
    utilqt::EditorFileObserver fileObserver_;

    std::function<void(const std::string&)> onTextChange_;
};

}  // namespace inviwo
