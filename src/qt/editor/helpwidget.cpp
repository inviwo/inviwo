/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2022 Inviwo Foundation
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

#include <inviwo/qt/editor/helpwidget.h>
#include <inviwo/qt/editor/processorpreview.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/common/inviwomodule.h>
#include <inviwo/core/util/fileobserver.h>
#include <modules/qtwidgets/inviwoqtutils.h>

#include <inviwo/core/util/docbuilder.h>
#include <inviwo/core/util/zip.h>
#include <inviwo/core/processors/processorfactory.h>
#include <inviwo/core/network/networkutils.h>

#include <fmt/format.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QFrame>
#include <QVBoxLayout>
#include <QTextBrowser>
#include <QHelpEngineCore>
#include <QEvent>
#include <QMap>
#include <QString>
#include <QUrl>
#include <QFile>
#include <QByteArray>
#include <QCoreApplication>
#include <QBuffer>
#include <QFileInfo>
#include <QImage>
#include <QUrlQuery>
#include <QTextBrowser>
#include <QTabWidget>
#include <QToolBar>
#include <QDesktopServices>
#include <QTimer>
#include <warn/pop>

namespace inviwo {

class HelpBrowser : public QTextBrowser {
public:
    HelpBrowser(HelpWidget* parent, InviwoApplication* app);
    virtual ~HelpBrowser() = default;

    void setCurrent(std::string_view processorClassIdentifier);

protected:
    QVariant loadResource(int type, const QUrl& name);

private:
    InviwoApplication* app_;

    QUrl current_;

    std::string currentModulePath_;
};

/*
 * Paths: qthelp://org.inviwo/doc/<files>
 *
 * This to do to generate info:
 * override css ( the default one is very large and make the text browser slow... )
 * Inviwo-dev\tools\doxygen\doc-qt\html> cp ..\..\style\qt-stylesheet.css .\doxygen.css
 *
 * generate qch (compressed help)
 * Inviwo-dev\tools\doxygen\doc-qt\html> qhelpgenerator.exe -o ..\inviwo.qch .\index.qhp
 *
 * Copy qch
 * Inviwo-dev\data\help> cp ..\..\tools\doxygen\doc-qt\inviwo.qch .
 *
 * Generate qhc (help collection)
 * Inviwo\Inviwo-dev\data\help> qcollectiongenerator.exe inviwo.qhcp -o inviwo.qhc
 */

HelpWidget::HelpWidget(InviwoMainWindow* mainWindow)
    : InviwoDockWidget(tr("Help"), mainWindow, "HelpWidget")
    , mainWindow_(mainWindow)
    , helpBrowser_(nullptr) {

    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    resize(utilqt::emToPx(this, QSizeF(60, 60)));  // default size

    auto* centralWidget = new QMainWindow();
    QToolBar* toolBar = new QToolBar();
    centralWidget->addToolBar(toolBar);
    toolBar->setFloatable(false);
    toolBar->setMovable(false);

    helpBrowser_ = new HelpBrowser(this, mainWindow_->getInviwoApplication());
    centralWidget->setCentralWidget(helpBrowser_);
    setWidget(centralWidget);

    {
        auto action = toolBar->addAction(QIcon(":/svgicons/link-left.svg"), tr("&Back"));
        action->setShortcut(QKeySequence::Back);
        action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        action->setToolTip("Back");
        centralWidget->addAction(action);
        connect(action, &QAction::triggered, this, [this]() { helpBrowser_->backward(); });

        connect(helpBrowser_, &QTextBrowser::backwardAvailable, action, &QAction::setEnabled);
    }
    {
        auto action = toolBar->addAction(QIcon(":/svgicons/link-right.svg"), tr("&Back"));
        action->setShortcut(QKeySequence::Back);
        action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        action->setToolTip("Forward");
        centralWidget->addAction(action);
        connect(action, &QAction::triggered, this, [this]() { helpBrowser_->forward(); });

        connect(helpBrowser_, &QTextBrowser::forwardAvailable, action, &QAction::setEnabled);
    }
    {
        auto action = toolBar->addAction(QIcon(":/svgicons/revert.svg"), tr("&Reload"));
        action->setShortcut(QKeySequence::Back);
        action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        action->setToolTip("Reload");
        centralWidget->addAction(action);
        connect(action, &QAction::triggered, this, [this]() { helpBrowser_->reload(); });
    }
}

HelpWidget::~HelpWidget() = default;

void HelpWidget::showDocForClassName(std::string_view classIdentifier) {
    helpBrowser_->setCurrent(classIdentifier);
}

void HelpWidget::resizeEvent(QResizeEvent* event) {
    helpBrowser_->reload();
    InviwoDockWidget::resizeEvent(event);
}

namespace {

constexpr std::string_view css = R"(
    h1,h2,h3 {
        color: #CdC9C5;
    }
    h4 {
        margin-bottom: 0px;
        color: #BdB9B5;
    }
    ul, ol {
        margin: 0px 0px 0px 0px;
    }
    li {
        margin: 7px 0px 0px -10px;
    }
    a {
        text-decoration: underline
    }
    a:link {
        color: #268BD2;
    }
    span.name {
        font-weight: 600;
        color: #CdC9C5;
    }
    span.id {
        font-style: italic;
        font-weight: 100;
    }
    div.help {
        margin: 5px 0px 5px 0px;
    }
    div.help p {
        margin: 0px 0px 5px 0px;
    }
    )";

template <typename T>
void link(T& factory, std::string_view id, Document::DocumentHandle& handle) {
    std::string base = "https://inviwo.org/inviwo/cpp-api/class";
    if (auto* fo = factory->getFactoryObject(id)) {
        auto& typeName = fo->getTypeName();
        std::string name{typeName.substr(0, typeName.find_first_of('<'))};
        std::string doxyName{name};
        replaceInString(doxyName, ":", "_1");
        handle.append("a", "", {{"href", base + doxyName}}).append("span", name, {{"class", "id"}});
    }
}

Document makeHtmlHelp(const help::HelpProcessor& processor, const ProcessorInfo& info,
                      const InviwoModule& module, InviwoApplication& app) {

    auto processorFactory = app.getProcessorFactory();
    auto propertyFactory = app.getPropertyFactory();
    auto inportFactory = app.getInportFactory();
    auto outportFactory = app.getOutportFactory();

    Document doc;

    auto html = doc.append("html");
    auto body = html.append("body");
    body.append("h3", fmt::format("{}", processor.displayName));

    auto tr = body.append("table").append("tr");
    tr.append("td").append("img", "",
                           {{"src", fmt::format("{0}.png?type=preview&classIdentifier={0}",
                                                processor.classIdentifier)}});
    auto td = tr.append("td");
    link(processorFactory, processor.classIdentifier, td);
    td.append("i", processor.classIdentifier);

    using P = Document::PathComponent;
    using H = utildoc::TableBuilder::Header;
    utildoc::TableBuilder tb(td, P::end());
    tb(H("Module"), module.getIdentifier());
    tb(H("Category"), info.category);
    tb(H("State"), info.codeState);
    tb(H("Tags"), info.tags);

    body.append("div", "", {{"class", "help"}}).append(processor.help);

    if (!processor.inports.empty()) {
        body.append("h4", "Inports");
        auto inports = body.append("ol", "", {{"class", "list"}});
        for (const auto& inport : processor.inports) {
            auto li = inports.append("li", "", {{"class", "item"}});
            li.append("span", inport.displayName, {{"class", "name"}});
            link(inportFactory, inport.classIdentifier, li);
            li.append("span", inport.classIdentifier, {{"class", "id"}});
            if (!inport.help.empty()) {
                li.append("div", "", {{"class", "help"}}).append(inport.help);
            }
        }
    }

    if (!processor.outports.empty()) {
        body.append("h4", "Outports");
        auto outports = body.append("ol", "", {{"class", "list"}});
        for (const auto& outport : processor.outports) {
            auto li = outports.append("li", "", {{"class", "item"}});
            li.append("span", outport.displayName, {{"class", "name"}});
            link(outportFactory, outport.classIdentifier, li);
            li.append("span", outport.classIdentifier, {{"class", "id"}});
            if (!outport.help.empty()) {
                li.append("div", "", {{"class", "help"}}).append(outport.help);
            }
        }
    }

    if (!processor.properties.empty()) {
        body.append("h4", "Properties");
        auto properties = body.append("ul", "", {{"class", "list"}});
        for (auto&& [idx, property] : util::enumerate(processor.properties)) {
            auto li = properties.append("li", "", {{"class", "item"}});
            if (!property.properties.empty()) {
                li.append("a", "",
                          {{"href", fmt::format("file:///{}/{}?type=processor",
                                                processor.classIdentifier, idx)}})
                    .append("span", property.displayName, {{"class", "name"}});
            } else {
                li.append("span", property.displayName, {{"class", "name"}});
            }
            link(propertyFactory, property.classIdentifier, li);
            li.append("span", property.classIdentifier, {{"class", "id"}});

            if (!property.help.empty()) {
                li.append("div", "", {{"class", "help"}}).append(property.help);
            }
        }
    }

    return doc;
}

Document makePropertyHelp(const help::HelpProperty& property, std::string_view path,
                          PropertyFactory* propertyFactory) {
    Document doc;

    auto html = doc.append("html");
    auto body = html.append("body");
    body.append("h3", property.displayName);
    link(propertyFactory, property.classIdentifier, body);
    body.append("div", property.classIdentifier);
    body.append("div", "", {{"class", "help"}}).append(property.help);

    if (!property.properties.empty()) {
        body.append("h4", "Properties");
        auto properties = body.append("ul", "", {{"class", "list"}});
        for (auto&& [idx, subProperty] : util::enumerate(property.properties)) {
            auto li = properties.append("li", "", {{"class", "item"}});
            if (!subProperty.properties.empty()) {
                li.append("a", "",
                          {{"href", fmt::format("file://{}/{}?type=processor", path, idx)}})
                    .append("span", subProperty.displayName, {{"class", "name"}});
            } else {
                li.append("span", subProperty.displayName, {{"class", "name"}});
            }
            link(propertyFactory, subProperty.classIdentifier, li);
            li.append("span", subProperty.classIdentifier, {{"class", "id"}});
            if (!subProperty.help.empty()) {
                li.append("div", "", {{"class", "help"}}).append(subProperty.help);
            }
        }
    }

    return doc;
}

InviwoModule& findModule(InviwoApplication& app, std::string_view processorClassIdentifier) {
    for (auto& module : app.getModules()) {
        for (auto& pfo : module->getProcessors()) {
            if (pfo->getClassIdentifier() == processorClassIdentifier) {
                return *module;
            }
        }
    }
    throw Exception(IVW_CONTEXT_CUSTOM("findModule"),
                    "ProcessorClassIdentifier {} is not registered", processorClassIdentifier);
}

std::tuple<std::string, std::string> loadIdUrl(const QUrl& url, InviwoApplication* app) {
    auto list = url.path().split('/');
    if (list.front().isEmpty()) list.pop_front();

    const auto processorClassIdentifier = utilqt::fromQString(list.front());
    try {
        if (auto processor = app->getProcessorFactory()->create(processorClassIdentifier, app)) {
            auto help = help::buildProcessorHelp(*processor);
            const auto& module = findModule(*app, processorClassIdentifier);

            if (list.length() == 1) {
                const auto helpText =
                    makeHtmlHelp(help, processor->getProcessorInfo(), module, *app);
                return {helpText, module.getPath()};
            } else {
                list.pop_front();
                auto* properties = &help.properties;
                help::HelpProperty* property = nullptr;
                for (const auto& elem : list) {
                    bool ok = true;
                    if (auto num = elem.toULongLong(&ok); ok && num < properties->size()) {
                        property = &(*properties)[num];
                        properties = &property->properties;
                    } else {
                        return {fmt::format("Could not create help for subproperty in: {}",
                                            processorClassIdentifier),
                                ""};
                    }
                }
                const auto helpText = makePropertyHelp(*property, utilqt::fromQString(url.path()),
                                                       app->getPropertyFactory());
                return {helpText, module.getPath()};
            }
        } else {
            return {fmt::format("Could not crate help for: {}", processorClassIdentifier), ""};
        }
    } catch (const Exception& e) {
        return {fmt::format("Could not crate help for: {}, {}", processorClassIdentifier,
                            e.getMessage()),
                ""};
    }
}

}  // namespace

HelpBrowser::HelpBrowser(HelpWidget* parent, InviwoApplication* app)
    : QTextBrowser(parent), app_(app) {
    setReadOnly(true);
    setUndoRedoEnabled(false);
    setContextMenuPolicy(Qt::NoContextMenu);
    setAcceptRichText(false);
    setOpenExternalLinks(true);

    setText("Select a processor in the processor list to see help");

    document()->setDefaultStyleSheet(utilqt::toQString(css));
}

void HelpBrowser::setCurrent(std::string_view processorClassIdentifier) {
    if (visibleRegion().isEmpty()) return;

    QUrl url;
    url.setScheme("file");
    url.setPath(utilqt::toQString(fmt::format("/{}", processorClassIdentifier)));
    url.setQuery(QUrlQuery({{"type", "processor"}}));
    setSource(url);
}

QVariant HelpBrowser::loadResource(int type, const QUrl& url) {
    const QUrlQuery query(url);

    auto toFilePath = [&](const QUrl& url) {
        auto filePath = utilqt::fromQString(url.path());
        replaceInString(filePath, "<modulePath>", currentModulePath_);
        replaceInString(filePath, "<basePath>", app_->getBasePath());
        return filePath;
    };

    if (query.hasQueryItem("type")) {
        const QString requestType = query.queryItemValue("type");

        if (requestType == "preview") {
            const QString cid = query.queryItemValue("classIdentifier");
            return utilqt::generatePreview(utilqt::fromQString(cid), app_->getProcessorFactory());
        } else if (requestType == "processor") {
            auto [html, mp] = loadIdUrl(url, app_);
            current_ = url;
            currentModulePath_ = mp;
            return utilqt::toQString(html);
        }
    }
    const auto filePath = toFilePath(url);
    if (filesystem::fileExists(filePath)) {
        if (filesystem::getFileExtension(filePath) == "inv") {
            try {
                util::appendProcessorNetwork(app_->getProcessorNetwork(), filePath, app_);
            } catch (const Exception& e) {
                util::log(
                    e.getContext(),
                    fmt::format("Unable to append network {} due to {}", filePath, e.getMessage()),
                    LogLevel::Error);
            }
            QTimer::singleShot(0, this, [this]() { backward(); });
            return {"Workspace loaded"};
        }
    }

    if (type == QTextDocument::ImageResource) {
        auto qFilePath = utilqt::toQString(filePath);
        if (QFile::exists(qFilePath)) {
            const auto maxImageWidth = (95 * width()) / 100;
            auto image = QImage(qFilePath);
            if (image.width() > maxImageWidth) {
                return image.scaledToWidth(maxImageWidth);
            } else {
                return image;
            }
        }
    }
    return {};
}

}  // namespace inviwo
