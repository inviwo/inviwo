/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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
#include <QFrame>
#include <QVBoxLayout>
#include <QTextBrowser>
#include <QHelpEngineCore>

#include <QMap>
#include <QString>
#include <QUrl>
#include <QFile>
#include <QByteArray>
#include <QCoreApplication>
#include <QBuffer>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QUrlQuery>
#include <QImageReader>
#include <QImageWriter>
#endif

namespace inviwo {

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

HelpWidget::HelpWidget(QWidget* parent)
    : InviwoDockWidget(tr("Help"), parent), helpBrowser_(nullptr), helpEngine_(nullptr) {
    setObjectName("HelpWidget");
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    QWidget* centralWidget = new QWidget();
    QVBoxLayout* vLayout = new QVBoxLayout(centralWidget);
    vLayout->setSpacing(7);
    vLayout->setContentsMargins(0, 0, 0, 0);

    std::string helpfile =
        InviwoApplication::getPtr()->getPath(InviwoApplication::PATH_HELP, "inviwo.qhc");

    helpEngine_ = new QHelpEngineCore(QString::fromStdString(helpfile), this);

    helpBrowser_ = new HelpBrowser(this, helpEngine_);
    helpBrowser_->setHtml(QString("Hello world"));
    vLayout->addWidget(helpBrowser_);
    centralWidget->setLayout(vLayout);
    setWidget(centralWidget);

    connect(helpEngine_, SIGNAL(setupFinished()), this, SLOT(setupFinished()));

    if (!helpEngine_->setupData()) {
        LogWarn("Faild to setup the help engine:" << helpEngine_->error().toUtf8().constData());
        delete helpEngine_;
        helpEngine_ = nullptr;
    }
}

void HelpWidget::setupFinished() {
    if (!helpEngine_) return;
    helpBrowser_->setSource(QUrl("qthelp://org.inviwo/doc/docpage-org_8inviwo_8Background.html"));
}

void HelpWidget::showDocForClassName(std::string classIdentifier) {
    if (!helpEngine_) return;
    std::string className = joinString(splitString(classIdentifier, '.'), "_8");

    QUrl url(QString::fromStdString("qthelp://org.inviwo/doc/docpage-" + className + ".html"));
    QUrl foundUrl = helpEngine_->findFile(url);

    if (foundUrl.isValid()) {
        helpBrowser_->setSource(foundUrl);
    } else {
        helpBrowser_->setText(QString("No documentation available"));
    }
}


HelpWidget::HelpBrowser::HelpBrowser(QWidget* parent, QHelpEngineCore* helpEngine)
    : QTextBrowser(parent)
    , helpEngine_(helpEngine) {

    setReadOnly(true);
    setUndoRedoEnabled(false);
    setContextMenuPolicy(Qt::NoContextMenu);
    setAcceptRichText(false);
    
}

HelpWidget::HelpBrowser::~HelpBrowser() {}

QVariant HelpWidget::HelpBrowser::loadResource(int type, const QUrl& name) {
    QUrl url(name);
    if (name.isRelative()) url = source().resolved(url);

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    QUrlQuery query(url);
    if (query.hasQueryItem("classIdentifier")) {
        QString cid = query.queryItemValue("classIdentifier");

        auto imageCache = InviwoApplication::getPtr()->getPath(InviwoApplication::PATH_SETTINGS);
        imageCache += "image-cache";
        filesystem::createDirectoryRecursivly(imageCache);

        QString imgname(QString::fromStdString(imageCache) + "/" + cid + ".png");
        QImageReader reader(imgname);
        QImage img = reader.read();
        if (!img.isNull()) {
            return img;
        } else {
            QImage img = utilqt::generatePreview(cid);
            QByteArray data;
            QBuffer buffer(&data);
            buffer.open(QIODevice::WriteOnly);
            img.save(&buffer, "PNG");

            QImageWriter writer(imgname);
            writer.write(img);

            return data;
        }
    }
#endif

#ifdef IVW_DEBUG  // Look for the html in the doc-qt folder.
    if (type == QTextDocument::HtmlResource || type == QTextDocument::ImageResource) {
        std::string docbase = InviwoApplication::getPtr()->getPath(InviwoApplication::PATH_DATA,
                                                                   "../tools/doxygen/doc-qt/html");
        QString file = name.toString();
        file.replace("qthelp://org.inviwo/doc", QString::fromStdString(docbase));
        QFile newfile(file);

        if (newfile.exists() && newfile.open(QIODevice::ReadOnly)) {
            QByteArray data = newfile.readAll();
            return data;
        }
    }
#endif

    QByteArray data = helpEngine_->fileData(url);
    switch (type) {
        case QTextDocument::HtmlResource:
            return data;
        case QTextDocument::ImageResource:
            return data;
        case QTextDocument::StyleSheetResource:
            return data;
    }
    return QTextBrowser::loadResource(type, url);
}

}  // namespace