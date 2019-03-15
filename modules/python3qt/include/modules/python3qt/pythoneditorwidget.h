/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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

#ifndef IVW_PYTHONEDITORYWIDGET_H
#define IVW_PYTHONEDITORYWIDGET_H

#include <modules/python3qt/python3qtmoduledefine.h>
#include <modules/python3/pythonscript.h>
#include <modules/python3/python3module.h>
#include <modules/python3/pythonexecutionoutputobservable.h>
#include <inviwo/core/network/processornetworkobserver.h>
#include <inviwo/core/util/fileobserver.h>
#include <modules/qtwidgets/inviwodockwidget.h>
#include <modules/qtwidgets/editorfileobserver.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QTextEdit>
#include <QColor>
#include <QToolButton>
#include <QSettings>
#include <QPlainTextEdit>
#include <warn/pop>

class QPlainTextEdit;
class QFocusEvent;
class QMainWindow;

namespace inviwo {

class InviwoApplication;
class SyntaxHighligther;
class CodeEdit;

class IVW_MODULE_PYTHON3QT_API PythonEditorWidget : public InviwoDockWidget,
                                                    public PythonExecutionOutputObeserver {

public:
    PythonEditorWidget(QWidget* parent, InviwoApplication* app);
    virtual ~PythonEditorWidget();

    void appendToOutput(const std::string& msg, bool error = false);
    void loadFile(std::string fileName, bool askForSave = true);

    virtual void onPyhonExecutionOutput(const std::string& msg,
                                        PythonOutputType outputType) override;

    void save();
    void saveAs();
    void open();
    void run();
    void show();
    void setDefaultText();
    void clearOutput();

    void onTextChange();

    virtual void loadState() override;

protected:
    virtual void closeEvent(QCloseEvent* event) override;

private:
    void setFileName(const std::string& filename);
    void updateWindowTitle();
    void readFile();
    bool askSaveChanges();

    virtual void saveState() override;

    QMainWindow* mainWindow_;
    CodeEdit* pythonCode_;
    CodeEdit* pythonOutput_;

    QColor infoTextColor_;
    QColor errorTextColor_;

    QAction* runAction_;

    PythonScript script_;
    std::string scriptFileName_;

    std::shared_ptr<std::function<void()>> syntaxCallback_;

    static PythonEditorWidget* instance_;

    InviwoApplication* app_;
    QAction* appendLog_;
    utilqt::EditorFileObserver fileObserver_;
};

}  // namespace inviwo

#endif  // IVW_PYTHONEDITORYWIDGET_H
