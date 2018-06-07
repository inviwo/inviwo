/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018 Inviwo Foundation
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

#include <modules/webbrowser/webrendererapp.h>
#include <modules/webbrowser/app_switches.h>


// This process will run the CEF web browser. Used as a sub-process by WebBrowserModule 
// See WebBrowserModule::WebBrowserModule
#ifdef WIN32
int main(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow) {
    // Enable High-DPI support on Windows 7 or newer.
    CefEnableHighDPISupport();

    // Provide CEF with command-line arguments.
    CefMainArgs mainArgs(hInstance);
#else
int main(int argc, char* argv[]) {
        // Provide CEF with command-line arguments.
        CefMainArgs mainArgs(argc, argv);
#endif
    
    // Create a temporary CommandLine object.
    CefRefPtr<CefCommandLine> command_line = CreateCommandLine(mainArgs);
    
    // Create a CefApp of the correct process type. The browser process is handled
    // by webbrowsermodule.cpp.
    CefRefPtr<CefApp> app = nullptr;
    switch (GetProcessType(command_line)) {
        case PROCESS_TYPE_RENDERER:
            app = new inviwo::WebRendererApp();
            break;
        case PROCESS_TYPE_OTHER:
            app = nullptr;
            break;
        default:
            break;
    }
    
    // Execute the sub-process.
    return CefExecuteProcess(mainArgs, app.get(), NULL);
}
