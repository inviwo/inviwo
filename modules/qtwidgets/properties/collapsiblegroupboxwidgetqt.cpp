/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2018 Inviwo Foundation
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
#include <modules/qtwidgets/properties/compositepropertywidgetqt.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/property.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/util/settings/settings.h>
#include <inviwo/core/util/settings/systemsettings.h>
#include <inviwo/core/properties/propertywidgetfactory.h>
#include <inviwo/core/network/processornetwork.h>
#include <modules/qtwidgets/editablelabelqt.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QLineEdit>
#include <QToolButton>
#include <QGroupBox>
#include <QPushButton>
#include <QGridLayout>
#include <QLabel>
#include <QCheckBox>
#include <QMenu>
#include <QAction>
#include <warn/pop>

namespace inviwo {

CollapsibleGroupBoxWidgetQt::CollapsibleGroupBoxWidgetQt(CompositeProperty* property,
                                                         bool isCheckable)
    : CollapsibleGroupBoxWidgetQt(property, property, property->getDisplayName(), isCheckable) {
}

CollapsibleGroupBoxWidgetQt::CollapsibleGroupBoxWidgetQt(Processor* processor, bool isCheckable)
    : CollapsibleGroupBoxWidgetQt(nullptr, processor, processor->getDisplayName(), isCheckable) {

    // add observer for onProcessorIdentifierChange
    processor->ProcessorObservable::addObserver(this);
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
    , collapsed_(false)
    , checked_(false)
    , propertyOwner_(owner)
    , showIfEmpty_(false)
    , checkable_(isCheckable) {
    setObjectName("CompositeWidget");

    // Add the widget as a property owner observer for dynamic property addition and removal
    owner->addObserver(this);

    propertyWidgetGroupLayout_ = new QGridLayout();
    propertyWidgetGroupLayout_->setAlignment(Qt::AlignTop);
    propertyWidgetGroupLayout_->setContentsMargins(
        PropertyWidgetQt::spacing, PropertyWidgetQt::spacing, 0, PropertyWidgetQt::spacing);
    propertyWidgetGroupLayout_->setHorizontalSpacing(0);
    propertyWidgetGroupLayout_->setVerticalSpacing(PropertyWidgetQt::spacing);

    propertyWidgetGroup_ = new QWidget(this);
    propertyWidgetGroup_->setObjectName("CompositeContents");
    propertyWidgetGroup_->setLayout(propertyWidgetGroupLayout_);

    defaultLabel_ = new QLabel("No properties available");

    propertyWidgetGroupLayout_->addWidget(defaultLabel_, 0, 0);
    propertyWidgetGroupLayout_->addItem(
        new QSpacerItem(PropertyWidgetQt::spacing, 1, QSizePolicy::Fixed), 0, 1);
    propertyWidgetGroupLayout_->setColumnStretch(0, 1);
    propertyWidgetGroupLayout_->setColumnStretch(1, 0);

    btnCollapse_ = new QToolButton(this);
    btnCollapse_->setObjectName("collapseButton");
    btnCollapse_->setIcon(QIcon(":/stylesheets/images/arrow_lighter_down.png"));
    connect(btnCollapse_, &QToolButton::clicked, this,
            &CollapsibleGroupBoxWidgetQt::toggleCollapsed);

    if (property_) {
        label_ = new EditableLabelQt(this, property_, false);
    } else {
        label_ = new EditableLabelQt(this, displayName_, false);
    }
    label_->setObjectName("compositeLabel");
    QSizePolicy labelPol = label_->sizePolicy();
    labelPol.setHorizontalStretch(10);
    label_->setSizePolicy(labelPol);
    connect(label_, &EditableLabelQt::textChanged, this,
            [&]() { setDisplayName(label_->getText()); });

    resetButton_ = new QToolButton(this);
    resetButton_->setIconSize(QSize(20, 20));
    resetButton_->setObjectName("resetButton");
    connect(resetButton_, &QToolButton::clicked, this, [&]() {
        if (property_) {
            property_->resetToDefaultState();
        } else if (propertyOwner_) {
            propertyOwner_->resetAllPoperties();
        }
    });

    resetButton_->setToolTip(tr("Reset the group of properties to its default state"));

    checkBox_ = new QCheckBox(this);
    checkBox_->setMinimumSize(5, 5);
    checkBox_->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred));
    checkBox_->setChecked(checked_);
    checkBox_->setVisible(checkable_);

    QObject::connect(checkBox_, &QCheckBox::clicked, this,
                     [&]() { setChecked(checkBox_->isChecked()); });

    QHBoxLayout* heading = new QHBoxLayout();
    heading->setContentsMargins(0, 0, 0, 0);
    heading->setSpacing(PropertyWidgetQt::spacing);
    heading->addWidget(btnCollapse_);
    heading->addWidget(label_);
    heading->addStretch(1);
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
    this->setContentsMargins(margin, margin, margin, margin);

    this->setLayout(layout);

    updateFromProperty();
}

std::unique_ptr<QMenu> CollapsibleGroupBoxWidgetQt::getContextMenu() {
    auto menu = PropertyWidgetQt::getContextMenu();

    if (propertyOwner_ && !property_) {
        menu->addAction(QString::fromStdString(displayName_));
        menu->addSeparator();

        auto resetAction = menu->addAction(tr("&Reset to default"));
        resetAction->setToolTip(tr("&RReset the group of properties to its default state"));
        connect(resetAction, &QAction::triggered, this,
                [&]() { propertyOwner_->resetAllPoperties(); });
    }
    return menu;
}

QSize CollapsibleGroupBoxWidgetQt::sizeHint() const {
    QSize size = layout()->sizeHint();
    size.setWidth(std::max(PropertyWidgetQt::minimumWidth, size.width()));
    return size;
}

QSize CollapsibleGroupBoxWidgetQt::minimumSizeHint() const {
    QSize size = layout()->sizeHint();
    QSize minSize = layout()->minimumSize();
    size.setWidth(std::max(PropertyWidgetQt::minimumWidth, minSize.width()));
    return size;
}

void CollapsibleGroupBoxWidgetQt::setReadOnly(bool readonly) {
    label_->setDisabled(readonly);
    checkBox_->setDisabled(readonly);
    resetButton_->setDisabled(readonly);
}

void CollapsibleGroupBoxWidgetQt::addProperty(Property* prop) {
    properties_.push_back(prop);
    PropertyObserver::addObservation(prop);

    auto factory = InviwoApplication::getPtr()->getPropertyWidgetFactory();
    if (auto propertyWidget = static_cast<PropertyWidgetQt*>(factory->create(prop).release())) {
        if (auto collapsibleWidget = dynamic_cast<CollapsibleGroupBoxWidgetQt*>(propertyWidget)) {
            collapsibleWidget->setNestedDepth(this->getNestedDepth() + 1);
            // make the collapsible widget go all the way to the right border
            propertyWidgetGroupLayout_->addWidget(propertyWidget,
                                                  propertyWidgetGroupLayout_->rowCount(), 0, 1, -1);
        } else {  // not a collapsible widget
            propertyWidget->setNestedDepth(this->getNestedDepth());
            // property widget should only be added to the left column of the layout
            propertyWidgetGroupLayout_->addWidget(propertyWidget,
                                                  propertyWidgetGroupLayout_->rowCount(), 0);
        }

        propertyWidgets_.push_back(propertyWidget);

        propertyWidget->setParentPropertyWidget(this);
        propertyWidget->initState();

    } else {
        LogWarn("Could not find a widget for property: " << prop->getClassIdentifier());
    }
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

const std::vector<Property*>& CollapsibleGroupBoxWidgetQt::getProperties() { return properties_; }

void CollapsibleGroupBoxWidgetQt::toggleCollapsed() { setCollapsed(!isCollapsed()); }

bool CollapsibleGroupBoxWidgetQt::isCollapsed() const { return collapsed_; }

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
}

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
    if (collapsed_ && !collapse) {
        propertyWidgetGroup_->show();
        btnCollapse_->setIcon(QIcon(":/stylesheets/images/arrow_lighter_down.png"));
    } else if (!collapsed_ && collapse) {
        propertyWidgetGroup_->hide();
        btnCollapse_->setIcon(QIcon(":/stylesheets/images/arrow_lighter_right.png"));
    }
    collapsed_ = collapse;
    setUpdatesEnabled(true);
}

void CollapsibleGroupBoxWidgetQt::updateFromProperty() { oldWidgets_.clear(); }

void CollapsibleGroupBoxWidgetQt::onDidAddProperty(Property* prop, size_t index) {
    setUpdatesEnabled(false);
    auto insertPoint = properties_.begin() + index;
    if (insertPoint != properties_.end()) ++insertPoint;

    properties_.insert(insertPoint, prop);
    PropertyObserver::addObservation(prop);

    auto factory = InviwoApplication::getPtr()->getPropertyWidgetFactory();
    if (auto propertyWidget = static_cast<PropertyWidgetQt*>(factory->create(prop).release())) {
        const int insertPos = static_cast<int>(index) + 1;

        if (auto collapsibleWidget = dynamic_cast<CollapsibleGroupBoxWidgetQt*>(propertyWidget)) {
            collapsibleWidget->setNestedDepth(this->getNestedDepth() + 1);
            // make the collapsible widget go all the way to the right border
            propertyWidgetGroupLayout_->addWidget(propertyWidget, insertPos, 0, 1, -1);

        } else {  // not a collapsible widget
            propertyWidget->setNestedDepth(this->getNestedDepth());
            // property widget should only be added to the left column of the layout
            propertyWidgetGroupLayout_->addWidget(propertyWidget, insertPos, 0);
        }

        auto widgetInsertPoint = propertyWidgets_.begin() + index;
        if (widgetInsertPoint != propertyWidgets_.end()) ++widgetInsertPoint;

        propertyWidgets_.insert(widgetInsertPoint, propertyWidget);

        propertyWidget->setParentPropertyWidget(this);
        propertyWidget->initState();

    } else {
        LogWarn("Could not find a widget for property: " << prop->getClassIdentifier());
    }
    setUpdatesEnabled(true);
}

void CollapsibleGroupBoxWidgetQt::onWillRemoveProperty(Property* /*prop*/, size_t index) {
    PropertyWidgetQt* propertyWidget = propertyWidgets_[index];

    propertyWidgetGroupLayout_->removeWidget(propertyWidget);
    propertyWidgets_.erase(propertyWidgets_.begin() + index);
    properties_.erase(properties_.begin() + index);
    delete propertyWidget;
}

void CollapsibleGroupBoxWidgetQt::onProcessorDisplayNameChanged(Processor* processor,
                                                                const std::string&) {
    displayName_ = processor->getDisplayName();
    label_->setText(displayName_);
}

void CollapsibleGroupBoxWidgetQt::onSetSemantics(Property* prop, const PropertySemantics&) {
    auto app = util::getInviwoApplication(prop);
    if (!app) return;
    auto factory = app->getPropertyWidgetFactory();

    setUpdatesEnabled(false);

    auto pit = std::find(properties_.begin(), properties_.end(), prop);
    auto wit = std::find_if(propertyWidgets_.begin(), propertyWidgets_.end(),
                            [&](auto w) { return w->getProperty() == prop; });

    if (pit != properties_.end() && wit != propertyWidgets_.end()) {
        if (auto newWidget = static_cast<PropertyWidgetQt*>(factory->create(prop).release())) {
            propertyWidgetGroupLayout_->replaceWidget(*wit, newWidget, Qt::FindDirectChildrenOnly);

            oldWidgets_.emplace_back(*wit);
            prop->deregisterWidget(*wit);
            (*wit)->setParent(nullptr);
            // Replace the item in propertyWidgets_;
            *wit = newWidget;

            newWidget->setNestedDepth(this->getNestedDepth());
            newWidget->setParentPropertyWidget(this);
            newWidget->initState();

        } else {
            LogWarn("Could not change semantic for property: " << prop->getClassIdentifier());
        }
    }
    setUpdatesEnabled(true);
}

void CollapsibleGroupBoxWidgetQt::onSetReadOnly(Property* property, bool readonly) {
    if (property == property_) {
        PropertyWidgetQt::onSetReadOnly(property, readonly);
    }
}

void CollapsibleGroupBoxWidgetQt::onSetVisible(Property* property, bool visible) {
    if (property == property_) {
        PropertyWidgetQt::onSetVisible(property, visible);
    }
}

void CollapsibleGroupBoxWidgetQt::onSetUsageMode(Property* property, UsageMode usageMode) {
    if (property == property_) {
        PropertyWidgetQt::onSetUsageMode(property, usageMode);
    }
}

void CollapsibleGroupBoxWidgetQt::setPropertyOwner(PropertyOwner* propertyOwner) {
    propertyOwner_ = propertyOwner;
}

PropertyOwner* CollapsibleGroupBoxWidgetQt::getPropertyOwner() const { return propertyOwner_; }

const std::vector<PropertyWidgetQt*>& CollapsibleGroupBoxWidgetQt::getPropertyWidgets() {
    return propertyWidgets_;
}

void CollapsibleGroupBoxWidgetQt::setShowIfEmpty(bool val) { showIfEmpty_ = val; }
}  // namespace inviwo
