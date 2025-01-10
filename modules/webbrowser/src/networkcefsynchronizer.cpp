/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024-2025 Inviwo Foundation
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

#include <modules/webbrowser/networkcefsynchronizer.h>
#include <inviwo/core/common/inviwoapplication.h>

#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/util/exception.h>

#include <modules/json/jsonmodule.h>
#include <modules/json/jsonpropertyconverter.h>
#include <modules/webbrowser/properties/propertywidgetcef.h>

#include <algorithm>
#include <inviwo/dataframe/datastructures/dataframe.h>
#include <inviwo/dataframe/jsondataframeconversion.h>
#include <modules/brushingandlinking/ports/brushingandlinkingports.h>

namespace inviwo {

static constexpr std::string_view subscribeCommand = "subscribe";
static constexpr std::string_view propertyCommand = "property";
static constexpr std::string_view subscribeProgress = "processor.progress.subscribe";
static constexpr std::string_view unsubscribeProgress = "processor.progress.unsubscribe";

static constexpr std::string_view callbackCommand = "callback";
static constexpr std::string_view networkCommand = "network.";
static constexpr std::string_view processorCommand = "processor.";
static constexpr std::string_view inportCommand = "inport.";
static constexpr std::string_view outportCommand = "outport.";
static constexpr std::string_view propCommand = "prop.";

NetWorkCefSynchronizer::NetWorkCefSynchronizer(InviwoApplication* app)
    : app_{app}
    , jsonPropertyConverter_{app_->getModuleByType<JSONModule>()->getJSONPropertyConverter()}
    , jsonInportConverter_{app_->getModuleByType<JSONModule>()->getJSONInportConverter()}
    , jsonOutportConverter_{app_->getModuleByType<JSONModule>()->getJSONOutportConverter()} {

    app_->getProcessorNetwork()->addObserver(this);
}

NetWorkCefSynchronizer::~NetWorkCefSynchronizer() = default;

void NetWorkCefSynchronizer::OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame>,
                                         TransitionType) {
    widgets_[browser->GetIdentifier()].clear();
}

void NetWorkCefSynchronizer::OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                       int /*httpStatusCode*/) {
    for (auto& widget : widgets_[browser->GetIdentifier()]) {
        widget->setFrame(frame);
    }

    for (auto& observer : progressObservers_) {
        observer.second.setFrame(frame);
    }
}

bool NetWorkCefSynchronizer::OnQuery(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                     int64_t query_id, const CefString& request, bool persistent,
                                     CefRefPtr<Callback> reponse) {

    const auto j = json::parse(request.ToString());

    try {
        const auto command = j.at("command").get<std::string_view>();

        if (command == subscribeCommand) {
            return propertySubscribe(j, browser, frame, reponse);
        } else if (command.starts_with(propertyCommand)) {
            return propertyAction(j, browser, frame, query_id, request, persistent, reponse);
        } else if (command == subscribeProgress) {
            return processorSubscribeProgress(j, frame, reponse);
        } else if (command == unsubscribeProgress) {
            return processorUnsubscribeProgress(j, reponse);
        } else if (command == callbackCommand) {
            return handleCallback(j, browser, reponse);
        } else if (command.starts_with(networkCommand)) {
            return handleNetwork(j, browser, reponse);
        } else if (command.starts_with(processorCommand)) {
            return handleProcessor(j, browser, reponse);
        } else if (command.starts_with(inportCommand)) {
            return handleInport(j, browser, reponse);
        } else if (command.starts_with(outportCommand)) {
            return handleOutport(j, browser, reponse);
        } else if (command.starts_with(propCommand)) {
            return handleProperty(j, browser, reponse);
        } else {
            reponse->Failure(0, fmt::format("Unknown command: {}", command));
            return true;
        }
    } catch (json::exception& ex) {
        LogError(ex.what());
        reponse->Failure(0, ex.what());
    } catch (inviwo::Exception& ex) {
        util::log(ex.getContext(), ex.getMessage(), LogLevel::Error);
        reponse->Failure(0, ex.what());
    } catch (std::exception& ex) {
        LogError(ex.what());
        reponse->Failure(0, ex.what());
    }
    return true;
}

bool NetWorkCefSynchronizer::handleInport(const json& j, CefRefPtr<CefBrowser>&,
                                          CefRefPtr<Callback>& reponse) {
    const auto command = j.at("command").get<std::string_view>().substr(inportCommand.size());
    const auto processorId = j.at("processor").get<std::string>();
    const auto portId = j.at("identifier").get<std::string>();

    auto* processor = app_->getProcessorNetwork()->getProcessorByIdentifier(processorId);
    if (!processor) {
        reponse->Failure(
            0, fmt::format("Trying to invoke an inport command: {} on an unknown processor {}",
                           command, processorId));
        return true;
    }

    auto* port = processor->getInport(portId);
    if (!port) {
        reponse->Failure(
            0, fmt::format("Trying to invoke an inport command: {} on an unknown port {}", command,
                           portId));
        return true;
    }

    if (command == "getData") {
        reponse->Success(CefString{jsonInportConverter_.toJSON(*port).dump()});

    } else if (command == "getFilteredIndices") {
        if (auto* brushing = dynamic_cast<BrushingAndLinkingInport*>(port)) {
            json result =
                brushing->getFilteredIndices(BrushingTarget(j.at("target").get<std::string>()));
            reponse->Success(CefString{result.dump()});
        } else {
            reponse->Failure(
                0, fmt::format("Inport {} is not of BrushingAndLinkingInport type", portId));
        }
    } else if (command == "getSelectedIndices") {
        if (auto* brushing = dynamic_cast<BrushingAndLinkingInport*>(port)) {
            json result =
                brushing->getSelectedIndices(BrushingTarget(j.at("target").get<std::string>()));
            reponse->Success(CefString{result.dump()});
        } else {
            reponse->Failure(
                0, fmt::format("Inport {} is not of BrushingAndLinkingInport type", portId));
        }
    } else if (command == "getHighlightedIndices") {
        if (auto* brushing = dynamic_cast<BrushingAndLinkingInport*>(port)) {
            json result =
                brushing->getHighlightedIndices(BrushingTarget(j.at("target").get<std::string>()));
            reponse->Success(CefString{result.dump()});
        } else {
            reponse->Failure(
                0, fmt::format("Inport {} is not of BrushingAndLinkingInport type", portId));
        }

    } else if (command == "filter") {
        if (auto* brushing = dynamic_cast<BrushingAndLinkingInport*>(port)) {
            const auto inds = j.at("indices").get<std::vector<uint32_t>>();
            brushing->filter(brushing->getIdentifier(), BitSet(inds),
                             BrushingTarget(j.at("target").get<std::string>()));
            reponse->Success("{}");
        } else {
            reponse->Failure(
                0, fmt::format("Inport {} is not of BrushingAndLinkingInport type", portId));
        }
    } else if (command == "select") {
        if (auto* brushing = dynamic_cast<BrushingAndLinkingInport*>(port)) {
            const auto inds = j.at("indices").get<std::vector<uint32_t>>();
            brushing->select(BitSet(inds), BrushingTarget(j.at("target").get<std::string>()));
            reponse->Success("{}");
        } else {
            reponse->Failure(
                0, fmt::format("Inport {} is not of BrushingAndLinkingInport type", portId));
        }
    } else if (command == "highlight") {
        if (auto* brushing = dynamic_cast<BrushingAndLinkingInport*>(port)) {
            const auto inds = j.at("indices").get<std::vector<uint32_t>>();
            brushing->highlight(BitSet(inds), BrushingTarget(j.at("target").get<std::string>()));
            reponse->Success("{}");
        } else {
            reponse->Failure(
                0, fmt::format("Inport {} is not of BrushingAndLinkingInport type", portId));
        }

    } else {
        reponse->Failure(0, fmt::format("Trying to invoke an unknow inport command: {}", command));
    }
    return true;
}
bool NetWorkCefSynchronizer::handleOutport(const json& j, CefRefPtr<CefBrowser>&,
                                           CefRefPtr<Callback>& reponse) {
    const auto command = j.at("command").get<std::string_view>().substr(inportCommand.size());
    const auto processorId = j.at("processor").get<std::string>();
    const auto portId = j.at("identifier").get<std::string>();

    auto* processor = app_->getProcessorNetwork()->getProcessorByIdentifier(processorId);
    if (!processor) {
        reponse->Failure(
            0, fmt::format("Trying to invoke an outport command: {} on an unknown processor {}",
                           command, processorId));
        return true;
    }

    auto* port = processor->getOutport(portId);
    if (!port) {
        reponse->Failure(
            0, fmt::format("Trying to invoke an outport command: {} on an unknown port {}", command,
                           portId));
        return true;
    }

    if (command == "setData") {
        jsonOutportConverter_.fromJSON(j.at("data"), *port);
        reponse->Success("{}");
    } else {
        reponse->Failure(0, fmt::format("Trying to invoke an unknow inport command: {}", command));
    }

    return true;
}
bool NetWorkCefSynchronizer::handleProperty(const json& j, CefRefPtr<CefBrowser>&,
                                            CefRefPtr<Callback>& reponse) {

    const auto command = j.at("command").get<std::string_view>().substr(propertyCommand.size());

    auto propertyPath = j.at("path").get<std::string>();
    auto* property = findProperty(propertyPath);
    if (!property) {
        reponse->Failure(
            0, fmt::format("Trying to invoke a property command: {} on an unknown property {}",
                           command, propertyPath));
        return true;
    }

    if (command == "get") {
        reponse->Success(CefString{jsonPropertyConverter_.toJSON(*property).dump()});
    } else if (command == "set") {
        jsonPropertyConverter_.fromJSON(j.at("data"), *property);
        reponse->Success("{}");
    }
    return true;
}

bool NetWorkCefSynchronizer::handleProcessor(const json& j, CefRefPtr<CefBrowser>&,
                                             CefRefPtr<Callback>& reponse) {
    const auto command = j.at("command").get<std::string_view>().substr(processorCommand.size());

    const auto identifier = j.at("identifier").get<std::string>();
    auto* processor = app_->getProcessorNetwork()->getProcessorByIdentifier(identifier);
    if (!processor) {
        reponse->Failure(
            0, fmt::format("Trying to invoke a processor command: {} on an unknown processor {}",
                           command, identifier));
        return true;
    }

    if (command == "properties") {
        json result;
        for (Property* property : *processor) {
            result.emplace_back(property->getIdentifier());
        }
        reponse->Success(CefString{result.dump()});
    } else if (command == "inports") {
        json result;
        for (Inport* inport : processor->getInports()) {
            result.emplace_back(inport->getIdentifier());
        }
        reponse->Success(CefString{result.dump()});
    } else if (command == "outports") {
        json result;
        for (Outport* outport : processor->getOutports()) {
            result.emplace_back(outport->getIdentifier());
        }
        reponse->Success(CefString{result.dump()});
    } else {
        reponse->Failure(0,
                         fmt::format("Trying to invoke an unknow processor command: {}", command));
    }
    return true;
}

bool NetWorkCefSynchronizer::handleNetwork(const json& j, CefRefPtr<CefBrowser>&,
                                           CefRefPtr<Callback>& reponse) {
    const auto command = j.at("command").get<std::string_view>().substr(networkCommand.size());
    if (command == "processors") {
        json result;
        app_->getProcessorNetwork()->forEachProcessor(
            [&](Processor* processor) { result.emplace_back(processor->getIdentifier()); });
        reponse->Success(CefString{result.dump()});
    } else {
        reponse->Failure(0, fmt::format("Trying to invoke an unknow network command: {}", command));
    }
    return true;
}

bool NetWorkCefSynchronizer::handleCallback(
    const json& j, CefRefPtr<CefBrowser>& browser,
    CefRefPtr<CefMessageRouterBrowserSide::Handler::Callback>& callback) {

    const auto callbackName = j.at("callback").get<std::string>();
    auto& callbacks = callbacks_[browser->GetIdentifier()];
    if (auto it = callbacks.find(callbackName); it != callbacks.end()) {
        if (auto func = it->second.lock()) {
            auto res = (*func)(j.at("data").get<std::string>());
            callback->Success(CefString{res});
        } else {
            callback->Failure(0, "Callback has been removed: " + callbackName);
            callbacks.erase(it);
        }
    } else {
        callback->Failure(0, "Trying to invoke a non-registered Callback: " + callbackName);
    }
    return true;
}

bool NetWorkCefSynchronizer::processorUnsubscribeProgress(
    const json& j, CefRefPtr<CefMessageRouterBrowserSide::Handler::Callback>& callback) {

    auto p = j.at("path").get<std::string>();
    if (auto processor = app_->getProcessorNetwork()->getProcessorByIdentifier(p)) {
        // Remove observer
        progressObservers_.erase(processor);
        callback->Success("{}");
    } else {
        callback->Failure(0, "Could not find processor: " + p);
    }
    return true;
}

bool NetWorkCefSynchronizer::processorSubscribeProgress(
    const json& j, CefRefPtr<CefFrame>& frame,
    CefRefPtr<CefMessageRouterBrowserSide::Handler::Callback>& callback) {

    const auto p = j.at("path").get<std::string>();
    if (auto processor = app_->getProcessorNetwork()->getProcessorByIdentifier(p)) {
        if (auto progressOwner = dynamic_cast<ProgressBarOwner*>(processor)) {
            const auto onProgressChange = j.at("onProgressChange").get<std::string>();
            const auto onVisibleChange = j.at("onVisibleChange").get<std::string>();

            progressObservers_[processor] =
                ProgressBarObserverCEF(frame, onProgressChange, onVisibleChange);
            progressOwner->getProgressBar().ProgressBarObservable::addObserver(
                &progressObservers_[processor]);
            callback->Success("{}");
        } else {
            callback->Failure(0, "Processor " + p + " is not ProgressBarObservable");
        }
    } else {
        callback->Failure(0, "Could not find processor: " + p);
    }
    return true;
}

bool NetWorkCefSynchronizer::propertySubscribe(const json& j, CefRefPtr<CefBrowser>& browser,
                                               CefRefPtr<CefFrame>& frame,
                                               CefRefPtr<Callback>& callback) {
    const auto path = j.at("path").get<std::string_view>();
    if (auto* property = findProperty(path)) {
        const auto onChange = j.at("onChange").get<std::string_view>();
        auto& widgets = widgets_[browser->GetIdentifier()];
        auto it = std::ranges::find_if(widgets, [&](const auto& w) {
            return property == w->getProperty() && onChange == w->getOnChange();
        });
        if (it == widgets.end()) {
            auto propertyObserver = j.at("propertyObserver").get<std::string_view>();
            widgets.push_back(createWidget(property, frame, onChange, propertyObserver));
            callback->Success("");
        }
    } else {
        auto msg = fmt::format("Could not find property: {}", path);
        LogWarn(msg);
        callback->Failure(0, msg);
    }
    return true;
}

bool NetWorkCefSynchronizer::propertyAction(const json& j, CefRefPtr<CefBrowser>& browser,
                                            CefRefPtr<CefFrame>& frame, int64_t query_id,
                                            const CefString& request, bool persistent,
                                            CefRefPtr<Callback>& callback) {
    const auto path = j.at("path").get<std::string_view>();
    auto* prop = findProperty(path);
    if (!prop) {
        throw Exception(fmt::format("Could not find property: {}", path), IVW_CONTEXT);
    }
    // Use synchronized widget if it exists
    // to avoid recursive loop when setting the property
    auto& widgets = widgets_[browser->GetIdentifier()];
    auto it =
        std::ranges::find_if(widgets, [&](const auto& w) { return prop == w->getProperty(); });
    if (it != widgets.end()) {
        return (*it)->onQuery(browser, frame, query_id, request, persistent, callback);
    } else {
        PropertyWidgetCEF w(prop, jsonPropertyConverter_);
        return w.onQuery(browser, frame, query_id, request, persistent, callback);
    }
}

void NetWorkCefSynchronizer::onWillRemoveProperty(Property* property, size_t /*index*/) {
    for (auto&& [key, widgets] : widgets_) {
        std::erase_if(widgets,
                      [property](const auto& widget) { return property == widget->getProperty(); });
    }
}

void NetWorkCefSynchronizer::onProcessorNetworkWillRemoveProcessor(Processor* p) {
    progressObservers_.erase(p);
}

void NetWorkCefSynchronizer::clear(CefRefPtr<CefBrowser> browser) {
    widgets_.erase(browser->GetIdentifier());
    callbacks_.erase(browser->GetIdentifier());
}

NetWorkCefSynchronizer::CallbackHandle NetWorkCefSynchronizer::registerCallback(
    CefRefPtr<CefBrowser> browser, const std::string& name, std::function<CallbackFunc> callback) {
    auto func = std::make_shared<std::function<CallbackFunc>>(callback);
    callbacks_[browser->GetIdentifier()][name] = func;
    return func;
}

// Searches first for the property in Processors and then in Settings
Property* NetWorkCefSynchronizer::findProperty(std::string_view path) {
    Property* prop = app_->getProcessorNetwork()->getProperty(path);
    if (!prop) {
        // Retrieves both system and module settings
        auto settings = app_->getModuleSettings();
        const auto [settingsIdentifier, propertyPath] = util::splitByFirst(path, '.');
        auto it =
            std::ranges::find_if(settings, [identifier = settingsIdentifier](const auto setting) {
                return setting->getIdentifier() == identifier;
            });
        if (it != settings.end()) {
            prop = (*it)->getPropertyByPath(propertyPath);
        }
    }
    return prop;
}

std::unique_ptr<PropertyWidgetCEF> NetWorkCefSynchronizer::createWidget(
    Property* property, CefRefPtr<CefFrame>& frame, std::string_view onChangeJS,
    std::string_view propertyObserverCallbackJS) {

    auto widget =
        std::make_unique<PropertyWidgetCEF>(property, jsonPropertyConverter_, frame, onChangeJS);

    widget->setPropertyObserverCallback(propertyObserverCallbackJS);
    if (auto owner = property->getOwner()) {
        owner->addObserver(this);
    }
    return widget;
}

}  // namespace inviwo
