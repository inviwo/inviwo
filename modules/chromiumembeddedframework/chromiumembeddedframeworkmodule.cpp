/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016 Inviwo Foundation
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

#include <modules/chromiumembeddedframework/chromiumembeddedframeworkmodule.h>
#include <modules/ChromiumEmbeddedFramework/processors/chromiumprocessor.h>

#include "include/cef_app.h"
#include "include/cef_command_line.h"

namespace inviwo {

ChromiumEmbeddedFrameworkModule::ChromiumEmbeddedFrameworkModule(InviwoApplication* app) : InviwoModule(app, "ChromiumEmbeddedFramework") {   


    CefMainArgs args;
    {
        CefSettings settings;

        // checkout detailed settings options http://magpcss.org/ceforum/apidocs/projects/%28default%29/_cef_settings_t.html
        // nearly all the settings can be set via args too.
        // settings.multi_threaded_message_loop = true; // not supported, except windows
        // CefString(&settings.browser_subprocess_path).FromASCII("sub_proccess path, by default uses and starts this executeable as child");
        // CefString(&settings.cache_path).FromASCII("");
        // CefString(&settings.log_file).FromASCII("");
        // settings.log_severity = LOGSEVERITY_DEFAULT;
        // CefString(&settings.resources_dir_path).FromASCII("");
        // CefString(&settings.locales_dir_path).FromASCII("");

        bool result = CefInitialize(args, settings, nullptr, nullptr);
        // CefInitialize creates a sub-proccess and executes the same executeable, as calling CefInitialize, if not set different in settings.browser_subprocess_path
        // if you create an extra program just for the childproccess you only have to call CefExecuteProcess(...) in it.
        if (!result)
        {
            throw ModuleInitException("Failed to initialize Chromium Embedded Framework");
        }
    }
    // Add a directory to the search path of the Shadermanager
    // ShaderManager::getPtr()->addShaderSearchPath(getPath(ModulePath::GLSL));

    // Register objects that can be shared with the rest of inviwo here:
    
    // Processors
    registerProcessor<ChromiumProcessor>();
    
    // Properties
    // registerProperty<ChromiumEmbeddedFrameworkProperty>();
    
    // Readers and writes
    // registerDataReader(util::make_unique<ChromiumEmbeddedFrameworkReader>());
    // registerDataWriter(util::make_unique<ChromiumEmbeddedFrameworkWriter>());
    
    // Data converters
    // registerRepresentationConverter(util::make_unique<ChromiumEmbeddedFrameworkDisk2RAMConverter>());

    // Ports
    // registerPort<ChromiumEmbeddedFrameworkOutport>("ChromiumEmbeddedFrameworkOutport");
    // registerPort<ChromiumEmbeddedFrameworkInport>("ChromiumEmbeddedFrameworkInport");

    // PropertyWidgets
    // registerPropertyWidget<ChromiumEmbeddedFrameworkPropertyWidget, ChromiumEmbeddedFrameworkProperty>("Default");
    
    // Dialogs
    // registerDialog<ChromiumEmbeddedFrameworkDialog>(ChromiumEmbeddedFrameworkOutport);
    
    // Other varius things
    // registerCapabilities(util::make_unique<ChromiumEmbeddedFrameworkCapabilities>());
    // registerSettings(util::make_unique<ChromiumEmbeddedFrameworkSettings>());
    // registerMetaData(util::make_unique<ChromiumEmbeddedFrameworkMetaData>());   
    // registerPortInspector("ChromiumEmbeddedFrameworkOutport", "path/workspace.inv");
    // registerProcessorWidget(std::string processorClassName, std::unique_ptr<ProcessorWidget> processorWidget);
    // registerDrawer(util::make_unique_ptr<ChromiumEmbeddedFrameworkDrawer>());  
}

ChromiumEmbeddedFrameworkModule::~ChromiumEmbeddedFrameworkModule() {
    CefShutdown();
}

} // namespace
