/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2022 Inviwo Foundation
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

#include <inviwo/core/common/inviwoapplication.h>          // for InviwoApplication
#include <inviwo/core/common/inviwoapplicationutil.h>      // for getInviwoApplication
#include <inviwo/core/common/inviwomodule.h>               // for InviwoModule
#include <inviwo/core/common/moduleaction.h>               // for ModuleCallbackAction
#include <inviwo/core/common/modulecallback.h>             // for ModuleCallback
#include <inviwo/core/io/serialization/deserializer.h>     // for Deserializer
#include <inviwo/core/io/serialization/serializer.h>       // for Serializer
#include <inviwo/core/network/networklock.h>               // for NetworkLock
#include <inviwo/core/network/workspacemanager.h>          // for WorkspaceManager
#include <inviwo/core/properties/property.h>               // for Property
#include <inviwo/core/properties/propertypresetmanager.h>  // for PropertyPresetType, operator<<
#include <inviwo/core/properties/propertysemantics.h>      // for PropertySemantics, operator==
#include <inviwo/core/properties/propertywidget.h>         // for PropertyWidget
#include <inviwo/core/properties/propertywidgetfactory.h>  // for PropertyWidgetFactory
#include <inviwo/core/util/document.h>                     // for Document, Document::DocumentHa...
#include <inviwo/core/util/exception.h>                    // for Exception, AbortException
#include <inviwo/core/util/rendercontext.h>                // for RenderContext
#include <inviwo/core/util/stringconversion.h>             // for toString
#include <inviwo/core/util/typetraits.h>                   // for alwaysTrue, identity
#include <inviwo/core/util/unindent.h>                     // for operator""_unindent
#include <modules/qtwidgets/inviwoqtutils.h>               // for toQString, emToPx, refEm, from...

#include <algorithm>                                       // for find_if
#include <array>                                           // for array, array<>::value_type
#include <initializer_list>                                // for initializer_list
#include <map>                                             // for map, __map_iterator, operator!=
#include <ostream>                                         // for operator<<, stringstream, basi...
#include <string>                                          // for string, char_traits, basic_string
#include <string_view>                                     // for string_view
#include <utility>                                         // for pair
#include <vector>                                          // for vector, __vector_base<>::value...

#include <QAction>                                         // for QAction
#include <QActionGroup>                                    // for QActionGroup
#include <QApplication>                                    // for QApplication
#include <QByteArray>                                      // for QByteArray
#include <QClipboard>                                      // for QClipboard
#include <QDrag>                                           // for QDrag
#include <QEvent>                                          // for QEvent
#include <QFlags>                                          // for QFlags
#include <QHelpEvent>                                      // for QHelpEvent
#include <QIcon>                                           // for QIcon
#include <QInputDialog>                                    // for QInputDialog
#include <QLayout>                                         // for QLayout
#include <QLineEdit>                                       // for QLineEdit, QLineEdit::Normal
#include <QList>                                           // for QList
#include <QMenu>                                           // for QMenu
#include <QMessageBox>                                     // for QMessageBox, QMessageBox::No
#include <QMimeData>                                       // for QMimeData
#include <QMouseEvent>                                     // for QMouseEvent
#include <QObject>                                         // for QObject
#include <QPainter>                                        // for QPainter
#include <QString>                                         // for QString
#include <QStringList>                                     // for QStringList
#include <QStyle>                                          // for QStyle, QStyle::PE_Widget
#include <QStyleOption>                                    // for QStyleOption
#include <QToolTip>                                        // for QToolTip
#include <QVariant>                                        // for QVariant
#include <QEvent>                                          // for QEvent (ptr only), QEvent::Mou...
#include <Qt>                                              // for LeftButton, MSWindowsFixedSize...

class QAction;
class QHelpEvent;
class QMouseEvent;
class QPaintEvent;

namespace inviwo {

const int PropertyWidgetQt::minimumWidth = 200;
const int PropertyWidgetQt::spacing = 7;
const int PropertyWidgetQt::margin = 0;

// The factor should be 16.0 at font size 12pt, we use Segoe UI at 9pt which gives an em at 9.0px
const double PropertyWidgetQt::minimumWidthEm =
    PropertyWidgetQt::minimumWidth / static_cast<double>(utilqt::refEm());
const double PropertyWidgetQt::spacingEm = utilqt::refSpaceEm();
const double PropertyWidgetQt::marginEm =
    PropertyWidgetQt::margin / static_cast<double>(utilqt::refEm());

PropertyWidgetQt::PropertyWidgetQt(Property* property)
    : QWidget()
    , PropertyWidget(property)
    , parent_(nullptr)
    , maxNumNestedShades_(4)
    , nestedDepth_(0) {

    if (property_) {
        property_->addObserver(this);
    }

    setNestedDepth(nestedDepth_);
    setObjectName("PropertyWidget");
    setContextMenuPolicy(Qt::PreventContextMenu);
}

PropertyWidgetQt::~PropertyWidgetQt() = default;

void PropertyWidgetQt::initState() {
    if (property_) {
        setReadOnly(property_->getReadOnly());
        setVisible(property_->getVisible());
    }
}

void PropertyWidgetQt::setVisible(bool visible) {
    bool wasVisible = QWidget::isVisible();
    QWidget::setVisible(visible);
    if (visible != wasVisible && parent_) parent_->onChildVisibilityChange(this);
}

void PropertyWidgetQt::onChildVisibilityChange(PropertyWidgetQt* /*child*/) {
    if (property_) {
        setVisible(property_->getVisible());
    }
}

void PropertyWidgetQt::setReadOnly(bool readonly) { setDisabled(readonly); }

void PropertyWidgetQt::onSetVisible(Property*, bool visible) {
    setVisible(visible);
    RenderContext::getPtr()->activateDefaultRenderContext();
}

void PropertyWidgetQt::onSetReadOnly(Property*, bool readonly) {
    setReadOnly(readonly);
    RenderContext::getPtr()->activateDefaultRenderContext();
}

std::unique_ptr<QMenu> PropertyWidgetQt::getContextMenu() {
    std::unique_ptr<QMenu> menu = std::make_unique<QMenu>();

    if (auto app = util::getInviwoApplication(property_)) {
        // need to replace all '&' with '&&'. Otherwise Qt will interpret it as keyboard shortcut.
        menu->addAction(QString::fromStdString(property_->getDisplayName()).replace("&", "&&"));
        menu->addSeparator();

        {
            auto copyAction = menu->addAction(QIcon(":/svgicons/edit-copy.svg"), "&Copy");
            connect(copyAction, &QAction::triggered, this, [this]() {
                if (!property_) return;
                QApplication::clipboard()->setMimeData(getPropertyMimeData().release());
            });

            auto pasteAction = menu->addAction(QIcon(":/svgicons/edit-paste.svg"), "&Paste");
            if (property_->getReadOnly()) pasteAction->setEnabled(false);
            connect(pasteAction, &QAction::triggered, this, [this, app]() {
                if (!property_) return;

                auto clipboard = QApplication::clipboard();
                auto mimeData = clipboard->mimeData();
                QByteArray data;
                if (mimeData->formats().contains(
                        QString("application/x.vnd.inviwo.property+xml"))) {
                    data = mimeData->data(QString("application/x.vnd.inviwo.property+xml"));
                } else if (mimeData->formats().contains(QString("text/plain"))) {
                    data = mimeData->data(QString("text/plain"));
                }
                std::stringstream ss;
                for (auto d : data) ss << d;

                try {
                    auto d = app->getWorkspaceManager()->createWorkspaceDeserializer(ss, "");
                    std::vector<std::unique_ptr<Property>> properties;
                    d.deserialize("Properties", properties, "Property");
                    if (!properties.empty() && properties.front()) {
                        NetworkLock lock(property_);
                        property_->set(properties.front().get());
                        PropertyPresetManager::appendPropertyPresets(property_,
                                                                     properties.front().get());
                    }
                } catch (AbortException&) {
                }
            });

            auto copyPathAction = menu->addAction("Copy Path");
            connect(copyPathAction, &QAction::triggered, this, [this]() {
                if (!property_) return;
                std::string path = property_->getPath();
                QApplication::clipboard()->setText(path.c_str());
            });
        }

        menu->addSeparator();

        {
            auto factory = app->getPropertyWidgetFactory();
            auto semantics = factory->getSupportedSemanicsForProperty(property_);
            if (semantics.size() > 1) {
                auto semanicsMenu = menu->addMenu(tr("&Semantics"));
                auto semanticsGroup = new QActionGroup(semanicsMenu);

                for (auto& semantic : semantics) {
                    auto action = semanicsMenu->addAction(utilqt::toQString(semantic.getString()));
                    semanticsGroup->addAction(action);
                    action->setCheckable(true);
                    action->setChecked(semantic == property_->getSemantics());
                    action->setData(utilqt::toQString(semantic.getString()));
                }

                connect(
                    semanticsGroup, &QActionGroup::triggered, this,
                    [prop = property_](QAction* action) {
                        PropertySemantics semantics(utilqt::fromQString(action->data().toString()));
                        prop->setSemantics(semantics);
                    });
            }
        }

        addPresetMenuActions(menu.get(), app);

        auto resetAction = menu->addAction(tr("&Reset to default"));
        resetAction->setToolTip(tr("&Reset the property back to it's initial state"));
        connect(resetAction, &QAction::triggered, this,
                [this]() { property_->resetToDefaultState(); });

        // Module actions.
        addModuleMenuActions(menu.get(), app);
    }

    return menu;
}

std::unique_ptr<QMimeData> PropertyWidgetQt::getPropertyMimeData() const {
    auto mimeData = std::make_unique<QMimeData>();
    if (!property_) return mimeData;

    Serializer serializer("");
    {
        // Need to set the serialization mode to all temporarily to be able to copy the
        // property.
        auto toReset = PropertyPresetManager::scopedSerializationModeAll(property_);
        std::vector<Property*> properties = {property_};
        serializer.serialize("Properties", properties, "Property");
    }
    std::stringstream ss;
    serializer.writeFile(ss);
    auto str = ss.str();
    QByteArray dataArray(str.c_str(), static_cast<int>(str.length()));

    mimeData->setData(QString("application/x.vnd.inviwo.property+xml"), dataArray);
    mimeData->setData(QString("text/plain"), dataArray);
    return mimeData;
}

int PropertyWidgetQt::getSpacing() const { return utilqt::emToPx(this, spacingEm); }

void PropertyWidgetQt::addModuleMenuActions(QMenu* menu, InviwoApplication* app) {
    std::map<std::string, std::vector<const ModuleCallbackAction*>> callbackMapPerModule;
    for (auto& moduleAction : app->getCallbackActions()) {
        auto moduleName = moduleAction->getModule()->getIdentifier();
        callbackMapPerModule[moduleName].push_back(moduleAction.get());
    }

    for (auto& elem : callbackMapPerModule) {
        const auto& moduleName = elem.first;
        const auto& actions = elem.second;
        if (actions.empty()) continue;
        auto submenu = menu->addMenu(QString::fromStdString(moduleName));
        for (const auto& mAction : actions) {
            auto actionName = mAction->getActionName();
            auto action = submenu->addAction(QString::fromStdString(actionName));
            connect(action, &QAction::triggered, this,
                    [app, actionName, property = property_](bool /*checked*/) {
                        const auto& mActions = app->getCallbackActions();
                        auto it = std::find_if(mActions.begin(), mActions.end(), [&](auto& item) {
                            return item->getActionName() == actionName;
                        });
                        if (it != mActions.end()) {
                            (*it)->getCallBack().invoke(property);
                        }
                    });
        }
    }
}

void PropertyWidgetQt::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        mousePressedPosition_ = event->pos();
    }
    QWidget::mousePressEvent(event);
}

void PropertyWidgetQt::mouseMoveEvent(QMouseEvent* event) {
    if (property_ && (event->buttons() & Qt::LeftButton) &&
        ((event->pos() - mousePressedPosition_).manhattanLength() <
         QApplication::startDragDistance())) {

        event->accept();

        QDrag* drag = new QDrag(this);
        auto mimeData = getPropertyMimeData();
        // Displayed while dragging property
        mimeData->setText(utilqt::toLocalQString(property_->getDisplayName()));
        drag->setMimeData(mimeData.release());
        drag->exec();
    } else {
        QWidget::mouseMoveEvent(event);
    }
}

void PropertyWidgetQt::addPresetMenuActions(QMenu* menu, InviwoApplication* app) {
    if (!property_) return;

    auto presetMenu = menu->addMenu("Presets");
    auto presetManager = app->getPropertyPresetManager();

    {
        auto addPresetToMenu = [presetManager, this](QMenu* menu, const std::string& name,
                                                     Property* property, PropertyPresetType type) {
            auto action = menu->addAction(QString::fromStdString(name));
            if (property->getReadOnly()) action->setEnabled(false);
            action->setToolTip(utilqt::toQString(toString(type) + " Preset"));
            this->connect(action, &QAction::triggered, menu,
                          [presetManager, name, property, type]() {
                              presetManager->loadPreset(name, property, type);
                          });
        };
        auto savePreset = [presetManager, property = property_](QWidget* parent,
                                                                PropertyPresetType type) {
            // will prompt the user to enter a preset name.
            // returns false if a preset with the same name already exits
            // and the user does not want to overwrite it.
            bool ok;
            QString text =
                QInputDialog::getText(parent, tr("Save Preset"), tr("Name:"), QLineEdit::Normal, "",
                                      &ok, Qt::WindowFlags() | Qt::MSWindowsFixedSizeDialogHint);
            if (ok && !text.isEmpty()) {
                const auto presetName = text.toStdString();
                bool actionExists = false;
                for (auto p : presetManager->getAvailablePresets(property, type)) {
                    if (p == presetName) {
                        actionExists = true;
                        break;
                    }
                }
                if (actionExists) {
                    if (QMessageBox::question(
                            parent, "Overwrite Preset?",
                            QString("Preset named \"%1\" already exists.<br><br>Overwrite?")
                                .arg(text)) == QMessageBox::No) {
                        return false;
                    }
                }
                // save current state as a preset
                presetManager->savePreset(presetName, property, type);
            }
            return true;
        };
        auto clearPresets = [presetManager, property = property_](PropertyPresetType type) {
            switch (type) {
                case PropertyPresetType::Property:
                    presetManager->clearPropertyPresets(property);
                    break;
                case PropertyPresetType::Workspace:
                    presetManager->clearWorkspacePresets();
                    break;
                case PropertyPresetType::Application:
                    break;
                default:
                    return;
            }
        };
        std::array<PropertyPresetType, 3> types = {PropertyPresetType::Property,
                                                   PropertyPresetType::Workspace,
                                                   PropertyPresetType::Application};
        auto saveMenu = presetMenu->addMenu("Save");
        for (auto& type : types) {
            auto saveAction = saveMenu->addAction(utilqt::toQString(toString(type) + " Preset..."));
            std::stringstream ss;
            ss << "Save preset in " << type << ". " << type << " presets are local to this "
               << type;
            saveAction->setToolTip(utilqt::toQString(ss.str()));
            connect(saveAction, &QAction::triggered, [=]() {
                while (!savePreset(saveMenu, type))
                    ;
            });
        }

        auto clearMenu = presetMenu->addMenu("Clear");
        for (auto& type : {PropertyPresetType::Property, PropertyPresetType::Workspace}) {
            auto clearAction = clearMenu->addAction(utilqt::toQString(toString(type) + " Presets"));
            connect(clearAction, &QAction::triggered, [=]() { clearPresets(type); });
        }

        presetMenu->addSeparator();
        for (auto& type : types) {
            for (auto preset : presetManager->getAvailablePresets(property_, type)) {
                addPresetToMenu(presetMenu, preset, property_, type);
            }
        }
    }
}

bool PropertyWidgetQt::event(QEvent* event) {
    if (event->type() == QEvent::ToolTip && property_) {
        event->accept();
        auto helpEvent = static_cast<QHelpEvent*>(event);

        Document desc{};
        auto html = desc.append("html");
        html.append("head").append("style", R"(
            div.name {
                font-size: 13pt;
                color: #c8ccd0;;
                font-weight: bold;
            }
            div.help {
                font-size: 12pt;
                margin: 10px 0px 10px 0px;
                padding: 0px 0px 0px 0px;
            }
            table {
                margin: 10px 0px 0px 0px;
                padding: 0px 0px 0px 0px;
            }
        )"_unindent);

        html.append("body").append(property_->getDescription());

        QToolTip::showText(helpEvent->globalPos(), utilqt::toQString(desc));
        return true;
    } else if (event->type() == QEvent::MouseButtonRelease) {
        auto mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::RightButton) {
            if (auto menu = getContextMenu()) {
                menu->exec(mouseEvent->globalPos());
                mouseEvent->accept();
                return true;
            }
        }
    }
    return QWidget::event(event);
}

void PropertyWidgetQt::setSpacingAndMargins(QLayout* layout) { setSpacingAndMargins(this, layout); }

void PropertyWidgetQt::setSpacingAndMargins(QWidget* w, QLayout* layout) {
    const auto m = utilqt::emToPx(w, marginEm);
    layout->setContentsMargins(m, m, m, m);
    layout->setSpacing(utilqt::emToPx(w, spacingEm));
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

void PropertyWidgetQt::setParentPropertyWidget(PropertyWidgetQt* parent) { parent_ = parent; }

void PropertyWidgetQt::paintEvent(QPaintEvent*) {
    QStyleOption o;
    o.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &o, &p, this);
}

}  // namespace inviwo
