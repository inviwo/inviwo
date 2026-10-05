/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2026 Inviwo Foundation
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
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/common/inviwomodule.h>
#include <inviwo/core/common/modulemanager.h>
#include <inviwo/core/util/fileobserver.h>
#include <inviwo/core/ports/portfactory.h>
#include <inviwo/core/properties/propertyfactory.h>
#include <inviwo/core/util/docbuilder.h>
#include <inviwo/core/util/zip.h>
#include <inviwo/core/processors/processorfactory.h>
#include <inviwo/core/processors/processorutils.h>
#include <inviwo/core/network/networkutils.h>

#include <modules/qtwidgets/inviwoqtutils.h>

#include <inviwo/qt/editor/inviwomainwindow.h>
#include <inviwo/qt/editor/processorpreview.h>
#include <inviwo/qt/editor/welcomewidget.h>

#include <fmt/format.h>
#include <fmt/std.h>

#include <filesystem>

#include <warn/push>
#include <warn/ignore/all>
#include <QFrame>
#include <QVBoxLayout>
#include <QTextBrowser>
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

    void setCurrent(std::string_view processorCId);

    std::string_view currentProcessorCId() const;

protected:
    virtual QVariant loadResource(int type, const QUrl& name) override;

private:
    InviwoApplication* app_;
    std::string currentProcessorCId_;
    QUrl current_;
    std::filesystem::path currentModulePath_;
};

HelpWidget::HelpWidget(InviwoMainWindow* mainWindow)
    : InviwoDockWidget(tr("Help"), mainWindow, "HelpWidget")
    , mainWindow_(mainWindow)
    , helpBrowser_(nullptr) {

    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    resize(utilqt::emToPx(this, QSizeF(60, 60)));  // default size

    auto* centralWidget = new QMainWindow();
    QToolBar* toolBar = new QToolBar();
    toolBar->setObjectName("HelpWidgetToolBar");
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
        auto action = toolBar->addAction(QIcon(":/svgicons/link-right.svg"), tr("&Forward"));
        action->setShortcut(QKeySequence::Forward);
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

    {
        auto action = toolBar->addAction(QIcon(":/svgicons/network.svg"), tr("&Find networks"));
        action->setToolTip("Find networks with processor");
        centralWidget->addAction(action);
        connect(action, &QAction::triggered, this, [this]() {
            if (auto* ww = mainWindow_->getWelcomeWidget()) {
                ww->setFilterString(fmt::format("p: {}", helpBrowser_->currentProcessorCId()));
                mainWindow_->showWelcomeScreen();
            }
        });
    }
}

HelpWidget::~HelpWidget() = default;

void HelpWidget::showDocForClassName(std::string_view classIdentifier) {
    helpBrowser_->setCurrent(classIdentifier);
}

void HelpWidget::resizeEvent(QResizeEvent* event) {
    InviwoDockWidget::resizeEvent(event);
    QTimer::singleShot(200, this, [this]() {
        if (isVisible()) {
            helpBrowser_->reload();
        }
    });
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
        text-decoration: underline;
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
        font-weight: 600;
    }
    div.help {
        margin: 5px 0px 5px 0px;
    }
    div.help p {
        margin: 0px 0px 5px 0px;
    }
    )";

InviwoModule& findModule(InviwoApplication& app, std::string_view cid) {
    if (InviwoModule* m = util::getProcessorModule(cid, app)) {
        return *m;
    } else {
        throw Exception(SourceContext{}, "ProcessorClassIdentifier {} is not registered", cid);
    }
}

std::tuple<std::string, std::filesystem::path> loadIdUrl(const QUrl& url, InviwoApplication* app) {
    auto list = url.path().split('/');
    if (list.front().isEmpty()) list.pop_front();

    const auto processorCId = utilqt::fromQString(list.front());
    try {
        if (auto processor = app->getProcessorFactory()->createShared(processorCId, app)) {
            auto help = help::buildProcessorHelp(*processor, *app);
            const auto& inviwoModule = findModule(*app, processorCId);

            if (list.length() == 1) {
                const auto helpText = help::toDocument(help);
                return {helpText, inviwoModule.getPath()};
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
                                            processorCId),
                                ""};
                    }
                }
                const auto helpText = help::toDocument(*property, utilqt::fromQString(url.path()));
                return {helpText, inviwoModule.getPath()};
            }
        } else {
            return {fmt::format("Could not create help for: {}", processorCId), ""};
        }
    } catch (const Exception& e) {
        return {fmt::format("Could not create help for: {}, {}", processorCId, e.getMessage()), ""};
    }
}

}  // namespace

HelpBrowser::HelpBrowser(HelpWidget* parent, InviwoApplication* app)
    : QTextBrowser(parent), app_(app) {
    setReadOnly(true);
    setUndoRedoEnabled(false);
    setAcceptRichText(false);
    setOpenExternalLinks(true);

    setText("Select a processor in the processor list to see help");

    document()->setDefaultStyleSheet(utilqt::toQString(css));
}

void HelpBrowser::setCurrent(std::string_view processorCId) {
    currentProcessorCId_ = processorCId;

    if (visibleRegion().isEmpty()) return;

    QUrl url;
    url.setScheme("file");
    url.setPath(utilqt::toQString(fmt::format("/{}", processorCId)));
    url.setQuery(QUrlQuery({{"type", "processor"}}));
    setSource(url);
}

std::string_view HelpBrowser::currentProcessorCId() const { return currentProcessorCId_; }

QVariant HelpBrowser::loadResource(int type, const QUrl& resourceUrl) {
    std::string s = utilqt::fromQString(resourceUrl.toString(QUrl::None));
    replaceInString(s, "~modulePath~", currentModulePath_.generic_string());
    replaceInString(s, "~basePath~", filesystem::findBasePath().generic_string());
    const QUrl url(utilqt::toQString(s), QUrl::TolerantMode);

    const QUrlQuery query(url);
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

    const auto filePath = utilqt::toPath(url.toLocalFile());
    if (std::filesystem::is_regular_file(filePath)) {
        if (type == QTextDocument::ImageResource) {
            const auto maxImageWidth = (95 * width()) / 100;
            auto image = QImage(url.toLocalFile());
            if (image.width() > maxImageWidth) {
                return image.scaledToWidth(maxImageWidth);
            } else {
                return image;
            }
        } else if (filePath.extension() == ".inv") {
            try {
                util::appendProcessorNetwork(app_->getProcessorNetwork(), filePath, app_);
            } catch (const Exception& e) {
                log::exception(e, "Unable to append network {} due to {}", filePath,
                               e.getMessage());
            }
            QTimer::singleShot(0, this, [this]() { backward(); });
            return {"Workspace loaded"};
        } else {
            QDesktopServices::openUrl(url);
            QTimer::singleShot(0, this, [this]() { backward(); });
            return {"File opened"};
        }
    }

    return {};
}

}  // namespace inviwo
