/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024 Inviwo Foundation
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

#include <modules/webbrowser/processors/basicwebbrowser.h>

#include <inviwo/dataframe/jsondataframeconversion.h>
#include <inviwo/core/util/rendercontext.h>
#include <inviwo/core/properties/propertyfactory.h>

#include <inviwo/core/common/factoryutil.h>
#include <modules/webbrowser/webbrowserutil.h>

#include <include/wrapper/cef_stream_resource_handler.h>
#include <include/wrapper/cef_byte_read_handler.h>
#include <include/base/cef_ref_counted.h>
#include <nlohmann/json.hpp>
#include <fmt/format.h>

namespace inviwo {

namespace {

static constexpr std::string_view htmlUrl = "https://inviwo/app/static/page.html";
static constexpr std::string_view javascriptUrl = "https://inviwo/app/static/code.js";

template <typename T>
class RefData : public virtual CefBaseRefCounted {
public:
    RefData(T data) : data{std::move(data)} {}
    T data;

    IMPLEMENT_REFCOUNTING(RefData);
};

CefRefPtr<CefStreamResourceHandler> streamData(std::string_view string,
                                               const CefString& mime_type) {
    // Need to keep the data alive until the request is finished
    auto data = CefRefPtr<RefData<std::string>>(new RefData<std::string>(std::string{string}));
    auto handler = CefRefPtr<CefByteReadHandler>(new CefByteReadHandler(
        reinterpret_cast<const unsigned char*>(data->data.data()), data->data.size(), data));
    auto response = CefStreamReader::CreateForHandler(handler);
    return CefRefPtr<CefStreamResourceHandler>(new CefStreamResourceHandler(mime_type, response));
}

}  // namespace

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo BasicWebBrowser::processorInfo_{
    "org.inviwo.BasicWebBrowser",    // Class identifier
    "Basic Web Browser",             // Display name
    "Web",                           // Category
    CodeState::Stable,               // Code state
    Tags::CPU | Tag{"Web Browser"},  // Tags
    R"(Simple processor to render a html page given as a string property)"_unindentHelp};

const ProcessorInfo BasicWebBrowser::getProcessorInfo() const { return processorInfo_; }

BasicWebBrowser::BasicWebBrowser(InviwoApplication* app)
    : Processor{}
    , dataframe_{"dataframe", "Input DataFrame"_help}
    , json_{"json", "Input JSON"_help}
    , brushing_{"brushing", "Brushing and Linking"_help}
    , background_{"background", "Background image"_help}
    , outport_{"webpage", "Rendered web page"_help, DataVec4UInt8::get()}
    , html_{"html",
            "HTML",
            "The html to render"_help,
            "",
            InvalidationLevel::Valid,
            PropertySemantics{"HtmlEditor"}}
    , code_{"code",
            "Javascript",
            "Javascript code"_help,
            "",
            InvalidationLevel::Valid,
            PropertySemantics{"JavascriptEditor"}}
    , reload_("reload", "Reload", "Reload the webpage"_help, InvalidationLevel::Valid)
    , zoom_{"zoom", "Zoom Factor", 1.0, 0.2, 5.0, 0.1, InvalidationLevel::Valid}
    , propertyTypes_{"propertyTypes", "Property Type",
                     [&]() {
                         std::vector<OptionPropertyOption<std::string>> options;
                         for (auto key : util::getPropertyFactory(app)->getKeyView()) {
                             options.emplace_back(key);
                         }
                         return OptionPropertyState<std::string>{
                             .options = options, .invalidationLevel = InvalidationLevel::Valid};
                     }()}
    , name_{"name", "Property Name", "extraProperty"}
    , add_("add", "Add extra property",
           [this, app]() {
               LogInfo(propertyTypes_.getSelectedValue());
               auto prop = util::getPropertyFactory(app)->create(propertyTypes_.getSelectedValue());
               prop->setIdentifier(name_.get());
               prop->setDisplayName(name_.get());
               extra_.addProperty(std::move(prop));
           })
    , extra_{"extra", "Extra Properties"}
    , browser_{new WebBrowserBase{app, this, htmlUrl, [this]() { render(); },
                                  [this](bool isLoading) {
                                      if (isLoading) {
                                          notifyObserversStartBackgroundWork(this, 1);
                                      } else {
                                          notifyObserversFinishBackgroundWork(this, 1);
                                          invalidate(InvalidationLevel::InvalidOutput);
                                      }
                                      loaded_ = !isLoading;
                                  }}} {

    isReady_.setUpdate(
        [this, super = Processor::getDefaultIsReadyUpdater(this)]() -> ProcessorStatus {
            if (!error_.empty()) {
                return {ProcessorStatus::Error, error_};
            } else {
                return super();
            }
        });

    addPorts(dataframe_, json_, brushing_, background_, outport_);
    dataframe_.setOptional(true);
    json_.setOptional(true);
    background_.setOptional(true);

    addProperties(html_, code_, reload_, zoom_, propertyTypes_, name_, add_, extra_);

    auto reload = [this]() {
        error_.clear();
        isReady_.update();
        browser_->reload();
    };

    html_.onChange(reload);
    code_.onChange(reload);
    reload_.onChange(reload);

    browser_->setZoom(zoom_);
    zoom_.onChange([this]() { browser_->setZoom(zoom_); });

    addInteractionHandler(browser_->getInteractionHandler());

    browser_->addStaticHandler(
        [this](const std::string& url, scoped_refptr<CefResourceManager::Request> request) -> bool {
            if (url == htmlUrl) {
                request->Continue(streamData(html_.get(), "text/html"));
                return true;
            } else if (url == javascriptUrl) {
                request->Continue(streamData(code_.get(), "text/javascript"));
                return true;
            }
            return false;
        });

    callbacks_.push_back(
        browser_->registerCallback("self", [this](const std::string&) -> std::string {
            return fmt::format("\"{}\"", getIdentifier());
        }));

    browser_->addMessageHandler([this](cef_log_severity_t cefLevel, const CefString& message,
                                       const CefString& cefSource, int line) {
        std::string source = [&]() {
            if (cefSource.ToString() == htmlUrl) {
                return std::string{"Html"};
            } else if (cefSource.ToString() == javascriptUrl) {
                return std::string{"Javascript"};
            } else {
                return cefSource.ToString();
            }
        }();

        const auto level = cefutil::logLevel(cefLevel);
        LogCentral::getPtr()->log(getIdentifier(), level, LogAudience::Developer, source, "", line,
                                  message.ToString());

        if (level == LogLevel::Error) {
            error_ = fmt::format("{} Error ({}): {}", source, line, message.ToString());
            isReady_.update();
        }
    });
}

void to_json(json& j, const BrushingTarget& t) { j = t.getString(); }
void to_json(json& j, const BrushingModifications& m) { j = fmt::to_string(m); }

void BasicWebBrowser::process() {
    std::vector<std::string_view> changedInports;
    for (const Inport* p : getInports()) {
        if (p->isChanged()) {
            changedInports.push_back(p->getIdentifier());
        }
    }

    std::vector<std::string_view> changedProperties;
    for (const Property* p : extra_) {
        if (p->isModified()) {
            changedProperties.push_back(p->getIdentifier());
        }
    }

    json brushing = brushing_.getModified();

    browser_->executeJavaScript(fmt::format(
        R"(
        if (typeof globalThis.inviwoProcess === 'function') {{
            let self = new globalThis.inviwo.Processor({:?});
            self.changedInports = [{:?}];
            self.changedProperties = [{:?}];
            self.changedBrushing = {};
            self.loaded = {};
            globalThis.inviwoProcess(self);
        }})",
        getIdentifier(), fmt::join(changedInports, ", "), fmt::join(changedProperties, ", "),
        brushing.dump(), loaded_ ? true : false));

    loaded_ = false;
}

void BasicWebBrowser::render() {
    rendercontext::activateDefault();
    browser_->render(outport_, &background_);
    notifyObserversInvalidationBegin(this);
    outport_.invalidate(InvalidationLevel::InvalidOutput);
    outport_.setValid();  // Since we don't process this, we need to
                          // call setValid on the outport ourself.
    notifyObserversInvalidationEnd(this);
}

}  // namespace inviwo
