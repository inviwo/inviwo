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

#include <inviwo/qt/editor/filetreewidget.h>
#include <inviwo/qt/editor/inviwomainwindow.h>
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

#include <warn/pop>

namespace inviwo {

WelcomeWidget::WelcomeWidget(InviwoMainWindow *window, bool shownOnStartup, QWidget *parent)
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

    auto updatePreviewImages =
        [this](const QString &filename) {
            auto createTab =
                [](const std::string &str) {
                    auto label_8 = new QLabel();
                    QSizePolicy sizePolicy3(QSizePolicy::Preferred, QSizePolicy::Fixed);
                    label_8->setSizePolicy(sizePolicy3);
                    label_8->setMaximumHeight(256);
                    // label_8->setPixmap(QPixmap(":/inviwo/inviwo-logo.png"));
                    QPixmap img;
                    if (str.empty()) {
                        img.loadFromData(
                            QByteArray::fromBase64(
                                R"(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)"));
                        // label_8->setScaledContents(true);
                    } else {
                        img.loadFromData(QByteArray::fromBase64(str.c_str()));
                    }
                    label_8->setPixmap(img);
                    return label_8;
                };

            previewImages_->setUpdatesEnabled(false);
            util::OnScopeExit([this]() { previewImages_->setUpdatesEnabled(true); });

            auto clearTabs = [this]() {
                while (previewImages_->count() > 0) {
                    auto widget = previewImages_->widget(0);
                    previewImages_->removeTab(0);
                    delete widget;
                }
            };
            clearTabs();

            auto file = utilqt::fromQString(filename);
            if (!filesystem::fileExists(file)) {
                previewImages_->addTab(createTab(""), "");
                previewImages_->setEnabled(false);
                return;
            }

            // parse workspace file and extract network screenshot and canvas images
            Deserializer d(utilqt::fromQString(filename));
            std::string networkImage;
            d.deserialize("NetworkImage", networkImage);
            std::vector<std::string> canvasImages;
            d.deserialize("CanvasImages", canvasImages, "Canvas");

            previewImages_->addTab(createTab(networkImage), "Network");
            int index = 1;
            for (auto &img : canvasImages) {
                previewImages_->addTab(createTab(img), QString("Canvas %1").arg(index++));
            }

            previewImages_->setCurrentIndex(0);
            previewImages_->setEnabled(true);
        };

    auto updateDetails = [this, window](const QString &filename) {
        QFileInfo info(filename);
        if (filename.isEmpty() || !info.exists()) {
            details_->setText("No file selected.");
            return;
        }

        const auto dateformat = window->locale().dateTimeFormat(QLocale::LongFormat);
        auto createdStr =
            utilqt::fromQString(info.fileTime(QFileDevice::FileBirthTime).toString(dateformat));
        auto modifiedStr = utilqt::fromQString(
            info.fileTime(QFileDevice::FileModificationTime).toString(dateformat));

        Document doc;
        using P = Document::PathComponent;
        using H = utildoc::TableBuilder::Header;
        auto t = doc.append("html").append("body").append("table");
        auto pi = t.append("tr").append("td");
        pi.append("b", utilqt::fromQString(info.fileName()), {{"style", "color:white;"}});
        utildoc::TableBuilder tb(pi, P::end());
        tb(H("Last Modified"), modifiedStr);
        tb(H("Created"), createdStr);

        doc.append("p", "TODO: add more information...");

        details_->setHtml(utilqt::toQString(doc));
    };

    // left column: list of recently used workspaces and examples
    filetree_ =
        new FileTreeWidget(window->getInviwoApplication(), window->getRecentWorkspaceList(), this);
    filetree_->setObjectName("FileTreeWidget");
    QSizePolicy sizePolicy5(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
    sizePolicy5.setHorizontalStretch(0);
    sizePolicy5.setVerticalStretch(100);
    filetree_->setSizePolicy(sizePolicy5);
    QObject::connect(filetree_, &FileTreeWidget::selectedFileChanged, this,
                     [this, window, updatePreviewImages, updateDetails](const QString &filename,
                                                                        bool isExample) {
                         // update preview images
                         updatePreviewImages(filename);
                         updateDetails(filename);

                         loadWorkspace_->disconnect();
                         QObject::connect(loadWorkspace_, &QToolButton::clicked, this,
                                          [window, filename, isExample]() {
                                              if (window->openWorkspace(filename, isExample)) {
                                                  window->hideWelcomeScreen();
                                              }
                                          });
                     });

    gridLayout->addWidget(filetree_, 1, 0, 3, 1);

    // center column: image tabwidget, workspace details, and buttons for loading workspaces

    // workspace preview images
    previewImages_ = new QTabWidget(this);
    previewImages_->setObjectName("PreviewImageTabWidget");
    previewImages_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    previewImages_->setTabPosition(QTabWidget::South);
    updatePreviewImages(QString());

    gridLayout->addWidget(previewImages_, 1, 1, 1, 1);

    // workspace details
    details_ = new QTextEdit(this);
    details_->setObjectName("NetworkDetails");
    details_->setReadOnly(true);
    details_->setFrameShape(QFrame::NoFrame);
    details_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    gridLayout->addWidget(details_, 2, 1, 1, 1);

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

    if (!shownOnStartup) {
        auto toolButton_7 = createButton("Back", ":/icons/large/left.png");
        toolButton_7->setObjectName("PreviousViewToolButton");
        QObject::connect(toolButton_7, &QToolButton::clicked, this,
                         [window]() { window->hideWelcomeScreen(); });

        horizontalLayout_2->addWidget(toolButton_7);
    }

    gridLayout->addLayout(horizontalLayout_2, 3, 1, 1, 1);

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

    gridLayout->addWidget(rightColumn, 0, 3, 4, 1);

    // final layout adjustments
    gridLayout->setRowStretch(1, 1);
    gridLayout->setRowStretch(2, 10);
    gridLayout->setRowStretch(3, 1);
    gridLayout->setColumnStretch(0, 1);
    gridLayout->setColumnStretch(1, 3);
    gridLayout->setColumnStretch(3, 2);

    setLayout(gridLayout);

    changelog_->setHtml(
        "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" "
        "\"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
        "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
        "p, li { white-space: pre-wrap; }\n"
        "</style></head><body style=\" font-family:'Calibra'; font-size:8.25pt; font-weight:400; "
        "font-style:normal;\">\n"
        "<p style=\" margin-top:24px; margin-bottom:16px; margin-left:0px; margin-right:0px; "
        "-qt-block-indent:0; text-indent:0px; line-height:125%;\"><span style=\" font-family:'MS "
        "Shell Dlg 2'; font-size:12pt; font-weight:600; color:#268bd2;\">Latest "
        "Changes</span></p>\n"
        "<p style=\" margin-top:24px; margin-bottom:16px; margin-left:0px; margin-right:0px; "
        "-qt-block-indent:0; text-indent:0px; line-height:125%;\"><span style=\" "
        "font-family:'-apple-system,BlinkMacSystemFont,Segoe UI,Helvetica,Arial,sans-serif,Apple "
        "Color Emoji,Segoe UI Emoji,Segoe UI Symbol'; font-size:8pt; font-weight:600; "
        "color:#f0f0f0;\">2018-12-18</span></p>\n"
        "<p style=\" margin-top:0px; margin-bottom:16px; margin-lef"
        "t:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" "
        "font-family:'-apple-system,BlinkMacSystemFont,Segoe UI,Helvetica,Arial,sans-serif,Apple "
        "Color Emoji,Segoe UI Emoji,Segoe UI Symbol'; font-size:8pt; color:#f0f0f0;\">Extended "
        "context menu of transfer function properties. It is now possible to import and export TFs "
        "directly in the property widget. Transfer functions located in </span><span style=\" "
        "font-family:'SFMono-Regular,Consolas,Liberation Mono,Menlo,Courier,monospace'; "
        "font-size:8pt; color:#f0f0f0; "
        "background-color:rgba(27,31,35,0.047059);\">data/transferfunctions/</span><span style=\" "
        "font-family:'-apple-system,BlinkMacSystemFont,Segoe UI,Helvetica,Arial,sans-serif,Apple "
        "Color Emoji,Segoe UI Emoji,Segoe UI Symbol'; font-size:8pt; color:#f0f0f0;\"> are "
        "accessible as TF presets in the context menu of both TF editor and TF "
        "property.</span></p>\n"
        "<p style=\" margin-top:24px; margin-bottom:16px; margin-left:0px; margin-right:0px; "
        "-qt-block-indent:0; text-indent:0px; line-height:1"
        "25%;\"><a name=\"user-content-2018-11-22\"></a><span style=\" "
        "font-family:'-apple-system,BlinkMacSystemFont,Segoe UI,Helvetica,Arial,sans-serif,Apple "
        "Color Emoji,Segoe UI Emoji,Segoe UI Symbol'; font-size:8pt; font-weight:600; "
        "color:#f0f0f0;\">2</span><span style=\" "
        "font-family:'-apple-system,BlinkMacSystemFont,Segoe UI,Helvetica,Arial,sans-serif,Apple "
        "Color Emoji,Segoe UI Emoji,Segoe UI Symbol'; font-size:8pt; font-weight:600; "
        "color:#f0f0f0;\">018-11-22</span></p>\n"
        "<p style=\" margin-top:0px; margin-bottom:16px; margin-left:0px; margin-right:0px; "
        "-qt-block-indent:0; text-indent:0px;\"><span style=\" "
        "font-family:'-apple-system,BlinkMacSystemFont,Segoe UI,Helvetica,Arial,sans-serif,Apple "
        "Color Emoji,Segoe UI Emoji,Segoe UI Symbol'; font-size:8pt; color:#f0f0f0;\">Better "
        "handling of linking in port inspectors. Show auto links when dragging in processors, "
        "disable auto links by pressing alt. Pressing shift while dragging when dragging in "
        "processors enables auto connection for inports.</span></p>\n"
        "<p style=\""
        " margin-top:24px; margin-bottom:16px; margin-left:0px; margin-right:0px; "
        "-qt-block-indent:0; text-indent:0px; line-height:125%;\"><a "
        "name=\"user-content-2018-11-19\"></a><span style=\" "
        "font-family:'-apple-system,BlinkMacSystemFont,Segoe UI,Helvetica,Arial,sans-serif,Apple "
        "Color Emoji,Segoe UI Emoji,Segoe UI Symbol'; font-size:8pt; font-weight:600; "
        "color:#f0f0f0;\">2</span><span style=\" "
        "font-family:'-apple-system,BlinkMacSystemFont,Segoe UI,Helvetica,Arial,sans-serif,Apple "
        "Color Emoji,Segoe UI Emoji,Segoe UI Symbol'; font-size:8pt; font-weight:600; "
        "color:#f0f0f0;\">018-11-19</span></p>\n"
        "<p style=\" margin-top:0px; margin-bottom:16px; margin-left:0px; margin-right:0px; "
        "-qt-block-indent:0; text-indent:0px;\"><span style=\" "
        "font-family:'-apple-system,BlinkMacSystemFont,Segoe UI,Helvetica,Arial,sans-serif,Apple "
        "Color Emoji,Segoe UI Emoji,Segoe UI Symbol'; font-size:8pt; color:#f0f0f0;\">Converted "
        "all Inviwo core modules to use the new structure with include and src "
        "folders.</span></p>\n"
        "<p style=\" margin-top"
        ":24px; margin-bottom:16px; margin-left:0px; margin-right:0px; -qt-block-indent:0; "
        "text-indent:0px; line-height:125%;\"><a name=\"user-content-2018-11-19-1\"></a><span "
        "style=\" font-family:'-apple-system,BlinkMacSystemFont,Segoe "
        "UI,Helvetica,Arial,sans-serif,Apple Color Emoji,Segoe UI Emoji,Segoe UI Symbol'; "
        "font-size:8pt; font-weight:600; color:#f0f0f0;\">2</span><span style=\" "
        "font-family:'-apple-system,BlinkMacSystemFont,Segoe UI,Helvetica,Arial,sans-serif,Apple "
        "Color Emoji,Segoe UI Emoji,Segoe UI Symbol'; font-size:8pt; font-weight:600; "
        "color:#f0f0f0;\">018-11-19</span></p>\n"
        "<p style=\" margin-top:0px; margin-bottom:16px; margin-left:0px; margin-right:0px; "
        "-qt-block-indent:0; text-indent:0px;\"><span style=\" "
        "font-family:'-apple-system,BlinkMacSystemFont,Segoe UI,Helvetica,Arial,sans-serif,Apple "
        "Color Emoji,Segoe UI Emoji,Segoe UI Symbol'; font-size:8pt; color:#f0f0f0;\">Added a "
        "</span><span style=\" font-family:'SFMono-Regular,Consolas,Liberation "
        "Mono,Menlo,Courier,monospace'; font-size:8pt; color:#f0f0"
        "f0; background-color:rgba(27,31,35,0.047059);\">--updateModule</span><span style=\" "
        "font-family:'-apple-system,BlinkMacSystemFont,Segoe UI,Helvetica,Arial,sans-serif,Apple "
        "Color Emoji,Segoe UI Emoji,Segoe UI Symbol'; font-size:8pt; color:#f0f0f0;\"> option to "
        "inviwo-meta-cli.exe it will update a module to use include and src folders. Move all "
        "</span><span style=\" font-family:'SFMono-Regular,Consolas,Liberation "
        "Mono,Menlo,Courier,monospace'; font-size:8pt; color:#f0f0f0; "
        "background-color:rgba(27,31,35,0.047059);\">.h</span><span style=\" "
        "font-family:'-apple-system,BlinkMacSystemFont,Segoe UI,Helvetica,Arial,sans-serif,Apple "
        "Color Emoji,Segoe UI Emoji,Segoe UI Symbol'; font-size:8pt; color:#f0f0f0;\">file into "
        "the </span><span style=\" font-family:'SFMono-Regular,Consolas,Liberation "
        "Mono,Menlo,Courier,monospace'; font-size:8pt; color:#f0f0f0; "
        "background-color:rgba(27,31,35,0.047059);\">include/&lt;org&gt;/&lt;module&gt;</"
        "span><span style=\" font-family:'-apple-system,BlinkMacSystemFont,Segoe "
        "UI,Helvetica,Arial,"
        "sans-serif,Apple Color Emoji,Segoe UI Emoji,Segoe UI Symbol'; font-size:8pt; "
        "color:#f0f0f0;\"> sub folder and all </span><span style=\" "
        "font-family:'SFMono-Regular,Consolas,Liberation Mono,Menlo,Courier,monospace'; "
        "font-size:8pt; color:#f0f0f0; "
        "background-color:rgba(27,31,35,0.047059);\">.cpp</span><span style=\" "
        "font-family:'-apple-system,BlinkMacSystemFont,Segoe UI,Helvetica,Arial,sans-serif,Apple "
        "Color Emoji,Segoe UI Emoji,Segoe UI Symbol'; font-size:8pt; color:#f0f0f0;\"> into the "
        "</span><span style=\" font-family:'SFMono-Regular,Consolas,Liberation "
        "Mono,Menlo,Courier,monospace'; font-size:8pt; color:#f0f0f0; "
        "background-color:rgba(27,31,35,0.047059);\">src </span><span style=\" "
        "font-family:'-apple-system,BlinkMacSystemFont,Segoe UI,Helvetica,Arial,sans-serif,Apple "
        "Color Emoji,Segoe UI Emoji,Segoe UI Symbol'; font-size:8pt; color:#f0f0f0;\">folder. "
        "except for files under </span><span style=\" "
        "font-family:'SFMono-Regular,Consolas,Liberation Mono,Menlo,Courier,monospace'; "
        "font-size:8pt; color:#f0f0f0; backgro"
        "und-color:rgba(27,31,35,0.047059);\">/ext</span><span style=\" "
        "font-family:'-apple-system,BlinkMacSystemFont,Segoe UI,Helvetica,Arial,sans-serif,Apple "
        "Color Emoji,Segoe UI Emoji,Segoe UI Symbol'; font-size:8pt; color:#f0f0f0;\">, "
        "</span><span style=\" font-family:'SFMono-Regular,Consolas,Liberation "
        "Mono,Menlo,Courier,monospace'; font-size:8pt; color:#f0f0f0; "
        "background-color:rgba(27,31,35,0.047059);\">/tests</span><span style=\" "
        "font-family:'-apple-system,BlinkMacSystemFont,Segoe UI,Helvetica,Arial,sans-serif,Apple "
        "Color Emoji,Segoe UI Emoji,Segoe UI Symbol'; font-size:8pt; color:#f0f0f0;\">, or paths "
        "excluded be the given filters.</span></p>\n"
        "<p style=\" margin-top:24px; margin-bottom:16px; margin-left:0px; margin-right:0px; "
        "-qt-block-indent:0; text-indent:0px; line-height:125%;\"><a "
        "name=\"user-content-2018-11-14\"></a><span style=\" "
        "font-family:'-apple-system,BlinkMacSystemFont,Segoe UI,Helvetica,Arial,sans-serif,Apple "
        "Color Emoji,Segoe UI Emoji,Segoe UI Symbol'; font-size:8pt; font-weight:600; color:#f0"
        "f0f0;\">2</span><span style=\" font-family:'-apple-system,BlinkMacSystemFont,Segoe "
        "UI,Helvetica,Arial,sans-serif,Apple Color Emoji,Segoe UI Emoji,Segoe UI Symbol'; "
        "font-size:8pt; font-weight:600; color:#f0f0f0;\">018-11-14</span></p>\n"
        "<p style=\" margin-top:0px; margin-bottom:16px; margin-left:0px; margin-right:0px; "
        "-qt-block-indent:0; text-indent:0px;\"><span style=\" "
        "font-family:'-apple-system,BlinkMacSystemFont,Segoe UI,Helvetica,Arial,sans-serif,Apple "
        "Color Emoji,Segoe UI Emoji,Segoe UI Symbol'; font-size:8pt; color:#f0f0f0;\">Added an "
        "option to control if a module should be on by default, and remove the old global setting. "
        "To enable the module by default add the following to the module </span><span style=\" "
        "font-family:'SFMono-Regular,Consolas,Liberation Mono,Menlo,Courier,monospace'; "
        "font-size:8pt; color:#f0f0f0; "
        "background-color:rgba(27,31,35,0.047059);\">depends.cmake</span><span style=\" "
        "font-family:'-apple-system,BlinkMacSystemFont,Segoe UI,Helvetica,Arial,sans-serif,Apple "
        "Color Emoji,Segoe UI Emo"
        "ji,Segoe UI Symbol'; font-size:8pt; color:#f0f0f0;\"> file:</span></p>\n"
        "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; "
        "-qt-block-indent:0; text-indent:0px; line-height:145%; background-color:#5d6060;\"><span "
        "style=\" font-family:'SFMono-Regular,Consolas,Liberation Mono,Menlo,Courier,monospace'; "
        "font-size:8pt; color:#f0f0f0; background-color:transparent;\">    set(EnableByDefault "
        "ON)</span></p>\n"
        "<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:16px; margin-left:0px; "
        "margin-right:0px; -qt-block-indent:0; text-indent:0px; line-height:145%; "
        "font-family:'SFMono-Regular,Consolas,Liberation Mono,Menlo,Courier,monospace'; "
        "font-size:8pt; color:#f0f0f0; background-color:#5d6060;\"><br /></p>\n"
        "<p style=\" margin-top:24px; margin-bottom:16px; margin-left:0px; margin-right:0px; "
        "-qt-block-indent:0; text-indent:0px; line-height:125%;\"><a "
        "name=\"user-content-2018-11-14-1\"></a><span style=\" "
        "font-family:'-apple-system,BlinkMacSystemFont,Segoe UI,Helvetica,Arial,sa"
        "ns-serif,Apple Color Emoji,Segoe UI Emoji,Segoe UI Symbol'; font-size:8pt; "
        "font-weight:600; color:#f0f0f0;\">2</span><span style=\" "
        "font-family:'-apple-system,BlinkMacSystemFont,Segoe UI,Helvetica,Arial,sans-serif,Apple "
        "Color Emoji,Segoe UI Emoji,Segoe UI Symbol'; font-size:8pt; font-weight:600; "
        "color:#f0f0f0;\">018-11-14</span></p>\n"
        "<p style=\" margin-top:0px; margin-bottom:16px; margin-left:0px; margin-right:0px; "
        "-qt-block-indent:0; text-indent:0px;\"><span style=\" "
        "font-family:'-apple-system,BlinkMacSystemFont,Segoe UI,Helvetica,Arial,sans-serif,Apple "
        "Color Emoji,Segoe UI Emoji,Segoe UI Symbol'; font-size:8pt; color:#f0f0f0;\">A new "
        "inviwo-meta library and an inviwo-meta-cli commandline app has been added to supersede "
        "the </span><span style=\" font-family:'SFMono-Regular,Consolas,Liberation "
        "Mono,Menlo,Courier,monospace'; font-size:8pt; color:#f0f0f0; "
        "background-color:rgba(27,31,35,0.047059);\">make-new-module.py</span><span style=\" "
        "font-family:'-apple-system,BlinkMacSystemFont,Segoe UI,Helvetica,Arial,s"
        "ans-serif,Apple Color Emoji,Segoe UI Emoji,Segoe UI Symbol'; font-size:8pt; "
        "color:#f0f0f0;\">and </span><span style=\" "
        "font-family:'SFMono-Regular,Consolas,Liberation Mono,Menlo,Courier,monospace'; "
        "font-size:8pt; color:#f0f0f0; "
        "background-color:rgba(27,31,35,0.047059);\">make-new-file.py</span><span style=\" "
        "font-family:'-apple-system,BlinkMacSystemFont,Segoe UI,Helvetica,Arial,sans-serif,Apple "
        "Color Emoji,Segoe UI Emoji,Segoe UI Symbol'; font-size:8pt; color:#f0f0f0;\"> python "
        "scripts. The library is also exposed in the tools menu of inviwo. Note that the "
        "inviwo-meta library relies on C++17 features and, thus, requires a recent "
        "compiler.</span></p>\n"
        "<p style=\" margin-top:24px; margin-bottom:16px; margin-left:0px; margin-right:0px; "
        "-qt-block-indent:0; text-indent:0px; line-height:125%;\"><a "
        "name=\"user-content-2018-11-14-2\"></a><span style=\" "
        "font-family:'-apple-system,BlinkMacSystemFont,Segoe UI,Helvetica,Arial,sans-serif,Apple "
        "Color Emoji,Segoe UI Emoji,Segoe UI Symbol'; font-size:8pt; font-weight:600; "
        "color:#f0f0f0;\">2</span><span style=\" "
        "font-family:'-apple-system,BlinkMacSystemFont,Segoe UI,Helvetica,Arial,sans-serif,Apple "
        "Color Emoji,Segoe UI Emoji,Segoe UI Symbol'; font-size:8pt; font-weight:600; "
        "color:#f0f0f0;\">018-11-14</span></p>\n"
        "<p style=\" margin-top:0px; margin-bottom:16px; margin-left:0px; margin-right:0px; "
        "-qt-block-indent:0; text-indent:0px;\"><span style=\" "
        "font-family:'-apple-system,BlinkMacSystemFont,Segoe UI,Helvetica,Arial,sans-serif,Apple "
        "Color Emoji,Segoe UI Emoji,Segoe UI Symbol'; font-size:8pt; color:#f0f0f0;\">Generated "
        "files are now stored in the corresponding </span><span style=\" "
        "font-family:'SFMono-Regular,Consolas,Liberation Mono,Menlo,Courier,monospace'; "
        "font-size:8pt; color:#f0f0f0; "
        "background-color:rgba(27,31,35,0.047059);\">CMAKE_CURRENT_BINAY_DIR</span><span style=\" "
        "font-family:'-apple-system,BlinkMacSystemFont,Segoe UI,Helvetica,Arial,sans-serif,Apple "
        "Color Emoji,Segoe UI Emoji,Segoe UI Symbol'; font-size:8pt; color:#f0f0f0;\"> for the "
        "subdirectory in question. For "
        "a module this means </span><span style=\" font-family:'SFMono-Regular,Consolas,Liberation "
        "Mono,Menlo,Courier,monospace'; font-size:8pt; color:#f0f0f0; "
        "background-color:rgba(27,31,35,0.047059);\">{build folder}/modules/{module "
        "name}</span><span style=\" font-family:'-apple-system,BlinkMacSystemFont,Segoe "
        "UI,Helvetica,Arial,sans-serif,Apple Color Emoji,Segoe UI Emoji,Segoe UI Symbol'; "
        "font-size:8pt; color:#f0f0f0;\">. </span><span style=\" "
        "font-family:'SFMono-Regular,Consolas,Liberation Mono,Menlo,Courier,monospace'; "
        "font-size:8pt; color:#f0f0f0; "
        "background-color:rgba(27,31,35,0.047059);\">CMAKE_CURRENT_BINAY_DIR/include</span><span "
        "style=\" font-family:'-apple-system,BlinkMacSystemFont,Segoe "
        "UI,Helvetica,Arial,sans-serif,Apple Color Emoji,Segoe UI Emoji,Segoe UI Symbol'; "
        "font-size:8pt; color:#f0f0f0;\"> path is added as an include path for each module. Hence, "
        "the generated headers are put in</span></p>\n"
        "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; "
        "-qt-block-indent:0; text"
        "-indent:0px; line-height:145%; background-color:#5d6060;\"><span style=\" "
        "font-family:'SFMono-Regular,Consolas,Liberation Mono,Menlo,Courier,monospace'; "
        "font-size:8pt; color:#f0f0f0; background-color:transparent;\">    {build "
        "folder}/modules/{module name}/include/{organization}/{module name}/</span></p>\n"
        "<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:16px; margin-left:0px; "
        "margin-right:0px; -qt-block-indent:0; text-indent:0px; line-height:145%; "
        "font-family:'SFMono-Regular,Consolas,Liberation Mono,Menlo,Courier,monospace'; "
        "font-size:8pt; color:#f0f0f0; background-color:#5d6060;\"><br /></p>\n"
        "<p style=\" margin-top:0px; margin-bottom:16px; margin-left:0px; margin-right:0px; "
        "-qt-block-indent:0; text-indent:0px;\"><span style=\" "
        "font-family:'-apple-system,BlinkMacSystemFont,Segoe UI,Helvetica,Arial,sans-serif,Apple "
        "Color Emoji,Segoe UI Emoji,Segoe UI Symbol'; font-size:8pt; color:#f0f0f0;\">Same is true "
        "for the generated headers of inviwo core, like </span><span style=\" font-family:'SFMo"
        "no-Regular,Consolas,Liberation Mono,Menlo,Courier,monospace'; font-size:8pt; "
        "color:#f0f0f0; "
        "background-color:rgba(27,31,35,0.047059);\">moduleregistration.h</span><span style=\" "
        "font-family:'-apple-system,BlinkMacSystemFont,Segoe UI,Helvetica,Arial,sans-serif,Apple "
        "Color Emoji,Segoe UI Emoji,Segoe UI Symbol'; font-size:8pt; color:#f0f0f0;\">. They are "
        "now placed in:</span></p>\n"
        "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; "
        "-qt-block-indent:0; text-indent:0px; line-height:145%; background-color:#5d6060;\"><span "
        "style=\" font-family:'SFMono-Regular,Consolas,Liberation Mono,Menlo,Courier,monospace'; "
        "font-size:8pt; color:#f0f0f0; background-color:transparent;\">    {build "
        "folder}/modules/core/include/inviwo/core/</span></p>\n"
        "<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:16px; margin-left:0px; "
        "margin-right:0px; -qt-block-indent:0; text-indent:0px; line-height:145%; "
        "font-family:'SFMono-Regular,Consolas,Liberation Mono,Menlo,Courier,monospace'; font-size:"
        "8pt; color:#f0f0f0; background-color:#5d6060;\"><br /></p>\n"
        "<p style=\" margin-top:0px; margin-bottom:16px; margin-left:0px; margin-right:0px; "
        "-qt-block-indent:0; text-indent:0px;\"><span style=\" "
        "font-family:'-apple-system,BlinkMacSystemFont,Segoe UI,Helvetica,Arial,sans-serif,Apple "
        "Color Emoji,Segoe UI Emoji,Segoe UI Symbol'; font-size:8pt; color:#f0f0f0;\">Which means "
        "that for the module loading in apps</span></p>\n"
        "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; "
        "-qt-block-indent:0; text-indent:0px; line-height:145%; background-color:#5d6060;\"><span "
        "style=\" font-family:'SFMono-Regular,Consolas,Liberation Mono,Menlo,Courier,monospace'; "
        "font-size:8pt; color:#f0f0f0; background-color:#5d6060;\">    #</span><span style=\" "
        "font-family:'SFMono-Regular,Consolas,Liberation Mono,Menlo,Courier,monospace'; "
        "font-size:8pt; color:#d73a49;\">include</span><span style=\" "
        "font-family:'SFMono-Regular,Consolas,Liberation Mono,Menlo,Courier,monospace'; "
        "font-size:8pt; color:#f0f0f0;\"> <"
        "/span><span style=\" font-family:'SFMono-Regular,Consolas,Liberation "
        "Mono,Menlo,Courier,monospace'; font-size:8pt; "
        "color:#032f62;\">&lt;moduleregistration.h&gt;</span></p>\n"
        "<p style=\" margin-top:0px; margin-bottom:16px; margin-left:0px; margin-right:0px; "
        "-qt-block-indent:0; text-indent:0px;\"><span style=\" "
        "font-family:'-apple-system,BlinkMacSystemFont,Segoe UI,Helvetica,Arial,sans-serif,Apple "
        "Color Emoji,Segoe UI Emoji,Segoe UI Symbol'; font-size:8pt; color:#f0f0f0;\">needs to be "
        "changed to</span></p>\n"
        "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; "
        "-qt-block-indent:0; text-indent:0px; line-height:145%; background-color:#5d6060;\"><span "
        "style=\" font-family:'SFMono-Regular,Consolas,Liberation Mono,Menlo,Courier,monospace'; "
        "font-size:8pt; color:#f0f0f0; background-color:#5d6060;\">    #</span><span style=\" "
        "font-family:'SFMono-Regular,Consolas,Liberation Mono,Menlo,Courier,monospace'; "
        "font-size:8pt; color:#d73a49;\">include</span><span style=\" font-family:'SFMono-Regul"
        "ar,Consolas,Liberation Mono,Menlo,Courier,monospace'; font-size:8pt; color:#f0f0f0;\"> "
        "</span><span style=\" font-family:'SFMono-Regular,Consolas,Liberation "
        "Mono,Menlo,Courier,monospace'; font-size:8pt; "
        "color:#032f62;\">&lt;inviwo/core/moduleregistration.h&gt;</span></p>\n"
        "<p style=\" margin-top:24px; margin-bottom:16px; margin-left:0px; margin-right:0px; "
        "-qt-block-indent:0; text-indent:0px; line-height:125%;\"><a "
        "name=\"user-content-2018-11-14-3\"></a><span style=\" "
        "font-family:'-apple-system,BlinkMacSystemFont,Segoe UI,Helvetica,Arial,sans-serif,Apple "
        "Color Emoji,Segoe UI Emoji,Segoe UI Symbol'; font-size:8pt; font-weight:600; "
        "color:#f0f0f0;\">2</span><span style=\" "
        "font-family:'-apple-system,BlinkMacSystemFont,Segoe UI,Helvetica,Arial,sans-serif,Apple "
        "Color Emoji,Segoe UI Emoji,Segoe UI Symbol'; font-size:8pt; font-weight:600; "
        "color:#f0f0f0;\">018-11-14</span></p>\n"
        "<p style=\" margin-top:0px; margin-bottom:16px; margin-left:0px; margin-right:0px; "
        "-qt-block-indent:0; text-indent:0px;\"><span style=\" fo"
        "nt-family:'-apple-system,BlinkMacSystemFont,Segoe UI,Helvetica,Arial,sans-serif,Apple "
        "Color Emoji,Segoe UI Emoji,Segoe UI Symbol'; font-size:8pt; color:#f0f0f0;\">New Module "
        "structure. We have introduced a new module structure where we separate headers and source "
        "files in the same way as it was already done for the core part of inviwo. Hence, module "
        "headers should now be placed under the include folder like:</span></p>\n"
        "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; "
        "-qt-block-indent:0; text-indent:0px; line-height:145%; background-color:#5d6060;\"><span "
        "style=\" font-family:'SFMono-Regular,Consolas,Liberation Mono,Menlo,Courier,monospace'; "
        "font-size:8pt; color:#f0f0f0; background-color:transparent;\">    .../{module "
        "name}/include/{organization}/{module name}/</span></p>\n"
        "<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:16px; margin-left:0px; "
        "margin-right:0px; -qt-block-indent:0; text-indent:0px; line-height:145%; "
        "font-family:'SFMono-Regular,Consolas,Li"
        "beration Mono,Menlo,Courier,monospace'; font-size:8pt; color:#f0f0f0; "
        "background-color:#5d6060;\"><br /></p>\n"
        "<p style=\" margin-top:0px; margin-bottom:16px; margin-left:0px; margin-right:0px; "
        "-qt-block-indent:0; text-indent:0px;\"><span style=\" "
        "font-family:'-apple-system,BlinkMacSystemFont,Segoe UI,Helvetica,Arial,sans-serif,Apple "
        "Color Emoji,Segoe UI Emoji,Segoe UI Symbol'; font-size:8pt; color:#f0f0f0;\">and sources "
        "goes in the source folder:</span></p>\n"
        "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; "
        "-qt-block-indent:0; text-indent:0px; line-height:145%; background-color:#5d6060;\"><span "
        "style=\" font-family:'SFMono-Regular,Consolas,Liberation Mono,Menlo,Courier,monospace'; "
        "font-size:8pt; color:#f0f0f0; background-color:transparent;\">    .../{module "
        "name}/src/</span></p>\n"
        "<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:16px; margin-left:0px; "
        "margin-right:0px; -qt-block-indent:0; text-indent:0px; line-height:145%; "
        "font-family:'SFMono-Regular,Co"
        "nsolas,Liberation Mono,Menlo,Courier,monospace'; font-size:8pt; color:#f0f0f0; "
        "background-color:#5d6060;\"><br /></p>\n"
        "<p style=\" margin-top:0px; margin-bottom:16px; margin-left:0px; margin-right:0px; "
        "-qt-block-indent:0; text-indent:0px;\"><span style=\" "
        "font-family:'SFMono-Regular,Consolas,Liberation Mono,Menlo,Courier,monospace'; "
        "font-size:8pt; color:#f0f0f0; background-color:rgba(27,31,35,0.047059);\">{module "
        "name}</span><span style=\" font-family:'-apple-system,BlinkMacSystemFont,Segoe "
        "UI,Helvetica,Arial,sans-serif,Apple Color Emoji,Segoe UI Emoji,Segoe UI Symbol'; "
        "font-size:8pt; color:#f0f0f0;\"> it the lower case name of the module, </span><span "
        "style=\" font-family:'SFMono-Regular,Consolas,Liberation Mono,Menlo,Courier,monospace'; "
        "font-size:8pt; color:#f0f0f0; "
        "background-color:rgba(27,31,35,0.047059);\">{organization}</span><span style=\" "
        "font-family:'-apple-system,BlinkMacSystemFont,Segoe UI,Helvetica,Arial,sans-serif,Apple "
        "Color Emoji,Segoe UI Emoji,Segoe UI Symbol'; font-size:8pt; color:#f0f0f0;\""
        "> default to inviwo but can be user-specified. The headers can then be included "
        "using</span></p>\n"
        "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; "
        "-qt-block-indent:0; text-indent:0px; line-height:145%; background-color:#5d6060;\"><span "
        "style=\" font-family:'SFMono-Regular,Consolas,Liberation Mono,Menlo,Courier,monospace'; "
        "font-size:8pt; color:#f0f0f0; background-color:#5d6060;\">    #</span><span style=\" "
        "font-family:'SFMono-Regular,Consolas,Liberation Mono,Menlo,Courier,monospace'; "
        "font-size:8pt; color:#d73a49;\">include</span><span style=\" "
        "font-family:'SFMono-Regular,Consolas,Liberation Mono,Menlo,Courier,monospace'; "
        "font-size:8pt; color:#f0f0f0;\"> </span><span style=\" "
        "font-family:'SFMono-Regular,Consolas,Liberation Mono,Menlo,Courier,monospace'; "
        "font-size:8pt; color:#032f62;\">&lt;{organization}/{module name}/header.h&gt;</span></p>\n"
        "<p style=\" margin-top:0px; margin-bottom:16px; margin-left:0px; margin-right:0px; "
        "-qt-block-indent:0; text-indent:0px;\"><span style="
        "\" font-family:'-apple-system,BlinkMacSystemFont,Segoe "
        "UI,Helvetica,Arial,sans-serif,Apple Color Emoji,Segoe UI Emoji,Segoe UI Symbol'; "
        "font-size:8pt; color:#f0f0f0;\">The implementation is backwards compatible so old modules "
        "can continue to exist, but the structure is considered deprecated. The main reasons for "
        "the change are to make packaging of headers easier and to prevent accidentally including "
        "headers from modules without depending on them.</span></p></body></html>");
}

}  // namespace inviwo
