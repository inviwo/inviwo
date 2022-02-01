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
#include <inviwo/core/algorithm/seachdsl.h>

#include <inviwo/qt/editor/workspacetreemodel.h>
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
#include <QSortFilterProxyModel>
#include <QItemSelectionModel>

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

using Role = WorkspaceTreeModel::Role;

namespace {

auto matcher = [](Role role) {
    return [role](const QModelIndex& i, std::string_view str) -> bool {
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
                 [](const QModelIndex& i, std::string_view str) -> bool {
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

    QSplitter* leftSplitter = nullptr;

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

        leftSplitter = new QSplitter(leftWidget);
        leftSplitter->setObjectName("WelcomeWidget");
        leftSplitter->setOrientation(Qt::Horizontal);
        leftSplitter->setHandleWidth(space / 2);
        leftSplitter->setContentsMargins(0, 0, 0, 0);
        leftLayout->addWidget(leftSplitter);
        leftSplitter->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

        {  // left column: workspace filter, list of recently used workspaces, and examples
            auto load = [this](const QString& filename, bool isExample) {
                updateDetails(filename);
                loadWorkspaceBtn_->disconnect();
                QObject::connect(
                    loadWorkspaceBtn_, &QToolButton::clicked, this,
                    [this, filename, isExample]() { emit loadWorkspace(filename, isExample); });
            };

            workspaceTreeView_ = new WorkspaceTreeView(filterModel_, this);
            workspaceTreeView_->setMinimumWidth(utilqt::emToPx(this, 25));
            workspaceTreeView_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            workspaceTreeView_->setVisible(!getSetting("showWorkSpaceGridView", true).toBool());

            QObject::connect(workspaceTreeView_, &WorkspaceTreeView::loadFile, this,
                             &WelcomeWidget::loadWorkspace);
            QObject::connect(workspaceTreeView_, &WorkspaceTreeView::selectFile, this, load);

            workspaceGridView_ = new WorkspaceGridView(filterModel_, this);
            workspaceGridView_->setMinimumWidth(utilqt::emToPx(this, 25));
            workspaceGridView_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            workspaceGridView_->setVisible(getSetting("showWorkSpaceGridView", true).toBool());

            QObject::connect(workspaceGridView_, &WorkspaceGridView::loadFile, this,
                             &WelcomeWidget::loadWorkspace);
            QObject::connect(workspaceGridView_, &WorkspaceGridView::selectFile, this, load);

            auto toolbarLayout = new QHBoxLayout();
            filterLineEdit_ = new QLineEdit();
            filterLineEdit_->setPlaceholderText("Search for Workspace...");
            filterLineEdit_->installEventFilter(
                new LineEditEventFilter(workspaceTreeView_, filterLineEdit_));
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

            auto viewsSelectionButtons = new QToolBar();
            toolbarLayout->addWidget(viewsSelectionButtons);

            auto expandCollapse = QIcon();
            expandCollapse.addFile(":/svgicons/treelist-expand.svg", QSize(), QIcon::Normal,
                                   QIcon::Off);
            expandCollapse.addFile(":/svgicons/treelist-collapse.svg", QSize(), QIcon::Normal,
                                   QIcon::On);

            viewsSelectionButtons
                ->addAction(expandCollapse, "Workspace Tree List",
                            [this](bool expand) {
                                if (expand) {
                                    workspaceGridView_->expandRecursively({});
                                    workspaceTreeView_->expandRecursively({});
                                } else {
                                    workspaceGridView_->collapseAll();
                                    workspaceTreeView_->collapseAll();
                                }
                            })
                ->setCheckable(true);

            viewsSelectionButtons->addAction(QIcon(":/svgicons/gridlist.svg"),
                                             "Workspace Grid List", [this]() {
                                                 setSetting("showWorkSpaceGridView", true);
                                                 workspaceGridView_->setVisible(true);
                                                 workspaceTreeView_->setVisible(false);
                                             });
            viewsSelectionButtons->addAction(QIcon(":svgicons/detaillist.svg"),
                                             "Workspace Tree List", [this]() {
                                                 setSetting("showWorkSpaceGridView", false);
                                                 workspaceGridView_->setVisible(false);
                                                 workspaceTreeView_->setVisible(true);
                                             });

            auto fileTreeLayout = new QVBoxLayout();
            fileTreeLayout->addLayout(toolbarLayout);
            fileTreeLayout->addWidget(workspaceTreeView_);
            fileTreeLayout->addWidget(workspaceGridView_);
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
                button->setIconSize(utilqt::emToPx(this, QSize(7, 7)));
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
        leftSplitter->setStretchFactor(0, 1);
        leftSplitter->setStretchFactor(1, 1.5);
        leftSplitter->handle(1)->setAttribute(Qt::WA_Hover);
    }
    {  // right splitter pane: changelog / options
        auto rightColumn = new QFrame(this);
        addWidget(rightColumn);
        setStretchFactor(0, 2);  // left + center widget (workspace view + details)
        setStretchFactor(1, 1);  // rightColumn (changelog)
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
            auto autoloadWorkspace = new QCheckBox("&Auto-load last workspace", rightColumn);
            autoloadWorkspace->setChecked(getSetting("autoloadLastWorkspace", true).toBool());
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

    // hide changelog on screens with less width than 1920
    auto getScreen = []() {
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
        return QGuiApplication::screenAt(utilqt::getApplicationMainWindow()->pos());
#else
        return utilqt::getApplicationMainWindow()->screen();
#endif
    };

    if (getScreen()->size().width() < 1200) {
        setSizes(QList<int>({1, 0}));
    }

    restoreState(getSetting("mainSplitterState").toByteArray());
    leftSplitter->restoreState(getSetting("leftSplitterState").toByteArray());

    connect(this, &QSplitter::splitterMoved, this,
            [this](int, int) { setSetting("mainSplitterState", saveState()); });
    connect(leftSplitter, &QSplitter::splitterMoved, this, [this, leftSplitter](int, int) {
        setSetting("leftSplitterState", leftSplitter->saveState());
    });

    setTabOrder(filterLineEdit_, workspaceTreeView_);
    setTabOrder(workspaceTreeView_, workspaceGridView_);
    setTabOrder(workspaceGridView_, loadWorkspaceBtn_);
    setTabOrder(details_, changelog_);

    changelog_->loadLog();
}

void WelcomeWidget::updateRecentWorkspaces(const QStringList& list) {
    model_->updateRecentWorkspaces(list);
}

void WelcomeWidget::enableRestoreButton(bool hasRestoreWorkspace) {
    restoreButton_->setEnabled(hasRestoreWorkspace);
}

void WelcomeWidget::setFilterFocus() { filterLineEdit_->setFocus(Qt::OtherFocusReason); }

void WelcomeWidget::selectFirstLeaf() const {
    // select first leaf node

    if (workspaceGridView_->isVisible()) {
        auto gridIndex = findFirstLeaf(workspaceGridView_->model());
        if (gridIndex.isValid()) {
            workspaceGridView_->selectionModel()->setCurrentIndex(
                gridIndex, QItemSelectionModel::ClearAndSelect);
        }
    }

    if (workspaceTreeView_->isVisible()) {
        auto treeIndex = findFirstLeaf(workspaceTreeView_->model());
        if (treeIndex.isValid()) {
            workspaceTreeView_->selectionModel()->setCurrentIndex(
                treeIndex, QItemSelectionModel::ClearAndSelect);
        }
    }
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
        model_->updateExampleEntries();
        model_->updateRegressionTestEntries();

        expandTreeView();
        selectFirstLeaf();
    }
    QWidget::showEvent(event);
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
                emit loadWorkspace(filename, isExample);
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
    std::optional<WorkspaceAnnotationsQt> annotations;
    try {
        annotations.emplace(utilqt::fromQString(filename), app_);
    } catch (Exception& e) {
    }

    const auto dateformat = "yyyy-MM-dd hh:mm:ss";
    const auto createdStr = utilqt::fromQString(info.birthTime().toString(dateformat));
    const auto modifiedStr = utilqt::fromQString(info.lastModified().toString(dateformat));
    const auto titleStr = [&]() {
        auto str = (annotations && !annotations->getTitle().empty())
                       ? annotations->getTitle()
                       : utilqt::fromQString(info.completeBaseName());
        if (!str.empty()) str[0] = static_cast<char>(std::toupper(str[0]));
        replaceInString(str, "-", " ");
        replaceInString(str, "_", " ");
        return str;
    }();

    const auto description = [&]() {
        if (!annotations) return std::string{};

        auto str = htmlEncode(annotations->getDescription());
        replaceInString(str, "\n", "<br/>");
        return str;
    }();

    Document doc;
    auto body = doc.append("html").append("body");
    body.append("div", titleStr,
                {{"style", "font-size:45pt; color:#268bd2; margin-top:0em; padding-top:0em;"}});

    auto table = body.append("table", "", {{"style", "margin-bottom:1em;"}});
    auto addRow = [&](std::string_view name, std::string_view value) {
        if (value.empty()) return;
        auto tr = table.append("tr");
        tr.append("td", name, {{"style", "text-align:right;"}});
        tr.append("td", htmlEncode(value), {{"style", "color:#ebe5df; font-weight:500;"}});
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

    auto content = body.append("span");
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

    details_->setHtml(utilqt::toQString(doc));
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

}  // namespace inviwo
