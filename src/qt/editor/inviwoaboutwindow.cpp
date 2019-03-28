/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
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
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/util/systemcapabilities.h>
#include <inviwo/core/common/inviwomodulefactoryobject.h>
#include <modules/qtwidgets/inviwoqtutils.h>
#include <inviwo/core/util/document.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/common/inviwomodule.h>
#include <modules/qtwidgets/inviwoqtutils.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QFrame>
#include <QVBoxLayout>
#include <QString>
#include <QTextBrowser>
#include <QByteArray>
#include <QBuffer>
#include <QImage>
#include <QUrl>
#include <QUrlQuery>
#include <warn/pop>

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
    resize(utilqt::emToPx(this, QSizeF(60, 60)));  // default size

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

    auto& syscap = app->getSystemCapabilities();
    auto buildYear = syscap.getBuildInfo().year;

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
            "a { color: #c8ccd0; font-weight: normal; text-decoration:none;}\n"
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
        cell.append("p", "Interactive Visualization Workshop. Version " + IVW_VERSION);
        cell.append("p", "&copy; 2012-" + toString(buildYear) + " The Inviwo Foundation");
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
                 "Peter Steneteg, Erik Sundén, Daniel Jönsson, Martin Falk, "
                 "Rickard Englund, Sathish Kottravel, Timo Ropinski");
    }
    {
        auto h = body.append("p");
        h.append("h3", "Contributors:");
        h.append("br");
        h.append("span",
                 "Robin Skånberg, Jochen Jankowai, Tino Weinkauf, "
                 "Wiebke Köpp, Anke Friederici, Dominik Engel, "
                 "Alexander Johansson, Andreas Valter, Johan Norén, Emanuel Winblad, "
                 "Hans-Christian Helltegen, Viktor Axelsson");
    }
    {
        auto h = body.append("p");
        h.append("h3", "Sponsors:");
        h.append("br");
        h.append("span",
                 "This work was supported by Linköping University, KTH Royal Institute of "
                 "Technology, Ulm University, and through grants from the Swedish e-Science "
                 "Research Centre (SeRC).");
        h.append("br");
        auto p = h.append("p");

        auto liu = p.append("a", "", {{"href", "http://www.liu.se"}});
        liu.append("img", "", makeImg(":/images/liu-white-crop.png", 50));

        auto kth = p.append("a", "", {{"href", "http://www.kth.se"}});
        kth.append("img", "", makeImg(":/images/kth.png", 50));

        auto serc = p.append("a", "", {{"href", "http://www.e-science.se"}});
        serc.append("img", "", makeImg(":/images/serc.png", 50));

        auto uulm = p.append("a", "", {{"href", "http://www.uni-ulm.de/en/"}});
        uulm.append("img", "", makeImg(":/images/uulm.png", 50));
    }
    {
        const auto& bi = syscap.getBuildInfo();
        auto h = body.append("p");
        h.append("h3", "Build Info: ");
        utildoc::TableBuilder tb(h, P::end());
        tb(H("Date"), bi.getDate());
        tb(H("Configuration"), bi.configuration);
        tb(H("Generator"), bi.generator);
        tb(H("Compiler"), bi.compiler + " " + bi.compilerVersion);
    }
    {
        auto h = body.append("p");
        h.append("h3", "Repos:");
        utildoc::TableBuilder tb(h, P::end());
        for (auto item : syscap.getBuildInfo().githashes) {
            tb(H(item.first), item.second);
        }
    }
    {
        auto h = body.append("p");
        h.append("h3", "Modules: ");
        h.append("br");
        const auto& mfos = app->getModuleManager().getModuleFactoryObjects();
        auto names = util::transform(
            mfos, [](const std::unique_ptr<InviwoModuleFactoryObject>& mfo) { return mfo->name; });
        std::sort(names.begin(), names.end());
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
        const auto& mfos = app->getModuleManager().getModuleFactoryObjects();
        for (auto& mfo : mfos) {
            for (auto& license : mfo->licenses) {
                QUrl url;
                url.setScheme("file");
                url.setHost("license.txt");
                QUrlQuery query;
                query.addQueryItem("module", utilqt::toQString(mfo->name));
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
                if (license.version != Version{0, 0, 0, 0}) {
                    dd += toString(license.version);
                    dd += " ";
                }
                dd += "(" + license.module + ")";
            }
        }
    }

    std::string str = doc;
    textdoc->setHtml(utilqt::toQString(str));

    auto showLicense = [str, textdoc, app, escape, makeBody](const QUrl& url) {
        if (url.hasQuery()) {
            QUrlQuery query(url);
            auto module = utilqt::fromQString(query.queryItemValue("module"));
            auto id = utilqt::fromQString(query.queryItemValue("id"));

            const auto& mfos = app->getModuleManager().getModuleFactoryObjects();
            auto mit = util::find_if(mfos, [&](auto& m) { return m->name == module; });
            if (mit != mfos.end()) {
                auto it = util::find_if((*mit)->licenses, [&](auto& l) { return l.id == id; });
                if (it != (*mit)->licenses.end()) {
                    Document doc;
                    auto body = makeBody(doc);
                    auto li = body.append("div");
                    li.append("p").append("a", "Back", {{"href", "file://home.txt"}});
                    li.append("br");
                    li.append("b", it->name);
                    if (it->version != Version{0, 0, 0, 0}) {
                        li += toString(it->version);
                    }
                    if (!it->url.empty()) {
                        li.append("a", it->url, {{"href", it->url}});
                    }
                    auto mod = app->getModuleByIdentifier(module);
                    if (!mod) return;
                    auto path = mod->getPath();
                    for (auto& file : it->files) {
                        auto licfile = path + "/" + file;
                        if (!filesystem::fileExists(licfile)) {
                            // Look for an installed file
                            licfile = path + "/licenses/" + it->id + "-" +
                                      filesystem::getFileNameWithExtension(file);
                        }

                        auto f = filesystem::ifstream(licfile);
                        std::stringstream buffer;
                        buffer << f.rdbuf();
                        li.append("pre", escape(buffer.str()), {{"style", "font: 12px;"}});
                    }
                    textdoc->setHtml(utilqt::toQString(doc));
                    return;
                }
            }
        }
        textdoc->setHtml(utilqt::toQString(str));
    };

    connect(textdoc, &QTextBrowser::anchorClicked, this, showLicense);
}

}  // namespace inviwo
