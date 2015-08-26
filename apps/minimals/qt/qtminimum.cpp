/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#ifdef _MSC_VER
#pragma comment(linker, "/SUBSYSTEM:CONSOLE")
#endif

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/network/processornetworkevaluator.h>
#include <inviwo/core/util/canvas.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/rendercontext.h>
#include <inviwo/core/util/utilities.h>
#include <inviwo/core/processors/processorwidgetfactory.h>
#include <inviwo/qt/widgets/inviwoapplicationqt.h>
#include <moduleregistration.h>

using namespace inviwo;

int main(int argc, char** argv) {
    LogCentral::init();
    LogCentral::getPtr()->registerLogger(new ConsoleLogger());

    std::string appName = "Inviwo v" + IVW_VERSION + " - QtApp";
    InviwoApplicationQt inviwoApp(appName, inviwo::filesystem::findBasePath(), argc, argv, false);
    inviwoApp.setAttribute(Qt::AA_NativeWindows);

    // Initialize all modules
    inviwoApp.initialize(&inviwo::registerAllModules);

    // Continue initialization of default context
    Canvas* sharedCanvas = RenderContext::getPtr()->getDefaultRenderContext();
    if (sharedCanvas) {
        sharedCanvas->initialize();
        sharedCanvas->activate();
    }

    // Set canvas as central widget
    QMainWindow mainWin;

    // Load workspace
    inviwoApp.getProcessorNetwork()->lock();
    const CommandLineParser* cmdparser = inviwoApp.getCommandLineParser();
    std::string workspace;

    if (cmdparser->getLoadWorkspaceFromArg())
        workspace = cmdparser->getWorkspacePath();
    else
#ifdef REG_INVIWOBASEGLMODULE
        workspace =
        inviwoApp.getPath(InviwoApplication::PATH_WORKSPACES, "boron.inv");
#else
        workspace = "";
#endif

    if (!workspace.empty()) {
        IvwDeserializer xmlDeserializer(workspace);
        inviwoApp.getProcessorNetwork()->deserialize(xmlDeserializer);
        std::vector<Processor*> processors = inviwoApp.getProcessorNetwork()->getProcessors();

        for (auto processor : processors) {
            processor->invalidate(INVALID_RESOURCES);

            if (auto processorWidget =
                    ProcessorWidgetFactory::getPtr()->create(processor).release()) {
                processorWidget->setProcessor(processor);
                processorWidget->initialize();
                processorWidget->setVisible(processorWidget->ProcessorWidget::isVisible());
                processor->setProcessorWidget(processorWidget);

                if (!mainWin.centralWidget()) {
                    mainWin.setCentralWidget(dynamic_cast<QWidget*>(processorWidget));
                }
            }
        }
    }

    inviwoApp.getProcessorNetwork()->setModified(true);
    inviwoApp.getProcessorNetwork()->unlock();

    if (cmdparser->getCaptureAfterStartup()) {
        std::string path = cmdparser->getOutputPath();

        if (path.empty()) path = inviwoApp.getPath(InviwoApplication::PATH_IMAGES);

        util::saveAllCanvases(inviwoApp.getProcessorNetwork(), path, cmdparser->getSnapshotName());
    }

    if (sharedCanvas)
        mainWin.resize(sharedCanvas->getScreenDimensions().x, sharedCanvas->getScreenDimensions().y);
    
    mainWin.show();

    if (cmdparser->getQuitApplicationAfterStartup()) {
        inviwoApp.closeInviwoApplication();
        inviwoApp.quit();
        return 0;
    }

    return inviwoApp.exec();
}
