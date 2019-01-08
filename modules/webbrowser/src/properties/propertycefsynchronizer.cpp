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

#include <modules/webbrowser/properties/propertycefsynchronizer.h>
#include <modules/webbrowser/properties/boolpropertywidgetcef.h>
#include <modules/webbrowser/properties/buttonpropertywidgetcef.h>
#include <modules/webbrowser/properties/ordinalpropertywidgetcef.h>
#include <modules/webbrowser/properties/stringpropertywidgetcef.h>

#include <modules/webbrowser/webbrowsermodule.h>

#include <warn/push>
#include <warn/ignore/all>
#include "include/cef_parser.h"
#include <warn/pop>

namespace inviwo {
PropertyCefSynchronizer::PropertyCefSynchronizer() {
    registerPropertyWidget<BoolPropertyWidgetCEF, BoolProperty>(PropertySemantics("Default"));
    registerPropertyWidget<ButtonPropertyWidgetCEF, ButtonProperty>(PropertySemantics("Default"));

    registerPropertyWidget<FloatPropertyWidgetCEF, FloatProperty>(PropertySemantics("Default"));
    registerPropertyWidget<DoublePropertyWidgetCEF, DoubleProperty>(PropertySemantics("Default"));
    registerPropertyWidget<IntPropertyWidgetCEF, IntProperty>(PropertySemantics("Default"));
    registerPropertyWidget<IntSizeTPropertyWidgetCEF, IntSizeTProperty>(
        PropertySemantics("Default"));
    registerPropertyWidget<Int64PropertyWidgetCEF, Int64Property>(PropertySemantics("Default"));

    registerPropertyWidget<StringPropertyWidgetCEF, StringProperty>(PropertySemantics("Default"));
};

void PropertyCefSynchronizer::OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                        int /*httpStatusCode*/) {
    // synchronize all properties
    // Ok to send javascript commands when frame loaded
    for (auto& widget : widgets_) {
        widget->setFrame(frame);
    }
}

void PropertyCefSynchronizer::OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                          CefLoadHandler::ErrorCode errorCode,
                                          const CefString& errorText, const CefString& failedUrl) {
    if (errorCode == ERR_ABORTED) {
        // Ignore page loading aborted (occurs during deserialization).
        // Prevents error page from showing after deserialization.
        return;
    }
    std::stringstream ss;
    ss << "<html><head><title>Page failed to load</title></head>"
          "<body bgcolor=\"white\">"
          "<h3>Page failed to load.</h3>"
          "URL: <a href=\""
       << failedUrl.ToString() << "\">" << failedUrl.ToString()
       << "</a><br/>Error: " << WebBrowserModule::getCefErrorString(errorCode) << " (" << errorCode
       << ")";

    if (!errorText.empty()) {
        ss << "<br/>Description: " << errorText.ToString();
    }
    ss << "</body></html>";

    frame->LoadURL(WebBrowserModule::getDataURI(ss.str(), "text/html"));
}

bool PropertyCefSynchronizer::OnQuery(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                      int64 query_id, const CefString& request, bool persistent,
                                      CefRefPtr<Callback> callback) {

    const std::string& requestStr = request;
    // Assume format "id":"htmlId"
    auto key = std::string(R"("id":")");
    auto keyStart = requestStr.find(key);
    if (keyStart == std::string::npos) {
        LogWarn(R"(No id found. Expected {"id":"elementId", "value":"x"})" + requestStr);
        return false;
    }
    size_t offset = keyStart + key.length();
    auto idEnd = requestStr.find('"', offset);
    if (idEnd == std::string::npos) {
        return false;
    }
    auto id = requestStr.substr(offset, idEnd - offset);
    auto widget = std::find_if(std::begin(widgets_), std::end(widgets_),
                               [id](const auto& widget) { return id == widget->getHtmlId(); });
    if (widget != widgets_.end()) {
        auto nextValPos = requestStr.find("}", idEnd);
        if (nextValPos == std::string::npos) {
            LogWarn("Missing enclosing } in: " + requestStr);
        } else {
            auto message = requestStr.substr(0, nextValPos);
            return (*widget)->onQuery(browser, frame, query_id, request, persistent, callback);
        }
    }
    return false;
}

void PropertyCefSynchronizer::startSynchronize(Property* property) {
    // We cannot use path since Processor is not set until after construction.
    // auto path = property->getPath();
    // auto htmlId = std::accumulate(std::next(path.begin()), path.end(), path[0],
    //                                     [](std::string &s, const std::string &piece) ->
    //                                     decltype(auto) { return s += "." + piece; });
    auto htmlId = property->getIdentifier();
    startSynchronize(property, htmlId);
}

void PropertyCefSynchronizer::startSynchronize(Property* property, std::string htmlId) {
    auto widget = dynamic_cast<PropertyWidgetCEF*>(htmlWidgetFactory_.create(property).release());
    if (!widget) {
        throw Exception("No HTML property widget for " + property->getClassIdentifier());
    }
    widget->setHtmlId(htmlId);
    // auto widget = std::make_unique<OrdinalPropertyWidgetCEF<T>>(property,
    // browser_->GetMainFrame(), htmlId);
    widgets_.emplace_back(std::move(widget));
}

void PropertyCefSynchronizer::stopSynchronize(Property* property) {
    util::erase_remove_if(widgets_,
                          [property](auto& widget) { return property == widget->getProperty(); });
}

}  // namespace inviwo
