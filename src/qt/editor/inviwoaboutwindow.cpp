/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2026 Inviwo Foundation
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

#include <inviwo/qt/editor/inviwoaboutwindow.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/common/inviwocommondefines.h>
#include <inviwo/core/common/inviwomodule.h>
#include <inviwo/core/common/inviwomodulefactoryobject.h>
#include <inviwo/core/common/modulemanager.h>
#include <inviwo/core/util/buildinfo.h>
#include <inviwo/core/util/document.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/qt/editor/inviwomainwindow.h>
#include <modules/qtwidgets/inviwoqtutils.h>

#include <chrono>

#include <QFrame>
#include <QVBoxLayout>
#include <QString>
#include <QTextBrowser>
#include <QByteArray>
#include <QBuffer>
#include <QImage>
#include <QUrl>
#include <QUrlQuery>

namespace inviwo {

namespace {
class AboutBrowser : public QTextBrowser {
public:
    AboutBrowser(QWidget* parent) : QTextBrowser(parent) {}
    virtual ~AboutBrowser() = default;

protected:
    // Override this to circumvent a warning message from QTextBrowser::loadResource
    // QTextBrowser has an internal QTextDocument that has a loadResource function that does the
    // right thing, but first it calls the overload from it's parent QTextBrowers which fails with a
    // message. We overload that and just return an empty QVariant and let QTextDocument handle it.
    virtual QVariant loadResource(int, const QUrl&) override { return QVariant{}; }
};

}  // namespace

InviwoAboutWindow::InviwoAboutWindow(InviwoMainWindow* mainWindow)
    : InviwoDockWidget("About", mainWindow, "AboutWidget") {

    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    resize(utilqt::emToPx(this, QSizeF(120, 120)));  // default size

    auto centralWidget = new QWidget();
    auto vLayout = new QVBoxLayout(centralWidget);
    vLayout->setSpacing(utilqt::refSpacePx(this));
    vLayout->setContentsMargins(0, 0, 0, 0);

    auto* app = mainWindow->getInviwoApplication();

    auto* textDoc = new AboutBrowser(this);
    vLayout->addWidget(textDoc);
    centralWidget->setLayout(vLayout);
    setWidget(centralWidget);

    textDoc->setReadOnly(true);
    textDoc->setUndoRedoEnabled(false);
    textDoc->setAcceptRichText(false);
    textDoc->setOpenExternalLinks(true);

    const std::chrono::time_point now{std::chrono::system_clock::now()};
    const std::chrono::year_month_day ymd{std::chrono::floor<std::chrono::days>(now)};

    using P = Document::PathComponent;
    using H = utildoc::TableBuilder::Header;

    auto escape = [](const std::string& str) -> std::string {
        return utilqt::fromQString(utilqt::toQString(str).toHtmlEscaped());
    };

    auto makeImg = [](QString path, int size) {
        const auto img = QImage(path);
        QByteArray imgData;
        QBuffer buffer(&imgData);
        buffer.open(QIODevice::WriteOnly);
        const auto scaledImg = img.scaledToHeight(size, Qt::SmoothTransformation);
        scaledImg.save(&buffer, "PNG");
        return UnorderedStringMap<std::string>{
            {"width", std::to_string(scaledImg.size().width())},
            {"height", std::to_string(scaledImg.size().height())},
            {"src", "data:image/png;base64," + std::string(imgData.toBase64().data())}};
    };

    auto makeBody = [](Document& doc) {
        auto html = doc.append("html");
        html.append("head").append("style",
                                   "a {"
                                   "    color: #268BD2;"
                                   "    font-weight: normal;"
                                   "    text-decoration:none;"
                                   "    margin-top: 0px;"
                                   "    margin-right: 0px;"
                                   "    margin-bottom: 0px;"
                                   "    margin-left: 0px;"
                                   "}\n"
                                   "body {"
                                   "    background-color: #323235;"
                                   "    margin-top: 7px;"
                                   "    margin-right: 7px;"
                                   "    margin-bottom: 7px;"
                                   "    margin-left: 7px;"
                                   "}\n "
                                   "table, div, p, dl, span {"
                                   "    color: #9d9995;"
                                   "    background-color: #323235;"
                                   "    font: 400 14px/18px Calibra, sans-serif;"
                                   "    margin-top: 3px;"
                                   "    margin-right: 0px;"
                                   "    margin-bottom: 3px;"
                                   "    margin-left: 0px;"
                                   "}\n "
                                   "h1, h2, h3, h4 {"
                                   "    color: #c8ccd0;"
                                   "    margin-top: 14px;"
                                   "    margin-right: 0px;"
                                   "    margin-bottom: 3px;"
                                   "    margin-left: 0px;"
                                   "}\n");
        return html.append("body");
    };

    Document doc;
    auto body = makeBody(doc);
    {
        auto table = body.append("table").append("tr");

        table.append("td", "", {{"style", "padding: 7px;"}})
            .append("img", "", makeImg(":/inviwo/inviwo-logo-light-600px.png", 128));

        auto cell = table.append("td");
        cell.append("h1", "Inviwo", {{"style", "color:white;"}});
        cell.append(
            "p", "Interactive Visualization Workshop. Version " + fmt::to_string(build::version));
        cell.append("p", "&copy; 2012-" + toString(static_cast<int>(ymd.year())) +
                             " The Inviwo Foundation");
        cell.append("a", "http://www.inviwo.org", {{"href", "http://www.inviwo.org"}});
        cell.append("p",
                    "Inviwo is a rapid prototyping environment for interactive "
                    "visualizations. It is licensed under the Simplified BSD license.");
    }
    {
        auto h = body.append("p");
        h.append("h3", "Core Team:");
        h.append("div", "Peter Steneteg, Martin Falk");
    }
    {
        auto h = body.append("p");
        h.append("h3", "Contributors:");

        h.append("div",
                 "Daniel Jönsson, Rickard Englund, Timo Ropinski, "
                 "Anke Friederici, Julian Kreiser, Dominik Engel, "
                 "Erik Sundén, Sathish Kottravel, Robin Skånberg, Jochen Jankowai, Tino Weinkauf, "
                 "Wiebke Köpp, Alexander Johansson, Andreas Valter, Johan Norén, Emanuel Winblad, "
                 "Hans-Christian Helltegen, Viktor Axelsson");
    }
    {
        auto h = body.append("p");
        h.append("h3", "Sponsors:");
        h.append("div",
                 "This work is supported by Linköping University, Ulm University, and KTH Royal "
                 "Institute of Technology as well as grants from the Swedish e-Science Research "
                 "Centre (SeRC), the Excellence Center at Linköping - Lund in Information "
                 "Technology (ELLIIT), the Swedish Research Council (Vetenskapsrådet), DFG (German "
                 "Research Foundation), and the BMBF.");
        h.append("br");
        auto p = h.append("p", "");
        for (auto&& [link, img] : std::to_array<std::pair<std::string_view, QString>>(
                 {{"https://www.liu.se", ":/images/liu-white-crop.svg"},
                  {"https://www.uni-ulm.de/en/", ":/images/uulm.svg"},
                  {"https://www.kth.se", ":/images/kth.svg"},
                  {"https://www.e-science.se", ":/images/serc.svg"},
                  {"https://old.liu.se/elliit?l=en", ":/images/elliit.svg"},
                  {"https://www.vr.se/english.html", ":images/vr.svg"},
                  {"https://www.dfg.de/en/index.jsp", ":images/dfg.svg"},
                  {"https://www.bmbf.de", ":images/bmbf-white.svg"}})) {
            auto l = p.append("a", "", {{"href", std::string{link}}});
            l.append("img", "", makeImg(img, 60));
            p.appendText("&nbsp;&nbsp;  ");
        }
    }

    auto h1 = body.append("div");
    h1.append("h3", "Build Info:");
    utildoc::TableBuilder tb1(h1, P::end());
    tb1(H("Configuration"), build::configuration);
    tb1(H("Generator"), build::generator);
    tb1(H("Compiler"), build::compiler);
    tb1(H("Compiler Version"), build::compilerVersion);

    if (auto bi = util::getBuildInfo()) {
        tb1(H("Date"), bi->getDate());
        StringMap<std::vector<util::BuildInfo::ModulesDir>> modulesDirsByRepo;
        for (auto& modulesDir : bi->modulesDirs) {
            modulesDirsByRepo[modulesDir.repo].push_back(modulesDir);
        }
        auto d0 = body.append("div");
        d0.append("h3", "Repos:");
        for (auto& [repo, modulesDirs] : modulesDirsByRepo) {
            const auto repoLink = repo.ends_with(".git") ? repo.substr(0, repo.size() - 4) : repo;
            const auto link = repoLink.contains("github.com")
                                  ? repoLink + "/tree/" + modulesDirs.front().sha
                                  : repoLink;
            auto mdiv = d0.append("div");
            mdiv.append("b", repoLink.substr(repoLink.find_last_of('/') + 1));
            mdiv.appendText(" ");
            mdiv.append("a", repoLink, {{"href", link}});
            mdiv.appendText(" ");
            mdiv.append("code", modulesDirs.front().sha);
            mdiv.appendText(modulesDirs.front().dirty ? " (dirty)" : "");

            auto list = d0.append("dl", "",  {{"style", "margin-bottom:10px;"}} );
            for (auto modulesDir : modulesDirs) {
                auto modules = app->getModuleManager().getFactoryObjects() |
                               std::views::filter([&](const InviwoModuleFactoryObject& mfo) {
                                   return mfo.srcPath.generic_string().starts_with(
                                       modulesDir.dir.generic_string());
                               }) |
                               std::views::transform(
                                   [](const InviwoModuleFactoryObject& m) { return m.name; }) |
                               std::ranges::to<std::vector>();
                if (modules.empty()) continue;
                std::ranges::sort(modules);
                list.append("dt", modulesDir.name);
                list.append("dd", fmt::format("{}", fmt::join(modules, ", ")));
            }
        }
    } else {
        auto h = body.append("p");
        h.append("h3", "Modules: ");
        std::vector<std::string> names;
        std::ranges::copy(
            app->getModuleManager().getFactoryObjects() |
                std::views::transform([](const InviwoModuleFactoryObject& m) { return m.name; }),
            std::back_inserter(names));

        std::ranges::sort(names);

        h.append("div", joinString(names, ", "));
    }
    {
        auto h = body.append("div");
        h.append("h3", "Qt Version:");
        h.append("div", QT_VERSION_STR);
    }
    {
        auto h = body.append("p");
        h.append("h3", "Libraries: ");
        auto dl = h.append("dl");
        std::map<std::string, LicenseInfo> licenses;
        for (const auto& mfo : app->getModuleManager().getFactoryObjects()) {
            for (auto& license : mfo.licenses) {
                licenses.try_emplace(license.id, license);
            }
        }
        for (auto&& [id, license] : licenses) {
            QUrl url;
            url.setScheme("file");
            url.setHost("license.txt");
            QUrlQuery query;
            query.addQueryItem("module", utilqt::toQString(license.module));
            query.addQueryItem("id", utilqt::toQString(license.id));
            url.setQuery(query);
            auto dt = dl.append("dt");
            dt.append("b", license.name);
            dt += " ";
            dt.append("a", license.type, {{"href", utilqt::fromQString(url.url())}});
            auto dd = dl.append("dd", "", {{"style", "margin-bottom:10px;"}});
            if (!license.url.empty()) {
                dd.append("a", license.url, {{"href", license.url}});
                dd += " ";
            }
            if (!license.version.empty()) {
                dd += license.version;
                dd += " ";
            }
        }
    }

    std::string str = doc;
    log::info("{}", str);
    textDoc->setHtml(utilqt::toQString(str));

    auto showLicense = [str, textDoc, app, escape, makeBody](const QUrl& url) {
        if (url.hasQuery()) {
            QUrlQuery query(url);
            auto moduleName = utilqt::fromQString(query.queryItemValue("module"));
            auto licenseId = utilqt::fromQString(query.queryItemValue("id"));

            auto mfo = app->getModuleManager().getFactoryObject(moduleName);
            if (!mfo) return;

            auto& licenses = mfo->licenses;

            auto lit = util::find_if(licenses, [&](auto& l) { return l.id == licenseId; });
            if (lit == licenses.end()) return;

            auto& license = *lit;

            Document doc;
            auto body = makeBody(doc);
            auto li = body.append("div");
            li.append("p").append("a", "Back", {{"href", "file://home.txt"}});
            li.append("br");
            li.append("b", license.name);
            if (!license.version.empty()) {
                li += license.version;
            }
            if (!license.url.empty()) {
                li.append("a", license.url, {{"href", license.url}});
            }

            auto mod = app->getModuleByIdentifier(moduleName);
            if (!mod) return;
            auto modulePath = mod->getPath();
            for (auto& file : license.files) {

                const auto defaultLicensePath = modulePath / "licenses" / file;
                const auto fallbackLicensePath = std::filesystem::path{build::binaryDirectory} /
                                                 "modules" / toLower(moduleName) / "licenses" /
                                                 file;

                std::stringstream buffer;
                if (std::filesystem::is_regular_file(defaultLicensePath)) {
                    auto f = std::ifstream(defaultLicensePath);
                    buffer << f.rdbuf();
                } else if (std::filesystem::is_regular_file(fallbackLicensePath)) {
                    auto f = std::ifstream(fallbackLicensePath);
                    buffer << f.rdbuf();
                } else {
                    buffer << "License file not found.";
                }

                li.append("pre", escape(buffer.str()), {{"style", "font: 12px;"}});
            }
            textDoc->setHtml(utilqt::toQString(doc));
            return;
        }
        textDoc->setHtml(utilqt::toQString(str));
    };

    connect(textDoc, &QTextBrowser::anchorClicked, this, showLicense);
}

}  // namespace inviwo
