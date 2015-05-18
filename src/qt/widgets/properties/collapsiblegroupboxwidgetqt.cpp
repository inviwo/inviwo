/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#include <inviwo/qt/widgets/properties/collapsiblegroupboxwidgetqt.h>
#include <inviwo/qt/widgets/properties/compositepropertywidgetqt.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/properties/property.h>
#include <inviwo/core/util/settings/systemsettings.h>
#include <inviwo/core/properties/propertywidgetfactory.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/qt/widgets/editablelabelqt.h>

#include <QLineEdit>
#include <QToolButton>
#include <QGroupBox>
#include <QPushButton>
#include <QGridLayout>
#include <QLabel>
#include <QCheckBox>

namespace inviwo {
    
CollapsibleGroupBoxWidgetQt::CollapsibleGroupBoxWidgetQt(std::string displayName, bool isCheckable)
    : PropertyWidgetQt()
    , PropertyOwnerObserver()
    , displayName_(displayName)
    , collapsed_(false)
    , checked_(false)
    , propertyOwner_(nullptr)
    , showIfEmpty_(false)
    , checkable_(isCheckable)
    , maxNumNestedShades_(4)
    , nestedDepth_(0) {
    setObjectName("CompositeWidget");

    setNestedDepth(nestedDepth_);

    generateWidget();
    updateFromProperty();
}

void CollapsibleGroupBoxWidgetQt::generateWidget() {
    propertyWidgetGroupLayout_ = new QGridLayout();
    propertyWidgetGroupLayout_->setAlignment(Qt::AlignTop);
    propertyWidgetGroupLayout_->setContentsMargins(
        PropertyWidgetQt::SPACING, PropertyWidgetQt::SPACING, 0, PropertyWidgetQt::SPACING);
    propertyWidgetGroupLayout_->setHorizontalSpacing(0);
    propertyWidgetGroupLayout_->setVerticalSpacing(PropertyWidgetQt::SPACING);

    propertyWidgetGroup_ = new QWidget(this);
    propertyWidgetGroup_->setObjectName("CompositeContents");
    propertyWidgetGroup_->setLayout(propertyWidgetGroupLayout_);

    defaultLabel_ = new QLabel("No properties available");

    propertyWidgetGroupLayout_->addWidget(defaultLabel_, 0, 0);
    propertyWidgetGroupLayout_->addItem(new QSpacerItem(PropertyWidgetQt::SPACING, 1, QSizePolicy::Fixed), 0, 1);
    propertyWidgetGroupLayout_->setColumnStretch(0, 1);
    propertyWidgetGroupLayout_->setColumnStretch(1, 0);
    //propertyWidgetGroupLayout_->setColumnMinimumWidth(1, PropertyWidgetQt::SPACING);
    

    btnCollapse_ = new QToolButton(this);
    btnCollapse_->setObjectName("collapseButton");
    btnCollapse_->setIcon(QIcon(":/stylesheets/images/arrow_lighter_down.png"));
    connect(btnCollapse_, SIGNAL(clicked()), this, SLOT(toggleCollapsed()));

    label_ = new EditableLabelQt(this, displayName_, false);
    label_->setObjectName("compositeLabel");
    QSizePolicy labelPol = label_->sizePolicy();
    labelPol.setHorizontalStretch(10);
    label_->setSizePolicy(labelPol);
    connect(label_, SIGNAL(textChanged()), this, SLOT(labelDidChange()));

    QToolButton* resetButton = new QToolButton(this);
    resetButton->setIconSize(QSize(20, 20));
    resetButton->setObjectName("resetButton");
    connect(resetButton, SIGNAL(clicked()), this, SLOT(resetPropertyToDefaultState()));
    resetButton->setToolTip(tr("Reset the group of properties to its default state"));

    checkBox_ = new QCheckBox(this);
    checkBox_->setMinimumSize(5, 5);
    //checkBox_->setMaximumSize(13, 13);
    checkBox_->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred));
    checkBox_->setChecked(checked_);
    checkBox_->setVisible(checkable_);

    QObject::connect(checkBox_, SIGNAL(clicked()), this, SLOT(checkedStateChanged()));

    QHBoxLayout* heading = new QHBoxLayout();
    heading->setContentsMargins(0, 0, 0, 0);
    heading->setSpacing(PropertyWidgetQt::SPACING);
    heading->addWidget(btnCollapse_);
    heading->addWidget(label_);
    heading->addStretch(1);
    heading->addWidget(checkBox_);
    heading->addWidget(resetButton);

    QVBoxLayout* layout = new QVBoxLayout();
    setSpacingAndMargins(layout);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addLayout(heading);
    layout->addWidget(propertyWidgetGroup_);

    // Adjust the margins when using a border, i.e. margin >= border width.
    // Otherwise the border might be overdrawn by children.
    const int margin = 0;
    this->setContentsMargins(margin, margin, margin, margin);

    this->setLayout(layout);
}

QSize CollapsibleGroupBoxWidgetQt::sizeHint() const {
    QSize size = layout()->sizeHint();
    size.setWidth(std::max(PropertyWidgetQt::MINIMUM_WIDTH, size.width()));
    return size;
}

QSize CollapsibleGroupBoxWidgetQt::minimumSizeHint() const {
    QSize size = layout()->sizeHint();
    QSize minSize = layout()->minimumSize();
    size.setWidth(std::max(PropertyWidgetQt::MINIMUM_WIDTH, minSize.width()));
    return size;
}

void CollapsibleGroupBoxWidgetQt::showWidget() {
    for (auto& elem : propertyWidgets_) {
        elem->showWidget();
    }
    PropertyWidgetQt::showWidget();
}

void CollapsibleGroupBoxWidgetQt::hideWidget() {
    PropertyWidgetQt::hideWidget();
    for (auto& elem : propertyWidgets_) {
        elem->hideWidget();
    }
}

void CollapsibleGroupBoxWidgetQt::addProperty(Property* prop) {
    properties_.push_back(prop);

    PropertyWidgetQt* propertyWidget =
        static_cast<PropertyWidgetQt*>(PropertyWidgetFactory::getPtr()->create(prop));

    if (propertyWidget) {
        propertyWidget->hideWidget();

        auto collapsibleWidget = dynamic_cast<CollapsibleGroupBoxWidgetQt *>(propertyWidget);
        if (collapsibleWidget) {
            collapsibleWidget->setNestedDepth(this->getNestedDepth() + 1);
            // make the collapsible widget go all the way to the right border
            propertyWidgetGroupLayout_->addWidget(propertyWidget, propertyWidgetGroupLayout_->rowCount(), 0, 1, -1);
        }
        else { // not a collapsible widget
            // property widget should only be added to the left column of the layout
            propertyWidgetGroupLayout_->addWidget(propertyWidget, propertyWidgetGroupLayout_->rowCount(), 0);
        }

        propertyWidgets_.push_back(propertyWidget);
        prop->registerWidget(propertyWidget);
        connect(propertyWidget, SIGNAL(usageModeChanged()), this, SLOT(updateContextMenu()));
        connect(propertyWidget, SIGNAL(updateSemantics(PropertyWidgetQt*)),
                this, SLOT(updatePropertyWidgetSemantics(PropertyWidgetQt*)));
        
    } else {
        LogWarn("Could not find a widget for property: " << prop->getClassIdentifier());
    }
}

std::string CollapsibleGroupBoxWidgetQt::getDisplayName() const { return displayName_; }

void CollapsibleGroupBoxWidgetQt::setDisplayName(const std::string& displayName) {
    displayName_ = displayName;
    if(propertyOwner_) {
        Processor* p = dynamic_cast<Processor*>(propertyOwner_);
        if(p) p->setIdentifier(displayName);
    }
}

std::vector<Property*> CollapsibleGroupBoxWidgetQt::getProperties() { return properties_; }

UsageMode CollapsibleGroupBoxWidgetQt::getUsageMode() const {
    UsageMode mode = DEVELOPMENT;
    for (auto& elem : propertyWidgets_) {
        mode = std::min(mode, elem->getUsageMode());
    }
    return mode;
};

bool CollapsibleGroupBoxWidgetQt::getVisible() const {
    bool visible = showIfEmpty_;
    for (auto& elem : propertyWidgets_) {
        visible = visible || elem->getVisible();
    }
    return visible;
}

void CollapsibleGroupBoxWidgetQt::updateVisibility() {
    UsageMode appMode = getApplicationUsageMode();
    
    if (appMode >= getUsageMode()) {
        
        showWidget();

        for (auto& elem : properties_) {
            elem->updateVisibility();
        }

        for (auto& elem : propertyWidgets_) {
            CollapsibleGroupBoxWidgetQt* collapsiveWidget =
                dynamic_cast<CollapsibleGroupBoxWidgetQt*>(elem);
            if (collapsiveWidget) {
                if (appMode >= collapsiveWidget->getUsageMode()) {
                    collapsiveWidget->showWidget();
                } else if (appMode < collapsiveWidget->getUsageMode() || !isVisible()) {
                    collapsiveWidget->hideWidget();
                }               
                collapsiveWidget->updateVisibility();
            }
        }
        
    } else if (appMode < getUsageMode() || !isVisible()) {
        hideWidget();
    }
    
    bool empty = true;
    for (auto& elem : propertyWidgets_) {
        empty &= elem->isHidden();
    }
    defaultLabel_->setVisible(empty);

    updateContextMenu();
}

void CollapsibleGroupBoxWidgetQt::setDeveloperUsageMode(bool value) {
    for (auto& elem : propertyWidgets_) {
        elem->setDeveloperUsageMode(value);
    }

    if(developerUsageModeAction_) {
        developerUsageModeAction_->setChecked(true);
    }
    updateWidgets();
    updateContextMenu();
}

void CollapsibleGroupBoxWidgetQt::setApplicationUsageMode(bool value) {
    for (auto& elem : propertyWidgets_) {
        elem->setApplicationUsageMode(value);
    }

    if(applicationUsageModeAction_){
        applicationUsageModeAction_->setChecked(true);
    }
    updateWidgets();
    updateContextMenu();
}

void CollapsibleGroupBoxWidgetQt::updateWidgets() {
    for (auto& elem : propertyWidgets_) elem->updateContextMenu();
}

void CollapsibleGroupBoxWidgetQt::resetPropertyToDefaultState() {
    NetworkLock lock;
    
    for (auto& elem : propertyWidgets_) {
        elem->resetPropertyToDefaultState();
    }
}

void CollapsibleGroupBoxWidgetQt::labelDidChange() {
    setDisplayName(label_->getText());
}

void CollapsibleGroupBoxWidgetQt::toggleCollapsed() {
    setCollapsed(!isCollapsed());
}

void CollapsibleGroupBoxWidgetQt::checkedStateChanged() {
    setChecked(checkBox_->isChecked());
}

bool CollapsibleGroupBoxWidgetQt::isCollapsed() const {
    return collapsed_;
}

bool CollapsibleGroupBoxWidgetQt::isChecked() const {
    return checked_;
}

void CollapsibleGroupBoxWidgetQt::setChecked(bool checked) {
    if (!checkable_) {
        return;
    }

    checked_ = checked;
    // update checkbox
    checkBox_->setChecked(checked_);
}

bool CollapsibleGroupBoxWidgetQt::isCheckable() const {
    return checkable_;
}

void CollapsibleGroupBoxWidgetQt::setCheckable(bool checkable) {
    if (checkable_ == checkable) {
        return;
    }

    checkable_ = checkable;
    // update header
    checkBox_->setVisible(checkable_);
}


void CollapsibleGroupBoxWidgetQt::setCollapsed(bool collapse) {
    if (collapsed_ && !collapse) {
        propertyWidgetGroup_->show();
        btnCollapse_->setIcon(QIcon(":/stylesheets/images/arrow_lighter_down.png"));
    } else if (!collapsed_ && collapse) {
        propertyWidgetGroup_->hide();
        btnCollapse_->setIcon(QIcon(":/stylesheets/images/arrow_lighter_right.png"));
    }
    collapsed_ = collapse;
}

    
void CollapsibleGroupBoxWidgetQt::updatePropertyWidgetSemantics(PropertyWidgetQt* widget) {
    Property* prop = widget->getProperty();
    
    bool visible = widget->isVisible();
    
    std::vector<Property*>::iterator pit =
        std::find(properties_.begin(), properties_.end(), prop);
    
    std::vector<PropertyWidgetQt*>::iterator wit =
        std::find(propertyWidgets_.begin(), propertyWidgets_.end(), widget);
    
    if (pit != properties_.end() && wit != propertyWidgets_.end()) {
        
        PropertyWidgetQt* propertyWidget =
            static_cast<PropertyWidgetQt*>(PropertyWidgetFactory::getPtr()->create(prop));
        
        if (propertyWidget) {
            // set visibility first
            if (visible) {
                propertyWidget->showWidget();
            }
            else{
                propertyWidget->hideWidget();
            }

#if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
            propertyWidgetGroupLayout_->replaceWidget(widget, propertyWidget, Qt::FindDirectChildrenOnly);
#else 
            int layoutPosition = propertyWidgetGroupLayout_->indexOf(widget);
            propertyWidgetGroupLayout_->removeWidget(widget);
            propertyWidgetGroupLayout_->addWidget(propertyWidget, layoutPosition, 0);
#endif // QT_VERSION >= 5.2
            
            prop->deregisterWidget(widget);
            prop->registerWidget(propertyWidget);
            widget->hideWidget();
            // TODO: do we need to clean up this widget? It is no longer part of the layout and not 
            //       parented to this container
                        
            // Replace the item in propertyWidgets_;
            *wit = propertyWidget;
            
            connect(propertyWidget, SIGNAL(usageModeChanged()), this, SLOT(updateContextMenu()));
            connect(propertyWidget, SIGNAL(updateSemantics(PropertyWidgetQt*)),
                    this, SLOT(updatePropertyWidgetSemantics(PropertyWidgetQt*)));
        } else {
            LogWarn("Could not change semantic for property: " << prop->getClassIdentifier());
        }
    }
}

void CollapsibleGroupBoxWidgetQt::onDidAddProperty(Property* prop, size_t index) {
    std::vector<Property*>::iterator insertPoint = properties_.begin() + index;
    if (insertPoint!=properties_.end()) ++insertPoint;
    
    properties_.insert(insertPoint, prop);

    PropertyWidgetQt* propertyWidget =
        static_cast<PropertyWidgetQt*>(PropertyWidgetFactory::getPtr()->create(prop));

    if (propertyWidget) {
        propertyWidget->showWidget();

        const int insertPos = static_cast<int>(index) + 1;
        auto collapsibleWidget = dynamic_cast<CollapsibleGroupBoxWidgetQt *>(propertyWidget);
        if (collapsibleWidget) {
            collapsibleWidget->setNestedDepth(this->getNestedDepth() + 1);
            // make the collapsible widget go all the way to the right border
            propertyWidgetGroupLayout_->addWidget(propertyWidget, insertPos, 0, 1, -1);
        }
        else { // not a collapsible widget
            // property widget should only be added to the left column of the layout
            propertyWidgetGroupLayout_->addWidget(propertyWidget, insertPos, 0);
        }

        auto widgetInsertPoint = propertyWidgets_.begin()+index;
        if (widgetInsertPoint != propertyWidgets_.end()) ++widgetInsertPoint;

        propertyWidgets_.insert(widgetInsertPoint, propertyWidget);
        prop->registerWidget(propertyWidget);
        connect(propertyWidget, SIGNAL(usageModeChanged()), this, SLOT(updateContextMenu()));
        connect(propertyWidget, SIGNAL(updateSemantics(PropertyWidgetQt*)),
                this, SLOT(updatePropertyWidgetSemantics(PropertyWidgetQt*)));
        
        
        updateVisibility();
    } else {
        LogWarn("Could not find a widget for property: " << prop->getClassIdentifier());
    }

}

void CollapsibleGroupBoxWidgetQt::onWillRemoveProperty(Property* prop, size_t index) {
    PropertyWidgetQt* propertyWidget = propertyWidgets_[index];
    propertyWidget->hideWidget();
    propertyWidgetGroupLayout_->removeWidget(propertyWidget);
    propertyWidgets_.erase(propertyWidgets_.begin()+index);
    properties_.erase(properties_.begin()+index);
    
    updateVisibility();
}

void CollapsibleGroupBoxWidgetQt::onProcessorIdentifierChange(Processor* processor) {
    displayName_ = processor->getIdentifier();
    label_->setText(processor->getIdentifier());
}

void CollapsibleGroupBoxWidgetQt::setPropertyOwner(PropertyOwner* propertyOwner) {
    propertyOwner_ = propertyOwner;
}

PropertyOwner* CollapsibleGroupBoxWidgetQt::getPropertyOwner() const {
    return propertyOwner_;
}

std::vector<PropertyWidgetQt*> CollapsibleGroupBoxWidgetQt::getPropertyWidgets() {
    return propertyWidgets_;
}

void CollapsibleGroupBoxWidgetQt::setShowIfEmpty(bool val) {
    showIfEmpty_ = val;
}

void CollapsibleGroupBoxWidgetQt::setNestedDepth(int depth) {
    nestedDepth_ = depth;
    if (nestedDepth_ == 0) {
        // special case for depth zero
        QObject::setProperty("bgType", "toplevel");
    }
    else {
        QObject::setProperty("bgType", nestedDepth_ % maxNumNestedShades_);
    }

    // update depth of all nested collapsible group box widgets
    for (auto& elem : propertyWidgets_) {
        auto collapsibleWidget = dynamic_cast<CollapsibleGroupBoxWidgetQt *>(elem);
        if (collapsibleWidget) {
            collapsibleWidget->setNestedDepth(nestedDepth_ + 1);
        }
    }
}

int CollapsibleGroupBoxWidgetQt::getNestedDepth() const {
    return nestedDepth_;
}



}  // namespace
