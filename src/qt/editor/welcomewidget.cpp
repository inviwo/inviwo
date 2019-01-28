/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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

#include <inviwo/qt/editor/welcomewidget.h>

#include <inviwo/core/util/document.h>
#include <inviwo/core/io/serialization/deserializer.h>
#include <inviwo/core/util/raiiutils.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/properties/propertyfactory.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/filesystem.h>

#include <inviwo/qt/editor/filetreewidget.h>
#include <inviwo/qt/editor/inviwomainwindow.h>
#include <inviwo/qt/editor/workspaceannotationsqt.h>
#include <inviwo/qt/applicationbase/inviwoapplicationqt.h>
#include <modules/qtwidgets/inviwoqtutils.h>

#include <warn/push>
#include <warn/ignore/all>

#include <QTabWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QSpacerItem>
#include <QCheckBox>
#include <QLabel>
#include <QTextEdit>
#include <QFrame>
#include <QSettings>
#include <QToolButton>
#include <QPixmap>
#include <QByteArray>
#include <QFileInfo>
#include <QDateTime>
#include <QShowEvent>
#include <QKeyEvent>
#include <QFile>

#include <warn/pop>

namespace inviwo {

WelcomeWidget::WelcomeWidget(InviwoMainWindow *window, QWidget *parent)
    : QSplitter(parent), mainWindow_(window) {

    setObjectName("WelcomeWidget");

    setOrientation(Qt::Horizontal);
    setHandleWidth(5);

    auto leftWidget = new QWidget(this);

    auto gridLayout = new QGridLayout();
    gridLayout->setContentsMargins(9, 0, 0, 9);
    gridLayout->setSpacing(6);

    // heading: logo + "get started"
    auto horizontalLayout = new QHBoxLayout();
    auto label_2 = new QLabel(leftWidget);
    label_2->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

    label_2->setPixmap(QPixmap(":/inviwo/inviwo_light.png"));
    label_2->setScaledContents(false);

    horizontalLayout->addWidget(label_2);

    auto label_6 = new QLabel("Get Started", leftWidget);
    label_6->setObjectName("WelcomeHeader");
    label_6->setAlignment(Qt::AlignLeading | Qt::AlignLeft | Qt::AlignTop);

    horizontalLayout->addWidget(label_6);

    gridLayout->addLayout(horizontalLayout, 0, 0, 1, 2);

    auto updateDetails = [this, window](const QString &filename) {
        QFileInfo info(filename);
        if (filename.isEmpty() || !info.exists()) {
            details_->clear();
            loadWorkspaceBtn_->setEnabled(false);
            return;
        }

        loadWorkspaceBtn_->setEnabled(true);

        // extract annotations including network screenshot and canvas images from workspace
        WorkspaceAnnotationsQt annotations;
        bool fileBroken = false;
        try {
            auto istream = filesystem::ifstream(utilqt::fromQString(filename));
            if (istream.is_open()) {
            Deserializer d(istream, utilqt::fromQString(filename));
            d.registerFactory(window->getInviwoApplication()->getPropertyFactory());
            d.deserialize("WorkspaceAnnotations", annotations);
            } else {
                fileBroken = true;
            }
        } catch (Exception &e) {
            util::log(e.getContext(), e.getMessage(), LogLevel::Warn);
            fileBroken = "true";
        }

        const auto dateformat = "yyyy-MM-dd hh:mm:ss";
        auto createdStr = utilqt::fromQString(info.created().toString(dateformat));
        auto modifiedStr = utilqt::fromQString(info.lastModified().toString(dateformat));

        const bool hasTitle = !annotations.getTitle().empty();
        const auto titleStr =
            hasTitle ? annotations.getTitle() : utilqt::fromQString(info.completeBaseName());
        auto description = htmlEncode(annotations.getDescription());
        replaceInString(description, "\n", "<br/>");

        Document doc;
        using P = Document::PathComponent;
        using H = utildoc::TableBuilder::Header;
        auto body = doc.append("html").append("body");
        body.append("h1", titleStr, {{"style", "font-size:20pt; color:#268bd2"}});

        auto t = body.append("p").append("table", "", {{"style", "margin-bottom:20px;"}});
        auto pi = t.append("tr").append("td");
        utildoc::TableBuilder tb(pi, P::end());
        tb(H("File Name"), htmlEncode(utilqt::fromQString(info.fileName())));
        tb(H("Last Modified"), modifiedStr);
        tb(H("Created"), createdStr);
        tb(H("Author"), htmlEncode(annotations.getAuthor()));
        tb(H("Tags"), htmlEncode(annotations.getTags()));
        tb(H("Categories"), htmlEncode(annotations.getCategories()));
        tb(H("Description"), description);

        if (fileBroken) {
            body.append("h3", "Workspace file could not be opened!",
                        {{"style", "color:firebrick;font-weight:600;margin-top:20px"}});
        }
        auto addImage = [&body](auto item) {
            const int fixedImgHeight = 256;

            if (!item.isValid()) return;
            body.append("h3", item.name, {{"style", "font-weight:600;margin-top:20px"}});
            body.append("p").append(
                "img", "",
                {{"height", std::to_string(std::min(fixedImgHeight, item.size.y))},
                 {"src", "data:image/png;base64," + item.base64png}});
        };

        for (auto &elem : annotations.getCanvasImages()) {
            addImage(elem);
        }
        addImage(annotations.getNetworkImage());

        details_->setHtml(utilqt::toQString(doc));
    };

    // left column: list of recently used workspaces and examples
    filetree_ = new FileTreeWidget(window->getInviwoApplication(), leftWidget);
    filetree_->setObjectName("FileTreeWidget");
    filetree_->setMinimumWidth(300);
    filetree_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QObject::connect(filetree_, &FileTreeWidget::selectedFileChanged, this,
                     [this, updateDetails](const QString &filename, bool isExample) {
                         updateDetails(filename);

                         loadWorkspaceBtn_->disconnect();
                         QObject::connect(
                             loadWorkspaceBtn_, &QToolButton::clicked, this,
                             [this, filename, isExample]() { loadWorkspace(filename, isExample); });
                     });
    QObject::connect(filetree_, &FileTreeWidget::loadFile, this, &WelcomeWidget::loadWorkspace);

    gridLayout->addWidget(filetree_, 1, 0, 2, 1);

    // center column: workspace details and buttons for loading workspaces

    // workspace details
    details_ = new QTextEdit(leftWidget);
    details_->setObjectName("NetworkDetails");
    details_->setReadOnly(true);
    details_->setFrameShape(QFrame::NoFrame);
    details_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    gridLayout->addWidget(details_, 1, 1, 1, 1);

    // tool buttons
    auto horizontalLayout_2 = new QHBoxLayout();
    horizontalLayout_2->setSpacing(6);

    auto createButton = [leftWidget](const QString &str, auto iconpath) {
        auto button = new QToolButton(leftWidget);
        button->setText(str);
        button->setIcon(QIcon(iconpath));
        button->setIconSize(QSize(48, 48));
        button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        return button;
    };

    loadWorkspaceBtn_ = createButton("Load", ":/icons/large/open.png");
    loadWorkspaceBtn_->setObjectName("LoadWorkspaceToolButton");

    horizontalLayout_2->addWidget(loadWorkspaceBtn_);

    auto horizontalSpacer = new QSpacerItem(18, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    horizontalLayout_2->addItem(horizontalSpacer);

    auto toolButton = createButton("New Workspace", ":/icons/large/newfile.png");
    toolButton->setObjectName("NewWorkspaceToolButton");
    QObject::connect(toolButton, &QToolButton::clicked, this, [window]() {
        if (window->newWorkspace()) {
            window->hideWelcomeScreen();
        }
    });

    horizontalLayout_2->addWidget(toolButton);

    auto toolButton_2 = createButton("Open Workspace", ":/icons/large/open.png");
    toolButton_2->setObjectName("OpenWorkspaceToolButton");
    QObject::connect(toolButton_2, &QToolButton::clicked, this, [window]() {
        if (window->openWorkspace()) {
            window->hideWelcomeScreen();
        }
    });

    horizontalLayout_2->addWidget(toolButton_2);

    gridLayout->addLayout(horizontalLayout_2, 2, 1, 1, 1);

    // add some space between center and right column
    auto horizontalSpacer_2 = new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);
    gridLayout->addItem(horizontalSpacer_2, 1, 2, 1, 1);

    leftWidget->setLayout(gridLayout);
    gridLayout->setRowStretch(1, 2);
    gridLayout->setRowStretch(2, 1);
    gridLayout->setColumnStretch(0, 1);
    gridLayout->setColumnStretch(1, 2);

    // right column: changelog and options

    auto rightColumn = new QFrame(this);
    rightColumn->setObjectName("WelcomeRightColumn");
    auto verticalLayout_3 = new QVBoxLayout(rightColumn);
    verticalLayout_3->setContentsMargins(20, 0, 0, 11);
    changelog_ = new QTextEdit(rightColumn);
    changelog_->setObjectName("Changelog");
    QSizePolicy sizePolicy1(changelog_->sizePolicy());
    sizePolicy1.setVerticalStretch(100);
    changelog_->setSizePolicy(sizePolicy1);
    changelog_->setFrameShape(QFrame::NoFrame);
    changelog_->setTextInteractionFlags(Qt::TextBrowserInteraction);

    verticalLayout_3->addWidget(changelog_);

    auto verticalSpacer_2 = new QSpacerItem(20, 30, QSizePolicy::Minimum, QSizePolicy::Fixed);
    verticalLayout_3->addItem(verticalSpacer_2);

    QSettings settings;
    settings.beginGroup("InviwoMainWindow");

    auto autoloadWorkspace =
        new QCheckBox("&Automatically load most recently used workspace", rightColumn);
    autoloadWorkspace->setChecked(settings.value("autoloadLastWorkspace", true).toBool());
    QObject::connect(autoloadWorkspace, &QCheckBox::toggled, this, [](bool checked) {
        QSettings settings;
        settings.beginGroup("InviwoMainWindow");
        settings.setValue("autoloadLastWorkspace", checked);
        settings.endGroup();
    });
    verticalLayout_3->addWidget(autoloadWorkspace);

    auto showOnStartup = new QCheckBox("&Show this page on startup", rightColumn);
    showOnStartup->setChecked(settings.value("showWelcomePage", true).toBool());
    QObject::connect(showOnStartup, &QCheckBox::toggled, this, [](bool checked) {
        QSettings settings;
        settings.beginGroup("InviwoMainWindow");
        settings.setValue("showWelcomePage", checked);
        settings.endGroup();
    });
    verticalLayout_3->addWidget(showOnStartup);

    settings.endGroup();

    addWidget(rightColumn);

    // ensure that the splitter handle responds to hover events
    // see https://bugreports.qt.io/browse/QTBUG-13768
    handle(1)->setAttribute(Qt::WA_Hover);

    initChangelog();
}

void WelcomeWidget::updateRecentWorkspaces() {
    filetree_->updateRecentWorkspaces(mainWindow_->getRecentWorkspaceList());
}

void WelcomeWidget::showEvent(QShowEvent *event) {
    if (!event->spontaneous()) {
        updateRecentWorkspaces();
        filetree_->updateExampleEntries();

        // select first entry of recent workspaces, if existing
        filetree_->selectRecentWorkspace(0);
    }
    QWidget::showEvent(event);
}

void WelcomeWidget::keyPressEvent(QKeyEvent *event) {
    if ((event->key() >= Qt::Key_0) && (event->key() <= Qt::Key_9)) {
        int number = [key = event->key()]() {
            if (key == Qt::Key_0) return 9;
            return key - Qt::Key_1;
        }();
        if (filetree_->selectRecentWorkspace(number)) {
            if ((event->modifiers() & Qt::ControlModifier) == Qt::ControlModifier) {
                loadWorkspaceBtn_->animateClick();
            }
            event->accept();
        }
    } else if ((event->key() == Qt::Key_Return) || (event->key() == Qt::Key_Enter)) {
        loadWorkspaceBtn_->animateClick();
        event->accept();
    }
    QWidget::keyPressEvent(event);
}

void WelcomeWidget::loadWorkspace(const QString &filename, bool isExample) const {
    if (mainWindow_->askToSaveWorkspaceChanges()) {
        bool hide = [&]() {
            bool controlPressed =
                (mainWindow_->getInviwoApplicationQt()->keyboardModifiers() == Qt::ControlModifier);
            if (isExample && !controlPressed) {
                return mainWindow_->openExample(filename);
            } else {
                return mainWindow_->openWorkspace(filename);
            }
        }();
        if (hide) {
            mainWindow_->hideWelcomeScreen();
        }
    }
}

void WelcomeWidget::initChangelog() {
    QFile file(":/changelog.html");
    if (file.open(QFile::ReadOnly | QFile::Text) && file.size() > 0) {
        changelog_->setHtml(file.readAll());
    } else {
        changelog_->setHtml(
            R"(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<style type="text/css">
    body {color: #9d9995;}
    a {color: #268bd2;}
    a:visited { color: #1E6A9E; }
    h1 { font-size: xx-large; color: #268bd2; margin-bottom:1em; }
    h2 { font-size: larger; color: #268bd2; margin-top:1em; margin-bottom:0em; }
    p { margin-bottom: 0.2em; margin-top: 0.1em; }
</style>
</head>
<body >
<h1>Latest Changes</h1>
<p>For latest changes see <a href='https://github.com/inviwo/inviwo/blob/master/CHANGELOG.md'>https://github.com/inviwo/inviwo/blob/master/CHANGELOG.md</a>.
</body>
</html>
    )");
    }
}

}  // namespace inviwo
