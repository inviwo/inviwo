/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2020 Inviwo Foundation
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

#include <modules/webbrowser/processors/processorcefsynchronizer.h>
#include <modules/webbrowser/webbrowsermodule.h>

#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/processors/progressbarowner.h>
#include <inviwo/core/util/stringconversion.h>

#include <warn/push>
#include <warn/ignore/all>
#include "include/cef_parser.h"
#include <warn/pop>

namespace inviwo {

ProcessorCefSynchronizer::ProcessorCefSynchronizer(const Processor* webBrowserSource)
    : parent_(webBrowserSource) {}

void ProcessorCefSynchronizer::OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                         int /*httpStatusCode*/) {
    // synchronize all properties
    // Ok to send javascript commands when frame loaded
    for (auto& observer : progressObservers_) {
        observer.second.setFrame(frame);
    }
}

auto ProcessorCefSynchronizer::registerCallback(const std::string& name,
                                                std::function<CallbackFunc> callback)
    -> CallbackHandle {
    if (name.empty()) return nullptr;
    return callbacks_[name].add(callback);
}

bool ProcessorCefSynchronizer::OnQuery(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                       int64, const CefString& request, bool,
                                       CefRefPtr<Callback> callback) {

    const std::string& requestStr = request;
    // Assume format "id":"htmlId"
    auto j = json::parse(requestStr);

    try {
        auto command = j.at("command").get<std::string>();
        constexpr std::string_view subscribeProgressCommand = "processor.progress.subscribe";
        constexpr std::string_view unsubscribeProgressCommand = "processor.progress.unsubscribe";
        constexpr std::string_view parentProcessorCommand = "parentwebbrowserprocessor";
        constexpr std::string_view callbackCommand = "callback";

        if (command == subscribeProgressCommand) {
            auto p = j.at("path").get<std::string>();
            auto network = parent_->getNetwork();

            if (auto processor = network->getProcessorByIdentifier(p)) {
                if (auto progressOwner = dynamic_cast<ProgressBarOwner*>(processor)) {
                    auto onProgressChange = j.at("onProgressChange").get<std::string>();
                    auto onVisibleChange = j.at("onVisibleChange").get<std::string>();

                    progressObservers_[processor] =
                        ProgressBarObserverCEF(frame, onProgressChange, onVisibleChange);
                    progressOwner->getProgressBar().ProgressBarObservable::addObserver(
                        &progressObservers_[processor]);
                    callback->Success("");
                    return true;
                } else {
                    callback->Failure(0, "Processor " + p + " is not ProgressBarObservable");
                }
            } else {
                callback->Failure(0, "Could not find processor: " + p);
            }
        } else if (command == unsubscribeProgressCommand) {
            auto p = j.at("path").get<std::string>();
            auto network = parent_->getNetwork();
            if (auto processor = network->getProcessorByIdentifier(p)) {
                // Remove observer
                progressObservers_.erase(processor);
                callback->Success("");
                return true;
            } else {
                callback->Failure(0, "Could not find processor: " + p);
            }
        } else if (command == parentProcessorCommand) {
            callback->Success(json{{"path", parent_->getIdentifier()}}.dump());
            return true;
        } else if (command == callbackCommand) {
            const auto callbackName = j.at("callback").get<std::string>();
            if (auto it = callbacks_.find(callbackName); it != callbacks_.end()) {
                it->second.invoke(j.at("data").get<std::string>());
                callback->Success("");
                return true;
            } else {
                callback->Failure(0, "Trying to invoke non-registered Callback: " + callbackName);
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

void ProcessorCefSynchronizer::onProcessorNetworkWillRemoveProcessor(Processor* p) {
    progressObservers_.erase(p);
}

}  // namespace inviwo
