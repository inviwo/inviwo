/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2024 Inviwo Foundation
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

#include <modules/webbrowser/properties/propertywidgetcef.h>

#include <inviwo/core/properties/property.h>             // for Property
#include <inviwo/core/properties/propertysemantics.h>    // for PropertySemantics
#include <inviwo/core/properties/propertywidget.h>       // for PropertyWidget
#include <inviwo/core/util/logcentral.h>                 // for LogCentral, LogError

#include <include/base/cef_scoped_refptr.h>  // for scoped_refptr
#include <include/cef_base.h>                // for CefRefPtr, CefString
#include <include/cef_frame.h>               // for CefFrame
#include <nlohmann/json.hpp>                 // for json_ref, basic_json, basic_json...

#include <stdexcept>  // for out_of_range
#include <utility>    // for move
#include <vector>     // for vector

class CefBrowser;

using json = nlohmann::json;

namespace inviwo {

PropertyWidgetCEF::PropertyWidgetCEF(Property* prop, const JSONPropertyConverter& converter,
                                     CefRefPtr<CefFrame> frame, std::string_view onChange)
    : PropertyWidget(prop), converter_(converter), onChange_(onChange), frame_(frame) {
    if (prop) {
        prop->addObserver(this);
    }
}
void PropertyWidgetCEF::setFrame(CefRefPtr<CefFrame> frame) {
    frame_ = frame;
    updateFromProperty();
}

bool PropertyWidgetCEF::onQuery(
    CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int64_t /*query_id*/,
    const CefString& request, bool /*persistent*/,
    CefRefPtr<CefMessageRouterBrowserSide::Handler::Callback> callback) {

    const std::string& requestString = request;
    auto j = json::parse(requestString);
    try {
        auto command = j.at("command").get<std::string>();
        auto p = getProperty();
        if (command == "property.set") {
            p->setInitiatingWidget(this);
            converter_.fromJSON(j.at("parameters"), *p);
            p->clearInitiatingWidget();
            callback->Success("");
        } else if (command == "property.get") {
            callback->Success(converter_.toJSON(*p).dump());
        }
    } catch (json::exception& ex) {
        LogError(ex.what());
        callback->Failure(0, ex.what());
    }

    return true;
}

void PropertyWidgetCEF::updateFromProperty() {
    // Frame might be null if for example webpage is not found on startup
    if (!frame_) {
        return;
    }

    const auto script =
        fmt::format("{}({})", getOnChange(), converter_.toJSON(*getProperty()).dump());
    frame_->ExecuteJavaScript(script, frame_->GetURL(), 0);
}

void PropertyWidgetCEF::onSetIdentifier(Property* /*property*/, const std::string& identifier) {
    if (!frame_) {
        return;
    }

    auto p = json{{"identifier", identifier}};
    const auto script = fmt::format("{}({})", getPropertyObserverCallback(), p.dump());
    frame_->ExecuteJavaScript(script, frame_->GetURL(), 0);
}

void PropertyWidgetCEF::onSetDisplayName(Property* /*property*/, const std::string& displayName) {
    if (!frame_) {
        return;
    }

    auto p = json{{"displayName", displayName}};
    const auto script = fmt::format("{}({})", getPropertyObserverCallback(), p.dump());
    frame_->ExecuteJavaScript(script, frame_->GetURL(), 0);
}
void PropertyWidgetCEF::onSetSemantics(Property* /*property*/, const PropertySemantics& semantics) {
    if (!frame_) {
        return;
    }

    auto p = json{{"semantics", semantics.getString()}};
    const auto script = fmt::format("{}({})", getPropertyObserverCallback(), p.dump());
    frame_->ExecuteJavaScript(script, frame_->GetURL(), 0);
}
void PropertyWidgetCEF::onSetReadOnly(Property* /*property*/, bool readonly) {
    if (!frame_) {
        return;
    }

    auto p = json{{"readOnly", (readonly ? "true" : "false")}};
    const auto script = fmt::format("{}({})", getPropertyObserverCallback(), p.dump());
    frame_->ExecuteJavaScript(script, frame_->GetURL(), 0);
}
void PropertyWidgetCEF::onSetVisible(Property* /*property*/, bool visible) {
    if (!frame_) {
        return;
    }

    auto p = json{{"visible", (visible ? "true" : "false")}};
    const auto script = fmt::format("{}({})", getPropertyObserverCallback(), p.dump());
    frame_->ExecuteJavaScript(script, frame_->GetURL(), 0);
}

}  // namespace inviwo
