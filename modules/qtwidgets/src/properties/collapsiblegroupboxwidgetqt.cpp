/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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
#include <modules/qtwidgets/inviwoqtutils.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/property.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/util/settings/settings.h>
#include <inviwo/core/util/settings/systemsettings.h>
#include <inviwo/core/properties/propertywidgetfactory.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/util/zip.h>
#include <inviwo/core/util/raiiutils.h>
#include <inviwo/core/network/networklock.h>
#include <modules/qtwidgets/editablelabelqt.h>
#include <inviwo/core/properties/propertypresetmanager.h>

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
#include <QClipboard>
#include <QMimeData>
#include <QApplication>
#include <warn/pop>

namespace inviwo {

CollapsibleGroupBoxWidgetQt::CollapsibleGroupBoxWidgetQt(CompositeProperty* property,
                                                         bool isCheckable)
    : CollapsibleGroupBoxWidgetQt(property, property, property->getDisplayName(), isCheckable) {}

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
    , checked_(false)
    , propertyOwner_(owner)
    , showIfEmpty_(false)
    , checkable_(isCheckable) {
    setObjectName("CompositeWidget");

    // Add the widget as a property owner observer for dynamic property addition and removal
    owner->addObserver(this);

    defaultLabel_ = new QLabel("No properties available");

    propertyWidgetGroup_ = createPropertyLayoutWidget().release();
    propertyWidgetGroupLayout_ = static_cast<QGridLayout*>(propertyWidgetGroup_->layout());

    btnCollapse_ = new QToolButton(this);
    btnCollapse_->setCheckable(true);
    btnCollapse_->setChecked(false);
    btnCollapse_->setObjectName("collapseButton");
    connect(btnCollapse_, &QToolButton::toggled, this, &CollapsibleGroupBoxWidgetQt::setCollapsed);
    btnCollapse_->setFocusPolicy(Qt::NoFocus);

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
    resetButton_->setFocusPolicy(Qt::NoFocus);

    resetButton_->setToolTip(tr("Reset the group of properties to its default state"));

    checkBox_ = new QCheckBox(this);
    checkBox_->setMinimumSize(5, 5);
    checkBox_->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred));
    checkBox_->setChecked(checked_);
    checkBox_->setVisible(checkable_);

    updateFocusPolicy();

    QObject::connect(checkBox_, &QCheckBox::clicked, this,
                     [&]() { setChecked(checkBox_->isChecked()); });

    QHBoxLayout* heading = new QHBoxLayout();
    heading->setContentsMargins(0, 0, 0, 0);
    heading->setSpacing(0);
    heading->addWidget(btnCollapse_);
    heading->addWidget(label_);
    heading->addSpacing(getSpacing());
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
                [&]() { propertyOwner_->resetAllPoperties(); });
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

const std::vector<Property*>& CollapsibleGroupBoxWidgetQt::getProperties() { return properties_; }

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
            // hide the underlying QWidget. Otherwise it will be shown as a floating window since it
            // no longer has a parent widget.
            static_cast<QWidget*>(*wit)->setVisible(false);
            // Replace the item in propertyWidgets_;
            *wit = newWidget;

            newWidget->setNestedDepth(this->getNestedDepth());
            newWidget->setParentPropertyWidget(this);
            newWidget->initState();

            // need to re-set tab order for all following widgets to ensure tab order is correct
            // (see http://doc.qt.io/qt-5/qwidget.html#setTabOrder)
            for (auto it = propertyWidgets_.begin() + 1; it != propertyWidgets_.end(); ++it) {
                setTabOrder((*(it - 1)), (*it));
            }
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

void CollapsibleGroupBoxWidgetQt::setEmptyLabelString(const std::string& str) {
    defaultLabel_->setText(utilqt::toQString(str));
}

std::unique_ptr<QWidget> CollapsibleGroupBoxWidgetQt::createPropertyLayoutWidget() {
    auto widget = std::make_unique<QWidget>(this);
    widget->setObjectName("CompositeContents");

    auto propertyLayout = std::make_unique<QGridLayout>();
    propertyLayout->setObjectName("PropertyWidgetLayout");
    propertyLayout->setAlignment(Qt::AlignTop);
    const auto space = getSpacing();
    propertyLayout->setContentsMargins(space * 2, space, 0, space);
    propertyLayout->setHorizontalSpacing(0);
    propertyLayout->setVerticalSpacing(space);

    auto layout = propertyLayout.release();
    widget->setLayout(layout);

    // add default label to widget layout, this will also set the correct stretching and spacing
    layout->addWidget(defaultLabel_, 0, 0);
    layout->addItem(new QSpacerItem(space, 1, QSizePolicy::Fixed), 0, 1);
    layout->setColumnStretch(0, 1);
    layout->setColumnStretch(1, 0);

    return widget;
}

void CollapsibleGroupBoxWidgetQt::addButtonLayout(QGridLayout* layout, int row, Property* prop) {
    auto createButton = [this](const std::string& objectName, const std::string& tooltip = "") {
        auto button = new QToolButton(this);
        button->setObjectName(utilqt::toQString(objectName));
        if (!tooltip.empty()) {
            button->setToolTip(utilqt::toQString(tooltip));
        }
        return button;
    };

    auto buttonWidget = new QWidget(this);
    layout->addWidget(buttonWidget, row, 2);

    const std::string str = "Remove property '" + prop->getDisplayName() + "'";

    auto removePropertyBtn = createButton("removeListItemButton", str);

    connect(removePropertyBtn, &QToolButton::clicked, this, [layout, prop, buttonWidget]() {
        if (prop->getOwner()) {
            prop->getOwner()->removeProperty(prop);
        } else {
            layout->removeWidget(buttonWidget);
            delete buttonWidget;
        }
    });

    /*
    // TODO: future functionality for reordering properties, see issue #178
    auto moveUpBtn = createButton(
        "moveListItemUpButton", "Move up");
    auto moveDownBtn = createButton(
        "moveListItemDownButton", "Move down");

    connect(moveUpBtn, &QToolButton::clicked, this,
            [this, prop]() { LogInfo("move up Property: " << prop->getDisplayName()); });
    connect(moveDownBtn, &QToolButton::clicked, this,
            [this, prop]() { LogInfo("move down Property: " << prop->getDisplayName()); });
    */

    auto buttonLayout = new QVBoxLayout();
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->setSpacing(0);
    buttonLayout->addWidget(removePropertyBtn);
    // buttonLayout->addWidget(moveUpBtn);
    // buttonLayout->addWidget(moveDownBtn);
    buttonLayout->addStretch(1);

    buttonWidget->setLayout(buttonLayout);
    buttonWidget->setVisible(prop->getVisible());
}

void CollapsibleGroupBoxWidgetQt::insertProperty(Property* prop, size_t index) {
    setUpdatesEnabled(false);
    util::OnScopeExit enableUpdates([&]() { setUpdatesEnabled(true); });

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
        auto layoutWidget = createPropertyLayoutWidget();
        auto layout = static_cast<QGridLayout*>(layoutWidget->layout());

        util::OnScopeExit freeOldLayout([&]() {
            // get rid of previous property widget and layout
            auto parentLayout = [&layoutWidget]() -> QLayout* {
                if (layoutWidget->parentWidget()) {
                    return layoutWidget->parentWidget()->layout();
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
    if (checkable_) {
        setFocusPolicy(checkBox_->focusPolicy());
        setFocusProxy(checkBox_);
    } else {
        setFocusPolicy(Qt::NoFocus);
    }
}

}  // namespace inviwo
