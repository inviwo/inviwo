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

#ifndef IVW_PROPERTYCEFSYNCHRONIZER_H
#define IVW_PROPERTYCEFSYNCHRONIZER_H

#include <modules/webbrowser/webbrowsermoduledefine.h>
#include <modules/webbrowser/properties/propertywidgetcef.h>

#include <inviwo/core/properties/property.h>
#include <inviwo/core/properties/propertywidgetfactory.h>

#include <warn/push>
#include <warn/ignore/all>
#include <include/cef_load_handler.h>
#include "include/wrapper/cef_message_router.h"
#include <warn/pop>


namespace inviwo {

/** \class PropertyCefSynchronizer
 *
 * Flow of information between PropertyWidgetCEF and browser.
 * Changes can start from Inviwo (left) or browser (right).
 * PropertyWidgetCEF::onQueryBlocker is used to prevent loops.
 *
 *     Inviwo           Browser (JavaScript)
 * Property change
 *        |
 * updateFromProperty()
 * onQueryBlocker_ += 1
 * ExecuteJavaScript()  --> set values
 *                          oninput()
 *                             |
 *                             |
 *     OnQuery  <---------  cefQuery
 * onQueryBlocker_ -= 1
 *   Deserialize
 *
 *
 */
class PropertyCefSynchronizer : public CefMessageRouterBrowserSide::Handler, public CefLoadHandler {
public:
    explicit PropertyCefSynchronizer();
    virtual ~PropertyCefSynchronizer() = default;
    /*
     * Synchronizes all widgets and sets their frame, called when frame has loaded.
     */
    virtual void OnLoadEnd(CefRefPtr<CefBrowser> browser,
                           CefRefPtr<CefFrame> frame,
                           int httpStatusCode) override;
    
    // Called due to cefQuery execution in message_router.html.
    bool OnQuery(CefRefPtr<CefBrowser> browser,
                 CefRefPtr<CefFrame> frame,
                 int64 query_id,
                 const CefString& request,
                 bool persistent,
                 CefRefPtr<Callback> callback) OVERRIDE {
        
        const std::string& message_name = request;
        auto widget = std::find_if(std::begin(widgets_), std::end(widgets_),
                                   [message_name](const auto& widget) {
                                       return message_name.find(widget->getHtmlId()) != std::string::npos;
                                   });
        if (widget != widgets_.end()) {
            return (*widget)->OnQuery(browser, frame, query_id, request,
                                      persistent, callback);
        }
        
        return false;
    }
    
    void addProperty(Property* property) {
        // We cannot use path since Processor is not set until after construction.
        //auto path = property->getPath();
        //auto htmlId = std::accumulate(std::next(path.begin()), path.end(), path[0],
        //                                     [](std::string &s, const std::string &piece) ->
        //                                     decltype(auto) { return s += "." + piece; });
        auto htmlId = property->getIdentifier();
        auto widget = dynamic_cast<PropertyWidgetCEF*>(htmlWidgetFactory_.create(property).release());
        if (!widget) {
            throw Exception("No HTML property widget for " + property->getClassIdentifier());
        }
        widget->setHtmlId(htmlId);
        //auto widget = std::make_unique<OrdinalPropertyWidgetCEF<T>>(property, browser_->GetMainFrame(), htmlId);
        widgets_.emplace_back(std::move(widget));
    }

private:
    // Use own widget factory for now. Multiple widget types are not supported in Inviwo yet
    template <typename T, typename P>
    void registerPropertyWidget(PropertySemantics semantics);
    std::vector<std::unique_ptr<PropertyWidgetFactoryObject>> propertyWidgets_;
    PropertyWidgetFactory htmlWidgetFactory_;

    std::vector<std::unique_ptr<PropertyWidgetCEF>> widgets_;
    IMPLEMENT_REFCOUNTING(PropertyCefSynchronizer)
};
    
template <typename T, typename P>
void PropertyCefSynchronizer::registerPropertyWidget(PropertySemantics semantics) {
    auto propertyWidget = util::make_unique<PropertyWidgetFactoryObjectTemplate<T, P>>(semantics);
    if (htmlWidgetFactory_.registerObject(propertyWidget.get())) {
        propertyWidgets_.push_back(std::move(propertyWidget));
    }
}
}  // namespace inviwo

#endif  // IVW_PROPERTYCEFSYNCHRONIZER_H
