/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2023 Inviwo Foundation
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
#include <inviwo/qt/editor/inviwomainwindow.h>
#include <inviwo/core/inviwocommondefines.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/util/buildinfo.h>
#include <inviwo/core/common/inviwomodulefactoryobject.h>
#include <modules/qtwidgets/inviwoqtutils.h>
#include <inviwo/core/util/document.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/common/inviwomodule.h>
#include <modules/qtwidgets/inviwoqtutils.h>
#include <inviwo/core/inviwocommondefines.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/common/modulemanager.h>

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

InviwoAboutWindow::InviwoAboutWindow(InviwoMainWindow* mainwindow)
    : InviwoDockWidget("About", mainwindow, "AboutWidget") {

    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    resize(utilqt::emToPx(this, QSizeF(120, 120)));  // default size

    auto centralWidget = new QWidget();
    auto vLayout = new QVBoxLayout(centralWidget);
    vLayout->setSpacing(utilqt::refSpacePx(this));
    vLayout->setContentsMargins(0, 0, 0, 0);

    auto app = mainwindow->getInviwoApplication();

    auto textdoc = new AboutBrowser(this);
    vLayout->addWidget(textdoc);
    centralWidget->setLayout(vLayout);
    setWidget(centralWidget);

    textdoc->setReadOnly(true);
    textdoc->setUndoRedoEnabled(false);
    textdoc->setAcceptRichText(false);
    textdoc->setOpenExternalLinks(true);

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
        return std::unordered_map<std::string, std::string>{
            {"width", std::to_string(scaledImg.size().width())},
            {"height", std::to_string(scaledImg.size().height())},
            {"src", "data:image/png;base64," + std::string(imgData.toBase64().data())}};
    };

    auto makeBody = [](Document& doc) {
        auto html = doc.append("html");
        html.append("head").append(
            "style",
            "a { color: #268BD2; font-weight: normal; text-decoration:none;}\n"
            "body, table, div, p, dl "
            "{color: #9d9995; background-color: #323235; font: 400 14px/18px"
            "Calibra, sans-serif;}\n "
            "h1, h2, h3, h4 {color: #c8ccd0; margin-bottom:3px;}");
        return html.append("body", "", {{"style", "margin: 7px;"}});
    };

    Document doc;
    auto body = makeBody(doc);
    {
        auto table = body.append("table").append("tr");

        table.append("td").append("img", "", makeImg(":/inviwo/inviwo-logo-light-600px.png", 128));

        auto cell = table.append("td");
        cell.append("h1", "Inviwo", {{"style", "color:white;"}});
        cell.append("p", "Interactive Visualization Workshop. Version " + toString(build::version));
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
        h.append("br");
        h.append("span",
                 "Peter Steneteg, Martin Falk, Daniel Jönsson, Rickard Englund, Timo Ropinski, "
                 "Anke Friederici, Julian Kreiser, Dominik Engel");
    }
    {
        auto h = body.append("p");
        h.append("h3", "Contributors:");
        h.append("br");

        h.append("span",
                 "Erik Sundén, Sathish Kottravel, Robin Skånberg, Jochen Jankowai, Tino Weinkauf, "
                 "Wiebke Köpp, Alexander Johansson, Andreas Valter, Johan Norén, Emanuel Winblad, "
                 "Hans-Christian Helltegen, Viktor Axelsson");
    }
    {
        auto h = body.append("p");
        h.append("h3", "Sponsors:");
        h.append("br");
        h.append("span",
                 "This work is supported by Linköping University, Ulm University, and KTH Royal "
                 "Institute of Technology as well as grants from the Swedish e-Science Research "
                 "Centre (SeRC), the Excellence Center at Linköping - Lund in Information "
                 "Technology (ELLIIT), the Swedish Research Council (Vetenskapsrådet), DFG (German "
                 "Research Foundation), and the BMBF.");
        h.append("br");
        auto p = h.append("p");

        auto liu = p.append("a", "", {{"href", "https://www.liu.se"}});
        liu.append("img", "", makeImg(":/images/liu-white-crop.svg", 60));

        auto uulm = p.append("a", "", {{"href", "https://www.uni-ulm.de/en/"}});
        uulm.append("img", "", makeImg(":/images/uulm.svg", 60));

        auto kth = p.append("a", "", {{"href", "https://www.kth.se"}});
        kth.append("img", "", makeImg(":/images/kth.svg", 60));

        auto serc = p.append("a", "", {{"href", "https://www.e-science.se"}});
        serc.append("img", "", makeImg(":/images/serc.svg", 60));

        auto elliit = p.append("a", "", {{"href", "https://old.liu.se/elliit?l=en"}});
        elliit.append("img", "", makeImg(":/images/elliit.svg", 60));

        auto vr = p.append("a", "", {{"href", "https://www.vr.se/english.html"}});
        vr.append("img", "", makeImg(":images/vr.svg", 60));

        auto dfg = p.append("a", "", {{"href", "https://www.dfg.de/en/index.jsp"}});
        dfg.append("img", "", makeImg(":images/dfg.svg", 60));

        auto bmbf = p.append("a", "", {{"href", "https://www.bmbf.de"}});
        bmbf.append("img", "", makeImg(":images/bmbf-white.svg", 60));
    }
    if (auto bi = util::getBuildInfo()) {
        auto h1 = body.append("p");
        h1.append("h3", "Build Info: ");
        utildoc::TableBuilder tb1(h1, P::end());
        tb1(H("Date"), bi->getDate());
        tb1(H("Configuration"), bi->configuration);
        tb1(H("Generator"), bi->generator);
        tb1(H("Compiler"), bi->compiler + " " + bi->compilerVersion);

        auto h2 = body.append("p");
        h2.append("h3", "Repos:");
        utildoc::TableBuilder tb2(h2, P::end());
        for (auto item : bi->githashes) {
            tb2(H(item.first), item.second);
        }
    }
    {
        auto h = body.append("p");
        h.append("h3", "Modules: ");
        h.append("br");
        std::vector<std::string> names;
        std::ranges::copy(
            app->getModuleManager().getFactoryObjects() |
                std::views::transform([](const InviwoModuleFactoryObject& m) { return m.name; }),
            std::back_inserter(names));

        std::ranges::sort(names);

        h.append("span", joinString(names, ", "));
    }
    {
        auto h = body.append("div");
        h.append("h3", "Qt Version: ");
        h.append("span", QT_VERSION_STR);
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
    textdoc->setHtml(utilqt::toQString(str));

    auto showLicense = [str, textdoc, app, escape, makeBody](const QUrl& url) {
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
            textdoc->setHtml(utilqt::toQString(doc));
            return;
        }
        textdoc->setHtml(utilqt::toQString(str));
    };

    connect(textdoc, &QTextBrowser::anchorClicked, this, showLicense);
}

}  // namespace inviwo
