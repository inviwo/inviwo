/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2022 Inviwo Foundation
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

#include <modules/webbrowser/webbrowsermoduledefine.h>               // for IVW_MODULE_WEBBROWSE...

#include <inviwo/core/common/inviwoapplication.h>                    // for InviwoApplication
#include <inviwo/core/common/inviwomodule.h>                         // for InviwoModule
#include <inviwo/core/util/timer.h>                                  // for Timer
#include <modules/json/jsonmodule.h>                                 // for JSONModule
#include <modules/webbrowser/properties/propertywidgetceffactory.h>  // for PropertyWidgetCEFFac...
#include <modules/webbrowser/webbrowserclient.h>                     // for WebBrowserClient

#include <memory>                                                    // for unique_ptr, make_unique
#include <string>                                                    // for string
#include <utility>                                                   // for move
#include <vector>                                                    // for vector

#include <warn/push>
#include <warn/ignore/all>

#include "include/internal/cef_types.h"
#include <include/cef_base.h>                                        // for CefRefPtr

namespace inviwo {
class PropertyWidgetCEFFactoryObject;
template <typename T, typename P> class PropertyWidgetCEFFactoryObjectTemplate;
}  // namespace inviwo
#if __APPLE__  // Mac
#include "include/wrapper/cef_library_loader.h"
#endif
#include <warn/pop>

namespace inviwo {

/*
 * Enable suppport for showing web pages using CEF
 */
class IVW_MODULE_WEBBROWSER_API WebBrowserModule : public InviwoModule {
public:
    WebBrowserModule(InviwoApplication* app);
    virtual ~WebBrowserModule();

    CefRefPtr<WebBrowserClient> getBrowserClient() { return browserClient_; }

    // Register a JSON converter and its corresponding HTML-synchronization widget.
    template <typename T, typename P>
    void registerPropertyJSONConverterAndWidget();

    // Use own widget factory for now. Multiple widget types are not supported in Inviwo yet
    template <typename T, typename P>
    void registerPropertyWidgetCEF();
    inline const PropertyWidgetCEFFactory* getPropertyWidgetCEFFactory() const;

    static std::string getDataURI(const std::string& data, const std::string& mime_type);

    // Return error code enum as string
    static std::string getCefErrorString(cef_errorcode_t code);

protected:
    CefRefPtr<WebBrowserClient> browserClient_;
    // HTML-property synchronization widget factory
    PropertyWidgetCEFFactory htmlWidgetFactory_;
    std::vector<std::unique_ptr<PropertyWidgetCEFFactoryObject>> propertyWidgets_;

    Timer doChromiumWork_;  /// Calls CefDoMessageLoopWork()
#ifdef __APPLE__            // Load library dynamically for Mac
    CefScopedLibraryLoader cefLib_;
#endif
};

template <typename T, typename P>
void WebBrowserModule::registerPropertyWidgetCEF() {
    auto propertyWidget = std::make_unique<PropertyWidgetCEFFactoryObjectTemplate<T, P>>(
        app_->getModuleByType<JSONModule>()->getPropertyJSONConverterFactory());
    if (htmlWidgetFactory_.registerObject(propertyWidget.get())) {
        propertyWidgets_.push_back(std::move(propertyWidget));
    }
}

inline const PropertyWidgetCEFFactory* WebBrowserModule::getPropertyWidgetCEFFactory() const {
    return &htmlWidgetFactory_;
}

}  // namespace inviwo
