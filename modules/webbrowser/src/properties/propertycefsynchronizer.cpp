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

#include <modules/webbrowser/properties/propertycefsynchronizer.h>
#include <modules/webbrowser/webbrowsermodule.h>

#include <inviwo/core/util/stringconversion.h>

#include <warn/push>
#include <warn/ignore/all>
#include "include/cef_parser.h"
#include <warn/pop>

namespace inviwo {

PropertyCefSynchronizer::PropertyCefSynchronizer(const PropertyWidgetCEFFactory* htmlWidgetFactory)
    : htmlWidgetFactory_(htmlWidgetFactory){

      };

void PropertyCefSynchronizer::OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                        int /*httpStatusCode*/) {
    // synchronize all properties
    // Ok to send javascript commands when frame loaded
    for (auto& widget : widgets_) {
        widget->setFrame(frame);
    }
}

bool PropertyCefSynchronizer::OnQuery(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                      int64 query_id, const CefString& request, bool persistent,
                                      CefRefPtr<Callback> callback) {

    const std::string& requestStr = request;
    // Assume format "id":"htmlId"
    auto j = json::parse(requestStr);

    try {
        auto command = j.at("command").get<std::string>();
        auto propCommand = std::string("property");
        if (command == "subscribe") {
            auto network = InviwoApplication::getPtr()->getProcessorNetwork();
            auto p = j.at("path").get<std::string>();
            auto path = splitString(p, '.');
            auto prop = network->getProperty(path);
            if (prop) {
                auto onChange = j.at("onChange").get<std::string>();
                auto widget = std::find_if(
                    std::begin(widgets_), std::end(widgets_), [onChange, prop](const auto& widget) {
                        return prop == widget->getProperty() && onChange == widget->getOnChange();
                    });
                if (widget == widgets_.end()) {
                    auto propertyObserver = j.at("propertyObserver").get<std::string>();
                    startSynchronize(prop, onChange, propertyObserver);
                    widget = --(widgets_.end());
                    (*widget)->setFrame(frame);
                }
            } else {
                callback->Failure(0, "Could not find property: " + p);
            }
        } else if (!command.compare(0, propCommand.size(), propCommand)) {
            auto network = InviwoApplication::getPtr()->getProcessorNetwork();
            auto propertyPath = j.at("path").get<std::string>();
            auto path = splitString(propertyPath, '.');
            auto prop = network->getProperty(path);
            if (!prop) {
                throw Exception("Could not find property " + propertyPath);
            }
            // Use synchronized widget if it exists
            // to avoid recursive loop when setting the property
            auto widget =
                std::find_if(std::begin(widgets_), std::end(widgets_),
                             [prop](const auto& widget) { return prop == widget->getProperty(); });
            if (widget != widgets_.end()) {
                return (*widget)->onQuery(browser, frame, query_id, request, persistent, callback);
            } else {
                auto w = htmlWidgetFactory_->create(prop->getClassIdentifier(), prop);
                if (!w) {
                    throw Exception("No HTML property widget for " + prop->getClassIdentifier());
                }
                return w->onQuery(browser, frame, query_id, request, persistent, callback);
            }
        }
    } catch (json::exception& ex) {
        LogError(ex.what());
        callback->Failure(0, ex.what());
    } catch (inviwo::Exception& ex) {
        util::log(ex.getContext(), ex.getMessage(), LogLevel::Error);
        callback->Failure(0, ex.what());
    } catch (std::exception& ex) {
        LogError(ex.what());
        callback->Failure(0, ex.what());
    }

    return false;
}

void PropertyCefSynchronizer::onWillRemoveProperty(Property* property, size_t) {
    stopSynchronize(property);
}

void PropertyCefSynchronizer::startSynchronize(Property* property, std::string onChange,
                                               std::string propertyObserverCallback) {
    auto widget = htmlWidgetFactory_->create(property->getClassIdentifier(), property);
    if (!widget) {
        throw Exception("No HTML property widget for " + property->getClassIdentifier());
    }
    widget->setOnChange(onChange);
    widget->setPropertyObserverCallback(propertyObserverCallback);
    // auto widget = std::make_unique<OrdinalPropertyWidgetCEF<T>>(property,
    // browser_->GetMainFrame(), htmlId);
    widgets_.emplace_back(std::move(widget));
    if (auto owner = property->getOwner()) {
        owner->addObserver(this);
    }
}

void PropertyCefSynchronizer::stopSynchronize(Property* property) {
    util::erase_remove_if(widgets_,
                          [property](auto& widget) { return property == widget->getProperty(); });
}

}  // namespace inviwo
