/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2021 Inviwo Foundation
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
#include <inviwo/core/util/logfilter.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/properties/propertyfactory.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/network/workspacemanager.h>

#include <inviwo/qt/editor/workspacetreeview.h>
#include <inviwo/qt/editor/workspacegridview.h>
#include <inviwo/qt/editor/workspaceannotationsqt.h>
#include <inviwo/qt/editor/lineediteventfilter.h>
#include <modules/qtwidgets/inviwoqtutils.h>

#include <warn/push>
#include <warn/ignore/all>

#include <QScreen>
#include <QTabWidget>
#include <QToolbar>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QSpacerItem>
#include <QCheckBox>
#include <QLineEdit>
#include <QLabel>
#include <QFrame>
#include <QSettings>
#include <QToolButton>
#include <QPixmap>
#include <QByteArray>
#include <QFileInfo>
#include <QDateTime>
#include <QShowEvent>
#include <QKeyEvent>
#include <QAction>
#include <QFile>
#include <QImage>
#include <QScrollArea>

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
#include <QWindow>
#include <QGuiApplication>
#endif

#include <warn/pop>

#ifndef INVIWO_ALL_DYN_LINK
struct InitQtChangelogResources {
    // Needed for loading of resources when building statically
    // see https://wiki.qt.io/QtResources#Q_INIT_RESOURCE
    InitQtChangelogResources() { Q_INIT_RESOURCE(changelog); }
    ~InitQtChangelogResources() { Q_CLEANUP_RESOURCE(changelog); }
} initQtChangelogResources;
#endif

namespace inviwo {

constexpr std::string_view placeholder = R"(<!DOCTYPE html>
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
<p>For latest changes see <a href='https://github.com/inviwo/inviwo/blob/master/CHANGELOG.md'>
https://github.com/inviwo/inviwo/blob/master/CHANGELOG.md</a>.
</body>
</html>
)";

class ChangeLog : public QTextEdit {
public:
    ChangeLog(QWidget* parent) : QTextEdit(parent) {
        setObjectName("Changelog");
        setFrameShape(QFrame::NoFrame);
        setTextInteractionFlags(Qt::TextBrowserInteraction);
        document()->setIndentWidth(utilqt::emToPx(this, 1.5));
        document()->setDocumentMargin(0.0);
        setContentsMargins(0, 0, 0, 0);
    }

    void resizeEvent(QResizeEvent* event) override {
        loadLog();
        QTextEdit::resizeEvent(event);
    }

    void loadLog() {
        QFile file(":/changelog.html");
        if (file.open(QFile::ReadOnly | QFile::Text) && file.size() > 0) {
            setHtml(file.readAll());
        } else {
            setHtml(utilqt::toQString(placeholder));
        }
    }

protected:
    QVariant loadResource(int type, const QUrl& name) override {
        if (type == QTextDocument::ImageResource) {
            const auto path = ":/" + name.path();
            if (QFile{path}.exists()) {
                auto img = QImage{path};
                return img.scaled(std::min(img.width(), width()), img.height(), Qt::KeepAspectRatio,
                                  Qt::SmoothTransformation);
            }
        }
        return QTextEdit::loadResource(type, name);
    }
};

WelcomeWidget::WelcomeWidget(InviwoApplication* app, QWidget* parent)
    : QSplitter(parent), app_(app), workspaceModel_(new WorkspaceTreeModel(app, this))
    , workspaceProxyModel_{new QSortFilterProxyModel{this}}
    , workspaceSelectionModel_(new QItemSelectionModel(workspaceProxyModel_, this)) {
    setObjectName("WelcomeWidget");
        
    workspaceProxyModel_->setSourceModel(workspaceModel_);
    workspaceProxyModel_->setRecursiveFilteringEnabled(true);
    workspaceProxyModel_->setFilterCaseSensitivity(Qt::CaseInsensitive);

    const auto space = utilqt::refSpacePx(this);
    setOrientation(Qt::Horizontal);
    setHandleWidth(space);
    setContentsMargins(0, 0, 0, 0);

    {  // Left splitter pane ( caption | splitter ( filetree | details ) )
        auto leftWidget = new QWidget(this);
        auto leftLayout = new QVBoxLayout();
        leftWidget->setLayout(leftLayout);
        leftWidget->setContentsMargins(0, 0, 0, 0);
        leftLayout->setContentsMargins(0, 0, 0, 0);
        leftLayout->setSpacing(0);

        addWidget(leftWidget);

        {  // caption: logo + "get started"
            auto captionLayout = new QHBoxLayout();
            auto captionWidget = new QWidget();
            captionWidget->setLayout(captionLayout);
            auto inviwoLogo = new QLabel(leftWidget);
            inviwoLogo->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
            inviwoLogo->setPixmap(QPixmap(":/inviwo/inviwo_light.png"));
            inviwoLogo->setScaledContents(false);
            captionLayout->addWidget(inviwoLogo);

            auto title = new QLabel("Get Started", leftWidget);
            title->setObjectName("WelcomeHeader");
            title->setAlignment(Qt::AlignLeading | Qt::AlignLeft | Qt::AlignTop);
            captionLayout->addWidget(title);
            captionWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
            captionWidget->setContentsMargins(0, 0, 0, 0);
            leftLayout->addWidget(captionWidget);
        }

        auto leftSplitter = new QSplitter(leftWidget);
        leftSplitter->setObjectName("WelcomeWidget");
        leftSplitter->setOrientation(Qt::Horizontal);
        leftSplitter->setHandleWidth(space / 2);
        leftSplitter->setContentsMargins(0, 0, 0, 0);
        leftLayout->addWidget(leftSplitter);
        leftSplitter->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

        {  // left column: workspace filter, list of recently used workspaces, and examples
            workspaceGridView_ = new WorkspaceGridView(workspaceModel_, workspaceProxyModel_, workspaceSelectionModel_, leftWidget);
            workspaceGridView_->setObjectName("WorkspaceView");
            // QScrollArea requires a minimum size
            workspaceGridView_->setMinimumWidth(utilqt::emToPx(this, 2));
            workspaceGridView_->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
            workspaceGridViewArea_ = new QScrollArea();
            workspaceGridViewArea_->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
            workspaceGridViewArea_->setVerticalScrollBarPolicy( Qt::ScrollBarAsNeeded );
            workspaceGridViewArea_->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
            workspaceGridViewArea_->setWidgetResizable( true );
            workspaceGridViewArea_->setWidget(workspaceGridView_);
            QSettings settings;
            settings.beginGroup("InviwoMainWindow");
            workspaceGridViewArea_->setVisible(settings.value("showWorkSpaceGridView", true).toBool());
            
            QObject::connect(workspaceGridView_, &WorkspaceGridView::selectedFileChanged, this,
                             [this](const QString& filename, bool isExample) {
                                 updateDetails(filename);

                                 loadWorkspaceBtn_->disconnect();
                                 QObject::connect(loadWorkspaceBtn_, &QToolButton::clicked, this,
                                                  [this, filename, isExample]() {
                                                      emit loadWorkspace(filename, isExample);
                                                  });
                             });
            QObject::connect(workspaceGridView_, &WorkspaceGridView::loadFile, this,
                             &WelcomeWidget::loadWorkspace);

            
            workspaceTreeView_ = new WorkspaceTreeView(workspaceModel_, workspaceProxyModel_, workspaceSelectionModel_, leftWidget);
            workspaceTreeView_->setVisible(!settings.value("showWorkSpaceGridView", true).toBool());
            workspaceTreeView_->setObjectName("WorkspaceTreeView");
            workspaceTreeView_->setMinimumWidth(utilqt::emToPx(this, 25));
            workspaceTreeView_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            QObject::connect(workspaceTreeView_, &WorkspaceTreeView::selectedFileChanged, this,
                             [this](const QString& filename, bool isExample) {
                                 updateDetails(filename);

                                 loadWorkspaceBtn_->disconnect();
                                 QObject::connect(loadWorkspaceBtn_, &QToolButton::clicked, this,
                                                  [this, filename, isExample]() {
                                                      emit loadWorkspace(filename, isExample);
                                                  });
                             });
            QObject::connect(workspaceTreeView_, &WorkspaceTreeView::loadFile, this,
                             &WelcomeWidget::loadWorkspace);
            
            auto workspaceViewToolbarWidget = new QLineEdit(leftWidget);
            auto toolbarLayout = new QHBoxLayout(workspaceViewToolbarWidget);
            filterLineEdit_ = new QLineEdit();
            filterLineEdit_->setPlaceholderText("Search for Workspace...");
            filterLineEdit_->installEventFilter(
                new LineEditEventFilter(workspaceTreeView_, filterLineEdit_));
            toolbarLayout->addWidget(filterLineEdit_);
            
            auto horizontalSpacer =
                new QSpacerItem(utilqt::emToPx(this, 1.0), 0,
                                QSizePolicy::Expanding, QSizePolicy::Minimum);
            toolbarLayout->addItem(horizontalSpacer);
            
            auto workspaceViewsToolbar = new QToolBar(leftWidget);
            workspaceViewsToolbar->addWidget(filterLineEdit_);
            workspaceViewsToolbar->addAction(QIcon(":/svgicons/gridlist.svg"), "Workspace Grid List", [this](){
                QSettings settings;
                settings.beginGroup("InviwoMainWindow");
                settings.setValue("showWorkSpaceGridView", true);
                settings.endGroup();
                workspaceGridViewArea_->setVisible(true);
                workspaceTreeView_->setVisible(false);
            });
            workspaceViewsToolbar->addAction(QIcon(":/svgicons/treelist.svg"), "Workspace Tree List", [this](){
                QSettings settings;
                settings.beginGroup("InviwoMainWindow");
                settings.setValue("showWorkSpaceGridView", false);
                settings.endGroup();
                workspaceGridViewArea_->setVisible(false);
                workspaceTreeView_->setVisible(true);
            });
            
            QIcon clearIcon;
            clearIcon.addFile(":/svgicons/lineedit-clear.svg",
                              utilqt::emToPx(this, QSizeF(0.3, 0.3)), QIcon::Normal);
            clearIcon.addFile(":/svgicons/lineedit-clear-active.svg",
                              utilqt::emToPx(this, QSizeF(0.3, 0.3)), QIcon::Active);
            clearIcon.addFile(":/svgicons/lineedit-clear-active.svg",
                              utilqt::emToPx(this, QSizeF(0.3, 0.3)), QIcon::Selected);
            auto clearAction = filterLineEdit_->addAction(clearIcon, QLineEdit::TrailingPosition);
            clearAction->setVisible(false);
            connect(clearAction, &QAction::triggered, filterLineEdit_, &QLineEdit::clear);
            connect(filterLineEdit_, &QLineEdit::textChanged, this,
                    [this, clearAction](const QString& str) {
                        workspaceProxyModel_->setFilterRegularExpression(str);
                        workspaceTreeView_->expandItems();

                        // select first leaf node
                        auto index = findFirstLeaf(QModelIndex());
                        if (index.isValid()) {
                            workspaceSelectionModel_->setCurrentIndex(
                                index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
                        }
                        clearAction->setVisible(!str.isEmpty());
                    });

            auto fileTreeLayout = new QVBoxLayout();
            fileTreeLayout->addWidget(workspaceViewsToolbar);
            fileTreeLayout->addWidget(workspaceGridViewArea_);
            fileTreeLayout->addWidget(workspaceTreeView_);
            auto filetreeWidget = new QWidget();
            filetreeWidget->setLayout(fileTreeLayout);
            fileTreeLayout->setContentsMargins(2 * space, 0, space, 2 * space);
            leftSplitter->addWidget(filetreeWidget);
        }

        {  // center column: workspace details and buttons for loading workspaces
            auto centerLayout = new QVBoxLayout();
            details_ = new QTextEdit(leftWidget);
            details_->setObjectName("NetworkDetails");
            details_->setReadOnly(true);
            details_->setFrameShape(QFrame::NoFrame);
            details_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
            details_->setContentsMargins(0, 0, 0, 0);
            details_->document()->setDocumentMargin(0.0);
            
            centerLayout->addWidget(details_);

            auto buttonLayout = new QHBoxLayout();
            buttonLayout->setSpacing(6);
            auto createButton = [&](const QString& str, const QString& iconpath,
                                    const QString& toolTip, const QString& objName) {
                auto button = new QToolButton(leftWidget);
                button->setText(str);
                button->setIcon(QIcon(iconpath));
                button->setIconSize(utilqt::emToPx(this, QSize(5, 5)));
                button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
                button->setToolTip(toolTip);
                button->setObjectName(objName);
                buttonLayout->addWidget(button);
                return button;
            };

            loadWorkspaceBtn_ = createButton("Load", ":/svgicons/open.svg",
                                             "Open selected workspace", "LoadWorkspaceToolButton");
            auto horizontalSpacer =
                new QSpacerItem(utilqt::emToPx(this, 2.0), utilqt::emToPx(this, 2.0),
                                QSizePolicy::Expanding, QSizePolicy::Minimum);
            buttonLayout->addItem(horizontalSpacer);

            auto newButton = createButton("New", ":/svgicons/newfile.svg",
                                          "Create an empty workspace", "NewWorkspaceToolButton");
            QObject::connect(newButton, &QToolButton::clicked, this,
                             [this]() { emit newWorkspace(); });

            auto openFileButton =
                createButton("Open...", ":/svgicons/open.svg", "Open workspace from disk",
                             "OpenWorkspaceToolButton");
            QObject::connect(openFileButton, &QToolButton::clicked, this,
                             [this]() { emit openWorkspace(); });

            restoreButton_ = createButton("Restore", ":/svgicons/revert.svg",
                                          "Restore last automatically saved workspace if available",
                                          "OpenWorkspaceToolButton");
            QObject::connect(restoreButton_, &QToolButton::clicked, this,
                             [this]() { emit restoreWorkspace(); });
            centerLayout->addLayout(buttonLayout);

            setTabOrder(loadWorkspaceBtn_, newButton);
            setTabOrder(newButton, openFileButton);
            setTabOrder(openFileButton, restoreButton_);
            setTabOrder(restoreButton_, details_);

            auto centerWidget = new QWidget();
            centerLayout->setContentsMargins(space, 0, space, 2 * space);
            centerLayout->setSpacing(2 * space);
            centerWidget->setLayout(centerLayout);
            centerWidget->setSizePolicy(QSizePolicy::MinimumExpanding,
                                        QSizePolicy::MinimumExpanding);
            leftSplitter->addWidget(centerWidget);
        }
        leftSplitter->setStretchFactor(0, 4);
        leftSplitter->setStretchFactor(1, 1);
        leftSplitter->handle(1)->setAttribute(Qt::WA_Hover);
    }
    {  // right splitter pane: changelog / options
        auto rightColumn = new QFrame(this);
        addWidget(rightColumn);
        rightColumn->setObjectName("WelcomeRightColumn");
        auto rightColumnLayout = new QVBoxLayout(rightColumn);
        rightColumnLayout->setContentsMargins(space, 0, space, 3 * space);

        {
            auto title = new QLabel("Latest Changes", rightColumn);
            title->setAlignment(Qt::AlignLeading | Qt::AlignLeft | Qt::AlignTop);
            title->setObjectName("ChangeLog");
            rightColumnLayout->addWidget(title);
        }

        {
            changelog_ = new ChangeLog(rightColumn);
            QSizePolicy sizePolicy1(changelog_->sizePolicy());
            sizePolicy1.setVerticalStretch(100);
            changelog_->setSizePolicy(sizePolicy1);
            rightColumnLayout->addWidget(changelog_);
        }

        rightColumnLayout->addItem(
            new QSpacerItem(space, space, QSizePolicy::Minimum, QSizePolicy::Fixed));

        {
            QSettings settings;
            settings.beginGroup("InviwoMainWindow");

            auto autoloadWorkspace =
                new QCheckBox("&Auto-load last workspace", rightColumn);
            autoloadWorkspace->setChecked(settings.value("autoloadLastWorkspace", true).toBool());
            QObject::connect(autoloadWorkspace, &QCheckBox::toggled, this, [](bool checked) {
                QSettings settings;
                settings.beginGroup("InviwoMainWindow");
                settings.setValue("autoloadLastWorkspace", checked);
                settings.endGroup();
            });
            rightColumnLayout->addWidget(autoloadWorkspace);

            auto showOnStartup = new QCheckBox("&Show this page on startup", rightColumn);
            showOnStartup->setChecked(settings.value("showWelcomePage", true).toBool());
            QObject::connect(showOnStartup, &QCheckBox::toggled, this, [](bool checked) {
                QSettings settings;
                settings.beginGroup("InviwoMainWindow");
                settings.setValue("showWelcomePage", checked);
                settings.endGroup();
            });
            rightColumnLayout->addWidget(showOnStartup);

            settings.endGroup();

            setTabOrder(changelog_, autoloadWorkspace);
            setTabOrder(autoloadWorkspace, showOnStartup);
        }
    }
    // ensure that the splitter handle responds to hover events
    // see https://bugreports.qt.io/browse/QTBUG-13768
    handle(1)->setAttribute(Qt::WA_Hover);

    // hide changelog on screens with less width than 1920
    auto getScreen = []() {
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
        return QGuiApplication::screenAt(utilqt::getApplicationMainWindow()->pos());
#else
        return utilqt::getApplicationMainWindow()->screen();
#endif
    };

    if (getScreen()->size().width() < 1200) {
        this->setSizes(QList<int>({1, 0}));
    }

    setTabOrder(filterLineEdit_, workspaceGridViewArea_);
    setTabOrder(workspaceGridViewArea_, workspaceTreeView_);
    setTabOrder(workspaceTreeView_, loadWorkspaceBtn_);
    setTabOrder(details_, changelog_);

    changelog_->loadLog();
}

void WelcomeWidget::updateRecentWorkspaces(const QStringList& list) {
    workspaceModel_->updateRecentWorkspaces(list);
}

void WelcomeWidget::enableRestoreButton(bool hasRestoreWorkspace) {
    restoreButton_->setEnabled(hasRestoreWorkspace);
}

void WelcomeWidget::setFilterFocus() { filterLineEdit_->setFocus(Qt::OtherFocusReason); }

void WelcomeWidget::showEvent(QShowEvent* event) {
    if (!event->spontaneous()) {
        QModelIndex index = workspaceSelectionModel_->currentIndex();
        workspaceModel_->updateExampleEntries();
        workspaceModel_->updateRegressionTestEntries();

        workspaceTreeView_->expandItems();

        if (index.isValid()) {
            workspaceSelectionModel_->setCurrentIndex(
                index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
        } else {
            workspaceTreeView_->selectRecentWorkspace(0);
        }
    }
    QWidget::showEvent(event);
}

void WelcomeWidget::keyPressEvent(QKeyEvent* event) {
    if ((event->key() >= Qt::Key_0) && (event->key() <= Qt::Key_9)) {
        int number = [key = event->key()]() {
            if (key == Qt::Key_0) return 9;
            return key - Qt::Key_1;
        }();
        if (workspaceTreeView_->selectRecentWorkspace(number)) {
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

void WelcomeWidget::updateDetails(const QString& filename) {
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
            LogFilter logger{LogCentral::getPtr(), LogVerbosity::None};
            auto d = app_->getWorkspaceManager()->createWorkspaceDeserializer(
                istream, utilqt::fromQString(filename), &logger);
            d.setExceptionHandler([](const ExceptionContext) {});
            d.deserialize("WorkspaceAnnotations", annotations);
        } else {
            fileBroken = true;
        }
    } catch (Exception& e) {
        util::log(e.getContext(), e.getMessage(), LogLevel::Warn);
        fileBroken = "true";
    }

    const auto dateformat = "yyyy-MM-dd hh:mm:ss";
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
    auto createdStr = utilqt::fromQString(info.created().toString(dateformat));
#else
    auto createdStr = utilqt::fromQString(info.birthTime().toString(dateformat));
#endif
    auto modifiedStr = utilqt::fromQString(info.lastModified().toString(dateformat));

    const bool hasTitle = !annotations.getTitle().empty();
    auto titleStr =
        hasTitle ? annotations.getTitle() : utilqt::fromQString(info.completeBaseName());
    auto description = htmlEncode(annotations.getDescription());
    replaceInString(description, "\n", "<br/>");

    if (titleStr.size() > 0) titleStr[0] = static_cast<char>(std::toupper(titleStr[0]));
    replaceInString(titleStr, "-", " ");
    replaceInString(titleStr, "_", " ");

    Document doc;
    auto body = doc.append("html").append("body");
    body.append("h2", titleStr,
                {{"style", "font-size:45pt; color:#268bd2; margin-top:0em; padding-top:0em;"}});

    auto table = body.append("table", "", {{"style", "margin-bottom:1em;"}});
    auto addrow = [&](std::string_view name, std::string_view value) {
        if (value.empty()) return;
        auto tr = table.append("tr");
        tr.append("td", name, {{"style", "text-align:right;"}});
        tr.append("td", htmlEncode(value), {{"style", "color:#ebe5df; font-weight:500;"}});
    };

    addrow("File", utilqt::fromQString(info.fileName()));
    addrow("Modified", modifiedStr);
    addrow("Created", createdStr);
    addrow("Author", annotations.getAuthor());
    addrow("Tags", annotations.getTags());
    addrow("Categories", annotations.getCategories());
    addrow("Description", description);

    if (fileBroken) {
        body.append("h3", "Workspace file could not be opened!",
                    {{"style", "color:firebrick;font-weight:600;"}});
    }
    auto content = body.append("span");
    auto addImage = [&content](auto item) {
        const int maxImgWidth = 128;
        if (!item.isValid()) return;
        auto img = content.append("table", "", {{"style", "float: left;"}});
        img.append("tr").append("td").append("h3", item.name, {{"style", "font-weight:600;"}});
        img.append("tr").append("td").append(
            "img", "",
            {{"width", std::to_string(std::min(maxImgWidth, item.size.x))},
             {"src", "data:image/jpeg;base64," + item.base64jpeg}});
    };

    for (auto& elem : annotations.getCanvasImages()) {
        addImage(elem);
    }
    addImage(annotations.getNetworkImage());

    details_->setHtml(utilqt::toQString(doc));
}

QModelIndex WelcomeWidget::findFirstLeaf(QModelIndex parent) const {
    if (!workspaceProxyModel_->hasChildren(parent)) return parent;
    return findFirstLeaf(workspaceProxyModel_->index(0, 0, parent));
}

}  // namespace inviwo
