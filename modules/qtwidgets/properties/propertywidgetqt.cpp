/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2017 Inviwo Foundation
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

#include <modules/qtwidgets/properties/propertywidgetqt.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/metadata/containermetadata.h>
#include <inviwo/core/util/settings/systemsettings.h>
#include <inviwo/core/properties/property.h>
#include <inviwo/core/properties/propertywidgetfactory.h>
#include <inviwo/core/properties/propertyowner.h>
#include <inviwo/core/properties/propertypresetmanager.h>
#include <inviwo/core/network/networklock.h>
#include <modules/qtwidgets/inviwoqtutils.h>
#include <inviwo/core/common/moduleaction.h>

#include <modules/qtwidgets/propertylistwidget.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QApplication>
#include <QDesktopWidget>
#include <QStyleOption>
#include <QPainter>
#include <QToolTip>
#include <QHelpEvent>
#include <QInputDialog>
#include <QClipboard>
#include <QMenu>
#include <QLayout>
#include <QMimeData>
#include <QMessageBox>
#include <warn/pop>

namespace inviwo {

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
    , parent_(nullptr)
    , baseContainer_(nullptr)
    , applicationUsageMode_(nullptr)
    , appModeCallback_(nullptr)
    , contextMenu_(nullptr)
    , maxNumNestedShades_(4)
    , nestedDepth_(0) {
    applicationUsageMode_ =
        &(InviwoApplication::getPtr()->getSettingsByType<SystemSettings>()->applicationUsageMode_);
    setNestedDepth(nestedDepth_);
    setObjectName("PropertyWidget");
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
    , parent_(nullptr)
    , baseContainer_(nullptr)
    , applicationUsageMode_(nullptr)
    , appModeCallback_(nullptr)
    , contextMenu_(nullptr)
    , maxNumNestedShades_(4)
    , nestedDepth_(0) {
    property_->addObserver(this);
    applicationUsageMode_ =
        &(InviwoApplication::getPtr()->getSettingsByType<SystemSettings>()->applicationUsageMode_);

    appModeCallback_ =
        applicationUsageMode_->onChange([this]() { onSetUsageMode(property_->getUsageMode()); });

    setNestedDepth(nestedDepth_);
    setObjectName("PropertyWidget");

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this,
            SLOT(showContextMenu(const QPoint&)));
}

PropertyWidgetQt::~PropertyWidgetQt() { applicationUsageMode_->removeOnChange(appModeCallback_); }

void PropertyWidgetQt::initState() {
    if (property_) {
        setDisabled(property_->getReadOnly());
        setVisible(property_->getVisible());
    }
}

void PropertyWidgetQt::setVisible(bool visible) {
    bool wasVisible = QWidget::isVisible();
    UsageMode appMode = getApplicationUsageMode();
    if (visible && property_ && property_->getUsageMode() == UsageMode::Development &&
        appMode == UsageMode::Application)
        visible = false;

    QWidget::setVisible(visible);

    if (visible != wasVisible && parent_) parent_->onChildVisibilityChange(this);
}

void PropertyWidgetQt::onSetVisible(bool visible) { setVisible(visible); }

void PropertyWidgetQt::onChildVisibilityChange(PropertyWidgetQt* child) {
    if (property_) {
        setVisible(property_->getVisible());
    }
}

void PropertyWidgetQt::onSetUsageMode(UsageMode usageMode) { setVisible(property_->getVisible()); }

void PropertyWidgetQt::onSetReadOnly(bool readonly) { setDisabled(readonly); }

void PropertyWidgetQt::onSetSemantics(const PropertySemantics& semantics) {
    emit updateSemantics(this);
}
// connected to the semanticsActionGroup_
void PropertyWidgetQt::changeSemantics(QAction* action) {
    PropertySemantics semantics(action->data().toString().toUtf8().constData());
    if (property_) property_->setSemantics(semantics);
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
    updateContextMenu();
    return contextMenu_;
}

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

        copyAction_ = new QAction("Copy", this);
        pasteAction_ = new QAction("Paste", this);
        copyPathAction_ = new QAction("Copy path", this);

        if (property_) {
            contextMenu_->addMenu(usageModeItem_);
            contextMenu_->addAction(copyAction_);
            contextMenu_->addAction(pasteAction_);
            contextMenu_->addAction(copyPathAction_);

            semanicsMenuItem_ = new QMenu(tr("&Semantics"), contextMenu_);
            semanticsActionGroup_ = new QActionGroup(this);

            auto factory = InviwoApplication::getPtr()->getPropertyWidgetFactory();
            auto semantics = factory->getSupportedSemanicsForProperty(property_);

            for (auto& semantic : semantics) {
                auto semanticAction = new QAction(QString::fromStdString(semantic.getString()),
                                                  semanticsActionGroup_);
                semanicsMenuItem_->addAction(semanticAction);
                semanticAction->setCheckable(true);
                semanticAction->setChecked(semantic == property_->getSemantics());
                semanticAction->setData(QString::fromStdString(semantic.getString()));
            }

            connect(semanticsActionGroup_, SIGNAL(triggered(QAction*)), this,
                    SLOT(changeSemantics(QAction*)));

            if (semantics.size() > 1) contextMenu_->addMenu(semanicsMenuItem_);

            generatePresetMenuActions();
        }

        QAction* resetAction = new QAction(tr("&Reset to default"), this);
        resetAction->setToolTip(tr("&Reset the property back to it's initial state"));
        contextMenu_->addAction(resetAction);

        connect(developerUsageModeAction_, SIGNAL(triggered(bool)), this,
                SLOT(setDeveloperUsageMode(bool)));

        connect(applicationUsageModeAction_, SIGNAL(triggered(bool)), this,
                SLOT(setApplicationUsageMode(bool)));

        connect(resetAction, SIGNAL(triggered()), this, SLOT(resetPropertyToDefaultState()));

        connect(copyAction_, &QAction::triggered, [&]() {
            if (!property_) return;

            Serializer serializer("");
            std::vector<Property*> properties = {property_};
            serializer.serialize("Properties", properties, "Property");

            std::stringstream ss;
            serializer.writeFile(ss);
            auto str = ss.str();
            QByteArray data(str.c_str(), static_cast<int>(str.length()));

            auto mimedata = util::make_unique<QMimeData>();
            mimedata->setData(QString("application/x.vnd.inviwo.property+xml"), data);

            mimedata->setData(QString("text/plain"), data);
            QApplication::clipboard()->setMimeData(mimedata.release());
        });
        connect(pasteAction_, &QAction::triggered, [&]() {
            if (!property_) return;

            auto clipboard = QApplication::clipboard();
            auto mimeData = clipboard->mimeData();
            QByteArray data;
            if (mimeData->formats().contains(QString("application/x.vnd.inviwo.property+xml"))) {
                data = mimeData->data(QString("application/x.vnd.inviwo.property+xml"));
            } else if (mimeData->formats().contains(QString("text/plain"))) {
                data = mimeData->data(QString("text/plain"));
            }
            std::stringstream ss;
            for (auto d : data) ss << d;

            try {
                Deserializer deserializer(ss, "");
                auto app = InviwoApplication::getPtr();
                deserializer.registerFactory(app->getPropertyFactory());
                deserializer.registerFactory(app->getMetaDataFactory());
                std::vector<std::unique_ptr<Property>> properties;
                deserializer.deserialize("Properties", properties, "Property");
                if (!properties.empty() && properties.front()) {
                    NetworkLock lock(property_->getOwner()->getProcessor()->getNetwork());
                    property_->set(properties.front().get());
                }
            } catch (AbortException&) {
            }
        });
        connect(copyPathAction_, &QAction::triggered, [&]() {
            if (!property_) return;
            std::string path = joinString(property_->getPath(), ".");
            QApplication::clipboard()->setText(path.c_str());
        });

        // Module actions.
        generateModuleMenuActions();
        for (const auto& item : moduleSubMenus_) {
            contextMenu_->addMenu(item.second.get());
        }
        updateModuleMenuActions();
    }
}

void PropertyWidgetQt::generateModuleMenuActions() {
    moduleSubMenus_.clear();

    std::map<std::string, std::vector<const ModuleCallbackAction*>> callbackMapPerModule;
    for (auto& moduleAction : InviwoApplication::getPtr()->getCallbackActions()) {
        callbackMapPerModule[moduleAction->getModule()->getIdentifier()].push_back(
            moduleAction.get());
    }

    for (auto& elem : callbackMapPerModule) {
        auto& actions = elem.second;

        if (!actions.empty()) {
            auto submenu = util::make_unique<QMenu>(QString::fromStdString(elem.first));
            for (auto& moduleAction : actions) {
                QAction* action =
                    new QAction(QString::fromStdString(moduleAction->getActionName()), this);
                action->setCheckable(true);
                submenu->addAction(action);
                action->setChecked(moduleAction->getActionState() ==
                                   ModuleCallBackActionState::Enabled);
                connect(action, &QAction::triggered, [
                    action, actionName = moduleAction->getActionName(), property = property_
                ](bool checked) {
                    InviwoApplication* app = InviwoApplication::getPtr();
                    const auto& moduleActions = app->getCallbackActions();

                    for (auto& mAction : moduleActions) {
                        if (mAction->getActionName() == actionName) {
                            mAction->getCallBack().invoke(property);
                            action->setChecked(mAction->getActionState() ==
                                               ModuleCallBackActionState::Enabled);
                        }
                    }
                });
            }
            moduleSubMenus_[elem.first] = std::move(submenu);
        }
    }
}

void PropertyWidgetQt::updateModuleMenuActions() {
    if (property_ && usageModeItem_) {
        // Update the current selection.
        if (property_->getUsageMode() == UsageMode::Development)
            developerUsageModeAction_->setChecked(true);
        else if (property_->getUsageMode() == UsageMode::Application)
            applicationUsageModeAction_->setChecked(true);

        // Disable the view mode buttons in Application mode
        UsageMode appVisibilityMode = getApplicationUsageMode();
        if (appVisibilityMode == UsageMode::Development) {
            developerUsageModeAction_->setEnabled(true);
            applicationUsageModeAction_->setEnabled(true);
        } else {
            developerUsageModeAction_->setEnabled(false);
            applicationUsageModeAction_->setEnabled(false);
        }

        const auto& moduleActions = InviwoApplication::getPtr()->getCallbackActions();

        for (auto& moduleAction : moduleActions) {
            std::string moduleName = moduleAction->getModule()->getIdentifier();
            auto it = moduleSubMenus_.find(moduleName);

            if (it != moduleSubMenus_.end()) {
                QList<QAction*> actions = it->second->actions();
                for (auto& action : actions) {
                    if (action->text().toLocal8Bit().constData() == moduleAction->getActionName()) {
                        // FIXME: Following setChecked is not behaving as expected on some special
                        // case. This needs to be investigated.
                        action->setChecked(moduleAction->getActionState() ==
                                           ModuleCallBackActionState::Enabled);
                    }
                }
            }
        }
    }
}

void PropertyWidgetQt::generatePresetMenuActions() {
    if (property_) {
        contextMenu_->addSeparator();
        propertyPresetMenu_ = contextMenu_->addMenu("Property Presets");
        workspacePresetMenu_ = contextMenu_->addMenu("Workspace Presets");
        appPresetMenu_ = contextMenu_->addMenu("Application Presets");
        contextMenu_->addSeparator();

        {
            auto addPresetToMenu = [=](QMenu* menu, const std::string& name, Property* property,
                                       PropertyPresetType type) {
                auto action = menu->addAction(QString::fromStdString(name));
                // mark menu entry/action as 'preset'
                action->setProperty("preset", true);
                connect(action, &QAction::triggered, [=]() {
                    auto presetManager = InviwoApplication::getPtr()->getPropertyPresetManager();
                    presetManager->loadPreset(name, property, type);
                });
            };
            auto savePreset = [=](QMenu* menu, PropertyPresetType type) {
                // will prompt the user to enter a preset name.
                // returns false if a preset with the same name already exits
                // and the user does not want to overwrite it.
                bool ok;
                QString text = QInputDialog::getText(this, tr("Save Preset"), tr("Name:"),
                                                     QLineEdit::Normal, "", &ok);
                if (ok && !text.isEmpty()) {
                    const auto presetName = text.toStdString();
                    auto presetManager = InviwoApplication::getPtr()->getPropertyPresetManager();
                    bool actionExists = false;
                    for (auto p : presetManager->getAvailablePresets(property_, type)) {
                        if (p == presetName) {
                            actionExists = true;
                            break;
                        }
                    }
                    if (actionExists) {
                        if (QMessageBox::question(
                                this, "Overwrite Preset?",
                                QString("Preset named \"%1\" already exists.<br><br>Overwrite?")
                                    .arg(text)) == QMessageBox::No) {
                            return false;
                        }
                    }
                    // save current state as a preset
                    presetManager->savePreset(presetName, property_, type);

                    if (!actionExists) {
                        // create entry in the context menu
                        addPresetToMenu(menu, presetName, property_, type);
                    }
                }
                return true;
            };
            auto clearPresets = [=](PropertyPresetType type) {
                auto presetManager = InviwoApplication::getPtr()->getPropertyPresetManager();
                QMenu* menu;
                switch (type) {
                    case PropertyPresetType::Property:
                        menu = propertyPresetMenu_;
                        presetManager->clearPropertyPresets(property_);
                        break;
                    case PropertyPresetType::Workspace:
                        menu = workspacePresetMenu_;
                        presetManager->clearWorkspacePresets();
                        break;
                    case PropertyPresetType::Application:
                        menu = appPresetMenu_;
                        break;
                    default:
                        return;
                }
                for (auto entry : menu->actions()) {
                    if (entry->property("preset").toBool()) {
                        menu->removeAction(entry);
                    }
                }
            };

            {
                // application presets
                auto saveAction = appPresetMenu_->addAction("Save...");
                connect(saveAction, &QAction::triggered, [=]() {
                    while (!savePreset(appPresetMenu_, PropertyPresetType::Application))
                        ;
                });
                appPresetMenu_->addSeparator();
                // menu entries will be populated in updateContextMenu()
            }
            {
                // workspace presets
                auto saveAction = workspacePresetMenu_->addAction("Save...");
                connect(saveAction, &QAction::triggered, [=]() {
                    while (!savePreset(workspacePresetMenu_, PropertyPresetType::Workspace))
                        ;
                });

                auto clearAction = workspacePresetMenu_->addAction("Clear");
                connect(clearAction, &QAction::triggered,
                        [=]() { clearPresets(PropertyPresetType::Workspace); });
                workspacePresetMenu_->addSeparator();
                // menu entries will be populated in updateContextMenu()
            }
            {
                // property presets
                auto saveAction = propertyPresetMenu_->addAction("Save...");
                connect(saveAction, &QAction::triggered, [=]() {
                    while (!savePreset(propertyPresetMenu_, PropertyPresetType::Property))
                        ;
                });
                auto clearAction = propertyPresetMenu_->addAction("Clear");
                connect(clearAction, &QAction::triggered,
                        [=]() { clearPresets(PropertyPresetType::Property); });
                propertyPresetMenu_->addSeparator();

                // add menu entries for already existing presets
                auto presetManager = InviwoApplication::getPtr()->getPropertyPresetManager();
                for (auto preset :
                     presetManager->getAvailablePresets(property_, PropertyPresetType::Property)) {
                    addPresetToMenu(propertyPresetMenu_, preset, property_,
                                    PropertyPresetType::Property);
                }
            }
        }
    }
}

void PropertyWidgetQt::updatePresetMenuActions() {
    auto addPresetToMenu = [=](QMenu* menu, const std::string& name, Property* property,
                               PropertyPresetType type) {
        auto action = menu->addAction(QString::fromStdString(name));
        // mark menu entry/action as 'preset'
        action->setProperty("preset", true);
        connect(action, &QAction::triggered, [=]() {
            auto presetManager = InviwoApplication::getPtr()->getPropertyPresetManager();
            presetManager->loadPreset(name, property, type);
        });
    };
    auto removePresetsFromMenu = [=](QMenu* menu) {
        for (auto entry : menu->actions()) {
            if (entry->property("preset").toBool()) {
                menu->removeAction(entry);
            }
        }
    };

    auto presetManager = InviwoApplication::getPtr()->getPropertyPresetManager();
    {
        // application presets
        removePresetsFromMenu(appPresetMenu_);
        // add menu entries for existing presets
        for (auto preset :
             presetManager->getAvailablePresets(property_, PropertyPresetType::Application)) {
            addPresetToMenu(appPresetMenu_, preset, property_, PropertyPresetType::Application);
        }
    }
    {
        // workspace presets
        removePresetsFromMenu(workspacePresetMenu_);
        // add menu entries for existing presets
        for (auto preset :
             presetManager->getAvailablePresets(property_, PropertyPresetType::Workspace)) {
            addPresetToMenu(workspacePresetMenu_, preset, property_, PropertyPresetType::Workspace);
        }
    }
}

// connected to developerUsageModeAction_
void PropertyWidgetQt::setDeveloperUsageMode(bool value) {
    property_->setUsageMode(UsageMode::Development);
}
// connected to applicationUsageModeAction_
void PropertyWidgetQt::setApplicationUsageMode(bool value) {
    property_->setUsageMode(UsageMode::Application);
}

UsageMode PropertyWidgetQt::getApplicationUsageMode() { return applicationUsageMode_->get(); }

void PropertyWidgetQt::updateContextMenu() {
    if (property_) {
        updateModuleMenuActions();
        updatePresetMenuActions();
    }
}

bool PropertyWidgetQt::event(QEvent* event) {
    if (event->type() == QEvent::ToolTip && property_) {
        auto helpEvent = static_cast<QHelpEvent*>(event);
        QToolTip::showText(helpEvent->globalPos(),
                           utilqt::toLocalQString(property_->getDescription()));
        return true;
    } else {
        return QWidget::event(event);
    }
}

void PropertyWidgetQt::resetPropertyToDefaultState() { property_->resetToDefaultState(); }

void PropertyWidgetQt::setSpacingAndMargins(QLayout* layout) {
    layout->setContentsMargins(MARGIN, MARGIN, MARGIN, MARGIN);
    layout->setSpacing(SPACING);
}

QSize PropertyWidgetQt::minimumSizeHint() const { return PropertyWidgetQt::sizeHint(); }

QSize PropertyWidgetQt::sizeHint() const { return layout()->sizeHint(); }

void PropertyWidgetQt::setNestedDepth(int depth) {
    nestedDepth_ = depth;
    if (nestedDepth_ == 0) {
        // special case for depth zero
        QObject::setProperty("bgType", "toplevel");
    } else {
        QObject::setProperty("bgType", nestedDepth_ % maxNumNestedShades_);
    }
}

int PropertyWidgetQt::getNestedDepth() const { return nestedDepth_; }

PropertyWidgetQt* PropertyWidgetQt::getParentPropertyWidget() const { return parent_; }

InviwoDockWidget* PropertyWidgetQt::getBaseContainer() const { return baseContainer_; }

void PropertyWidgetQt::setParentPropertyWidget(PropertyWidgetQt* parent, InviwoDockWidget* widget) {
    parent_ = parent;
    baseContainer_ = widget;
}

void PropertyWidgetQt::paintEvent(QPaintEvent* pe) {
    QStyleOption o;
    o.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &o, &p, this);
}

}  // namespace
