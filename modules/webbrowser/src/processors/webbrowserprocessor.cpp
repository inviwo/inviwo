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

#include <modules/webbrowser/processors/webbrowserprocessor.h>

#include <inviwo/core/util/staticstring.h>
#include <modules/webbrowser/cefimageconverter.h>
#include <modules/webbrowser/interaction/cefinteractionhandler.h>
#include <modules/webbrowser/renderhandlergl.h>
#include <modules/webbrowser/webbrowserclient.h>
#include <modules/webbrowser/webbrowsermodule.h>
#include <modules/webbrowser/webbrowserutil.h>

#include <include/base/cef_scoped_refptr.h>
#include <include/cef_base.h>
#include <include/cef_browser.h>
#include <include/cef_client.h>
#include <include/cef_frame.h>
#include <include/cef_request_context.h>
#include <include/cef_values.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo WebBrowserProcessor::processorInfo_{
    "org.inviwo.webbrowser",        // Class identifier
    "Web browser",                  // Display name
    "Web",                          // Category
    CodeState::Stable,              // Code state
    Tags::GL | Tag{"Web Browser"},  // Tags
    R"(Display webpage, including transparency, on top of optional background and
enable synchronization of properties.

Example networks:
+ [webbrowser/synchronization.inv](file:~modulePath~/data/workspaces/synchronization.inv)
+ [webbrowser/browser.inv](file:~modulePath~/data/workspaces/browser.inv)
)"_unindentHelp,
};
const ProcessorInfo WebBrowserProcessor::getProcessorInfo() const { return processorInfo_; }

WebBrowserProcessor::WebBrowserProcessor(InviwoApplication* app)
    : Processor()
    // Output from CEF is 8-bits per channel
    , background_{"background"}
    , outport_{"webpage", DataVec4UInt8::get()}
    , sourceType_{"sourceType", "Source",
                  OptionPropertyState<SourceType>{
                      .options = {{"localFile", "Local File", SourceType::LocalFile},
                                  {"webAddress", "Web Address", SourceType::WebAddress}},
                      .invalidationLevel = InvalidationLevel::Valid,
                  }
                      .setSelectedValue(SourceType::WebAddress)}
    , fileName_{"fileName", "HTML file", {}, "html", InvalidationLevel::Valid}
    , autoReloadFile_{"autoReloadFile", "Auto Reload", true, InvalidationLevel::Valid}
    , url_{"URL", "URL", "https://www.inviwo.org", InvalidationLevel::Valid}
    , reload_{"reload", "Reload", InvalidationLevel::Valid}
    , zoom_{"zoom", "Zoom Factor", 1.0, 0.2, 5.0}
    , runJS_{"runJS", "Run JS"}
    , js_{"js", "JavaScript", "", InvalidationLevel::Valid}
    , browser_{app, this} {

    addPorts(background_, outport_);
    background_.setOptional(true);
    addProperties(sourceType_, fileName_, autoReloadFile_, url_, reload_, zoom_, runJS_, js_);

    fileName_.visibilityDependsOn(sourceType_, [](auto& p) { return p == SourceType::LocalFile; });
    autoReloadFile_.visibilityDependsOn(sourceType_,
                                        [](auto& p) { return p == SourceType::LocalFile; });
    url_.visibilityDependsOn(sourceType_, [](auto& p) { return p == SourceType::WebAddress; });

    sourceType_.onChange([this]() { updateSource(); });
    fileName_.onChange([this]() {
        if (autoReloadFile_) {
            fileObserver_.setFilename(fileName_);
        }
        updateSource();
    });
    autoReloadFile_.onChange([this]() {
        if (autoReloadFile_) {
            fileObserver_.setFilename(fileName_);
        } else {
            fileObserver_.stop();
        }
    });

    url_.onChange([this]() { updateSource(); });
    reload_.onChange([this]() { updateSource(); });

    fileObserver_.onChange([this]() {
        if (sourceType_ == SourceType::LocalFile) {
            updateSource();
        }
    });

    addInteractionHandler(browser_.getInteractionHandler());
    updateSource();
}

void WebBrowserProcessor::process() {
    if (browser_.isLoading()) {
        return;
    }

    if (js_.isModified() && !js_.get().empty()) {
        browser_.executeJavaScript(js_, 1);
    }
    if (zoom_.isModified()) {
        browser_.setZoom(zoom_);
    }

    browser_.render(outport_, &background_);
}

void WebBrowserProcessor::deserialize(Deserializer& d) {
    Processor::deserialize(d);
    // Must reload page to connect property with Frame, see PropertyCefSynchronizer::OnLoadEnd
    updateSource();
}

void WebBrowserProcessor::updateSource() {
    switch (sourceType_) {
        case SourceType::LocalFile:
            browser_.setSource(fileName_);
            break;
        case SourceType::WebAddress:
            browser_.setSource(url_);
            break;
        default:
            browser_.setSource(std::string_view{"https://www.inviwo.org"});
            break;
    }
}

}  // namespace inviwo
