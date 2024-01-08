/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2024 Inviwo Foundation
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

#include <modules/qtwidgets/properties/collapsiblegroupboxwidgetqt.h>

#include <inviwo/core/common/inviwoapplication.h>           // for InviwoApplication
#include <inviwo/core/common/inviwoapplicationutil.h>       // for getInviwoApplication
#include <inviwo/core/io/serialization/deserializer.h>      // for Deserializer
#include <inviwo/core/io/serialization/serializer.h>        // for Serializer
#include <inviwo/core/network/networklock.h>                // for NetworkLock
#include <inviwo/core/network/workspacemanager.h>           // for WorkspaceManager
#include <inviwo/core/processors/processor.h>               // for Processor, Processor::NameDis...
#include <inviwo/core/properties/compositeproperty.h>       // for CompositeProperty, CompositeP...
#include <inviwo/core/properties/property.h>                // for Property
#include <inviwo/core/properties/propertyobserver.h>        // for PropertyObserver
#include <inviwo/core/properties/propertyowner.h>           // for PropertyOwner
#include <inviwo/core/properties/propertyownerobserver.h>   // for PropertyOwnerObserver
#include <inviwo/core/properties/propertypresetmanager.h>   // for PropertyPresetManager
#include <inviwo/core/properties/propertywidgetfactory.h>   // for PropertyWidgetFactory
#include <inviwo/core/util/exception.h>                     // for Exception
#include <inviwo/core/util/logcentral.h>                    // for LogCentral, LogWarn, LogError
#include <inviwo/core/util/raiiutils.h>                     // for OnScopeExit, OnScopeExit::Exi...
#include <inviwo/core/util/rendercontext.h>                 // for RenderContext
#include <inviwo/core/util/settings/settings.h>             // for Settings
#include <inviwo/core/util/stdextensions.h>                 // for all_of
#include <inviwo/core/util/zip.h>                           // for zip, zipIterator, zipper, proxy
#include <modules/qtwidgets/editablelabelqt.h>              // for EditableLabelQt
#include <modules/qtwidgets/inviwoqtutils.h>                // for emToPx, toQString
#include <modules/qtwidgets/properties/propertywidgetqt.h>  // for PropertyWidgetQt, PropertyWid...

#include <algorithm>   // for max, find, find_if, min
#include <functional>  // for __base
#include <ostream>     // for operator<<, stringstream, bas...

#include <QAction>       // for QAction
#include <QApplication>  // for QApplication
#include <QByteArray>    // for QByteArray
#include <QCheckBox>     // for QCheckBox
#include <QClipboard>    // for QClipboard
#include <QFontMetrics>  // for QFontMetrics
#include <QGridLayout>   // for QGridLayout
#include <QHBoxLayout>   // for QHBoxLayout
#include <QIcon>         // for QIcon
#include <QLabel>        // for QLabel
#include <QLayout>       // for QLayout
#include <QLayoutItem>   // for QLayoutItem
#include <QList>         // for QList
#include <QMenu>         // for QMenu
#include <QMimeData>     // for QMimeData
#include <QObject>       // for QObject
#include <QRect>         // for QRect
#include <QSizeF>        // for QSizeF
#include <QSizePolicy>   // for QSizePolicy, QSizePolicy::Fixed
#include <QSpacerItem>   // for QSpacerItem
#include <QString>       // for QString
#include <QStringList>   // for QStringList
#include <QToolButton>   // for QToolButton
#include <QVBoxLayout>   // for QVBoxLayout
#include <QWidget>       // for QWidget
#include <Qt>            // for NoFocus, RightToLeft, AlignTop

class QHBoxLayout;
class QVBoxLayout;

namespace inviwo {

class PropertySemantics;

CollapsibleGroupBoxWidgetQt::CollapsibleGroupBoxWidgetQt(CompositeProperty* property,
                                                         bool isCheckable)
    : CollapsibleGroupBoxWidgetQt(property, property, property->getDisplayName(), isCheckable) {}

CollapsibleGroupBoxWidgetQt::CollapsibleGroupBoxWidgetQt(Processor* processor, bool isCheckable)
    : CollapsibleGroupBoxWidgetQt(nullptr, processor, processor->getDisplayName(), isCheckable) {

    // add observer for onProcessorIdentifierChange
    nameChange_ =
        processor->onDisplayNameChange([this](std::string_view newName, std::string_view) {
            displayName_ = newName;
            label_->setText(displayName_);
        });
    setShowIfEmpty(true);
}

CollapsibleGroupBoxWidgetQt::CollapsibleGroupBoxWidgetQt(Settings* settings, bool isCheckable)
    : CollapsibleGroupBoxWidgetQt(nullptr, settings, settings->getIdentifier(), isCheckable) {}

CollapsibleGroupBoxWidgetQt::CollapsibleGroupBoxWidgetQt(Property* property, PropertyOwner* owner,
                                                         const std::string& displayName,
                                                         bool isCheckable)
    : PropertyWidgetQt(property)
    , PropertyOwnerObserver()
    , displayName_(displayName)
    , checked_(false)
    , propertyOwner_(owner)
    , showIfEmpty_(false)
    , checkable_(isCheckable)
    , defaultLabel_{new QLabel("No properties available")}
    , propertyWidgetGroup_{createPropertyLayoutWidget(defaultLabel_).release()}
    , propertyWidgetGroupLayout_{static_cast<QGridLayout*>(propertyWidgetGroup_->layout())}
    , btnCollapse_{[this]() {
        auto button = new QToolButton(this);
        button->setCheckable(true);
        button->setChecked(false);
        button->setObjectName("collapseButton");
        connect(button, &QToolButton::toggled, this, &CollapsibleGroupBoxWidgetQt::setCollapsed);
        button->setFocusPolicy(Qt::StrongFocus);
        return button;
    }()}
    , label_{[this]() {
        auto label = property_ ? new EditableLabelQt(this, property_, false)
                               : new EditableLabelQt(this, displayName_, false);
        label->setObjectName("compositeLabel");
        auto pol = label->sizePolicy();
        pol.setHorizontalStretch(3);
        label->setSizePolicy(pol);
        connect(label, &EditableLabelQt::textChanged, this,
                [this, label]() { setDisplayName(label->getText()); });
        return label;
    }()}
    , resetButton_{[this]() {
        auto button = new QToolButton(this);
        button->setIconSize(utilqt::emToPx(this, QSizeF(2, 2)));
        button->setMinimumSize(utilqt::emToPx(this, QSizeF(2, 2)));
        button->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
        button->setObjectName("resetButton");
        connect(button, &QToolButton::clicked, this, [this]() {
            if (property_) {
                property_->resetToDefaultState();
            } else if (propertyOwner_) {
                propertyOwner_->resetAllProperties();
            }
        });
        button->setFocusPolicy(Qt::NoFocus);

        button->setToolTip(tr("Reset the group of properties to its default state"));
        return button;
    }()}
    , checkBox_{[this]() {
        auto checkBox = new QCheckBox(this);
        checkBox->setMinimumSize(utilqt::emToPx(this, QSizeF(2, 2)));
        auto pol = checkBox->sizePolicy();
        pol.setHorizontalStretch(0);
        checkBox->setSizePolicy(pol);
        checkBox->setChecked(checked_);
        checkBox->setVisible(checkable_);
        QObject::connect(checkBox, &QCheckBox::clicked, this,
                         [this, checkBox]() { setChecked(checkBox->isChecked()); });
        checkBox->setLayoutDirection(Qt::RightToLeft);
        checkBox->setFocusPolicy(Qt::StrongFocus);
        return checkBox;
    }()} {

    setObjectName("CompositeWidget");

    // Add the widget as a property owner observer for dynamic property addition and removal
    owner->addObserver(this);

    updateFocusPolicy();

    QHBoxLayout* heading = new QHBoxLayout();
    heading->setContentsMargins(0, 0, 0, 0);
    heading->setSpacing(getSpacing());
    heading->addWidget(btnCollapse_);
    heading->addWidget(label_);
    heading->addSpacing(getSpacing());
    heading->addWidget(checkBox_);
    heading->addWidget(resetButton_);

    QVBoxLayout* layout = new QVBoxLayout();
    setSpacingAndMargins(layout);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addLayout(heading);
    layout->addWidget(propertyWidgetGroup_);

    // Adjust the margins when using a border, i.e. margin >= border width.
    // Otherwise the border might be overdrawn by children.
    setContentsMargins(margin, margin, margin, margin);
    setLayout(layout);

    updateFromProperty();
}

std::unique_ptr<QMenu> CollapsibleGroupBoxWidgetQt::getContextMenu() {
    auto menu = PropertyWidgetQt::getContextMenu();

    menu->addSeparator();

    if (auto compProperty = dynamic_cast<CompositeProperty*>(property_)) {
        auto addActions = [this, compProperty](QMenu* menu,
                                               CompositeProperty::CollapseAction action) {
            auto current = menu->addAction("&Current");
            auto recursive = menu->addAction("&Recursive");
            auto children = menu->addAction("C&hildren");
            auto siblings = menu->addAction("&Siblings");

            connect(current, &QAction::triggered, this, [compProperty, action]() {
                compProperty->setCollapsed(action, CompositeProperty::CollapseTarget::Current);
            });
            connect(recursive, &QAction::triggered, this, [compProperty, action]() {
                compProperty->setCollapsed(action, CompositeProperty::CollapseTarget::Recursive);
            });
            connect(children, &QAction::triggered, this, [compProperty, action]() {
                compProperty->setCollapsed(action, CompositeProperty::CollapseTarget::Children);
            });
            connect(siblings, &QAction::triggered, this, [compProperty, action]() {
                compProperty->setCollapsed(action, CompositeProperty::CollapseTarget::Siblings);
            });
        };

        auto collapse = menu->addMenu("C&ollapse");
        addActions(collapse, CompositeProperty::CollapseAction::Collapse);

        auto expand = menu->addMenu("&Expand");
        addActions(expand, CompositeProperty::CollapseAction::Expand);
    }

    if (propertyOwner_ && !property_) {
        menu->addAction(utilqt::toQString(displayName_));
        menu->addSeparator();

        auto copyAction = menu->addAction(QIcon(":/svgicons/edit-copy.svg"), "&Copy");
        connect(copyAction, &QAction::triggered, this, [this]() {
            if (!propertyOwner_) return;
            QApplication::clipboard()->setMimeData(getPropertyOwnerMimeData().release());
        });

        auto pasteAction = menu->addAction(QIcon(":/svgicons/edit-paste.svg"), "&Paste");
        pasteAction->setEnabled(QApplication::clipboard()->mimeData()->formats().contains(
            QString("application/x.vnd.inviwo.propertyowner+xml")));

        connect(pasteAction, &QAction::triggered, this, [this]() {
            if (!propertyOwner_) return;

            auto clipboard = QApplication::clipboard();
            auto mimeData = clipboard->mimeData();
            QByteArray data;
            if (mimeData->formats().contains(
                    QString("application/x.vnd.inviwo.propertyowner+xml"))) {
                data = mimeData->data(QString("application/x.vnd.inviwo.propertyowner+xml"));
            } else {
                return;
            }
            std::stringstream ss;
            for (auto d : data) ss << d;

            auto app = propertyOwner_->getInviwoApplication();
            RenderContext::getPtr()->activateDefaultRenderContext();
            try {
                auto d = app->getWorkspaceManager()->createWorkspaceDeserializer(ss, "");
                std::unique_ptr<Processor> propertyOwner;
                d.deserialize("Processor", propertyOwner);
                if (propertyOwner) {
                    NetworkLock lock(propertyOwner_->getProcessor()->getNetwork());
                    for (const auto& source : propertyOwner->getProperties()) {
                        if (auto target =
                                propertyOwner_->getPropertyByIdentifier(source->getIdentifier())) {
                            target->set(source);
                        }
                    }
                }
            } catch (Exception& e) {
                LogError(e.getMessage());
            }
        });

        auto resetAction = menu->addAction(tr("&Reset to default"));
        resetAction->setToolTip(tr("&Reset the group of properties to its default state"));
        connect(resetAction, &QAction::triggered, this,
                [&]() { propertyOwner_->resetAllProperties(); });
    }
    return menu;
}

std::unique_ptr<QMimeData> CollapsibleGroupBoxWidgetQt::getPropertyOwnerMimeData() const {
    auto mimeData = std::make_unique<QMimeData>();
    if (!propertyOwner_) return mimeData;

    Serializer serializer("");
    {
        // Need to set the serialization mode to all temporarily to be able to copy the
        // property.
        std::vector<util::OnScopeExit> toReset;
        for (auto p : propertyOwner_->getPropertiesRecursive()) {
            toReset.emplace_back(PropertyPresetManager::scopedSerializationModeAll(p));
        }
        serializer.serialize("Processor", static_cast<Processor*>(propertyOwner_));
    }
    std::stringstream ss;
    serializer.writeFile(ss);
    auto str = ss.str();
    QByteArray dataArray(str.c_str(), static_cast<int>(str.length()));

    mimeData->setData(QString("application/x.vnd.inviwo.propertyowner+xml"), dataArray);
    mimeData->setData(QString("text/plain"), dataArray);
    return mimeData;
}

QSize CollapsibleGroupBoxWidgetQt::sizeHint() const {
    QSize size = layout()->sizeHint();
    const auto em = fontMetrics().boundingRect('M').width();

    size.setWidth(std::max(static_cast<int>(PropertyWidgetQt::minimumWidthEm * em), size.width()));
    return size;
}

QSize CollapsibleGroupBoxWidgetQt::minimumSizeHint() const {
    QSize size = layout()->sizeHint();
    QSize minSize = layout()->minimumSize();
    const auto em = fontMetrics().boundingRect('M').width();
    size.setWidth(
        std::max(static_cast<int>(PropertyWidgetQt::minimumWidthEm * em), minSize.width()));
    return size;
}

void CollapsibleGroupBoxWidgetQt::setReadOnly(bool readonly) {
    // Disable only child widgets so that the group box can still be collapsed and uncollapsed.
    // Do _not_ call the base class as this would disable the entire group box widget.
    label_->setDisabled(readonly);
    checkBox_->setDisabled(readonly);
    resetButton_->setDisabled(readonly);
}

void CollapsibleGroupBoxWidgetQt::addProperty(Property* prop) {
    insertProperty(prop, properties_.size());
}

std::string CollapsibleGroupBoxWidgetQt::getDisplayName() const { return displayName_; }

void CollapsibleGroupBoxWidgetQt::setDisplayName(const std::string& displayName) {
    displayName_ = displayName;
    if (propertyOwner_) {
        if (auto p = dynamic_cast<Processor*>(propertyOwner_)) {
            p->setDisplayName(displayName);
        }
    }
}

const std::vector<Property*>& CollapsibleGroupBoxWidgetQt::getProperties() const {
    return properties_;
}
const std::vector<PropertyWidgetQt*>& CollapsibleGroupBoxWidgetQt::getPropertyWidgets() const {
    return propertyWidgets_;
}

PropertyWidgetQt* CollapsibleGroupBoxWidgetQt::widgetForProperty(Property* property) const {
    if (auto it = std::find(properties_.begin(), properties_.end(), property);
        it != properties_.end()) {
        return *(propertyWidgets_.begin() + std::distance(properties_.begin(), it));
    }
    return nullptr;
}
Property* CollapsibleGroupBoxWidgetQt::propertyForWidget(PropertyWidgetQt* widget) const {
    if (auto it = std::find(propertyWidgets_.begin(), propertyWidgets_.end(), widget);
        it != propertyWidgets_.end()) {
        return *(properties_.begin() + std::distance(propertyWidgets_.begin(), it));
    }
    return nullptr;
}

void CollapsibleGroupBoxWidgetQt::toggleCollapsed() { setCollapsed(!isCollapsed()); }

bool CollapsibleGroupBoxWidgetQt::isCollapsed() const { return btnCollapse_->isChecked(); }

bool CollapsibleGroupBoxWidgetQt::isChecked() const { return checked_; }

void CollapsibleGroupBoxWidgetQt::setChecked(bool checked) {
    if (!checkable_) {
        return;
    }

    checked_ = checked;
    // update checkbox
    checkBox_->setChecked(checked_);
}

bool CollapsibleGroupBoxWidgetQt::isCheckable() const { return checkable_; }

void CollapsibleGroupBoxWidgetQt::setCheckable(bool checkable) {
    if (checkable_ == checkable) {
        return;
    }

    checkable_ = checkable;
    // update header
    checkBox_->setVisible(checkable_);
    updateFocusPolicy();
}

void CollapsibleGroupBoxWidgetQt::setCheckBoxText(std::string_view text) {
    auto qText = utilqt::toQString(text);

    auto fm = checkBox_->fontMetrics();
    auto mw = fm.boundingRect(qText).width();
    checkBox_->setMinimumWidth(mw + utilqt::emToPx(checkBox_, 3));
    checkBox_->setText(qText);
}

void CollapsibleGroupBoxWidgetQt::setCheckBoxVisible(bool visible) {
    checkBox_->setVisible(visible);
}
void CollapsibleGroupBoxWidgetQt::setCheckBoxReadonly(bool readonly) {
    checkBox_->setDisabled(readonly);
}

bool CollapsibleGroupBoxWidgetQt::isChildRemovable() const { return false; }

void CollapsibleGroupBoxWidgetQt::setVisible(bool visible) {
    bool empty = util::all_of(properties_, [](Property* w) { return !w->getVisible(); });
    defaultLabel_->setVisible(empty);

    if (showIfEmpty_ || !empty) {
        PropertyWidgetQt::setVisible(visible);
    } else {
        PropertyWidgetQt::setVisible(false);
    }
}

void CollapsibleGroupBoxWidgetQt::setCollapsed(bool collapse) {
    setUpdatesEnabled(false);
    propertyWidgetGroup_->setVisible(!collapse);
    btnCollapse_->setChecked(collapse);
    setUpdatesEnabled(true);
}

void CollapsibleGroupBoxWidgetQt::updateFromProperty() { oldWidgets_.clear(); }

void CollapsibleGroupBoxWidgetQt::onDidAddProperty(Property* prop, size_t index) {
    insertProperty(prop, index);
}

void CollapsibleGroupBoxWidgetQt::onWillRemoveProperty(Property* /*prop*/, size_t index) {
    setUpdatesEnabled(false);
    propertyWidgetGroupLayout_->setEnabled(false);
    util::OnScopeExit enableUpdates([&]() {
        propertyWidgetGroupLayout_->setEnabled(true);
        setUpdatesEnabled(true);
    });

    PropertyWidgetQt* propertyWidget = propertyWidgets_[index];

    if (propertyWidget) {
        if (isChildRemovable()) {
            const int widgetIndex = propertyWidgetGroupLayout_->indexOf(propertyWidget);
            int row = 0, col = 0, rowSpan = 0, colSpan = 0;
            propertyWidgetGroupLayout_->getItemPosition(widgetIndex, &row, &col, &rowSpan,
                                                        &colSpan);

            // remove additional widget containing the removal button
            if (auto layoutItem = propertyWidgetGroupLayout_->itemAtPosition(row, 2)) {
                if (auto w = layoutItem->widget()) {
                    propertyWidgetGroupLayout_->removeWidget(w);
                    delete w;
                }
            }
        }

        propertyWidgetGroupLayout_->removeWidget(propertyWidget);
    }
    propertyWidgets_.erase(propertyWidgets_.begin() + index);
    properties_.erase(properties_.begin() + index);
    delete propertyWidget;

    bool empty = util::all_of(properties_, [](Property* w) { return !w->getVisible(); });
    defaultLabel_->setVisible(empty);
}

void CollapsibleGroupBoxWidgetQt::onSetSemantics(Property* prop, const PropertySemantics&) {
    auto app = util::getInviwoApplication(prop);
    if (!app) return;
    auto factory = app->getPropertyWidgetFactory();

    setUpdatesEnabled(false);
    propertyWidgetGroupLayout_->setEnabled(false);
    util::OnScopeExit enableUpdates([&]() {
        propertyWidgetGroupLayout_->setEnabled(true);
        setUpdatesEnabled(true);
    });

    auto pit = std::find(properties_.begin(), properties_.end(), prop);
    auto wit = std::find_if(propertyWidgets_.begin(), propertyWidgets_.end(),
                            [&](auto w) { return w->getProperty() == prop; });

    if (pit != properties_.end() && wit != propertyWidgets_.end()) {
        if (auto newWidget = static_cast<PropertyWidgetQt*>(factory->create(prop).release())) {
            propertyWidgetGroupLayout_->replaceWidget(*wit, newWidget, Qt::FindDirectChildrenOnly);

            oldWidgets_.emplace_back(*wit);
            prop->deregisterWidget(*wit);
            (*wit)->setParent(nullptr);
            // hide the underlying QWidget. Otherwise it will be shown as a floating window
            // since it no longer has a parent widget.
            static_cast<QWidget*>(*wit)->setVisible(false);
            // Replace the item in propertyWidgets_;
            *wit = newWidget;

            newWidget->setNestedDepth(this->getNestedDepth());
            newWidget->setParentPropertyWidget(this);
            newWidget->initState();
            RenderContext::getPtr()->activateDefaultRenderContext();

            // need to re-set tab order for all following widgets to ensure tab order is correct
            // (see http://doc.qt.io/qt-5/qwidget.html#setTabOrder)
            for (auto it = propertyWidgets_.begin() + 1; it != propertyWidgets_.end(); ++it) {
                setTabOrder((*(it - 1)), (*it));
            }
        } else {
            LogWarn("Could not change semantic for property: " << prop->getClassIdentifier());
        }
    }
}

void CollapsibleGroupBoxWidgetQt::onSetReadOnly(Property* property, bool readonly) {
    if (property == property_) {
        PropertyWidgetQt::onSetReadOnly(property, readonly);
    }
}

void CollapsibleGroupBoxWidgetQt::onSetVisible(Property* property, bool visible) {
    setUpdatesEnabled(false);
    propertyWidgetGroupLayout_->setEnabled(false);
    util::OnScopeExit enableUpdates([&]() {
        propertyWidgetGroupLayout_->setEnabled(true);
        setUpdatesEnabled(true);
    });

    if (property == property_) {
        PropertyWidgetQt::onSetVisible(property, visible);
    }

    if (isChildRemovable()) {
        for (auto&& elem : util::zip(properties_, propertyWidgets_)) {
            if (elem.first() == property) {
                const int widgetIndex = propertyWidgetGroupLayout_->indexOf(elem.second());
                int row = 0, col = 0, rowSpan = 0, colSpan = 0;
                propertyWidgetGroupLayout_->getItemPosition(widgetIndex, &row, &col, &rowSpan,
                                                            &colSpan);

                if (auto layoutItem = propertyWidgetGroupLayout_->itemAtPosition(row, 2)) {
                    if (auto w = layoutItem->widget()) {
                        // change visibility of button widgets
                        w->setVisible(visible);
                    }
                }
                break;
            }
        }
    }
}

void CollapsibleGroupBoxWidgetQt::setPropertyOwner(PropertyOwner* propertyOwner) {
    propertyOwner_ = propertyOwner;
}

PropertyOwner* CollapsibleGroupBoxWidgetQt::getPropertyOwner() const { return propertyOwner_; }

void CollapsibleGroupBoxWidgetQt::setShowIfEmpty(bool val) { showIfEmpty_ = val; }

void CollapsibleGroupBoxWidgetQt::setEmptyLabelString(const std::string& str) {
    defaultLabel_->setText(utilqt::toQString(str));
}

std::unique_ptr<QWidget> CollapsibleGroupBoxWidgetQt::createPropertyLayoutWidget(
    QLabel* defaultLabel) {
    auto widget = std::make_unique<QWidget>();
    widget->setObjectName("CompositeContents");

    auto propertyLayout = std::make_unique<QGridLayout>();
    propertyLayout->setObjectName("PropertyWidgetLayout");
    propertyLayout->setAlignment(Qt::AlignTop);
    const auto space = utilqt::emToPx(widget.get(), spacingEm);
    propertyLayout->setContentsMargins(space * 2, space, 0, space);
    propertyLayout->setHorizontalSpacing(0);
    propertyLayout->setVerticalSpacing(space);

    auto layout = propertyLayout.release();
    widget->setLayout(layout);

    // add default label to widget layout, this will also set the correct stretching and spacing
    layout->addWidget(defaultLabel, 0, 0);
    layout->addItem(new QSpacerItem(space, 1, QSizePolicy::Fixed), 0, 1);
    layout->setColumnStretch(0, 1);
    layout->setColumnStretch(1, 0);

    return widget;
}

void CollapsibleGroupBoxWidgetQt::addButtonLayout(QGridLayout* layout, int row, Property* prop) {
    auto createButton = [this](const std::string& objectName, const std::string& tooltip) {
        auto button = new QToolButton(this);
        button->setObjectName(utilqt::toQString(objectName));
        button->setToolTip(utilqt::toQString(tooltip));
        return button;
    };

    auto buttonWidget = new QWidget(this);
    layout->addWidget(buttonWidget, row, 2);

    auto removePropertyBtn = createButton(
        "removeListItemButton", fmt::format("Remove property '{}'", prop->getDisplayName()));

    connect(removePropertyBtn, &QToolButton::clicked, this, [layout, prop, buttonWidget]() {
        // need to activate the default render context in case the property contains member
        // depending on OpenGL
        RenderContext::getPtr()->activateDefaultRenderContext();
        if (prop->getOwner()) {
            prop->getOwner()->removeProperty(prop);
        } else {
            layout->removeWidget(buttonWidget);
            delete buttonWidget;
        }
    });

    auto moveUpBtn = createButton("moveListItemUpButton", "Move up");
    auto moveDownBtn = createButton("moveListItemDownButton", "Move down");
    connect(moveUpBtn, &QToolButton::clicked, this, [prop]() {
        if (auto owner = prop->getOwner()) {
            if (auto it = owner->find(prop); it != owner->cend()) {
                if (it != owner->cbegin()) {
                    RenderContext::getPtr()->activateDefaultRenderContext();
                    owner->move(prop, std::distance(owner->cbegin(), it) - 1);
                }
            }
        }
    });
    connect(moveDownBtn, &QToolButton::clicked, this, [prop]() {
        if (auto owner = prop->getOwner()) {
            if (auto it = owner->find(prop); it != owner->cend()) {
                if (it + 1 != owner->cend()) {
                    RenderContext::getPtr()->activateDefaultRenderContext();
                    owner->move(prop, std::distance(owner->cbegin(), it) + 1);
                }
            }
        }
    });

    auto buttonLayout = new QVBoxLayout();
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->setSpacing(0);
    buttonLayout->addWidget(removePropertyBtn);
    buttonLayout->addWidget(moveUpBtn);
    buttonLayout->addWidget(moveDownBtn);
    buttonLayout->addStretch(1);

    buttonWidget->setLayout(buttonLayout);
    buttonWidget->setVisible(prop->getVisible());
}

void CollapsibleGroupBoxWidgetQt::insertProperty(Property* prop, size_t index) {
    setUpdatesEnabled(false);
    propertyWidgetGroupLayout_->setEnabled(false);
    util::OnScopeExit enableUpdates([&]() {
        propertyWidgetGroupLayout_->setEnabled(true);
        setUpdatesEnabled(true);
    });

    const size_t insertIndex = std::min(index, properties_.size());
    const bool insertAtEnd = (insertIndex == properties_.size());

    auto insertPoint = properties_.begin() + insertIndex;
    auto widgetInsertPoint = propertyWidgets_.begin() + insertIndex;

    properties_.insert(insertPoint, prop);
    PropertyObserver::addObservation(prop);

    auto factory = InviwoApplication::getPtr()->getPropertyWidgetFactory();
    if (auto propertyWidget = static_cast<PropertyWidgetQt*>(factory->create(prop).release())) {
        propertyWidgets_.insert(widgetInsertPoint, propertyWidget);

        insertPropertyWidget(propertyWidget, insertAtEnd);
        RenderContext::getPtr()->activateDefaultRenderContext();

        // need to re-set tab order for all following widgets to ensure tab order is correct
        // (see http://doc.qt.io/qt-5/qwidget.html#setTabOrder)
        for (auto wit = propertyWidgets_.begin() + 1; wit != propertyWidgets_.end(); ++wit) {
            setTabOrder(*(wit - 1), *wit);
        }
    } else {
        LogWarn("Could not find a widget for property: " << prop->getClassIdentifier());

        // insert empty element to keep property widget vector in sync with property vector
        propertyWidgets_.insert(widgetInsertPoint, nullptr);
    }
}

void CollapsibleGroupBoxWidgetQt::insertPropertyWidget(PropertyWidgetQt* propertyWidget,
                                                       bool insertAtEnd) {
    auto addPropertyWidget = [&](QGridLayout* layout, int row, PropertyWidgetQt* widget) {
        if (auto collapsibleWidget = dynamic_cast<CollapsibleGroupBoxWidgetQt*>(widget)) {
            collapsibleWidget->setNestedDepth(this->getNestedDepth() + 1);
            // make the collapsible widget go all the way to the right border
            layout->addWidget(widget, row, 0, 1, 2);
        } else {  // not a collapsible widget
            widget->setNestedDepth(this->getNestedDepth());
            // property widget should only be added to the left column of the layout
            layout->addWidget(widget, row, 0);
        }

        if (isChildRemovable()) {
            addButtonLayout(layout, row, widget->getProperty());
        }

        widget->setParentPropertyWidget(this);
        widget->initState();
    };

    if (insertAtEnd) {
        // append property widget
        addPropertyWidget(propertyWidgetGroupLayout_, propertyWidgetGroupLayout_->rowCount(),
                          propertyWidget);
    } else {
        // recreate property widget grid layout from scratch since QGridLayout does not support
        // inserting rows.
        //
        // Assigning another widget to an already occupied grid cell is possible, but the
        // widgets are put on top of each other.
        auto layoutWidget = createPropertyLayoutWidget(defaultLabel_);
        auto layout = static_cast<QGridLayout*>(layoutWidget->layout());

        util::OnScopeExit freeOldLayout([&]() {
            // get rid of previous property widget and layout
            auto parentLayout = [&]() -> QLayout* {
                if (propertyWidgetGroup_->parentWidget()) {
                    return propertyWidgetGroup_->parentWidget()->layout();
                } else {
                    return nullptr;
                }
            }();

            if (parentLayout) {
                parentLayout->removeWidget(propertyWidgetGroup_);
                parentLayout->addWidget(layoutWidget.get());
            }

            delete propertyWidgetGroup_;
            propertyWidgetGroup_ = layoutWidget.release();
            propertyWidgetGroupLayout_ = layout;
        });

        for (auto w : propertyWidgets_) {
            if (!w) continue;

            int insertRow = layout->rowCount();
            if (w == propertyWidget) {
                // widget not yet in layout
                addPropertyWidget(layout, insertRow, w);
            } else {
                int widgetIndex = propertyWidgetGroupLayout_->indexOf(w);
                if (widgetIndex > -1) {
                    // move widget from previous layout to new layout
                    int row = 0, col = 0, rowSpan = 0, colSpan = 0;
                    propertyWidgetGroupLayout_->getItemPosition(widgetIndex, &row, &col, &rowSpan,
                                                                &colSpan);
                    layout->addWidget(w, insertRow, 0, rowSpan, colSpan);

                    // take care of additional widget containing the removal button
                    if (auto item = propertyWidgetGroupLayout_->itemAtPosition(row, 2)) {
                        if (auto buttons = item->widget()) {
                            layout->addWidget(buttons, insertRow, 2, 1, 1);
                        }
                    }
                }
            }
        }
    }
}

void CollapsibleGroupBoxWidgetQt::updateFocusPolicy() {
    setFocusPolicy(btnCollapse_->focusPolicy());
    setFocusProxy(btnCollapse_);
}

}  // namespace inviwo
