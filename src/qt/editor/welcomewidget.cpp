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

#include <warn/pop>

namespace inviwo {

WelcomeWidget::WelcomeWidget(InviwoMainWindow *window, QWidget *parent)
    : QWidget(parent), mainWindow_(window) {

    setObjectName("WelcomeWidget");
    setAttribute(Qt::WA_DeleteOnClose);

    auto gridLayout = new QGridLayout();
    gridLayout->setContentsMargins(9, 0, 0, 0);
    gridLayout->setSpacing(6);

    // heading: logo + "get started"
    auto horizontalLayout = new QHBoxLayout();
    auto label_2 = new QLabel(this);
    label_2->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

    label_2->setPixmap(QPixmap(":/inviwo/inviwo_light.png"));
    label_2->setScaledContents(false);

    horizontalLayout->addWidget(label_2);

    auto label_6 = new QLabel("Get Started", this);
    label_6->setObjectName("WelcomeHeader");
    label_6->setAlignment(Qt::AlignLeading | Qt::AlignLeft | Qt::AlignTop);

    horizontalLayout->addWidget(label_6);

    gridLayout->addLayout(horizontalLayout, 0, 0, 1, 2);

    auto updateDetails = [this, window](const QString &filename) {
        QFileInfo info(filename);
        if (filename.isEmpty() || !info.exists()) {
            details_->clear();
            loadWorkspace_->setEnabled(false);
            return;
        }

        loadWorkspace_->setEnabled(true);

        // extract annotations including network screenshot and canvas images from workspace
        Deserializer d(utilqt::fromQString(filename));
        d.registerFactory(window->getInviwoApplication()->getPropertyFactory());
        WorkspaceAnnotationsQt annotations;
        d.deserialize("WorkspaceAnnotations", annotations);

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

        auto addImage = [&body](auto item) {
            const int fixedImgHeight = 256;

            if (!item.isValid()) return;
            body.append("h3", item.name, {{"style", "font-weight:600;margin-top:20px"}});
            auto p = body.append("p").append(
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
    filetree_ = new FileTreeWidget(window->getInviwoApplication(), this);
    filetree_->setObjectName("FileTreeWidget");
    filetree_->setMinimumWidth(300);
    filetree_->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
    QObject::connect(filetree_, &FileTreeWidget::selectedFileChanged, this,
                     [this, window, updateDetails](const QString &filename, bool isExample) {
                         updateDetails(filename);

                         loadWorkspace_->disconnect();
                         QObject::connect(
                             loadWorkspace_, &QToolButton::clicked, this,
                             [this, filename, isExample]() { loadWorkspace(filename, isExample); });
                     });
    QObject::connect(filetree_, &FileTreeWidget::loadFile, this, &WelcomeWidget::loadWorkspace);

    gridLayout->addWidget(filetree_, 1, 0, 2, 1);

    // center column: workspace details, and buttons for loading workspaces

    // workspace details
    details_ = new QTextEdit(this);
    details_->setObjectName("NetworkDetails");
    details_->setReadOnly(true);
    details_->setFrameShape(QFrame::NoFrame);
    details_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    gridLayout->addWidget(details_, 1, 1, 1, 1);

    // tool buttons
    auto horizontalLayout_2 = new QHBoxLayout();
    horizontalLayout_2->setSpacing(6);

    auto createButton = [this](const QString &str, auto iconpath) {
        auto button = new QToolButton(this);
        button->setText(str);
        button->setIcon(QIcon(iconpath));
        button->setIconSize(QSize(48, 48));
        button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        return button;
    };

    loadWorkspace_ = createButton("Load", ":/icons/large/open.png");
    loadWorkspace_->setObjectName("LoadWorkspaceToolButton");

    horizontalLayout_2->addWidget(loadWorkspace_);

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
    auto horizontalSpacer_2 = new QSpacerItem(10, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);
    gridLayout->addItem(horizontalSpacer_2, 1, 2, 1, 1);

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
    changelog_->setReadOnly(true);

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

    gridLayout->addWidget(rightColumn, 0, 3, 3, 1);

    // final layout adjustments
    gridLayout->setRowStretch(1, 10);
    gridLayout->setRowStretch(2, 1);
    gridLayout->setColumnStretch(0, 1);
    gridLayout->setColumnStretch(1, 3);
    gridLayout->setColumnStretch(3, 2);

    setLayout(gridLayout);

    initChangelog();
}

void WelcomeWidget::updateRecentWorkspaces() {
    filetree_->updateRecentWorkspaces(mainWindow_->getRecentWorkspaceList());
}

void WelcomeWidget::showEvent(QShowEvent *event) {
    if (!event->spontaneous()) {
        updateRecentWorkspaces();
        filetree_->updateExampleEntries();
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
                loadWorkspace_->animateClick();
            }
            event->accept();
        }
    } else if ((event->key() == Qt::Key_Return) || (event->key() == Qt::Key_Enter)) {
        loadWorkspace_->animateClick();
        event->accept();
    }
    QWidget::keyPressEvent(event);
}

void WelcomeWidget::loadWorkspace(const QString &filename, bool isExample) const {
    bool controlPressed =
        (mainWindow_->getInviwoApplicationQt()->keyboardModifiers() == Qt::ControlModifier);
    if (mainWindow_->askToSaveWorkspaceChanges() &&
        mainWindow_->openWorkspace(filename, isExample && !controlPressed)) {
        mainWindow_->hideWelcomeScreen();
    }
}

void WelcomeWidget::initChangelog() {
    changelog_->setHtml(
        "<!DOCTYPE HTML PUBLIC '-//W3C//DTD HTML 4.0//EN' "
        "'http://www.w3.org/TR/REC-html40/strict.dtd'>"
        "<html><head><meta name='qrichtext' content='1' /><style type='text/css'>"
        "p, li { white-space: pre-wrap; }"
        "</style></head><body style=' font-family:'Calibra'; font-size:8pt; font-weight:400; "
        "font-style:normal;'>"
        "<p style=' margin-top:0px; margin-bottom:5px;'><span "
        "style='font-size:x-large; font-weight:600; color:#268bd2;'>Latest Changes</span></p>"
        "<p style=' margin-top:20px; margin-bottom:2px;'><span "
        "style='font-size:large; font-weight:600; color:#268bd2;'>2019-01-16 Get Started and "
        "Workspace Annotations</span></p>"
        "<p style=' margin-bottom:4px;'><span style='color: white;'>Get Started screen provides an "
        "overview over recently used workspaces and available examples next to the latest "
        "changes.</span></p>"
        "<p style=' margin-bottom:4px;'><span style='color: white;'>Inviwo workspaces now also "
        "feature annotations like title, author, tags, and a description stored along with the "
        "network. The annotation widget allows to edit the annotations in the Qt application "
        "(accessible via the \"View\" menu).</span></p>"
        "<p style=' margin-top:20px; margin-bottom:2px;'><a name='20181218'></a><span "
        "style='font-size:large; font-weight:600; color:#268bd2;'>2018-12-18  Transfer Function "
        "Import/Export</span></p>"
        "<p style=' margin-bottom:4px;'><span style='color: white;'>Extended context menu of "
        "transfer function properties. It is now possible to import and export TFs directly in the "
        "property widget. Transfer functions located in </span><span style=' "
        "font-family:'SFMono-Regular,Consolas,Liberation Mono,Menlo,Courier,monospace'; color: "
        "white; background-color:rgba(27,31,35,0.047059);'>data/transferfunctions/</span><span "
        "style='color: white;'> are accessible as TF presets in the context menu of both TF editor "
        "and TF property.</span></p>"
        "<p style=' margin-top:20px; margin-bottom:2px;'><a name='20181122'></a><span "
        "style='font-size:large; font-weight:600; color:#268bd2;'>2018-11-22</span></p>"
        "<p style=' margin-bottom:4px;'><span style='color: white;'>Better handling of linking in "
        "port inspectors. Show auto links when dragging in processors, disable auto links by "
        "pressing alt. Pressing shift while dragging when dragging in processors enables auto "
        "connection for inports.</span></p>"
        "<p style=' margin-top:20px; margin-bottom:2px;'><a name='20181119'></a><span "
        "style='font-size:large; font-weight:600; color:#268bd2;'>2018-11-19</span></p>"
        "<p style=' margin-bottom:4px;'><span style='color: white;'>Converted all Inviwo core "
        "modules to use the new structure with include and src folders.</span></p>"
        "<p style=' margin-top:20px; margin-bottom:2px;'><a name='20181119-1'></a><span "
        "style='font-size:large; font-weight:600; color:#268bd2;'>2018-11-19</span></p>"
        "<p style=' margin-bottom:4px;'><span style='color: white;'>Added a </span><span style=' "
        "font-family:'SFMono-Regular,Consolas,Liberation Mono,Menlo,Courier,monospace'; color: "
        "white; background-color:rgba(27,31,35,0.047059);'>--updateModule</span><span "
        "style='color: white;'> option to inviwo-meta-cli.exe it will update a module to use "
        "include and src folders. Move all </span><span style=' "
        "font-family:'SFMono-Regular,Consolas,Liberation Mono,Menlo,Courier,monospace'; color: "
        "white; background-color:rgba(27,31,35,0.047059);'>.h</span><span style='color: white;'> "
        "file into the </span><span style=' font-family:'SFMono-Regular,Consolas,Liberation "
        "Mono,Menlo,Courier,monospace'; color: white; "
        "background-color:rgba(27,31,35,0.047059);'>include/&lt;org&gt;/&lt;module&gt;</span><span "
        "style='color: white;'> sub folder and all </span><span style=' "
        "font-family:'SFMono-Regular,Consolas,Liberation Mono,Menlo,Courier,monospace'; color: "
        "white; background-color:rgba(27,31,35,0.047059);'>.cpp</span><span style='color: white;'> "
        "into the </span><span style=' font-family:'SFMono-Regular,Consolas,Liberation "
        "Mono,Menlo,Courier,monospace'; color: white; "
        "background-color:rgba(27,31,35,0.047059);'>src</span><span style='color: white;'> folder. "
        "except for files under </span><span style=' "
        "font-family:'SFMono-Regular,Consolas,Liberation Mono,Menlo,Courier,monospace'; color: "
        "white; background-color:rgba(27,31,35,0.047059);'>/ext</span><span style='color: "
        "white;'>, </span><span style=' font-family:'SFMono-Regular,Consolas,Liberation "
        "Mono,Menlo,Courier,monospace'; color: white; "
        "background-color:rgba(27,31,35,0.047059);'>/tests</span><span style='color: white;'>, or "
        "paths excluded be the given filters.</span></p>"
        "<p style=' margin-top:20px; margin-bottom:2px;'><a name='20181114'></a><span "
        "style='font-size:large; font-weight:600; color:#268bd2;'>2018-11-14</span></p>"
        "<p style=' margin-bottom:4px;'><span style='color: white;'>Added an option to control if "
        "a module should be on by default, and remove the old global setting. To enable the module "
        "by default add the following to the module </span><span style=' "
        "font-family:'SFMono-Regular,Consolas,Liberation Mono,Menlo,Courier,monospace'; color: "
        "white; background-color:rgba(27,31,35,0.047059);'>depends.cmake</span><span style='color: "
        "white;'> file:</span></p>"
        "<pre style=' margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; "
        "-qt-block-indent:0; text-indent:0px; line-height:145%; background-color:#5d6060;'><span "
        "style=' font-family:'SFMono-Regular,Consolas,Liberation Mono,Menlo,Courier,monospace'; "
        "color: white; background-color:transparent;'>    set(EnableByDefault ON)</span></pre>"
        "<p style=' margin-top:20px; margin-bottom:2px;'><a name='20181114-1'></a><span "
        "style='font-size:large; font-weight:600; color:#268bd2;'>2018-11-14</span></p>"
        "<p style=' margin-bottom:4px;'><span style='color: white;'>A new inviwo-meta library and "
        "an inviwo-meta-cli commandline app has been added to supersede the </span><span style=' "
        "font-family:'SFMono-Regular,Consolas,Liberation Mono,Menlo,Courier,monospace'; color: "
        "white; background-color:rgba(27,31,35,0.047059);'>make-new-module.py</span><span "
        "style='color: white;'> and </span><span style=' "
        "font-family:'SFMono-Regular,Consolas,Liberation Mono,Menlo,Courier,monospace'; color: "
        "white; background-color:rgba(27,31,35,0.047059);'>make-new-file.py</span><span "
        "style='color: white;'> python scripts. The library is also exposed in the tools menu of "
        "inviwo. Note that the inviwo-meta library relies on C++17 features and, thus, requires a "
        "recent compiler.</span></p>"
        "<p style=' margin-top:20px; margin-bottom:2px;'><a name='20181114-2'></a><span "
        "style='font-size:large; font-weight:600; color:#268bd2;'>2018-11-14</span></p>"
        "<p style=' margin-bottom:4px;'><span style='color: white;'>Generated files are now stored "
        "in the corresponding </span><span style=' font-family:'SFMono-Regular,Consolas,Liberation "
        "Mono,Menlo,Courier,monospace'; color: white; "
        "background-color:rgba(27,31,35,0.047059);'>CMAKE_CURRENT_BINAY_DIR</span><span "
        "style='color: white;'> for the subdirectory in question. For a module this means "
        "</span><span style=' font-family:'SFMono-Regular,Consolas,Liberation "
        "Mono,Menlo,Courier,monospace'; color: white; "
        "background-color:rgba(27,31,35,0.047059);'>{build folder}/modules/{module "
        "name}</span><span style='color: white;'>. </span><span style=' "
        "font-family:'SFMono-Regular,Consolas,Liberation Mono,Menlo,Courier,monospace'; color: "
        "white; "
        "background-color:rgba(27,31,35,0.047059);'>CMAKE_CURRENT_BINAY_DIR/include</span><span "
        "style='color: white;'> path is added as an include path for each module. Hence, the "
        "generated headers are put in</span></p>"
        "<pre style=' margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; "
        "-qt-block-indent:0; text-indent:0px; line-height:145%; background-color:#5d6060;'><span "
        "style=' font-family:'SFMono-Regular,Consolas,Liberation Mono,Menlo,Courier,monospace'; "
        "color: white; background-color:transparent;'>    {build folder}/modules/{module "
        "name}/include/{organization}/{module name}/</span></pre>"
        "<p style=' margin-bottom:4px;'><span style='color: white;'>Same is true for the generated "
        "headers of inviwo core, like </span><span style=' "
        "font-family:'SFMono-Regular,Consolas,Liberation Mono,Menlo,Courier,monospace'; color: "
        "white; background-color:rgba(27,31,35,0.047059);'>moduleregistration.h</span><span "
        "style='color: white;'>. They are now placed in:</span></p>"
        "<pre style=' margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; "
        "-qt-block-indent:0; text-indent:0px; line-height:145%; background-color:#5d6060;'><span "
        "style=' font-family:'SFMono-Regular,Consolas,Liberation Mono,Menlo,Courier,monospace'; "
        "color: white; background-color:transparent;'>    {build "
        "folder}/modules/core/include/inviwo/core/</span></pre>"
        "<p style=' margin-bottom:4px;'><span style='color: white;'>Which means that for the "
        "module loading in apps</span></p>"
        "<pre style=' margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; "
        "-qt-block-indent:0; text-indent:0px; line-height:145%; background-color:#5d6060;'><span "
        "style=' font-family:'SFMono-Regular,Consolas,Liberation Mono,Menlo,Courier,monospace'; "
        "color: white; background-color:transparent;'>    #include "
        "&lt;moduleregistration.h&gt;</span></pre>"
        "<p style=' margin-bottom:4px;'><span style='color: white;'>needs to be changed "
        "to</span></p>"
        "<pre style=' margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; "
        "-qt-block-indent:0; text-indent:0px; line-height:145%; background-color:#5d6060;'><span "
        "style=' font-family:'SFMono-Regular,Consolas,Liberation Mono,Menlo,Courier,monospace'; "
        "color: white; background-color:transparent;'>    #include "
        "&lt;inviwo/core/moduleregistration.h&gt;</span></pre>"
        "<p style=' margin-top:20px; margin-bottom:2px;'><a name='20181114-3'></a><span "
        "style='font-size:large; font-weight:600; color:#268bd2;'>2018-11-14</span></p>"
        "<p style=' margin-bottom:4px;'><span style='color: white;'>New Module structure. We have "
        "introduced a new module structure where we separate headers and source files in the same "
        "way as it was already done for the core part of inviwo. Hence, module headers should now "
        "be placed under the include folder like:</span></p>"
        "<pre style=' margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; "
        "-qt-block-indent:0; text-indent:0px; line-height:145%; background-color:#5d6060;'><span "
        "style=' font-family:'SFMono-Regular,Consolas,Liberation Mono,Menlo,Courier,monospace'; "
        "color: white; background-color:transparent;'>    .../{module "
        "name}/include/{organization}/{module name}/</span></pre>"
        "<p style=' margin-bottom:4px;'><span style='color: white;'>and sources goes in the source "
        "folder:</span></p>"
        "<pre style=' margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; "
        "-qt-block-indent:0; text-indent:0px; line-height:145%; background-color:#5d6060;'><span "
        "style=' font-family:'SFMono-Regular,Consolas,Liberation Mono,Menlo,Courier,monospace'; "
        "color: white; background-color:transparent;'>    .../{module name}/src/</span></pre>"
        "<p style=' margin-bottom:4px;'><span style=' "
        "font-family:'SFMono-Regular,Consolas,Liberation Mono,Menlo,Courier,monospace'; color: "
        "white; background-color:rgba(27,31,35,0.047059);'>{module name}</span><span style='color: "
        "white;'> it the lower case name of the module, </span><span style=' "
        "font-family:'SFMono-Regular,Consolas,Liberation Mono,Menlo,Courier,monospace'; color: "
        "white; background-color:rgba(27,31,35,0.047059);'>{organization}</span><span "
        "style='color: white;'> default to inviwo but can be user-specified. The headers can then "
        "be included using</span></p>"
        "<pre style=' margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; "
        "-qt-block-indent:0; text-indent:0px; line-height:145%; background-color:#5d6060;'><span "
        "style=' font-family:'SFMono-Regular,Consolas,Liberation Mono,Menlo,Courier,monospace'; "
        "color: white; background-color:transparent;'>    #include &lt;{organization}/{module "
        "name}/header.h&gt;</span></pre>"
        "<p style=' margin-bottom:4px;'><span style='color: white;'>The implementation is "
        "backwards compatible so old modules can continue to exist, but the structure is "
        "considered deprecated. The main reasons for the change are to make packaging of headers "
        "easier and to prevent accidentally including headers from modules without depending on "
        "them.</span></p>"
        "<p style=' margin-top:20px; margin-bottom:2px;'><a name='20181105'></a><span "
        "style='font-size:large; font-weight:600; color:#268bd2;'>2018-11-05</span></p>"
        "<p style=' margin-bottom:4px;'><span style='color: white;'>The SplitImage processor now "
        "features a draggable handle. This handle allows to adjust the split position in the "
        "canvas with either mouse or touch.</span></p>"
        "<p style=' margin-top:20px; margin-bottom:2px;'><a "
        "name='20181004colorscalelegend'></a><span style='font-size:large; font-weight:600; "
        "color:#268bd2;'>2018-10-04 Color Scale Legend</span></p>"
        "<p style=' margin-bottom:4px;'><span style='color: white;'>Added a Color Scale Legend "
        "processor to the plottinggl module that draws a 1D transfer function and the "
        "corresponding value axis on top of an image.</span></p>"
        "<p style=' margin-top:20px; margin-bottom:2px;'><a "
        "name='20180927fullscreenonstartup'></a><span style='font-size:large; font-weight:600; "
        "color:#268bd2;'>2018-09-27 Full screen on startup</span></p>"
        "<p style=' margin-bottom:4px;'><span style='color: white;'>Core: Renamed key mapping to "
        "fullscreenEvent and instead use fullscreen for a bool property. You will need to change "
        "the key mapping again if you have changed it from shift + f.</span></p>"
        "<p style=' margin-bottom:4px;'><span style='color: white;'>Python: Full screen is now "
        "exposed as a property in the canvas instead of a function.</span></p></body></html>");
}

}  // namespace inviwo
