/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2023 Inviwo Foundation
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

#pragma once

#include <modules/qtwidgets/qtwidgetsmoduledefine.h>  // for IVW_MODULE_QTWIDGETS_API

#include <inviwo/core/network/networklock.h>            // for NetworkLock
#include <inviwo/core/properties/constraintbehavior.h>  // for ConstraintBehavior, ConstraintBeh...
#include <inviwo/core/properties/minmaxproperty.h>      // for MinMaxProperty
// Core
#include <inviwo/core/properties/propertywidget.h>  // for PropertyWidget
#include <inviwo/core/util/glmcomp.h>               // for glmcomp
#include <inviwo/core/util/glmutils.h>              // for value_type
#include <inviwo/core/util/glmvec.h>                // for uvec2
#include <inviwo/core/util/logcentral.h>            // for LogCentral, LogError
#include <modules/qtwidgets/inviwoqtutils.h>        // for refSpacePx, toQString
#include <modules/qtwidgets/qstringhelper.h>        // for QStringHelper

#include <array>    // for array
#include <cstddef>  // for size_t
#include <ostream>  // for operator<<, stringstream, basic_o...
#include <string>   // for allocator, operator+, char_traits
#include <vector>   // for vector

#include <QCheckBox>                 // for QCheckBox
#include <QCloseEvent>               // for QCloseEvent
#include <QDialog>                   // for QDialog
#include <QGridLayout>               // for QGridLayout
#include <QKeyEvent>                 // for QKeyEvent
#include <QLabel>                    // for QLabel
#include <QLineEdit>                 // for QLineEdit
#include <QLocale>                   // for QLocale
#include <QPushButton>               // for QPushButton
#include <QSignalBlocker>            // for QSignalBlocker
#include <QString>                   // for QString
#include <Qt>                        // for Key_Enter, Key_Escape, Key_Return
#include <glm/common.hpp>            // for max, min
#include <glm/detail/qualifier.hpp>  // for defaultp, tvec2
#include <glm/vec2.hpp>              // for vec<>::(anonymous)

class QCloseEvent;
class QKeyEvent;
class QWidget;

namespace inviwo {
template <typename T>
class OrdinalProperty;
template <typename T>
class OrdinalRefProperty;
}  // namespace inviwo

namespace inviwo {

class IVW_MODULE_QTWIDGETS_API SinglePropertySetting {
public:
    SinglePropertySetting(QWidget* widget, std::string label);

    QLineEdit* addField();
    double getFieldAsDouble(size_t i);

    QLabel* label_;
    QWidget* widget_;
    std::vector<QLineEdit*> additionalFields_;
};

template <typename Prop>
class OrdinalLikePropertySettingsWidgetQt : public QDialog, public PropertyWidget {
public:
    using T = typename Prop::value_type;
    using BT = typename util::value_type<T>::type;
    OrdinalLikePropertySettingsWidgetQt(Prop* property, QWidget* widget);
    virtual ~OrdinalLikePropertySettingsWidgetQt();

    virtual void updateFromProperty() override;
    /**
     * \brief shows the widget and registers the widget with the property
     */
    void showWidget();
    /**
     * \brief hides the widget and deregisters it from the property
     */
    void hideWidget();
    bool getVisible() const;

protected:
    virtual void keyPressEvent(QKeyEvent* event) override;
    virtual void closeEvent(QCloseEvent* e) override;

private:
    void save();
    void apply();
    void reload();
    void cancel();

    QPushButton* btnApply_;
    QPushButton* btnOk_;
    QPushButton* btnCancel_;
    std::vector<SinglePropertySetting> settings_;
    Prop* property_;
};

template <typename T>
using OrdinalPropertySettingsWidgetQt = OrdinalLikePropertySettingsWidgetQt<OrdinalProperty<T>>;

template <typename T>
using OrdinalRefPropertySettingsWidgetQt =
    OrdinalLikePropertySettingsWidgetQt<OrdinalRefProperty<T>>;

template <typename Prop>
OrdinalLikePropertySettingsWidgetQt<Prop>::OrdinalLikePropertySettingsWidgetQt(Prop* property,
                                                                               QWidget* widget)
    : QDialog(widget)
    , PropertyWidget(property)
    , btnApply_(new QPushButton("Apply", this))
    , btnOk_(new QPushButton("Ok", this))
    , btnCancel_(new QPushButton("Cancel", this))
    , property_(property) {

    setModal(false);
    // remove help button from title bar
    Qt::WindowFlags flags = windowFlags() ^ Qt::WindowContextHelpButtonHint;
    setWindowFlags(flags);

    auto gridLayout = new QGridLayout();
    const auto space = utilqt::refSpacePx(this);
    gridLayout->setContentsMargins(space, space, space, space);
    gridLayout->setSpacing(space);

    const std::array<QCheckBox*, 4> same = {
        new QCheckBox(
            utilqt::toQString(fmt::format("Min ({})", property_->getMinConstraintBehaviour())),
            this),
        new QCheckBox("Value", this),
        new QCheckBox(
            utilqt::toQString(fmt::format("Max ({})", property_->getMaxConstraintBehaviour())),
            this),
        new QCheckBox("Increment", this)};
    for (auto* s : same) {
        s->setToolTip("Same for all channels");
    }
    const std::array<QWidget*, 5> labels = {new QLabel("Component"), same[0], same[1], same[2],
                                            same[3]};

    for (size_t i = 0; i < labels.size(); ++i) {
        gridLayout->addWidget(labels[i], 0, static_cast<int>(i));
    }
    const std::array<char, 4> desc = {'x', 'y', 'z', 'w'};
    const uvec2 components = Prop::getDim();

    int count = 0;
    for (size_t i = 0; i < components.x; i++) {
        for (size_t j = 0; j < components.y; j++) {
            std::stringstream ss;
            ss << desc[i] << (components.y == 1 ? "" : (std::string{", "} + desc[j]));
            settings_.emplace_back(this, ss.str());
            gridLayout->addWidget(settings_[count].label_, count + 1, 0);

            for (int k = 0; k < 4; ++k) {
                QLineEdit* edit = settings_[count].addField();
                gridLayout->addWidget(edit, count + 1, k + 1);
            }
            count++;
        }
    }

    auto symmetric = new QCheckBox("Symmetric", this);
    symmetric->setToolTip("Symmetric min and max value ( min = -max )");

    auto updateOthers = [this, same, symmetric](size_t srcRow, size_t srcCol) {
        return [this, same, symmetric, srcRow, srcCol](const QString&) {
            if (same[srcCol]->isChecked()) {
                for (size_t row = 0; row < settings_.size(); row++) {
                    if (row != srcRow) {
                        QSignalBlocker block{settings_[row].additionalFields_[srcCol]};
                        settings_[row].additionalFields_[srcCol]->setText(
                            settings_[srcRow].additionalFields_[srcCol]->text());
                    }
                }
                if (symmetric->isChecked() && (srcCol == 0 || srcCol == 2)) {
                    const size_t dstCol = srcCol == 0 ? 2 : 0;
                    for (size_t row = 0; row < settings_.size(); row++) {
                        QSignalBlocker block{settings_[row].additionalFields_[dstCol]};
                        QLocale locale = settings_[row].additionalFields_[dstCol]->locale();
                        settings_[row].additionalFields_[dstCol]->setText(
                            QStringHelper<BT>::toLocaleString(
                                locale, static_cast<BT>(-settings_[row].getFieldAsDouble(srcCol))));
                    }
                }
            } else if (symmetric->isChecked() && (srcCol == 0 || srcCol == 2)) {
                const size_t dstCol = srcCol == 0 ? 2 : 0;
                QSignalBlocker block{settings_[srcRow].additionalFields_[dstCol]};
                QLocale locale = settings_[srcRow].additionalFields_[dstCol]->locale();
                settings_[srcRow].additionalFields_[dstCol]->setText(
                    QStringHelper<BT>::toLocaleString(
                        locale, static_cast<BT>(-settings_[srcRow].getFieldAsDouble(srcCol))));
            }
        };
    };

    for (size_t row = 0; row < settings_.size(); row++) {
        for (size_t col = 0; col < same.size(); col++) {
            connect(settings_[row].additionalFields_[col], &QLineEdit::textChanged, this,
                    updateOthers(row, col));
        }
    }

    gridLayout->addWidget(symmetric, count + 1, 0, 1, 1);
    gridLayout->addWidget(btnApply_, count + 1, 2, 1, 1);
    gridLayout->addWidget(btnOk_, count + 1, 3, 1, 1);
    gridLayout->addWidget(btnCancel_, count + 1, 4, 1, 1);
    gridLayout->setColumnStretch(2, 2);

    setLayout(gridLayout);

    connect(btnApply_, &QPushButton::clicked, this,
            &OrdinalLikePropertySettingsWidgetQt<Prop>::apply);
    connect(btnOk_, &QPushButton::clicked, this, &OrdinalLikePropertySettingsWidgetQt<Prop>::save);
    connect(btnCancel_, &QPushButton::clicked, this,
            &OrdinalLikePropertySettingsWidgetQt<Prop>::cancel);

    reload();

    for (size_t col = 0; col < same.size(); col++) {
        const auto val = settings_[0].getFieldAsDouble(col);
        bool allSame = true;
        for (size_t row = 1; row < settings_.size(); row++) {
            allSame &= val == settings_[row].getFieldAsDouble(col);
        }
        same[col]->setChecked(allSame);
    }

    setWindowTitle(utilqt::toQString(property_->getDisplayName()));
}

template <typename Prop>
OrdinalLikePropertySettingsWidgetQt<Prop>::~OrdinalLikePropertySettingsWidgetQt() {
    if (property_) property_->deregisterWidget(this);
}

template <typename Prop>
void OrdinalLikePropertySettingsWidgetQt<Prop>::apply() {
    NetworkLock lock(property_);

    std::array<T, 4> vals{};
    for (size_t i = 0; i < settings_.size(); i++) {
        for (size_t k = 0; k < 4; ++k) {
            util::glmcomp(vals[k], i) = static_cast<BT>(settings_[i].getFieldAsDouble(k));
        }
    }

    for (size_t i = 0; i < settings_.size(); i++) {
        if (util::glmcomp(vals[0], i) > util::glmcomp(vals[1], i) ||
            util::glmcomp(vals[1], i) > util::glmcomp(vals[2], i)) {
            LogError(fmt::format("Invalid range found elem {}: {} <= {} <= {}", i,
                                 util::glmcomp(vals[0], i), util::glmcomp(vals[1], i),
                                 util::glmcomp(vals[1], i)));
            return;
        }
    }

    property_->setInitiatingWidget(this);
    for (int k = 0; k < 4; ++k) {
        property_->set(vals[1], vals[0], vals[2], vals[3]);
    }
    property_->clearInitiatingWidget();
}

template <typename Prop>
void OrdinalLikePropertySettingsWidgetQt<Prop>::reload() {
    std::array<T, 4> vals{property_->getMinValue(), property_->get(), property_->getMaxValue(),
                          property_->getIncrement()};

    QLocale locale = settings_[0].additionalFields_[0]->locale();
    for (size_t i = 0; i < settings_.size(); i++) {
        for (size_t k = 0; k < vals.size(); ++k) {
            settings_[i].additionalFields_[k]->setText(
                QStringHelper<BT>::toLocaleString(locale, util::glmcomp(vals[k], i)));
        }

        settings_[i].additionalFields_[0]->setEnabled(
            property_->getMinConstraintBehaviour() == ConstraintBehavior::Editable ||
            property_->getMinConstraintBehaviour() == ConstraintBehavior::Ignore);

        settings_[i].additionalFields_[2]->setEnabled(
            property_->getMaxConstraintBehaviour() == ConstraintBehavior::Editable ||
            property_->getMaxConstraintBehaviour() == ConstraintBehavior::Ignore);
    }
}

template <typename Prop>
void OrdinalLikePropertySettingsWidgetQt<Prop>::cancel() {
    hideWidget();
    reload();
}

template <typename Prop>
void OrdinalLikePropertySettingsWidgetQt<Prop>::save() {
    hideWidget();
    apply();
}

template <typename Prop>
bool OrdinalLikePropertySettingsWidgetQt<Prop>::getVisible() const {
    return isVisible();
}

template <typename Prop>
void OrdinalLikePropertySettingsWidgetQt<Prop>::hideWidget() {
    property_->deregisterWidget(this);
    setVisible(false);
}

template <typename Prop>
void OrdinalLikePropertySettingsWidgetQt<Prop>::showWidget() {
    property_->registerWidget(this);
    updateFromProperty();
    setVisible(true);
}

template <typename Prop>
void OrdinalLikePropertySettingsWidgetQt<Prop>::updateFromProperty() {
    reload();
}

template <typename Prop>
void OrdinalLikePropertySettingsWidgetQt<Prop>::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Escape) {
        cancel();
    } else if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        apply();
    }
    QDialog::keyPressEvent(event);
}

template <typename Prop>
void OrdinalLikePropertySettingsWidgetQt<Prop>::closeEvent(QCloseEvent* event) {
    event->ignore();
    cancel();
}

template <typename T>
class MinMaxPropertySettingsWidgetQt : public QDialog, public PropertyWidget {
public:
    using BT = typename util::value_type<T>::type;
    MinMaxPropertySettingsWidgetQt(MinMaxProperty<T>* property, QWidget* widget);
    virtual ~MinMaxPropertySettingsWidgetQt();

    using V = glm::tvec2<T, glm::defaultp>;

    virtual void updateFromProperty() override;
    /**
     * \brief shows the widget and registers the widget with the property
     */
    void showWidget();
    /**
     * \brief hides the widget and deregisters it from the property
     */
    void hideWidget();
    bool getVisible() const;

protected:
    virtual void keyPressEvent(QKeyEvent* event) override;
    virtual void closeEvent(QCloseEvent* e) override;

private:
    void save();
    void apply();
    void reload();
    void cancel();

    QPushButton* btnApply_;
    QPushButton* btnOk_;
    QPushButton* btnCancel_;
    SinglePropertySetting setting_;
    MinMaxProperty<T>* property_;
};

template <typename T>
MinMaxPropertySettingsWidgetQt<T>::MinMaxPropertySettingsWidgetQt(MinMaxProperty<T>* property,
                                                                  QWidget* widget)
    : QDialog(widget)
    , PropertyWidget(property)
    , btnApply_(new QPushButton("Apply", this))
    , btnOk_(new QPushButton("Ok", this))
    , btnCancel_(new QPushButton("Cancel", this))
    , setting_{this, ""}
    , property_(property) {

    setModal(false);
    // remove help button from title bar
    Qt::WindowFlags flags = windowFlags() ^ Qt::WindowContextHelpButtonHint;
    setWindowFlags(flags);

    auto gridLayout = new QGridLayout();
    const auto space = utilqt::refSpacePx(this);
    gridLayout->setContentsMargins(space, space, space, space);
    gridLayout->setSpacing(space);

    const std::array<QString, 6> labels = {"Min", "Start", "End", "Max", "Separation", "Increment"};
    for (size_t i = 0; i < labels.size(); ++i) {
        gridLayout->addWidget(new QLabel(labels[i], this), 0, static_cast<int>(i));
    }

    for (int k = 0; k < 6; ++k) {
        QLineEdit* edit = setting_.addField();
        gridLayout->addWidget(edit, 1, k);
    }

    gridLayout->addWidget(btnApply_, 2, 3);
    gridLayout->addWidget(btnOk_, 2, 4);
    gridLayout->addWidget(btnCancel_, 2, 5);
    gridLayout->setColumnStretch(2, 2);

    setLayout(gridLayout);

    connect(btnApply_, &QPushButton::clicked, this, &MinMaxPropertySettingsWidgetQt<T>::apply);
    connect(btnOk_, &QPushButton::clicked, this, &MinMaxPropertySettingsWidgetQt<T>::save);
    connect(btnCancel_, &QPushButton::clicked, this, &MinMaxPropertySettingsWidgetQt<T>::cancel);

    reload();
    setWindowTitle(utilqt::toQString(property_->getDisplayName()));
}

template <typename T>
MinMaxPropertySettingsWidgetQt<T>::~MinMaxPropertySettingsWidgetQt() {
    if (property_) property_->deregisterWidget(this);
}

template <typename T>
void MinMaxPropertySettingsWidgetQt<T>::apply() {
    NetworkLock lock(property_);

    using range_type = typename MinMaxProperty<T>::value_type;

    // order of values stored in setting_:
    // "Component", "Min Bound", "Start","End", "Max Bound", "MinSeparation", "Increment"
    auto getVal = [&](size_t index) { return static_cast<BT>(setting_.getFieldAsDouble(index)); };

    range_type newVal(getVal(1), getVal(2));
    range_type newRange(getVal(0), getVal(3));
    T minSep = getVal(4);
    T increment = getVal(5);

    // swap values if necessary
    if (newVal.x > newVal.y) newVal = range_type(newVal.y, newVal.x);
    // swap range if necessary
    if (newRange.x > newRange.y) newRange = range_type(newRange.y, newRange.x);

    // perform sanity check whether new values are out of bounds
    bool outOfBounds = ((newVal.x < newRange.x) || (newVal.y > newRange.y));
    if (outOfBounds) {
        // adjust ranges
        newRange.x = glm::min(newRange.x, newVal.x);
        newRange.y = glm::max(newRange.y, newVal.y);
    }

    property_->setInitiatingWidget(this);
    property_->set(newVal, newRange, increment, minSep);
    property_->clearInitiatingWidget();
    // the values stored in the property might be different from the ones shown in the dialog
    reload();
}

template <typename T>
void MinMaxPropertySettingsWidgetQt<T>::reload() {
    std::array<T, 6> vals{property_->getRangeMin(),      property_->getStart(),
                          property_->getEnd(),           property_->getRangeMax(),
                          property_->getMinSeparation(), property_->getIncrement()};

    QLocale locale = setting_.additionalFields_[0]->locale();
    for (size_t k = 0; k < vals.size(); ++k) {
        setting_.additionalFields_[k]->setText(QStringHelper<BT>::toLocaleString(locale, vals[k]));
    }
}

template <typename T>
void MinMaxPropertySettingsWidgetQt<T>::save() {
    hideWidget();
    apply();
}

template <typename T>
void MinMaxPropertySettingsWidgetQt<T>::cancel() {
    hideWidget();
    reload();
}

template <typename T>
bool MinMaxPropertySettingsWidgetQt<T>::getVisible() const {
    return isVisible();
}

template <typename T>
void MinMaxPropertySettingsWidgetQt<T>::hideWidget() {
    property_->deregisterWidget(this);
    QDialog::setVisible(false);
}

template <typename T>
void MinMaxPropertySettingsWidgetQt<T>::showWidget() {
    property_->registerWidget(this);
    updateFromProperty();
    QDialog::setVisible(true);
}

template <typename T>
void MinMaxPropertySettingsWidgetQt<T>::updateFromProperty() {
    reload();
}

template <typename T>
void MinMaxPropertySettingsWidgetQt<T>::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Escape) {
        cancel();
    } else if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        apply();
    }
    QDialog::keyPressEvent(event);
}

template <typename T>
void MinMaxPropertySettingsWidgetQt<T>::closeEvent(QCloseEvent* event) {
    event->ignore();
    cancel();
}

}  // namespace inviwo
