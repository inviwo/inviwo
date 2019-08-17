/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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
#include <warn/pop>

namespace inviwo {

class HelpBrowser : public QTextBrowser {
public:
    HelpBrowser(HelpWidget* parent, QHelpEngineCore* helpEngine);
    virtual ~HelpBrowser() = default;

protected:
    QVariant loadResource(int type, const QUrl& name);

private:
    QHelpEngineCore* helpEngine_;
};

class QCHFileObserver : public FileObserver {
public:
    QCHFileObserver(QHelpEngineCore* engine) : engine_(engine) {}
    virtual ~QCHFileObserver() = default;

    void addFile(const std::string& fileName) {
        reload(fileName);
        if (!isObserved(fileName)) {
            startFileObservation(fileName);
        }
    }
    void removeAll() {
        stopAllObservation();
        for (const auto& ns : engine_->registeredDocumentations()) {
            engine_->unregisterDocumentation(ns);
        }
    }
    virtual void fileChanged(const std::string& fileName) override { reload(fileName); }

private:
    void reload(const std::string& fileName) {
        const auto file = QString::fromStdString(fileName);
        const auto ns = QHelpEngineCore::namespaceName(file);

        engine_->unregisterDocumentation(ns);

        if (!engine_->registerDocumentation(file)) {
            LogWarn("Problem loading help file : " << fileName << " "
                                                   << engine_->error().toStdString());
        }
    }
    QHelpEngineCore* engine_;
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

HelpWidget::HelpWidget(InviwoMainWindow* mainwindow)
    : InviwoDockWidget(tr("Help"), mainwindow, "HelpWidget")
    , mainwindow_(mainwindow)
    , helpEngine_(nullptr)
    , helpBrowser_(nullptr) {

    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    resize(utilqt::emToPx(this, QSizeF(60, 60)));  // default size

    QWidget* centralWidget = new QWidget();
    QVBoxLayout* vLayout = new QVBoxLayout(centralWidget);
    vLayout->setSpacing(utilqt::refSpacePx(this));
    vLayout->setContentsMargins(0, 0, 0, 0);

    auto app = mainwindow->getInviwoApplication();

    // The help engine needs a file backed db "inviwo.qhc", this file is created on demand in the
    // settings folder, we will create the folder if it does not exists
    const std::string helpfile = app->getPath(PathType::Settings, "/inviwo.qhc", true);

    helpEngine_ = new QHelpEngineCore(QString::fromStdString(helpfile), this);
    // Any old data will be left in the since last time, we want to clear that so we can load the
    // new qch files.
    for (const auto& ns : helpEngine_->registeredDocumentations()) {
        helpEngine_->unregisterDocumentation(ns);
    }

    helpBrowser_ = new HelpBrowser(this, helpEngine_);
    helpBrowser_->setHtml(QString("Hello world"));
    vLayout->addWidget(helpBrowser_);
    centralWidget->setLayout(vLayout);
    setWidget(centralWidget);

    connect(helpEngine_, &QHelpEngineCore::setupFinished, this, [&]() {
        if (!helpEngine_) return;
        if (current_.empty()) {
            helpBrowser_->setText("Select a processor in the processor list to see help");
        } else {
            showDocForClassName(current_);
        }
    });

    if (!helpEngine_->setupData()) {
        const std::string error{helpEngine_->error().toUtf8().constData()};
        delete helpEngine_;
        throw Exception("Failed to setup the help engine:" + error);
    }

    fileObserver_ = std::make_unique<QCHFileObserver>(helpEngine_);

    onModulesDidRegister_ =
        app->getModuleManager().onModulesDidRegister([this]() { registerQCHFiles(); });
    onModulesWillUnregister_ =
        app->getModuleManager().onModulesWillUnregister([this]() { fileObserver_->removeAll(); });

    connect(this, &HelpWidget::visibilityChanged, this, &HelpWidget::updateDoc);
}

HelpWidget::~HelpWidget() = default;

void HelpWidget::registerQCHFiles() {
    auto app = mainwindow_->getInviwoApplication();
    for (const auto& module : app->getModules()) {
        const auto moduleQchFile =
            module->getPath(ModulePath::Docs) + "/" + module->getIdentifier() + ".qch";
        if (filesystem::fileExists(moduleQchFile)) {
            fileObserver_->addFile(moduleQchFile);
        }
    }
}

void HelpWidget::showDocForClassName(std::string classIdentifier) {
    if (!helpEngine_) return;
    requested_ = classIdentifier;
    updateDoc();
}

void HelpWidget::resizeEvent(QResizeEvent* event) {
    helpBrowser_->reload();
    InviwoDockWidget::resizeEvent(event);
}

void HelpWidget::updateDoc() {
    if (current_ == requested_) return;

    if (visibleRegion().isEmpty()) return;
    current_ = requested_;

    const QString path("qthelp://org.inviwo.base/doc/docpage-%1.html");
    QUrl foundUrl = helpEngine_->findFile(QUrl(path.arg(QString::fromStdString(requested_))));

    if (foundUrl.isValid() && !helpEngine_->fileData(foundUrl).isEmpty()) {
        auto txt = foundUrl.toString();
        helpBrowser_->setSource(foundUrl);
        return;
    }

    std::string classIdentifier = requested_;
    replaceInString(classIdentifier, ".", "_8");
    foundUrl = helpEngine_->findFile(QUrl(path.arg(QString::fromStdString(classIdentifier))));
    if (foundUrl.isValid() && !helpEngine_->fileData(foundUrl).isEmpty()) {
        auto txt = foundUrl.toString();
        helpBrowser_->setSource(foundUrl);
        return;
    }

    helpBrowser_->setText(QString::fromStdString("No documentation available for: " + requested_));
}

HelpBrowser::HelpBrowser(HelpWidget* parent, QHelpEngineCore* helpEngine)
    : QTextBrowser(parent), helpEngine_(helpEngine) {
    setReadOnly(true);
    setUndoRedoEnabled(false);
    setContextMenuPolicy(Qt::NoContextMenu);
    setAcceptRichText(false);
}

QVariant HelpBrowser::loadResource(int type, const QUrl& name) {
    QUrl url(name);
    if (name.isRelative()) url = source().resolved(url);

    QUrlQuery query(url);
    if (query.hasQueryItem("classIdentifier")) {
        QString cid = query.queryItemValue("classIdentifier");

        auto img = utilqt::generatePreview(cid);
        if (img.isNull()) return QVariant();
        QByteArray imgData;
        QBuffer buffer(&imgData);
        buffer.open(QIODevice::WriteOnly);
        img.save(&buffer, "PNG");

        return imgData;
    }

    QByteArray fileData = helpEngine_->fileData(url);
    switch (type) {
        case QTextDocument::HtmlResource:
            return fileData;
        case QTextDocument::StyleSheetResource:
            return fileData;
        case QTextDocument::ImageResource: {
            auto image =
                QImage::fromData(fileData, QFileInfo(url.path()).suffix().toLatin1().data());
            QImage resized{
                image.scaled(std::max(200, width() - 60), image.height(), Qt::KeepAspectRatio)};

            if (name.toString().contains("form_")) {

                for (int y = 0; y < resized.height(); y++) {
                    for (int x = 0; x < resized.width(); x++) {
                        auto p = resized.pixelColor(x, y);
                        if (p.alpha() > 0) {
                            p.setRed(0x9d);
                            p.setGreen(0x99);
                            p.setBlue(0x95);
                            resized.setPixelColor(x, y, p);
                        }
                    }
                }
            }

            QByteArray smalldata;
            QBuffer buffer(&smalldata);
            resized.save(&buffer, QFileInfo(url.path()).suffix().toLatin1().data());

            return smalldata;
        }
    }
    return QTextBrowser::loadResource(type, url);
}

}  // namespace inviwo
