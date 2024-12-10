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

#include <modules/webbrowser/processors/basicwebbrowser.h>

#include <inviwo/dataframe/jsondataframeconversion.h>
#include <inviwo/core/util/rendercontext.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/properties/propertyfactory.h>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/common/factoryutil.h>
#include <inviwo/core/util/moduleutils.h>
#include <modules/webbrowser/webbrowserutil.h>

#include <include/wrapper/cef_stream_resource_handler.h>
#include <include/wrapper/cef_byte_read_handler.h>
#include <include/base/cef_ref_counted.h>

#include <modules/json/jsonmodule.h>
#include <modules/json/jsonpropertyconverter.h>

#include <nlohmann/json.hpp>
#include <fmt/format.h>

namespace inviwo {

namespace {

constexpr std::string_view htmlUrl = "https://inviwo/app/static/page.html";
constexpr std::string_view javascriptUrl = "https://inviwo/app/static/code.js";

constexpr std::string_view defaultHtml = R"(<!DOCTYPE html>
<!DOCTYPE html>
<html>
  <head>
    <meta charset="utf-8">
    
    <!--
    * "https://inviwo/yourmodulename/" will be redirected to the
        corresponding module directory.
    * "https://inviwo/app/" will be redirected to the application
        base path (InviwoApplication::getBasePath()) directory
    * "https://inviwo/app/static/code.js" resolves to the JavaScript 
        property of this processor
    -->

    <style>
      body {
        background: white;
      }
      h1 {
        font-family: sans-serif;
      }
    </style>
  </head>

  <body>
    <h1> Hello world</h1>
    <!-- Create a div where to put dynamic content --> 
    <div id="data"></div>
  </body>

  <script type="module">
    // load the javascript code associated with the processor
    import * as code from "https://inviwo/app/static/code.js"
  </script>
</html>
)";

constexpr std::string_view defaultJS = R"(// Load the inviwo javascript api
import * as inviwo from "https://inviwo/webbrowser/data/js/inviwoapiv2.js"

// Put the inviwo module into global scope so that the API can interact with it from C++
globalThis.inviwo = inviwo

// Register a function for inviwo to call on processor process().
// The function will be called with an instance of the processor. 
// The processor has the following properties:
//  * `changedInports` A list of the identifiers for all the ports that have changed
//  * `changedProperties` A list of the identifiers for all the identifiers that have changed
//  * `self.changedBrushing` A list of the modified brushing targets that have changed
//  * `loaded` A boolean specifying whether the webpage was just loaded
// Then there are the standard processor functions
//  * properties()
//  * property(identifier)
//  * inports()
//  * inport(identifier)
//  * outports()
//  * outport(identifier)  
globalThis.inviwoProcess = async function (self) {
    console.log("Process")
    try {
        const data = globalThis.document.getElementById("data");
        data.replaceChildren();

        if (self.changedInports.includes("dataframe") || self.loaded) {   
            const json = await self.inport("dataframe").getData();
            data.appendChild(globalThis.document.createTextNode(JSON.stringify(json)));
        }
        if (self.changedBrushing.map((x) => x[1]).includes("Highlighted") || self.loaded) {
            const rows = await self.inport("brushing").getHighlightedIndices("row");
            const cols = await self.inport("brushing").getHighlightedIndices("column");

            data.appendChild(globalThis.document.createTextNode(
                "rows: " +JSON.stringify(rows)+ " cols: " + JSON.stringify(cols))
            );
        }

        // properties can be accessed via `self.properties`, for example:
        // const color = await self.property("extra").property("color").get(); 
        
    } catch(e) {
        if (e instanceof Error) {
            console.error(e.toString())
        } else {
            console.error(JSON.stringify(e))
        }
    }
}

)";

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

OptionPropertyState<std::string> convertableProperties(InviwoApplication* app) {
    const auto& conv = util::getModuleByTypeOrThrow<JSONModule>(app).getJSONPropertyConverter();

    std::vector<OptionPropertyOption<std::string>> options;
    for (auto key : conv.getKeyView()) {
        if (key.starts_with("org.inviwo.")) {
            options.emplace_back(key, camelCaseToHeader(key.substr(11)), key);
        } else {
            options.emplace_back(key, camelCaseToHeader(key), key);
        }
        std::ranges::sort(options, std::less<>{}, &OptionPropertyOption<std::string>::name_);
    }
    return OptionPropertyState<std::string>{.options = options,
                                            .invalidationLevel = InvalidationLevel::Valid};
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

const ProcessorInfo& BasicWebBrowser::getProcessorInfo() const { return processorInfo_; }

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
            defaultHtml,
            InvalidationLevel::Valid,
            PropertySemantics{"HtmlEditor"}}
    , code_{"code",
            "Javascript",
            "Javascript code"_help,
            defaultJS,
            InvalidationLevel::Valid,
            PropertySemantics{"JavascriptEditor"}}
    , reload_("reload", "Reload", "Reload the webpage"_help, InvalidationLevel::Valid)
    , zoom_{"zoom", "Zoom Factor", 1.0, 0.2, 5.0, 0.1, InvalidationLevel::Valid}
    , propertyTypes_{"propertyTypes", "Property Type", convertableProperties(app)}
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
    , browser_{new WebBrowserBase{app, *this, outport_, &background_, htmlUrl,
                                  [this]() { render(); },
                                  [this](bool isLoading) {
                                      loaded_ = !isLoading;
                                      if (loaded_) invalidate(InvalidationLevel::InvalidOutput);
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
    notifyObserversInvalidationBegin(this);
    outport_.invalidate(InvalidationLevel::InvalidOutput);
    outport_.setValid();  // Since we don't process this, we need to
                          // call setValid on the outport ourself.
    notifyObserversInvalidationEnd(this);
}

}  // namespace inviwo
