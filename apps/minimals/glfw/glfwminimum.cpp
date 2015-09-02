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

#ifdef WIN32
#include <windows.h>
#endif

#include <modules/opengl/inviwoopengl.h>
#include <modules/glfw/canvasglfw.h>

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/network/processornetworkevaluator.h>
#include <inviwo/core/processors/processorwidget.h>
#include <inviwo/core/processors/canvasprocessor.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/rendercontext.h>
#include <inviwo/core/util/utilities.h>
#include <moduleregistration.h>
#include <inviwo/core/processors/processorwidgetfactory.h>

using namespace inviwo;

int main(int argc, char** argv) {
    LogCentral::init();
    LogCentral::getPtr()->registerLogger(new ConsoleLogger());

    InviwoApplication inviwoApp(argc, argv, "Inviwo v"+IVW_VERSION + " - GLFWApp", inviwo::filesystem::findBasePath());
    inviwoApp.setPostEnqueueFront([](){glfwPostEmptyEvent();});

    CanvasGLFW::setAlwaysOnTopByDefault(false);

    //Initialize all modules
    inviwoApp.initialize(&inviwo::registerAllModules);

    //Continue initialization of default context
    CanvasGLFW* sharedCanvas = static_cast<CanvasGLFW*>(RenderContext::getPtr()->getDefaultRenderContext());
    sharedCanvas->initialize();
    sharedCanvas->activate();

    // Load simple scene
    inviwoApp.getProcessorNetwork()->lock();
    const CommandLineParser* cmdparser = inviwoApp.getCommandLineParser();
    std::string workspace;

    if (cmdparser->getLoadWorkspaceFromArg())
        workspace = cmdparser->getWorkspacePath();
    else
        workspace = inviwoApp.getPath(InviwoApplication::PATH_WORKSPACES, "/boron.inv");

    auto widgetDeleter = [](ProcessorWidget* w) {delete w; };
    std::vector<std::unique_ptr<ProcessorWidget, decltype(widgetDeleter)>> widgets;

    try
    {
        if (!workspace.empty()){
            IvwDeserializer xmlDeserializer(workspace);
            inviwoApp.getProcessorNetwork()->deserialize(xmlDeserializer);
            std::vector<Processor*> processors = inviwoApp.getProcessorNetwork()->getProcessors();

            for (std::vector<Processor*>::iterator it = processors.begin(); it != processors.end(); ++it) {
                Processor* processor = *it;
                processor->invalidate(INVALID_RESOURCES);

                ProcessorWidget* processorWidget = ProcessorWidgetFactory::getPtr()->create(processor).release();
                if (processorWidget) {
                    widgets.emplace_back(processorWidget, widgetDeleter);
                    processorWidget->setProcessor(processor);
                    processorWidget->initialize();
                    processorWidget->setVisible(processorWidget->ProcessorWidget::isVisible());
                    processor->setProcessorWidget(processorWidget);
                }
            }
        }
    }
    catch (const AbortException& exception) {
        util::log(exception.getContext(),
            "Unable to load network " + workspace + " due to " + exception.getMessage(),
            LogLevel::Error);
        return 1;
    }
    catch (const IgnoreException& exception) {
        util::log(exception.getContext(),
            "Incomplete network loading " + workspace + " due to " + exception.getMessage(),
            LogLevel::Error);
        return 1;
    }
    catch (const ticpp::Exception& exception) {
        LogErrorCustom("glfwminimum", "Unable to load network " + workspace + " due to deserialization error: " + exception.what());
        return 1;
    }

    inviwoApp.getProcessorNetwork()->setModified(true);
    inviwoApp.getProcessorNetwork()->unlock();

    if (cmdparser->getCaptureAfterStartup()) {
        std::string path = cmdparser->getOutputPath();

        if (path.empty())
            path = inviwoApp.getPath(InviwoApplication::PATH_IMAGES);

        util::saveAllCanvases(inviwoApp.getProcessorNetwork(), path, cmdparser->getSnapshotName());
    }

    if (cmdparser->getQuitApplicationAfterStartup()){
        glfwTerminate();
        return 0;
    }


    while (CanvasGLFW::getWindowCount()>0) {
        glfwWaitEvents();
        //glfwPollEvents();
        inviwoApp.processFront();
    }

    inviwoApp.deinitialize();

    return 0;
}
