/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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
#include <inviwo/core/properties/property.h>
#include <inviwo/core/io/serialization/serialization.h>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace inviwo {

// Checks if widget html id exist in the frame and sets it if it does.
// Note: Cannot use CefDOMVisitor since it requires the renderer process.
class CefDOMSearchId : public CefStringVisitor {
public:
    CefDOMSearchId(const std::string& stringToFind, PropertyWidgetCEF* widget,
                   const CefRefPtr<CefFrame> frame)
        : CefStringVisitor(), stringToFind_(stringToFind), widget_(widget), frame_(frame){};

    void Visit(const CefString& string) OVERRIDE {
        std::string domString = string;

        // Remove all the html comments to avoid finding element id's in the comments.
        while (true) {
            auto start = domString.find("<!--");
            auto stop = domString.find("-->");
            if (start != std::string::npos)
                domString.erase(start, stop - start + 3);
            else
                break;
        }
        // If the widget's html-id is in the given frame's DOM-document, set it's frame.
        if (domString.find(stringToFind_) != std::string::npos) {
            widget_->frame_ = frame_;
            widget_->updateFromProperty();
        }
    };

private:
    std::string stringToFind_;
    PropertyWidgetCEF* widget_;
    const CefRefPtr<CefFrame> frame_;
    IMPLEMENT_REFCOUNTING(CefDOMSearchId);
};

PropertyWidgetCEF::PropertyWidgetCEF(Property* prop, CefRefPtr<CefFrame> frame, std::string onChange)
    : PropertyWidget(prop), onChange_(onChange), frame_(frame)  {
    if (prop) {
        prop->addObserver(this);
    }
}
void PropertyWidgetCEF::setFrame(CefRefPtr<CefFrame> frame) {
    setFrameIfPartOfFrame(frame);
    // frame_ = frame;
    // Make sure that we do not block synchronizations from new page.
    onQueryBlocker_ = 0;
}

void PropertyWidgetCEF::setFrameIfPartOfFrame(CefRefPtr<CefFrame> frame) {
    // Create a visitor from this widget and run it on the frame to see if the widget id can be
    // found in the frame's html code.
    std::stringstream function;
    function << "function " << getOnChange();
    CefRefPtr<CefDOMSearchId> visitor = new CefDOMSearchId(function.str(), this, frame);
    frame->GetSource(visitor);
}

void PropertyWidgetCEF::deserialize(Deserializer& d) {
    if (onQueryBlocker_ > 0) {
        onQueryBlocker_--;
        return;
    }
    property_->setInitiatingWidget(this);
    d.deserialize("Property", *property_);
    property_->clearInitiatingWidget();
}

void PropertyWidgetCEF::onSetIdentifier(Property* /*property*/, const std::string& identifier) {
    std::stringstream script;
    auto p = json{{"identifier", identifier}};
    script << this->getPropertyObserverCallback() << "(" << p.dump() << ");";
    frame_->ExecuteJavaScript(script.str(), frame_->GetURL(), 0);
}

void PropertyWidgetCEF::onSetDisplayName(Property* /*property*/, const std::string& displayName) {
    std::stringstream script;
    auto p = json{{"displayName", displayName}};
    script << this->getPropertyObserverCallback() << "(" << p.dump() << ");";
    frame_->ExecuteJavaScript(script.str(), frame_->GetURL(), 0);
}
void PropertyWidgetCEF::onSetSemantics(Property* /*property*/, const PropertySemantics& semantics) {
    std::stringstream script;
    auto p = json{{"semantics", semantics.getString()}};
    script << this->getPropertyObserverCallback() << "(" << p.dump() << ");";
    frame_->ExecuteJavaScript(script.str(), frame_->GetURL(), 0);
}
void PropertyWidgetCEF::onSetReadOnly(Property* /*property*/, bool readonly) {
    std::stringstream script;
    auto p = json{{"readOnly", (readonly ? "true" : "false")}};
    script << this->getPropertyObserverCallback() << "(" << p.dump() << ");";
    frame_->ExecuteJavaScript(script.str(), frame_->GetURL(), 0);
}
void PropertyWidgetCEF::onSetVisible(Property* /*property*/, bool visible) {
    std::stringstream script;
    auto p = json{{"visible", (visible ? "true" : "false")}};
    script << this->getPropertyObserverCallback() << "(" << p.dump() << ");";
    frame_->ExecuteJavaScript(script.str(), frame_->GetURL(), 0);
}

void PropertyWidgetCEF::onSetUsageMode(Property* /*property*/, UsageMode usageMode) {
    std::stringstream script;
    std::stringstream mode;
    mode << usageMode;
    auto p = json{{"usageMode", mode.str()}};
    script << this->getPropertyObserverCallback() << "(" << p.dump() << ");";
    frame_->ExecuteJavaScript(script.str(), frame_->GetURL(), 0);
}

}  // namespace inviwo
