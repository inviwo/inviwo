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

#include <modules/webbrowser/properties/propertycefsynchronizer.h>
#include <modules/webbrowser/webbrowsermodule.h>

#include <inviwo/core/util/settings/settings.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/network/processornetwork.h>

#include <fmt/format.h>

#include <string_view>

#include <warn/push>
#include <warn/ignore/all>
#include "include/cef_parser.h"
#include <warn/pop>

namespace inviwo {

PropertyCefSynchronizer::PropertyCefSynchronizer(CefRefPtr<CefBrowser> browser,
                                                 const PropertyWidgetCEFFactory* htmlWidgetFactory)
    : htmlWidgetFactory_(htmlWidgetFactory), browserIdentifier_(browser->GetIdentifier()) {}

void PropertyCefSynchronizer::OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                          [[maybe_unused]] TransitionType transition_type) {
    if (browser->GetIdentifier() != browserIdentifier_) {
        return;
    }
    widgets_.clear();
}

void PropertyCefSynchronizer::OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                        int /*httpStatusCode*/) {
    if (browser->GetIdentifier() != browserIdentifier_) {
        return;
    }
    // synchronize all properties
    // Ok to send javascript commands when frame loaded
    for (auto& widget : widgets_) {
        widget->setFrame(frame);
    }
}

bool PropertyCefSynchronizer::OnQuery(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                      int64 query_id, const CefString& request, bool persistent,
                                      CefRefPtr<Callback> callback) {
    if (browser->GetIdentifier() != browserIdentifier_) {
        return false;
    }
    const std::string& requestStr = request;
    // Assume format "id":"htmlId"
    const auto j = json::parse(requestStr);
    // Searches first for the property in Processors and then in Settings
    auto findProperty = [](const std::string_view path) -> Property* {
        auto network = InviwoApplication::getPtr()->getProcessorNetwork();
        Property* prop = network->getProperty(path);
        if (!prop) {
            // Retrieves both system and module settings
            auto settings = InviwoApplication::getPtr()->getModuleSettings();
            const auto [settingsIdentifier, propertyPath] = util::splitByFirst(path, '.');
            auto it = util::find_if(settings,
                                    [settingsIdentifier = settingsIdentifier](const auto setting) {
                                        return setting->getIdentifier() == settingsIdentifier;
                                    });
            if (it != settings.end()) {
                prop = (*it)->getPropertyByPath(propertyPath);
            }
        }
        return prop;
    };

    try {
        const auto command = j.at("command").get<std::string_view>();
        constexpr std::string_view subscribeCommand = "subscribe";
        constexpr std::string_view propCommand = "property";
        if (command == subscribeCommand) {
            const auto path = j.at("path").get<std::string_view>();
            if (auto prop = findProperty(path)) {
                const auto onChange = j.at("onChange").get<std::string_view>();
                auto widget =
                    std::find_if(widgets_.begin(), widgets_.end(), [&](const auto& widget) {
                        return prop == widget->getProperty() && onChange == widget->getOnChange();
                    });
                if (widget == widgets_.end()) {
                    auto propertyObserver = j.at("propertyObserver").get<std::string_view>();
                    startSynchronize(prop, onChange, propertyObserver);
                    widget = --(widgets_.end());
                    (*widget)->setFrame(frame);
                    callback->Success("");
                    return true;
                }
            } else {
                auto msg = fmt::format("Could not find property: {}", path);
                LogWarn(msg);
                callback->Failure(0, msg);
            }
        } else if (!command.compare(0, propCommand.size(), propCommand)) {
            const auto path = j.at("path").get<std::string_view>();
            auto prop = findProperty(path);
            if (!prop) {
                throw Exception(fmt::format("Could not find property: {}", path), IVW_CONTEXT);
            }
            // Use synchronized widget if it exists
            // to avoid recursive loop when setting the property
            auto widget = std::find_if(widgets_.begin(), widgets_.end(), [&](const auto& widget) {
                return prop == widget->getProperty();
            });
            if (widget != widgets_.end()) {
                return (*widget)->onQuery(browser, frame, query_id, request, persistent, callback);
            } else {
                auto w = htmlWidgetFactory_->create(prop->getClassIdentifier(), prop);
                if (!w) {
                    throw Exception("No HTML property widget for " + prop->getClassIdentifier(),
                                    IVW_CONTEXT);
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

void PropertyCefSynchronizer::startSynchronize(Property* property, std::string_view onChange,
                                               std::string_view propertyObserverCallback) {
    auto widget = htmlWidgetFactory_->create(property->getClassIdentifier(), property);
    if (!widget) {
        throw Exception("No HTML property widget for " + property->getClassIdentifier(),
                        IVW_CONTEXT);
    }
    widget->setOnChange(onChange);
    widget->setPropertyObserverCallback(propertyObserverCallback);

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
