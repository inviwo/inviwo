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

#include <inviwo/qt/widgets/properties/propertywidgetqt.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/settings/systemsettings.h>
#include <inviwo/core/properties/property.h>
#include <inviwo/core/properties/propertywidgetfactory.h>
#include <inviwo/core/properties/propertyowner.h>
#include <inviwo/qt/widgets/inviwoapplicationqt.h>
#include <inviwo/qt/widgets/inviwoqtutils.h>
#include <inviwo/core/common/moduleaction.h>
#include <QDesktopWidget>

#include <QStyleOption>
#include <QPainter>
#include <QToolTip>
#include <QHelpEvent>
#include <QClipboard>

namespace inviwo {

IvwLineEdit::IvwLineEdit(QWidget* parent) : QLineEdit(parent) {}
IvwLineEdit::~IvwLineEdit() {}
QSize IvwLineEdit::sizeHint() const { return QSize(18, 18); }

IvwPushButton::IvwPushButton(QWidget* parent) : QPushButton(parent) {
    QSizePolicy sp = sizePolicy();
    sp.setHorizontalPolicy(QSizePolicy::Minimum);
    sp.setHorizontalStretch(3);
    setSizePolicy(sp);
}
IvwPushButton::~IvwPushButton() {}
QSize IvwPushButton::sizeHint() const { return QSize(18, 18); }
QSize IvwPushButton::minimumSizeHint() const { return sizeHint(); }

IvwComboBox::IvwComboBox(QWidget* parent) : QComboBox(parent) {
    QSizePolicy sp = sizePolicy();
    sp.setHorizontalPolicy(QSizePolicy::Minimum);
    sp.setHorizontalStretch(3);
    setSizePolicy(sp);
}
IvwComboBox::~IvwComboBox() {}
QSize IvwComboBox::sizeHint() const { return QSize(18, QComboBox::sizeHint().height()); }
QSize IvwComboBox::minimumSizeHint() const { return sizeHint(); }

int PropertyWidgetQt::MINIMUM_WIDTH = 250;
int PropertyWidgetQt::SPACING = 7;
int PropertyWidgetQt::MARGIN = 0;

PropertyWidgetQt::PropertyWidgetQt()
    : QWidget()
    , PropertyWidget()
    , usageModeItem_(nullptr)
    , usageModeActionGroup_(nullptr)
    , developerUsageModeAction_(nullptr)
    , applicationUsageModeAction_(nullptr)
    , copyAction_(nullptr)
    , pasteAction_(nullptr)
    , copyPathAction_(nullptr)
    , semanicsMenuItem_(nullptr)
    , semanticsActionGroup_(nullptr)
    , contextMenu_(nullptr) {
    this->setObjectName("PropertyWidget");
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this,
            SLOT(showContextMenu(const QPoint&)));
}

PropertyWidgetQt::PropertyWidgetQt(Property* property)
    : QWidget()
    , PropertyWidget(property)
    , usageModeItem_(nullptr)
    , usageModeActionGroup_(nullptr)
    , developerUsageModeAction_(nullptr)
    , applicationUsageModeAction_(nullptr)
    , semanicsMenuItem_(nullptr)
    , semanticsActionGroup_(nullptr)
    , contextMenu_(nullptr) {
    this->setObjectName("PropertyWidget");
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this,
            SLOT(showContextMenu(const QPoint&)));
}

PropertyWidgetQt::~PropertyWidgetQt() {}

void PropertyWidgetQt::setVisible(bool visible) {
    if (visible) {
        QWidget::setVisible(true);
    } else {
        QWidget::setVisible(false);
    }
}

void PropertyWidgetQt::showWidget() {
    if (isHidden() && getVisible()) {
        updateContextMenu();
        this->show();
    }
}

void PropertyWidgetQt::hideWidget() {
    if (isVisible()) {
        this->hide();
    }
}

void PropertyWidgetQt::initializeEditorWidgetsMetaData() {
    if (hasEditorWidget()) {
        // Validates editor widget position
        PropertyEditorWidgetQt* propertyEditorWidget =
            dynamic_cast<PropertyEditorWidgetQt*>(getEditorWidget());
        InviwoApplicationQt* app = dynamic_cast<InviwoApplicationQt*>(InviwoApplication::getPtr());
        if (!propertyEditorWidget) return;

        // set widget meta data stuff

        PropertyEditorWidgetDockStatus docStatus = propertyEditorWidget->getEditorDockStatus();

        if (app) {
            if (docStatus == PropertyEditorWidgetDockStatus::DockedLeft) {
                app->getMainWindow()->addDockWidget(Qt::LeftDockWidgetArea, propertyEditorWidget);
                propertyEditorWidget->setFloating(false);
            }
            else if (docStatus == PropertyEditorWidgetDockStatus::DockedRight) {
                app->getMainWindow()->addDockWidget(Qt::RightDockWidgetArea, propertyEditorWidget);
                propertyEditorWidget->setFloating(false);
            }
            else {
                app->getMainWindow()->addDockWidget(Qt::RightDockWidgetArea, propertyEditorWidget);
                propertyEditorWidget->setFloating(true);
            }
        }

        propertyEditorWidget->hide();

        ivec2 widgetDimension = getEditorWidget()->getEditorDimensionMetaData();
        propertyEditorWidget->resize(QSize(widgetDimension.x, widgetDimension.y));

        ivec2 pos = getEditorWidget()->getEditorPositionMetaData();

        if (app) {
            QPoint newPos = app->movePointOntoDesktop(QPoint(pos.x, pos.y), QSize(widgetDimension.x, widgetDimension.y), false);

            if (!(newPos.x() == 0 && newPos.y() == 0)) {
                propertyEditorWidget->move(newPos);
            }
            else { // We guess that this is a new widget and give a new position
                newPos = app->getMainWindow()->pos();
                newPos += app->offsetWidget();
                propertyEditorWidget->move(newPos);
            }
        }

        bool visible = getEditorWidget()->getEditorVisibilityMetaData();
        if (!visible)
            propertyEditorWidget->hide();
        else
            propertyEditorWidget->show();
    }
}

void PropertyWidgetQt::paintEvent(QPaintEvent* pe) {
    QStyleOption o;
    o.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &o, &p, this);
};

void PropertyWidgetQt::generateContextMenu() {
    if (!contextMenu_) {
        contextMenu_ = new QMenu(this);

        // View mode actions (Developer / Application)
        usageModeItem_ = new QMenu(tr("&Usage mode"), contextMenu_);
        developerUsageModeAction_ = new QAction(tr("&Developer"), this);
        developerUsageModeAction_->setCheckable(true);
        usageModeItem_->addAction(developerUsageModeAction_);

        applicationUsageModeAction_ = new QAction(tr("&Application"), this);
        applicationUsageModeAction_->setCheckable(true);
        usageModeItem_->addAction(applicationUsageModeAction_);

        usageModeActionGroup_ = new QActionGroup(this);
        usageModeActionGroup_->addAction(developerUsageModeAction_);
        usageModeActionGroup_->addAction(applicationUsageModeAction_);
        contextMenu_->addMenu(usageModeItem_);

        copyAction_ = new QAction("Copy", this);
        pasteAction_ = new QAction("Paste", this);
        copyPathAction_ = new QAction("Copy path", this);

        if (property_){
            contextMenu_->addAction(copyAction_);
            contextMenu_->addAction(pasteAction_);
            contextMenu_->addAction(copyPathAction_);

            semanicsMenuItem_ = new QMenu(tr("&Semantics"), contextMenu_);
            semanticsActionGroup_ = new QActionGroup(this);
            
            std::vector<PropertySemantics> semantics =
                PropertyWidgetFactory::getPtr()->getSupportedSemanicsForProperty(property_);

            for (auto& semantic : semantics) {
                QAction* semanticAction = new QAction(QString::fromStdString(semantic.getString()),
                                                      semanticsActionGroup_);
                semanticAction->setCheckable(true);
                semanicsMenuItem_->addAction(semanticAction);
                if (semantic == property_->getSemantics()) {
                    semanticAction->setChecked(true);
                } else {
                    semanticAction->setChecked(false);
                }
                semanticAction->setData(QString::fromStdString(semantic.getString()));
            }
            
            connect(semanticsActionGroup_, SIGNAL(triggered(QAction*)),
                    this, SLOT(changeSemantics(QAction*)));
        
            if (semantics.size() > 1) {
                contextMenu_->addMenu(semanicsMenuItem_);
            }
        }
        
        QAction* resetAction = new QAction(tr("&Reset to default"), this);
        resetAction->setToolTip(tr("&Reset the property back to it's initial state"));
        contextMenu_->addAction(resetAction);

        connect(developerUsageModeAction_, SIGNAL(triggered(bool)), this,
                SLOT(setDeveloperUsageMode(bool)));

        connect(applicationUsageModeAction_, SIGNAL(triggered(bool)), this,
                SLOT(setApplicationUsageMode(bool)));

        connect(resetAction, SIGNAL(triggered()), this, SLOT(resetPropertyToDefaultState()));

        connect(copyAction_, SIGNAL(triggered()), this, SLOT(copy()));
        connect(pasteAction_, SIGNAL(triggered()), this, SLOT(paste()));
        connect(copyPathAction_, SIGNAL(triggered()), this, SLOT(copyPath()));

        // Module actions.
        generateModuleMenuActions();
        QMapIterator<QString, QMenu*> it(moduleSubMenus_);

        while (it.hasNext()) {
            it.next();
            contextMenu_->addMenu(it.value());
        }

        // Update to current state.
        updateContextMenu();
    }
}

void PropertyWidgetQt::changeSemantics(QAction* action) {
    PropertySemantics semantics(action->data().toString().toUtf8().constData());
    if (property_) {
        property_->setSemantics(semantics);
        emit updateSemantics(this);
    }
}
    
void PropertyWidgetQt::showContextMenu(const QPoint& pos) {
    if (!contextMenu_) {
        generateContextMenu();
    }
    updateContextMenu();
    contextMenu_->exec(this->mapToGlobal(pos));
}

QMenu* PropertyWidgetQt::getContextMenu() {
    if (!contextMenu_) {
        generateContextMenu();
    }
    return contextMenu_;
}

void PropertyWidgetQt::generateModuleMenuActions() {
    moduleSubMenus_.clear();
    InviwoApplication* app = InviwoApplication::getPtr();
    std::vector<ModuleCallbackAction*> moduleActions = app->getCallbackActions();
    std::map<std::string, std::vector<ModuleCallbackAction*> > callbackMapPerModule;

    for (auto& moduleAction : moduleActions)
        callbackMapPerModule[moduleAction->getModule()->getIdentifier()].push_back(moduleAction);

    std::map<std::string, std::vector<ModuleCallbackAction*> >::iterator mapIt;

    for (mapIt = callbackMapPerModule.begin(); mapIt != callbackMapPerModule.end(); mapIt++) {
        std::vector<ModuleCallbackAction*> moduleActions = mapIt->second;

        if (moduleActions.size()) {
            QMenu* submenu = new QMenu(tr(mapIt->first.c_str()));
            moduleSubMenus_[mapIt->first.c_str()] = submenu;

            for (auto& moduleAction : moduleActions) {
                QAction* action = new QAction(tr(moduleAction->getActionName().c_str()), this);
                action->setCheckable(true);
                submenu->addAction(action);
                action->setChecked(moduleAction->getActionState() ==
                                   ModuleCallBackActionState::Enabled);
                connect(action, SIGNAL(triggered()), this, SLOT(moduleAction()));
            }
        }
    }
}

void PropertyWidgetQt::updateModuleMenuActions() {
    InviwoApplication* app = InviwoApplication::getPtr();
    std::vector<ModuleCallbackAction*> moduleActions = app->getCallbackActions();

    for (auto& moduleAction : moduleActions) {
        std::string moduleName = moduleAction->getModule()->getIdentifier();
        QMapIterator<QString, QMenu*> it(moduleSubMenus_);

        while (it.hasNext()) {
            it.next();

            if (it.key().toLocal8Bit().constData() == moduleName) {
                QList<QAction*> actions = it.value()->actions();

                for (auto& action : actions) {
                    if (action->text().toLocal8Bit().constData() == moduleAction->getActionName()) {
                        // FIXME: Following setChecked is not behaving as expected on some special
                        // case. This needs to be investigated.
                        // bool blockState = actions[j]->blockSignals(true);
                        action->setChecked(moduleAction->getActionState() ==
                                           ModuleCallBackActionState::Enabled);
                        // actions[j]->blockSignals(blockState);
                    }
                }
            }
        }
    }
}

UsageMode PropertyWidgetQt::getUsageMode() const {
    return property_->getUsageMode();
};

bool PropertyWidgetQt::getVisible() const {
    return property_->getVisible();
}

void PropertyWidgetQt::setDeveloperUsageMode(bool value) {
    property_->setUsageMode(DEVELOPMENT);
    updateContextMenu();
    emit usageModeChanged();
}

void PropertyWidgetQt::setApplicationUsageMode(bool value) {
    property_->setUsageMode(APPLICATION);
    updateContextMenu();
    emit usageModeChanged();
}

UsageMode PropertyWidgetQt::getApplicationUsageMode() {
    return InviwoApplication::getPtr()->getSettingsByType<SystemSettings>()
                                      ->getApplicationUsageMode();
}

void PropertyWidgetQt::moduleAction() {
    QAction* action = qobject_cast<QAction*>(QObject::sender());

    if (action) {
        InviwoApplication* app = InviwoApplication::getPtr();
        std::vector<ModuleCallbackAction*> moduleActions = app->getCallbackActions();
        std::string actionName(action->text().toLocal8Bit().constData());

        for (auto& moduleAction : moduleActions) {
            if (moduleAction->getActionName() == actionName) {
                moduleAction->getCallBack()->invoke(property_);
                action->setChecked(moduleAction->getActionState() ==
                                   ModuleCallBackActionState::Enabled);
            }
        }
    }
}

void PropertyWidgetQt::updateContextMenu() {
    if (usageModeItem_) {
        // Update the current selection.
        if (getUsageMode() == DEVELOPMENT)
            developerUsageModeAction_->setChecked(true);
        else if (getUsageMode() == APPLICATION)
            applicationUsageModeAction_->setChecked(true);

        // Disable the view mode buttons in Application mode
        UsageMode appVisibilityMode = getApplicationUsageMode();
        if (appVisibilityMode == DEVELOPMENT) {
            developerUsageModeAction_->setEnabled(true);
            applicationUsageModeAction_->setEnabled(true);
        } else {
            developerUsageModeAction_->setEnabled(false);
            applicationUsageModeAction_->setEnabled(false);
        }
        updateModuleMenuActions();
    }
}

bool PropertyWidgetQt::event(QEvent* event) {
    if (event->type() == QEvent::ToolTip) {
        QHelpEvent* helpEvent = static_cast<QHelpEvent*>(event);
        QToolTip::showText(helpEvent->globalPos(), QString::fromStdString(getToolTipText()));
        return true;
    }
    return QWidget::event(event);
}

std::string PropertyWidgetQt::makeToolTipTop(std::string item) const {
    return "<html><head>\
            <style>\
            table { border-color:white;white-space:pre;margin-top:5px;margin-bottom:5px; }\
            table > tr > td { padding-left:5px; padding-right:5px; }\
            </style><head/><body>\
            <b style='color:white;'>" + item + "</b>";
}

std::string PropertyWidgetQt::makeToolTipTableTop() const {
    return "<table border='0' cellspacing='0' cellpadding='0'\
            style='border-color:white;white-space:pre;margin: 5px 0;'>";
}

std::string PropertyWidgetQt::makeToolTipRow(std::string item, std::string val, bool tablehead) const {
    std::stringstream ss;
    std::string td = (tablehead ? "th" : "td");
    ss << "<tr><" << td << " style='color:#bbb;padding-right:8px;'>" << item << "</" << td << ">";
    ss << "<" << td << "><nobr>" + val + "</nobr></" << td << ">";
    ss << "</tr>" << std::endl;
    
    return ss.str();
}

std::string PropertyWidgetQt::makeToolTipRow(std::string item, std::vector<std::string> vals, bool tablehead) const {
    std::stringstream ss;
    std::string td = (tablehead ? "th" : "td");
    ss << "<tr><" << td << " style='color:#bbb;padding-right:8px;'>" << item << "</" << td << ">";

    for (auto& val : vals) {
        ss << "<" << td << " align=center><nobr>" + val + "</nobr></" << td << ">";
    }
    ss << "</tr>" << std::endl;
    
    return ss.str();
}

std::string PropertyWidgetQt::makeToolTipTableBottom() const {
    return "</table>";
}

std::string PropertyWidgetQt::makeToolTipBottom() const {
    return "</body></html>";
}

std::string PropertyWidgetQt::getToolTipText() {
    if (property_) {
        std::stringstream ss;
        utilqt::localizeStream(ss);

        ss << makeToolTipTop(property_->getDisplayName());
        ss << makeToolTipTableTop();
        ss << makeToolTipRow("Identifier", property_->getIdentifier());
        ss << makeToolTipRow("Path", joinString(property_->getPath(),"."));
        ss << makeToolTipRow("Semantics", property_->getSemantics().getString());
        ss << makeToolTipRow("Validation Level", PropertyOwner::invalidationLevelToString(
                                                     property_->getInvalidationLevel()));
        ss << makeToolTipTableBottom();
        ss << makeToolTipBottom();

        return ss.str();
    } else {
        return "";
    }
}

void PropertyWidgetQt::setProperty(Property* property) { PropertyWidget::setProperty(property); }

void PropertyWidgetQt::resetPropertyToDefaultState() { property_->resetToDefaultState(); }

void PropertyWidgetQt::setSpacingAndMargins(QLayout* layout) {
    layout->setContentsMargins(MARGIN, MARGIN, MARGIN, MARGIN);
    layout->setSpacing(SPACING);
}

QSize PropertyWidgetQt::sizeHint() const {
    return layout()->sizeHint();
}

QSize PropertyWidgetQt::minimumSizeHint() const {
    return layout()->sizeHint();
}

void PropertyWidgetQt::copy() {
    copySource = property_;
}

void PropertyWidgetQt::paste() {
    if(copySource) {
        property_->set(copySource);
    }
}

void PropertyWidgetQt::copyPath() {
    if (property_ == nullptr) {
        return;
    }
    std::string path = joinString(property_->getPath(),".");
    QApplication::clipboard()->setText(path.c_str());
}

const Property* PropertyWidgetQt::copySource = nullptr;

//////////////////////////////////////////////////////////////////////////

PropertyEditorWidgetQt::PropertyEditorWidgetQt(std::string widgetName, QWidget* parent)
    : InviwoDockWidget(QString(widgetName.c_str()), parent), PropertyEditorWidget() {}

PropertyEditorWidgetQt::~PropertyEditorWidgetQt() {}

void PropertyEditorWidgetQt::initialize(Property* property) {
    PropertyEditorWidget::initialize(property);
}

void PropertyEditorWidgetQt::deinitialize() {
    PropertyEditorWidget::deinitialize();
    hide();
}

}  // namespace
