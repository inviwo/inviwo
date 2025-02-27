/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2025 Inviwo Foundation
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

#include <modules/webbrowser/webbrowsermodule.h>

#include <inviwo/core/common/inviwoapplication.h>      // for InviwoApplication
#include <inviwo/core/common/inviwomodule.h>           // for ModulePath
#include <inviwo/core/util/commandlineparser.h>        // for CommandLineParser
#include <inviwo/core/util/exception.h>                // for ModuleInitExce...
#include <inviwo/core/util/filesystem.h>               // for getExecutablePath
#include <inviwo/core/util/logcentral.h>               // for LogCentral
#include <inviwo/core/util/settings/settings.h>        // for Settings
#include <inviwo/core/util/settings/systemsettings.h>  // for SystemSettings
#include <inviwo/core/util/staticstring.h>             // for operator+
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/util/timer.h>               // for Timer, Timer::...
#include <modules/opengl/shader/shadermanager.h>  // for ShaderManager
#include <modules/webbrowser/processors/basicwebbrowser.h>
#include <modules/webbrowser/processors/webbrowserprocessor.h>  // for WebBrowserProc...
#include <modules/webbrowser/properties/propertywidgetcef.h>    // for PropertyWidgetCEF
#include <modules/webbrowser/shader_resources.h>                // for addShaderResou...
#include <modules/webbrowser/webbrowserapp.h>                   // for WebBrowserApp
#include <modules/webbrowser/webbrowserclient.h>                // for WebBrowserClient
#include <modules/webbrowser/webbrowsersettings.h>              // for WebBrowserSett...

#include <cstddef>      // for size_t, NULL
#include <functional>   // for __base, function
#include <locale>       // for locale
#include <string_view>  // for string_view
#include <utility>      // for move

#include <fmt/std.h>

#include <warn/push>
#include <warn/ignore/all>
#include <include/cef_app.h>
#include <include/cef_base.h>  // for CefSettings
#include <warn/pop>

namespace inviwo {

WebBrowserModule::WebBrowserModule(InviwoApplication* app)
    : InviwoModule(app, "WebBrowser")
    , doChromiumWork_(Timer::Milliseconds(16), []() { CefDoMessageLoopWork(); }) {

    auto moduleSettings = std::make_unique<WebBrowserSettings>();

    moduleSettings->refreshRate_.onChange([this, ptr = moduleSettings.get()]() {
        doChromiumWork_.setInterval(Timer::Milliseconds(1000 / ptr->refreshRate_));
    });
    doChromiumWork_.setInterval(Timer::Milliseconds(1000 / moduleSettings->refreshRate_));

    registerSettings(std::move(moduleSettings));

    if (!app->getSystemSettings().enablePickingProperty_) {
        LogInfo(
            "Enabling picking system setting since it is required for interaction "
            "(View->Settings->System settings->Enable picking).");
        app->getSystemSettings().enablePickingProperty_.set(true);
    }
    // CEF initialization
    // Specify the path for the sub-process executable.
    auto exeExtension = filesystem::getExecutablePath().extension();
    // Assume that inviwo_web_helper is next to the main executable
    auto exeDirectory = filesystem::getExecutablePath().parent_path();

    auto locale = app->getUILocale().name();
    if (locale == "C") {
        // Crash when default locale "C" is used. Reproduce with GLFWMinimum application
        locale = std::locale("en_US.UTF-8").name();
    }

    void* sandbox_info = NULL;  // Windows specific

#ifdef __APPLE__  // Mac specific

    // Find CEF framework and helper app in
    // exe.app/Contents/Frameworks directory first
    auto cefParentDir = std::filesystem::weakly_canonical(exeDirectory / "..");
    auto frameworkDirectory = cefParentDir / "Frameworks/Chromium Embedded Framework.framework";
    auto frameworkPath = frameworkDirectory / "Chromium Embedded Framework";
    // Load the CEF framework library at runtime instead of linking directly
    // as required by the macOS sandbox implementation.
    if (!cefLib_.LoadInMain()) {
        throw ModuleInitException("Could not find Chromium Embedded Framework.framework: " +
                                  frameworkPath.string());
    }

    CefSettings settings;
    // Setting locales_dir_path does not seem to work (tested debug mode with Xcode).
    // We have therefore created symbolic links from the bundle Resources directory to the
    // framework resource files using CMake.
    // See documentation about settings in cef_types.h:
    // "This value [locales_dir_path] is ignored on MacOS where pack files are always
    // loaded from the app bundle Resources directory".
    //
    // We still set the variable to potentially avoid other problems such as the one below.
    // Crashes if not set and non-default locale is used
    CefString(&settings.locales_dir_path).FromWString((frameworkDirectory / "Resources").wstring());

    // resources_dir_path specified location of:
    // cef.pak
    // cef_100_percent.pak
    // cef_200_percent.pak
    //      These files contain non-localized resources used by CEF, Chromium and Blink.
    //      Without these files arbitrary Web components may display incorrectly.
    //
    // cef_extensions.pak
    //      This file contains non-localized resources required for extension loading.
    //      Pass the `--disable-extensions` command-line flag to disable use of this
    //      file. Without this file components that depend on the extension system,
    //      such as the PDF viewer, will not function.
    //
    // devtools_resources.pak
    //      This file contains non-localized resources required for Chrome Developer
    //      Tools. Without this file Chrome Developer Tools will not function.
    CefString(&settings.resources_dir_path)
        .FromWString((frameworkDirectory / "Resources").wstring());
    // Locale returns "en_US.UFT8" but "en.UTF8" is needed by CEF
    auto startErasePos = locale.find('_');
    if (startErasePos != std::string::npos) {
        locale.erase(startErasePos, locale.find('.') - startErasePos);
    }

#else
    CefSettings settings;
    // Non-mac systems uses a single helper executable so here we can specify name
    // Linux will have empty extension
    auto subProcessExecutable = exeDirectory / "cef_web_helper";
    subProcessExecutable += exeExtension;
    if (!std::filesystem::is_regular_file(subProcessExecutable)) {
        throw ModuleInitException(
            fmt::format("Could not find web helper executable: {}", subProcessExecutable));
    }

    // Necessary to run helpers in separate sub-processes
    // Needed since we do not want to edit the "main" function
    CefString(&settings.browser_subprocess_path).FromWString(subProcessExecutable.wstring());
#endif

#ifdef WIN32
#if defined(CEF_USE_SANDBOX)
    // Manage the life span of the sandbox information object. This is necessary
    // for sandbox support on Windows. See cef_sandbox_win.h for complete details.
    CefScopedSandboxInfo scoped_sandbox;
    sandbox_info = scoped_sandbox.sandbox_info();
#endif
#endif
    // When generating projects with CMake the CEF_USE_SANDBOX value will be defined
    // automatically. Pass -DUSE_SANDBOX=OFF to the CMake command-line to disable
    // use of the sandbox.
#if !defined(CEF_USE_SANDBOX)
    settings.no_sandbox = true;
#endif
    // checkout detailed settings options
    // http://magpcss.org/ceforum/apidocs/projects/%28default%29/_cef_settings_t.html nearly all
    // the settings can be set via args too.
    settings.multi_threaded_message_loop = false;  // not supported, except windows

    // We want to use off-screen rendering
    settings.windowless_rendering_enabled = true;

    // Let the Inviwo application (Qt/GLFW) handle operating system event processing
    // instead of CEF. Setting external message pump to false will cause mouse events
    // to be processed in CefDoMessageLoopWork() instead of in the Qt application loop.
    settings.external_message_pump = true;

    const auto settingsPath = []() {
        const auto base = filesystem::getPath(PathType::Settings) / "CEF";
        for (int i = 0; true; ++i) {
            auto path = base / fmt::format("{:03}", i);
#if defined(WIN32)
            if (!std::filesystem::exists(path / "lockfile")) {
                std::filesystem::create_directories(path);
                return path;
            }
#elif defined(__APPLE__)
            if (!std::filesystem::exists(path / "SingletonSocket")) {
                std::filesystem::create_directories(path);
                return path;
            }
#else
            return path;
#endif
        }
    }();
    CefString(&settings.root_cache_path).FromString(settingsPath.generic_string());

    CefString(&settings.locale).FromString(locale);

    // Optional implementation of the CefApp interface.
    CefRefPtr<WebBrowserApp> browserApp(new WebBrowserApp);

    CefMainArgs args;
    if (!CefInitialize(args, settings, browserApp, sandbox_info)) {
        throw ModuleInitException("Failed to initialize Chromium Embedded Framework");
    }

    // Add a directory to the search path of the ShaderManager
    webbrowser::addShaderResources(ShaderManager::getPtr(), {getPath(ModulePath::GLSL)});
    // ShaderManager::getPtr()->addShaderSearchPath(getPath(ModulePath::GLSL));

    // Register objects that can be shared with the rest of inviwo here:

    // Processors
    registerProcessor<BasicWebBrowser>();
    registerProcessor<WebBrowserProcessor>();

    doChromiumWork_.start();

    browserClient_ = new WebBrowserClient(app);
}

WebBrowserModule::~WebBrowserModule() {
    // Stop message pumping and make sure that app has finished processing before CefShutdown
    doChromiumWork_.stop();
    app_->waitForPool();
    CefShutdown();
}

}  // namespace inviwo
