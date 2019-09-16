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
#include <inviwo/core/util/logfilter.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/properties/propertyfactory.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/network/workspacemanager.h>

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
#include <QLineEdit>
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
#include <QCoreApplication>
#include <QFile>

#include <warn/pop>

namespace inviwo {

namespace {

class LineEditEventFilter : public QObject {
public:
    explicit LineEditEventFilter(QWidget* w, QObject* parent = nullptr)
        : QObject(parent), widget_(w) {}
    virtual ~LineEditEventFilter() = default;

    bool eventFilter(QObject* obj, QEvent* e) {
        switch (e->type()) {
            case QEvent::KeyPress: {
                QKeyEvent* keyEvent = static_cast<QKeyEvent*>(e);
                switch (keyEvent->key()) {
                    case Qt::Key_Down: {
                        // set focus to the tree widget
                        widget_->setFocus(Qt::ShortcutFocusReason);
                        // move cursor down one row
                        QKeyEvent* keydown =
                            new QKeyEvent(QKeyEvent::KeyPress, Qt::Key_Down, Qt::NoModifier);
                        QCoreApplication::postEvent(widget_, keydown);
                        break;
                    }
                    case Qt::Key_Escape:
                        reinterpret_cast<QLineEdit*>(parent())->clear();
                        break;
                    default:
                        break;
                }
                break;
            }
            default:
                break;
        }
        return QObject::eventFilter(obj, e);
    }

private:
    QWidget* widget_;
};

}  // namespace

WelcomeWidget::WelcomeWidget(InviwoMainWindow* window, QWidget* parent)
    : QSplitter(parent), mainWindow_(window) {

    setObjectName("WelcomeWidget");

    setOrientation(Qt::Horizontal);
    setHandleWidth(5);

    auto leftWidget = new QWidget(this);

    auto gridLayout = new QGridLayout();
    const auto space = utilqt::refSpacePx(this);
    gridLayout->setContentsMargins(static_cast<int>(space * 1.5), 0, 0,
                                   static_cast<int>(space * 1.5));
    gridLayout->setSpacing(space);

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

    auto updateDetails = [this](const QString& filename) {
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
                auto d = mainWindow_->getInviwoApplication()
                             ->getWorkspaceManager()
                             ->createWorkspaceDeserializer(istream, utilqt::fromQString(filename),
                                                           &logger);
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
                 {"src", "data:image/jpeg;base64," + item.base64jpeg}});
        };

        for (auto& elem : annotations.getCanvasImages()) {
            addImage(elem);
        }
        addImage(annotations.getNetworkImage());

        details_->setHtml(utilqt::toQString(doc));
    };

    // left column: workspace filter, list of recently used workspaces, and examples
    filetree_ = new FileTreeWidget(window->getInviwoApplication(), leftWidget);
    filetree_->setObjectName("FileTreeWidget");
    filetree_->setMinimumWidth(300);
    filetree_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QObject::connect(filetree_, &FileTreeWidget::selectedFileChanged, this,
                     [this, updateDetails](const QString& filename, bool isExample) {
                         updateDetails(filename);

                         loadWorkspaceBtn_->disconnect();
                         QObject::connect(
                             loadWorkspaceBtn_, &QToolButton::clicked, this,
                             [this, filename, isExample]() { loadWorkspace(filename, isExample); });
                     });
    QObject::connect(filetree_, &FileTreeWidget::loadFile, this, &WelcomeWidget::loadWorkspace);

    filterLineEdit_ = new QLineEdit(leftWidget);
    filterLineEdit_->setPlaceholderText("Search for Workspace...");
    filterLineEdit_->installEventFilter(new LineEditEventFilter(filetree_, filterLineEdit_));

    QIcon clearIcon;
    clearIcon.addFile(":/svgicons/lineedit-clear.svg", utilqt::emToPx(this, QSizeF(0.3, 0.3)),
                      QIcon::Normal);
    clearIcon.addFile(":/svgicons/lineedit-clear-active.svg",
                      utilqt::emToPx(this, QSizeF(0.3, 0.3)), QIcon::Active);
    clearIcon.addFile(":/svgicons/lineedit-clear-active.svg",
                      utilqt::emToPx(this, QSizeF(0.3, 0.3)), QIcon::Selected);
    auto clearAction = filterLineEdit_->addAction(clearIcon, QLineEdit::TrailingPosition);
    clearAction->setVisible(false);
    connect(clearAction, &QAction::triggered, filterLineEdit_, &QLineEdit::clear);
    connect(filterLineEdit_, &QLineEdit::textChanged, this,
            [this, clearAction](const QString& str) {
                filetree_->setFilter(str);
                clearAction->setVisible(!str.isEmpty());
            });
    gridLayout->addWidget(filterLineEdit_, 1, 0, 1, 1);
    gridLayout->addWidget(filetree_, 2, 0, 2, 1);

    // center column: workspace details and buttons for loading workspaces

    // workspace details
    details_ = new QTextEdit(leftWidget);
    details_->setObjectName("NetworkDetails");
    details_->setReadOnly(true);
    details_->setFrameShape(QFrame::NoFrame);
    details_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    gridLayout->addWidget(details_, 1, 1, 2, 1);

    // tool buttons
    auto buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(6);

    auto createButton = [leftWidget, this](const QString& str, auto iconpath) {
        auto button = new QToolButton(leftWidget);
        button->setText(str);
        button->setIcon(QIcon(iconpath));
        button->setIconSize(utilqt::emToPx(this, QSize(7, 7)));
        button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        return button;
    };

    loadWorkspaceBtn_ = createButton("Load", ":/svgicons/open.svg");
    loadWorkspaceBtn_->setToolTip("Open selected workspace");
    loadWorkspaceBtn_->setObjectName("LoadWorkspaceToolButton");
    buttonLayout->addWidget(loadWorkspaceBtn_);

    auto horizontalSpacer = new QSpacerItem(utilqt::emToPx(this, 2.0), utilqt::emToPx(this, 2.0),
                                            QSizePolicy::Expanding, QSizePolicy::Minimum);
    buttonLayout->addItem(horizontalSpacer);

    auto newButton = createButton("New", ":/svgicons/newfile.svg");
    newButton->setObjectName("NewWorkspaceToolButton");
    newButton->setToolTip("Create an empty workspace");
    QObject::connect(newButton, &QToolButton::clicked, this, [window]() {
        if (window->newWorkspace()) {
            window->hideWelcomeScreen();
        }
    });
    buttonLayout->addWidget(newButton);

    auto openFileButton = createButton("Open...", ":/svgicons/open.svg");
    openFileButton->setObjectName("OpenWorkspaceToolButton");
    openFileButton->setToolTip("Open workspace from disk");
    QObject::connect(openFileButton, &QToolButton::clicked, this, [window]() {
        if (window->openWorkspace()) {
            window->hideWelcomeScreen();
        }
    });
    buttonLayout->addWidget(openFileButton);

    auto restoreButton = createButton("Restore", ":/svgicons/revert.svg");
    restoreButton->setObjectName("OpenWorkspaceToolButton");
    restoreButton->setEnabled(window->hasRestoreWorkspace());
    restoreButton->setToolTip("Restore last automatically saved workspace if available");
    QObject::connect(restoreButton, &QToolButton::clicked, this, [window]() {
        window->restoreWorkspace();
        window->hideWelcomeScreen();
    });
    buttonLayout->addWidget(restoreButton);

    gridLayout->addLayout(buttonLayout, 3, 1, 1, 1);

    // add some space between center and right column
    auto horizontalSpacer_2 = new QSpacerItem(utilqt::emToPx(this, 2.0), utilqt::emToPx(this, 2.0),
                                              QSizePolicy::Fixed, QSizePolicy::Minimum);
    gridLayout->addItem(horizontalSpacer_2, 1, 2, 1, 1);

    leftWidget->setLayout(gridLayout);
    gridLayout->setRowStretch(1, 0);
    gridLayout->setRowStretch(2, 2);
    gridLayout->setRowStretch(3, 1);
    gridLayout->setColumnStretch(0, 1);
    gridLayout->setColumnStretch(1, 2);

    // right column: changelog and options

    auto rightColumn = new QFrame(this);
    rightColumn->setObjectName("WelcomeRightColumn");
    auto verticalLayout_3 = new QVBoxLayout(rightColumn);
    verticalLayout_3->setContentsMargins(utilqt::emToPx(this, 2.0), 0, 0,
                                         utilqt::emToPx(this, 1.0));
    changelog_ = new QTextEdit(rightColumn);
    changelog_->setObjectName("Changelog");
    QSizePolicy sizePolicy1(changelog_->sizePolicy());
    sizePolicy1.setVerticalStretch(100);
    changelog_->setSizePolicy(sizePolicy1);
    changelog_->setFrameShape(QFrame::NoFrame);
    changelog_->setTextInteractionFlags(Qt::TextBrowserInteraction);

    verticalLayout_3->addWidget(changelog_);

    auto verticalSpacer_2 = new QSpacerItem(utilqt::emToPx(this, 2.0), utilqt::emToPx(this, 3.0),
                                            QSizePolicy::Minimum, QSizePolicy::Fixed);
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

    setTabOrder(filterLineEdit_, filetree_);
    setTabOrder(filetree_, loadWorkspaceBtn_);
    setTabOrder(loadWorkspaceBtn_, newButton);
    setTabOrder(newButton, openFileButton);
    setTabOrder(openFileButton, restoreButton);
    setTabOrder(restoreButton, details_);
    setTabOrder(details_, changelog_);
    setTabOrder(changelog_, autoloadWorkspace);
    setTabOrder(autoloadWorkspace, showOnStartup);

    initChangelog();
}

void WelcomeWidget::updateRecentWorkspaces() {
    filetree_->updateRecentWorkspaces(mainWindow_->getRecentWorkspaceList());
}

void WelcomeWidget::setFilterFocus() { filterLineEdit_->setFocus(Qt::OtherFocusReason); }

void WelcomeWidget::showEvent(QShowEvent* event) {
    if (!event->spontaneous()) {
        QModelIndex index = filetree_->selectionModel()->currentIndex();

        updateRecentWorkspaces();
        filetree_->updateExampleEntries();
        filetree_->updateRegressionTestEntries();

        filetree_->expandItems();

        if (index.isValid()) {
            filetree_->selectionModel()->setCurrentIndex(
                index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
        } else {
            filetree_->selectRecentWorkspace(0);
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

void WelcomeWidget::loadWorkspace(const QString& filename, bool isExample) const {
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
