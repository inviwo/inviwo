/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2023 Inviwo Foundation
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
#include <inviwo/core/algorithm/searchdsl.h>

#include <inviwo/qt/editor/workspacetreemodel.h>
#include <inviwo/qt/editor/workspacetreeview.h>
#include <inviwo/qt/editor/workspacegridview.h>
#include <inviwo/qt/editor/workspaceannotationsqt.h>
#include <inviwo/qt/editor/lineediteventfilter.h>
#include <inviwo/qt/editor/verticallabel.h>
#include <modules/qtwidgets/inviwoqtutils.h>
#include <modules/qtwidgets/keyboardutils.h>

#include <QScreen>
#include <QTabWidget>
#include <QToolBar>
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
#include <QTextEdit>
#include <QStackedWidget>
#include <QKeyEvent>
#include <QAction>
#include <QFile>
#include <QImage>
#include <QScrollArea>
#include <QSortFilterProxyModel>
#include <QSvgWidget>
#include <QSvgRenderer>
#include <QItemSelectionModel>
#include <QApplication>
#include <QSplitterHandle>
#include <QMainWindow>

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

using Role = WorkspaceTreeModel::Role;

namespace {

auto matcher = [](Role role) {
    return [role](std::string_view str, const QModelIndex& i) -> bool {
        return utilqt::getData(i, role).toString().contains(utilqt::toQString(str),
                                                            Qt::CaseInsensitive);
    };
};

}

class WorkspaceFilter : public QSortFilterProxyModel {
public:
    WorkspaceFilter(QObject* parent = nullptr)
        : QSortFilterProxyModel(parent)
        , dsl_{{{"file", "f", "file name", true, matcher(Role::Name)},
                {"path", "/", "file path", true, matcher(Role::Path)},
                {"title", "t", "workspace title", true, matcher(Role::Title)},
                {"author", "a", "workspace author", true, matcher(Role::Author)},
                {"tags", "#", "workspace tags", true, matcher(Role::Tags)},
                {"category", "c", "workspace category", true, matcher(Role::Categories)},
                {"processors", "p",
                 "search processor identifiers, display names, and class identifiers", false,
                 [](std::string_view str, const QModelIndex& i) -> bool {
                     const auto list = utilqt::getData(i, Role::Processors).toStringList();
                     const auto qstr = utilqt::toQString(str);
                     for (const auto& item : list) {
                         if (item.contains(qstr, Qt::CaseInsensitive)) {
                             return true;
                         }
                     }
                     return false;
                 }}}} {}

    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override {
        auto index = sourceModel()->index(source_row, 0, source_parent);
        return dsl_.match(index);
    };

    void setCustomFilter(const QString filter) {
        if (dsl_.setSearchString(utilqt::fromQString(filter))) {
            invalidateFilter();
        }
    }

    Document description() const {
        using P = Document::PathComponent;
        auto doc = dsl_.description();
        auto b = doc.get({P{"html"}, P{"body"}});
        auto desc = b.insert(P::first(), "div");
        desc.append("b", "Search Workspaces", {{"style", "color:white;"}});
        desc.append("p", "Example: processors:\"Volume Raycaster\" author:Martin");
        return doc;
    }

private:
    SearchDSL<QModelIndex> dsl_;
};

WelcomeWidget::WelcomeWidget(InviwoApplication* app, QWidget* parent)
    : QSplitter(parent)
    , app_(app)
    , model_(new WorkspaceTreeModel(app, this))
    , filterModel_{new WorkspaceFilter{this}} {

    setObjectName("WelcomeWidget");

    filterModel_->setSourceModel(model_);
    filterModel_->setRecursiveFilteringEnabled(true);
    filterModel_->setFilterCaseSensitivity(Qt::CaseInsensitive);

    const auto space = utilqt::refSpacePx(this);
    setOrientation(Qt::Horizontal);
    setHandleWidth(space);
    setContentsMargins(0, 0, 0, 0);

    {  // Left splitter pane ( caption | splitter ( filetree | details ) )
        auto leftWidget = new QWidget();
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

        workspaceSplitter_ = new QSplitter(leftWidget);
        workspaceSplitter_->setObjectName("WelcomeWidget");
        workspaceSplitter_->setOrientation(Qt::Horizontal);
        workspaceSplitter_->setHandleWidth(space / 2);
        workspaceSplitter_->setContentsMargins(0, 0, 0, 0);
        leftLayout->addWidget(workspaceSplitter_);
        workspaceSplitter_->setSizePolicy(QSizePolicy::MinimumExpanding,
                                          QSizePolicy::MinimumExpanding);

        {  // left column: workspace filter, list of recently used workspaces, and examples
            auto loadFile = [this](const QString& filename, bool isExample) {
                const auto action = util::getModifierAction(QApplication::keyboardModifiers());
                const auto file = utilqt::toPath(filename);
                switch (action) {
                    case ModifierAction::AppendWorkspace:
                        emit appendWorkspace(file);
                        break;
                    case ModifierAction::OpenWithPath:
                        emit loadWorkspace(file, false);
                        break;
                    case ModifierAction::None:
                    default:
                        emit loadWorkspace(file, isExample);
                        break;
                }
            };
            auto updateLoadButtons = [this](const QModelIndex& index) {
                updateDetails(index);
                loadWorkspaceBtn_->disconnect();
                appendWorkspaceBtn_->disconnect();

                if (index.isValid()) {
                    const auto filename = utilqt::getData(index, Role::FilePath).toString();
                    const auto isExample = utilqt::getData(index, Role::isExample).toBool();
                    const auto file = utilqt::toPath(filename);

                    QObject::connect(
                        loadWorkspaceBtn_, &QToolButton::clicked, this,
                        [this, file, isExample]() { emit loadWorkspace(file, isExample); });
                    QObject::connect(appendWorkspaceBtn_, &QToolButton::clicked, this,
                                     [this, file]() { emit appendWorkspace(file); });
                }
            };

            workspaceTreeView_ = new WorkspaceTreeView(filterModel_, this);
            workspaceTreeView_->setMinimumWidth(utilqt::emToPx(this, 25));
            workspaceTreeView_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            workspaceTreeView_->setVisible(!getSetting("showWorkSpaceGridView", true).toBool());

            QObject::connect(workspaceTreeView_, &WorkspaceTreeView::loadFile, this, loadFile);
            QObject::connect(workspaceTreeView_, &WorkspaceTreeView::selectFile, this,
                             updateLoadButtons);

            workspaceGridView_ = new WorkspaceGridView(filterModel_, this);
            workspaceGridView_->setMinimumWidth(utilqt::emToPx(this, 25));
            workspaceGridView_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            workspaceGridView_->setVisible(getSetting("showWorkSpaceGridView", true).toBool());

            QObject::connect(workspaceGridView_, &WorkspaceGridView::loadFile, this, loadFile);
            QObject::connect(workspaceGridView_, &WorkspaceGridView::selectFile, this,
                             updateLoadButtons);

            auto toolbarLayout = new QHBoxLayout();
            filterLineEdit_ = new QLineEdit();
            filterLineEdit_->setPlaceholderText("Search for Workspace...");
            filterLineEdit_->installEventFilter(
                new LineEditEventFilter(workspaceTreeView_, filterLineEdit_, false));
            filterLineEdit_->installEventFilter(
                new LineEditEventFilter(workspaceGridView_, filterLineEdit_, false));
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
                        filterModel_->setCustomFilter(str);

                        expandTreeView();
                        selectFirstLeaf();

                        clearAction->setVisible(!str.isEmpty());
                    });
            filterLineEdit_->setToolTip(utilqt::toQString(filterModel_->description()));

            toolbarLayout->addWidget(filterLineEdit_);

            auto horizontalSpacer = new QSpacerItem(utilqt::emToPx(this, 1.0), 0,
                                                    QSizePolicy::Minimum, QSizePolicy::Minimum);
            toolbarLayout->addItem(horizontalSpacer);

            auto expandCollapse = QIcon();
            expandCollapse.addFile(":/svgicons/treelist-expand.svg", QSize(), QIcon::Normal,
                                   QIcon::Off);
            expandCollapse.addFile(":/svgicons/treelist-collapse.svg", QSize(), QIcon::Normal,
                                   QIcon::On);

            auto createToolButton = [&](auto text, QIcon icon, auto callback) {
                auto btn = new QToolButton();
                btn->setToolTip(text);
                btn->setIcon(icon);
                btn->setToolButtonStyle(Qt::ToolButtonIconOnly);
                btn->setIconSize(utilqt::emToPx(this, QSizeF(2.5, 2.5)));
                connect(btn, &QToolButton::clicked, this, callback);
                return btn;
            };

            auto expandCollapseBtn =
                createToolButton("Expand/Collapse", expandCollapse, [this](bool expand) {
                    if (expand) {
                        workspaceGridView_->expandRecursively({});
                        workspaceTreeView_->expandRecursively({});
                    } else {
                        workspaceGridView_->collapseAll();
                        workspaceTreeView_->collapseAll();
                    }
                });
            expandCollapseBtn->setCheckable(true);
            auto gridListBtn =
                createToolButton("Workspace Grid View", QIcon(":/svgicons/gridlist.svg"), [this]() {
                    setSetting("showWorkSpaceGridView", true);
                    workspaceGridView_->setVisible(true);
                    workspaceTreeView_->setVisible(false);
                });
            auto treeViewBtn = createToolButton("Workspace Tree View",
                                                QIcon(":svgicons/detaillist.svg"), [this]() {
                                                    setSetting("showWorkSpaceGridView", false);
                                                    workspaceGridView_->setVisible(false);
                                                    workspaceTreeView_->setVisible(true);
                                                });
            toolbarLayout->addWidget(expandCollapseBtn);
            toolbarLayout->addWidget(gridListBtn);
            toolbarLayout->addWidget(treeViewBtn);

            auto fileTreeLayout = new QVBoxLayout();
            fileTreeLayout->addLayout(toolbarLayout);
            fileTreeLayout->addWidget(workspaceTreeView_);
            fileTreeLayout->addWidget(workspaceGridView_);
            auto filetreeWidget = new QWidget();
            filetreeWidget->setLayout(fileTreeLayout);
            fileTreeLayout->setContentsMargins(2 * space, 0, 0, 0);
            workspaceSplitter_->addWidget(filetreeWidget);
        }

        {  // center column:
            // stacked widget with drag&drop hint/workspace details
            // and buttons for loading workspaces
            centerStackedWidget_ = new QStackedWidget();
            centerStackedWidget_->setContentsMargins(space, 0, 0, 2 * space);

            {
                auto dragAndDrop = new QSvgWidget(QString(":/svgicons/dragndrop.svg"));
                dragAndDrop->setToolTip("Drag & drop data here to load");
                dragAndDrop->setObjectName("dragndrop");
                dragAndDrop->renderer()->setAspectRatioMode(Qt::AspectRatioMode::KeepAspectRatio);
                dragAndDrop->setMaximumHeight(600);
                auto dragAndDropLayout = new QVBoxLayout();
                dragAndDropLayout->addStretch();
                dragAndDropLayout->addWidget(dragAndDrop);
                dragAndDropLayout->addStretch();
                auto w = new QWidget();
                w->setLayout(dragAndDropLayout);

                centerStackedWidget_->addWidget(w);
            }

            details_ = new QTextEdit(leftWidget);
            details_->setObjectName("NetworkDetails");
            details_->setReadOnly(true);
            details_->setFrameShape(QFrame::NoFrame);
            details_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
            details_->setContentsMargins(0, 0, 0, 0);
            details_->document()->setDocumentMargin(0.0);
            details_->setVisible(false);

            centerStackedWidget_->addWidget(details_);

            auto buttonLayout = new QHBoxLayout();
            buttonLayout->setSpacing(6);
            auto createButton = [&](const QString& str, const QString& iconpath,
                                    const QString& toolTip, const QString& objName) {
                auto button = new QToolButton(leftWidget);
                button->setText(str);
                button->setIcon(QIcon(iconpath));
                button->setIconSize(utilqt::emToPx(this, QSize(7, 7)));
                button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
                button->setToolTip(toolTip);
                button->setObjectName(objName);
                buttonLayout->addWidget(button);
                return button;
            };

            loadWorkspaceBtn_ = createButton("Load", ":/svgicons/open.svg",
                                             "Open selected workspace", "LoadWorkspaceToolButton");
            loadWorkspaceBtn_->setEnabled(false);
            appendWorkspaceBtn_ = createButton("Append", ":/svgicons/open-append.svg",
                                               "Append selected workspace to current one",
                                               "AppendWorkspaceToolButton");
            appendWorkspaceBtn_->setEnabled(false);
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

            setTabOrder(loadWorkspaceBtn_, appendWorkspaceBtn_);
            setTabOrder(appendWorkspaceBtn_, newButton);
            setTabOrder(newButton, openFileButton);
            setTabOrder(openFileButton, restoreButton_);
            setTabOrder(restoreButton_, details_);

            auto centerWidget = new QWidget();
            auto centerLayout = new QVBoxLayout();
            centerLayout->addWidget(centerStackedWidget_);
            centerLayout->addLayout(buttonLayout);
            centerLayout->setContentsMargins(space, 0, 0, 2 * space);
            centerLayout->setSpacing(2 * space);
            centerWidget->setLayout(centerLayout);
            centerWidget->setSizePolicy(QSizePolicy::MinimumExpanding,
                                        QSizePolicy::MinimumExpanding);

            workspaceSplitter_->addWidget(centerWidget);
        }
        workspaceSplitter_->setStretchFactor(0, 2);  // FileTree
        workspaceSplitter_->setStretchFactor(1, 3);  // Center widget
        workspaceSplitter_->setCollapsible(0, false);
        workspaceSplitter_->setCollapsible(1, false);
        workspaceSplitter_->handle(1)->setAttribute(Qt::WA_Hover);
    }
    {  // right splitter pane: changelog / options
        auto rightColumn = new QFrame(this);
        rightColumn->setContentsMargins(0, 0, 0, 0);

        addWidget(rightColumn);
        setStretchFactor(0, 2);  // left + center widget (workspace view + details)
        setStretchFactor(1, 1);  // rightColumn (changelog)
        rightColumn->setObjectName("WelcomeRightColumn");
        auto rightColumnLayout = new QVBoxLayout(rightColumn);
        rightColumnLayout->setContentsMargins(space, 0, 0, 3 * space);

        {
            auto title = new QLabel("Latest Changes", rightColumn);
            title->setObjectName("ChangeLog");
            rightColumnLayout->addWidget(title);

            changelog_ = new ChangeLog(rightColumn);
            QSizePolicy sizePolicy1(changelog_->sizePolicy());
            sizePolicy1.setVerticalStretch(100);
            changelog_->setSizePolicy(sizePolicy1);
            rightColumnLayout->addWidget(changelog_);
        }

        rightColumnLayout->addItem(
            new QSpacerItem(space, space, QSizePolicy::Minimum, QSizePolicy::Fixed));

        {
            auto autoloadWorkspace = new QCheckBox("&Auto-load last workspace", rightColumn);
            autoloadWorkspace->setChecked(getSetting("autoloadLastWorkspace", false).toBool());
            QObject::connect(autoloadWorkspace, &QCheckBox::toggled, this, [this](bool checked) {
                setSetting("autoloadLastWorkspace", checked);
            });
            rightColumnLayout->addWidget(autoloadWorkspace);

            auto showOnStartup = new QCheckBox("&Show this page on startup", rightColumn);
            showOnStartup->setChecked(getSetting("showWelcomePage", true).toBool());
            QObject::connect(showOnStartup, &QCheckBox::toggled, this,
                             [this](bool checked) { setSetting("showWelcomePage", checked); });
            rightColumnLayout->addWidget(showOnStartup);

            setTabOrder(changelog_, autoloadWorkspace);
            setTabOrder(autoloadWorkspace, showOnStartup);
        }
    }
    // ensure that the splitter handle responds to hover events
    // see https://bugreports.qt.io/browse/QTBUG-13768
    handle(1)->setAttribute(Qt::WA_Hover);

    // hide changelog on screens with less width than 1400
    auto getScreen = []() { return utilqt::getApplicationMainWindow()->screen(); };

    if (getScreen()->size().width() < 1400) {
        setSizes(QList<int>({1, 0}));
    }

    restoreState(getSetting("changelogSplitter").toByteArray());
    // update internal status of splitter handle since restoreState() does not trigger
    // splitterMoved() itself
    splitterMoved(sizes()[0], 1);
    workspaceSplitter_->restoreState(getSetting("workspaceSplitter").toByteArray());

    setTabOrder(filterLineEdit_, workspaceTreeView_);
    setTabOrder(workspaceTreeView_, workspaceGridView_);
    setTabOrder(workspaceGridView_, loadWorkspaceBtn_);
    setTabOrder(details_, changelog_);

    setFocusProxy(filterLineEdit_);
}

void WelcomeWidget::updateRecentWorkspaces(const QStringList& list) {
    model_->updateRecentWorkspaces(list);
}

void WelcomeWidget::enableRestoreButton(bool hasRestoreWorkspace) {
    restoreButton_->setEnabled(hasRestoreWorkspace);
}

void WelcomeWidget::selectFirstLeaf() {
    // select first leaf node
    QTreeView* view = workspaceGridView_->isVisible() ? static_cast<QTreeView*>(workspaceGridView_)
                                                      : static_cast<QTreeView*>(workspaceTreeView_);
    auto gridIndex = findFirstLeaf(view->model());
    view->selectionModel()->setCurrentIndex(gridIndex, QItemSelectionModel::ClearAndSelect);

    // Seems we don't get any notification about the current item changing when we clear it.
    if (!gridIndex.isValid()) updateDetails(QModelIndex{});
}

void WelcomeWidget::expandTreeView() const {
    auto expandSection = [&](std::string_view section) {
        auto modelIdx = model_->getCategoryIndex(section);
        if (auto filterIdx = filterModel_->mapFromSource(modelIdx); filterIdx.isValid()) {
            workspaceTreeView_->expandRecursively(filterIdx);

            if (auto proxyIdx = workspaceGridView_->proxy().mapFromSource(filterIdx);
                proxyIdx.isValid()) {
                workspaceGridView_->expandRecursively(proxyIdx);
            }
        }
    };

    if (filterLineEdit_->text().isEmpty()) {
        expandSection(WorkspaceTreeModel::recent);
        expandSection(WorkspaceTreeModel::examples);
    } else {
        workspaceTreeView_->expandAll();
        workspaceGridView_->expandAll();
    }
}

void WelcomeWidget::showEvent(QShowEvent* event) {
    if (!event->spontaneous()) {
        try {
            model_->updateRestoreEntries();
            model_->updateCustomEntries();
            model_->updateExampleEntries();
            model_->updateRegressionTestEntries();
        } catch (const Exception& e) {
            util::log(e.getContext(), e.getMessage(), LogLevel::Error);
        } catch (const std::exception& e) {
            util::log(IVW_CONTEXT, e.what(), LogLevel::Error);
        }
        expandTreeView();
    }

    QSplitter::showEvent(event);
    filterLineEdit_->setFocus(Qt::OtherFocusReason);
}

void WelcomeWidget::hideEvent(QHideEvent* event) {
    setSetting("changelogSplitter", saveState());
    setSetting("workspaceSplitter", workspaceSplitter_->saveState());

    QSplitter::hideEvent(event);
}

void WelcomeWidget::keyPressEvent(QKeyEvent* event) {
    if ((event->key() >= Qt::Key_0) && (event->key() <= Qt::Key_9) &&
        event->modifiers().testFlag(Qt::ControlModifier)) {
        int number = [key = event->key()]() {
            if (key == Qt::Key_0) return 9;
            return key - Qt::Key_1;
        }();
        auto index = model_->getCategoryIndex(WorkspaceTreeModel::recent);
        if (auto selIndex = filterModel_->mapFromSource(index); selIndex.isValid()) {
            if (auto wsIndex = filterModel_->index(number, 0, selIndex); wsIndex.isValid()) {
                const auto filename = utilqt::getData(wsIndex, Role::FilePath).toString();
                const auto isExample = utilqt::getData(wsIndex, Role::isExample).toBool();
                emit loadWorkspace(utilqt::toPath(filename), isExample);
            }
            event->accept();
        }
    } else if ((event->key() == Qt::Key_Return) || (event->key() == Qt::Key_Enter)) {
        auto action = util::getModifierAction(QApplication::queryKeyboardModifiers());
        if (action == ModifierAction::AppendWorkspace) {
            appendWorkspaceBtn_->animateClick();
        } else {
            loadWorkspaceBtn_->animateClick();
        }
        event->accept();
    }
    QWidget::keyPressEvent(event);
}

namespace {
std::string formatTitle(std::string str) {
    if (!str.empty()) str[0] = static_cast<char>(std::toupper(str[0]));
    replaceInString(str, "-", " ");
    replaceInString(str, "_", " ");
    return str;
}

std::string formatDescription(std::string_view str) {
    auto string = util::htmlEncode(str);
    replaceInString(string, "\n", "<br/>");
    return string;
}
}  // namespace

void WelcomeWidget::updateDetails(const QModelIndex& index) {
    loadWorkspaceBtn_->setEnabled(index.isValid());
    appendWorkspaceBtn_->setEnabled(index.isValid());
    if (!index.isValid()) {
        centerStackedWidget_->setCurrentIndex(0);
        details_->clear();
        return;
    }

    const auto filename = utilqt::getData(index, Role::FilePath).toString();
    const QFileInfo info(filename);

    // extract annotations including network screenshot and canvas images from workspace
    auto path = utilqt::toPath(filename);
    std::optional<WorkspaceAnnotationsQt> annotations;
    try {
        annotations.emplace(path, app_);
    } catch (Exception&) {
    }

    const auto dateFormat = "yyyy-MM-dd hh:mm:ss";
    const auto createdStr = utilqt::fromQString(info.birthTime().toString(dateFormat));
    const auto modifiedStr = utilqt::fromQString(info.lastModified().toString(dateFormat));
    const auto titleStr = (annotations && !annotations->getTitle().empty())
                              ? annotations->getTitle()
                              : formatTitle(utilqt::fromQString(info.completeBaseName()));
    const auto description =
        annotations ? formatDescription(annotations->getDescription()) : std::string{};

    Document doc;
    auto body = doc.append("html").append("body");
    body.append("div", titleStr,
                {{"style", "font-size:45pt; color:#268bd2; margin-top:0em; padding-top:0em;"}});

    auto table = body.append("table", "", {{"style", "margin-bottom:1em;"}});
    auto addRow = [&](std::string_view name, std::string_view value) {
        if (value.empty()) return;
        auto tr = table.append("tr");
        tr.append("td", name, {{"style", "text-align:right;padding-right:0.4em"}});
        tr.append("td", util::htmlEncode(value), {{"style", "color:#ebe5df; font-weight:500;"}});
    };

    addRow("File", utilqt::fromQString(info.fileName()));
    addRow("Path", utilqt::fromQString(info.path()));
    addRow("Modified", modifiedStr);
    addRow("Created", createdStr);
    if (annotations) {
        addRow("Author", annotations->getAuthor());
        addRow("Tags", annotations->getTags());
        addRow("Categories", annotations->getCategories());
        addRow("Description", description);
    } else {
        body.append("h3", "Workspace file could not be opened!",
                    {{"style", "color:firebrick;font-weight:600;"}});
    }

    auto content = body.append("div").append("span");
    const int maxImageSize = utilqt::emToPx(this, 32);
    auto addImage = [&content, maxImageSize](auto item) {
        if (!item.isValid()) return;
        auto img = content.append("table", "", {{"style", "float: left;"}});
        img.append("tr").append("td").append("h3", item.name, {{"style", "font-weight:600;"}});
        img.append("tr").append("td").append(
            "img", "",
            {[&]() -> std::pair<const std::string, std::string> {
                 if (item.size.x > item.size.y) {
                     return {"width", std::to_string(std::min(maxImageSize, item.size.x))};
                 } else {
                     return {"height", std::to_string(std::min(maxImageSize, item.size.y))};
                 }
             }(),
             {"src", "data:image/jpeg;base64," + item.base64jpeg}});
    };

    if (annotations) {
        for (auto& elem : annotations->getCanvasImages()) {
            addImage(elem);
        }
        addImage(annotations->getNetworkImage());
    }

    try {
        auto processorCounts = WorkspaceAnnotationsQt::workspaceProcessorsCounts(path, app_);

        auto processorList = content.append("tabel", "", {{"style", "float: left;"}});
        processorList.append("tr").append("td").append("h3", "Processors",
                                                       {{"style", "font-weight:600;"}});
        if (!processorCounts.empty()) {
            auto pTable = processorList.append("tr").append("td").append(
                "table", "", {{"style", "min-width:15em"}});
            for (auto&& [name, count] : processorCounts) {
                auto tr = pTable.append("tr");
                tr.append("td", util::htmlEncode(name),
                          {{"style", "text-align:left;padding-right:1em"}});
                tr.append("td", fmt::to_string(count), {{"style", "font-weight:500;"}});
            }
        } else {
            processorList.append("tr").append("td", "Network is empty");
        }
    } catch (Exception&) {
    }

    details_->setHtml(utilqt::toQString(doc));
    centerStackedWidget_->setCurrentIndex(1);
}

QModelIndex WelcomeWidget::findFirstLeaf(QAbstractItemModel* model, QModelIndex parent) {
    if (!model->hasChildren(parent)) return parent;
    return findFirstLeaf(model, model->index(0, 0, parent));
}

WelcomeWidget& WelcomeWidget::setSetting(const QString& key, const QVariant& value) {
    QSettings settings;
    settings.beginGroup(objectName());
    settings.setValue(key, value);
    settings.endGroup();
    return *this;
}
QVariant WelcomeWidget::getSetting(const QString& key, const QVariant& defaultValue) const {
    QSettings settings;
    settings.beginGroup(objectName());
    auto value = settings.value(key, defaultValue);
    settings.endGroup();
    return value;
}

namespace detail {

class ChangelogSplitterHandle : public QSplitterHandle {
public:
    ChangelogSplitterHandle(Qt::Orientation orientation, QSplitter* parent);
    virtual QSize sizeHint() const override;
    virtual void mouseDoubleClickEvent(QMouseEvent*) override { collapseButton_->click(); }

private:
    QLabel* label_;
    QToolButton* collapseButton_;
    int prevWidth_;
    bool collapsed_;
};

ChangelogSplitterHandle::ChangelogSplitterHandle(Qt::Orientation orientation, QSplitter* parent)
    : QSplitterHandle{orientation, parent}, prevWidth_{250}, collapsed_{false} {
    const auto space = utilqt::refSpacePx(this);

    label_ = (this->orientation() == Qt::Horizontal) ? new VerticalLabel() : new QLabel();
    label_->setObjectName("splitterHandleTitle");
    label_->setSizePolicy(QSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed));
    label_->setMinimumHeight(20);
    label_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    if (collapsed_) {
        label_->setText("Latest Changes");
    }
    collapseButton_ = new QToolButton(this);
    collapseButton_->setObjectName("collapseButton");
    QIcon icon;
    icon.addFile(":/svgicons/arrow-right-enabled.svg", QSize(), QIcon::Mode::Active,
                 QIcon::State::Off);
    icon.addFile(":/svgicons/arrow-left-enabled.svg", QSize(), QIcon::Mode::Active,
                 QIcon::State::On);
    collapseButton_->setIcon(icon);
    collapseButton_->setCheckable(true);
    collapseButton_->setChecked(collapsed_);
    collapseButton_->setFixedSize(QSize(19, 19));
    collapseButton_->setCursor(QCursor(Qt::ArrowCursor));

    auto l = new QVBoxLayout();
    l->setContentsMargins(0, 0, 0, 0);
    l->setSpacing(space);
    l->setSizeConstraint(QLayout::SetMinimumSize);
    l->addStretch(2);
    l->addWidget(label_);
    l->addWidget(collapseButton_);
    l->addStretch(2);
    setLayout(l);

    connect(parent, &QSplitter::splitterMoved, this, [this](int, int index) {
        auto s = splitter();
        if (const int idx = s->indexOf(this); idx != index) return;

        const bool collapsed = (s->sizes()[index] == 0);
        if (collapsed_ != collapsed) {
            collapsed_ = collapsed;
            collapseButton_->setChecked(collapsed_);
            label_->setText(collapsed_ ? "Latest Changes" : "");
        }
    });

    connect(collapseButton_, &QToolButton::clicked, this, [this]() {
        auto s = splitter();
        const int index = s->indexOf(this);

        const bool isCollapsed = (s->sizes()[index] == 0);
        if (isCollapsed == collapsed_) {
            if (!collapsed_) {
                prevWidth_ = s->sizes()[index];
            }
            ivec2 range(0);
            s->getRange(index, &range.x, &range.y);
            const int newWidth = !collapsed_ ? 0 : prevWidth_;
            moveSplitter(range.y - newWidth);
        }
    });
}

QSize ChangelogSplitterHandle::sizeHint() const {
    QSize size = QSplitterHandle::sizeHint();
    // ensure the label fits
    if (orientation() == Qt::Horizontal) {
        size.rwidth() = label_->width();
    } else {
        size.rheight() = label_->width();
    }
    return size;
}

}  // namespace detail

QSplitterHandle* WelcomeWidget::createHandle() {
    return new detail::ChangelogSplitterHandle(orientation(), this);
}

}  // namespace inviwo
