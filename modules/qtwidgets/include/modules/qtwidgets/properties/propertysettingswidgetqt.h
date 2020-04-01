/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2020 Inviwo Foundation
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

#include <modules/qtwidgets/qtwidgetsmoduledefine.h>
// Core
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/properties/propertywidget.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/core/network/networklock.h>
#include <modules/qtwidgets/qstringhelper.h>
#include <modules/qtwidgets/inviwoqtutils.h>

#include <warn/push>
#include <warn/ignore/all>
// Qt
#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QLocale>
#include <QValidator>
#include <QGridLayout>
#include <QPushButton>
#include <QKeyEvent>
#include <QString>
#include <QCheckBox>
#include <warn/pop>

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

template <typename T>
class OrdinalPropertySettingsWidgetQt : public QDialog, public PropertyWidget {
public:
    using BT = typename util::value_type<T>::type;
    OrdinalPropertySettingsWidgetQt(OrdinalProperty<T>* property, QWidget* widget);
    virtual ~OrdinalPropertySettingsWidgetQt();

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

    // virtual void reject() override;

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
    std::vector<std::unique_ptr<SinglePropertySetting>> settings_;
    OrdinalProperty<T>* property_;
};

template <typename T>
OrdinalPropertySettingsWidgetQt<T>::OrdinalPropertySettingsWidgetQt(OrdinalProperty<T>* property,
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
    // flags |= Qt::Popup;  // make it a tool window
    // flags |= Qt::Dialog;
    setWindowFlags(flags);

    // setAttribute(Qt::WA_DeleteOnClose, false);

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
    const uvec2 components = OrdinalProperty<T>::getDim();

    int count = 0;
    for (size_t i = 0; i < components.x; i++) {
        for (size_t j = 0; j < components.y; j++) {
            std::stringstream ss;
            ss << desc[i] << (components.y == 1 ? "" : (std::string{", "} + desc[j]));
            settings_.push_back(std::make_unique<SinglePropertySetting>(this, ss.str()));
            gridLayout->addWidget(settings_[count]->label_, count + 1, 0);

            for (int k = 0; k < 4; ++k) {
                QLineEdit* edit = settings_[count]->addField();
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
                        QSignalBlocker block{settings_[row]->additionalFields_[srcCol]};
                        settings_[row]->additionalFields_[srcCol]->setText(
                            settings_[srcRow]->additionalFields_[srcCol]->text());
                    }
                }
                if (symmetric->isChecked() && (srcCol == 0 || srcCol == 2)) {
                    const size_t dstCol = srcCol == 0 ? 2 : 0;
                    for (size_t row = 0; row < settings_.size(); row++) {
                        QSignalBlocker block{settings_[row]->additionalFields_[dstCol]};
                        QLocale locale = settings_[row]->additionalFields_[dstCol]->locale();
                        settings_[row]->additionalFields_[dstCol]->setText(
                            QStringHelper<BT>::toLocaleString(
                                locale,
                                static_cast<BT>(-settings_[row]->getFieldAsDouble(srcCol))));
                    }
                }
            } else if (symmetric->isChecked() && (srcCol == 0 || srcCol == 2)) {
                const size_t dstCol = srcCol == 0 ? 2 : 0;
                QSignalBlocker block{settings_[srcRow]->additionalFields_[dstCol]};
                QLocale locale = settings_[srcRow]->additionalFields_[dstCol]->locale();
                settings_[srcRow]->additionalFields_[dstCol]->setText(
                    QStringHelper<BT>::toLocaleString(
                        locale, static_cast<BT>(-settings_[srcRow]->getFieldAsDouble(srcCol))));
            }
        };
    };

    for (size_t row = 0; row < settings_.size(); row++) {
        for (size_t col = 0; col < same.size(); col++) {
            connect(settings_[row]->additionalFields_[col], &QLineEdit::textChanged, this,
                    updateOthers(row, col));
        }
    }

    gridLayout->addWidget(symmetric, count + 1, 0, 1, 1);
    gridLayout->addWidget(btnApply_, count + 1, 2, 1, 1);
    gridLayout->addWidget(btnOk_, count + 1, 3, 1, 1);
    gridLayout->addWidget(btnCancel_, count + 1, 4, 1, 1);
    gridLayout->setColumnStretch(2, 2);

    setLayout(gridLayout);

    connect(btnApply_, &QPushButton::clicked, this, &OrdinalPropertySettingsWidgetQt<T>::apply);
    connect(btnOk_, &QPushButton::clicked, this, &OrdinalPropertySettingsWidgetQt<T>::save);
    connect(btnCancel_, &QPushButton::clicked, this, &OrdinalPropertySettingsWidgetQt<T>::cancel);

    reload();

    for (size_t col = 0; col < same.size(); col++) {
        const auto val = settings_[0]->getFieldAsDouble(col);
        bool allSame = true;
        for (size_t row = 1; row < settings_.size(); row++) {
            allSame &= val == settings_[row]->getFieldAsDouble(col);
        }
        same[col]->setChecked(allSame);
    }

    setWindowTitle(utilqt::toQString(property_->getDisplayName()));
}

template <typename T>
OrdinalPropertySettingsWidgetQt<T>::~OrdinalPropertySettingsWidgetQt() {
    if (property_) property_->deregisterWidget(this);
}

template <typename T>
void OrdinalPropertySettingsWidgetQt<T>::apply() {
    NetworkLock lock(property_);

    std::array<T, 4> vals{};
    for (size_t i = 0; i < settings_.size(); i++) {
        for (size_t k = 0; k < 4; ++k) {
            util::glmcomp(vals[k], i) = static_cast<BT>(settings_[i]->getFieldAsDouble(k));
        }
    }

    property_->setInitiatingWidget(this);
    for (int k = 0; k < 4; ++k) {
        property_->set(vals[1], vals[0], vals[2], vals[3]);
    }
    property_->clearInitiatingWidget();
}

template <typename T>
void OrdinalPropertySettingsWidgetQt<T>::reload() {
    std::array<T, 4> vals{property_->getMinValue(), property_->get(), property_->getMaxValue(),
                          property_->getIncrement()};

    QLocale locale = settings_[0]->additionalFields_[0]->locale();
    for (size_t i = 0; i < settings_.size(); i++) {
        for (size_t k = 0; k < vals.size(); ++k) {
            settings_[i]->additionalFields_[k]->setText(
                QStringHelper<BT>::toLocaleString(locale, util::glmcomp(vals[k], i)));
        }

        settings_[i]->additionalFields_[0]->setEnabled(
            property_->getMinConstraintBehaviour() == ConstraintBehaviour::Editable ||
            property_->getMinConstraintBehaviour() == ConstraintBehaviour::Ignore);

        settings_[i]->additionalFields_[2]->setEnabled(
            property_->getMaxConstraintBehaviour() == ConstraintBehaviour::Editable ||
            property_->getMaxConstraintBehaviour() == ConstraintBehaviour::Ignore);
    }
}

template <typename T>
void OrdinalPropertySettingsWidgetQt<T>::cancel() {
    hideWidget();
    reload();
}

template <typename T>
void OrdinalPropertySettingsWidgetQt<T>::save() {
    hideWidget();
    apply();
}

template <typename T>
bool OrdinalPropertySettingsWidgetQt<T>::getVisible() const {
    return isVisible();
}

template <typename T>
void OrdinalPropertySettingsWidgetQt<T>::hideWidget() {
    property_->deregisterWidget(this);
    setVisible(false);
}

template <typename T>
void OrdinalPropertySettingsWidgetQt<T>::showWidget() {
    property_->registerWidget(this);
    updateFromProperty();
    setVisible(true);
}

template <typename T>
void OrdinalPropertySettingsWidgetQt<T>::updateFromProperty() {
    reload();
}

template <typename T>
void OrdinalPropertySettingsWidgetQt<T>::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Escape) {
        cancel();
    } else if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        apply();
    }
    QDialog::keyPressEvent(event);
}

template <typename T>
void OrdinalPropertySettingsWidgetQt<T>::closeEvent(QCloseEvent* event) {
   event->ignore();
   cancel();
}


template <typename T>
class TemplateMinMaxPropertySettingsWidgetQt : public QDialog, public PropertyWidget {
public:
    using BT = typename util::value_type<T>::type;
    TemplateMinMaxPropertySettingsWidgetQt(MinMaxProperty<T>* property, QWidget* widget);
    virtual ~TemplateMinMaxPropertySettingsWidgetQt();

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

private:
    void save();
    void apply();
    void reload();
    void cancel();

    QPushButton* btnApply_;
    QPushButton* btnOk_;
    QPushButton* btnCancel_;
    std::vector<std::unique_ptr<SinglePropertySetting>> settings_;
    MinMaxProperty<T>* property_;
};

template <typename T>
TemplateMinMaxPropertySettingsWidgetQt<T>::TemplateMinMaxPropertySettingsWidgetQt(
    MinMaxProperty<T>* property, QWidget* widget)
    : QDialog(widget)
    , PropertyWidget(property)
    , btnApply_(new QPushButton("Apply", this))
    , btnOk_(new QPushButton("Ok", this))
    , btnCancel_(new QPushButton("Cancel", this))
    , property_(property) {

    this->setModal(false);
    // remove help button from title bar
    Qt::WindowFlags flags = this->windowFlags() ^ Qt::WindowContextHelpButtonHint;
    // make it a tool window
    flags |= Qt::Popup;
    this->setWindowFlags(flags);

    auto gridLayout = new QGridLayout();
    const auto space = utilqt::refSpacePx(this);
    gridLayout->setContentsMargins(space, space, space, space);
    gridLayout->setSpacing(space);

    const std::array<QString, 7> labels = {"Component", "Min Bound",     "Start",    "End",
                                           "Max Bound", "MinSeparation", "Increment"};
    for (size_t i = 0; i < labels.size(); ++i) {
        gridLayout->addWidget(new QLabel(labels[i], this), 0, static_cast<int>(i));
    }
    const std::array<char, 4> desc = {'x', 'y', 'z', 'w'};
    const uvec2 components = OrdinalProperty<T>::getDim();

    int count = 0;
    for (size_t i = 0; i < components.x; i++) {
        for (size_t j = 0; j < components.y; j++) {
            std::stringstream ss;
            ss << desc[i] << (components.y == 1 ? "" : (std::string{", "} + desc[j]));
            settings_.push_back(std::make_unique<SinglePropertySetting>(this, ss.str()));
            gridLayout->addWidget(settings_[count]->label_, count + 1, 0);

            for (int k = 0; k < 6; ++k) {
                QLineEdit* edit = settings_[count]->addField();
                gridLayout->addWidget(edit, count + 1, k + 1);
            }
            count++;
        }
    }

    gridLayout->addWidget(btnApply_, count + 1, 0, 1, 1);
    gridLayout->addWidget(btnOk_, count + 1, 1, 1, 2);
    gridLayout->addWidget(btnCancel_, count + 1, 3, 1, 2);
    gridLayout->setColumnStretch(2, 2);

    setLayout(gridLayout);

    connect(btnApply_, &QPushButton::clicked, this,
            &TemplateMinMaxPropertySettingsWidgetQt<T>::apply);
    connect(btnOk_, &QPushButton::clicked, this, &TemplateMinMaxPropertySettingsWidgetQt<T>::save);
    connect(btnCancel_, &QPushButton::clicked, this,
            &TemplateMinMaxPropertySettingsWidgetQt<T>::cancel);

    reload();
    setWindowTitle(QString::fromStdString(property_->getDisplayName().c_str()));
}

template <typename T>
TemplateMinMaxPropertySettingsWidgetQt<T>::~TemplateMinMaxPropertySettingsWidgetQt() {
    if (property_) property_->deregisterWidget(this);
}

template <typename T>
void TemplateMinMaxPropertySettingsWidgetQt<T>::apply() {
    NetworkLock lock(property_);

    using range_type = typename MinMaxProperty<T>::range_type;

    // order of values stored in setting_:
    // "Component", "Min Bound", "Start","End", "Max Bound", "MinSeparation", "Increment"
    auto getVal = [&](size_t index) {
        return static_cast<BT>(settings_[0]->getFieldAsDouble(index));
    };

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
void TemplateMinMaxPropertySettingsWidgetQt<T>::reload() {
    std::array<T, 6> vals{property_->getRangeMin(),      property_->getStart(),
                          property_->getEnd(),           property_->getRangeMax(),
                          property_->getMinSeparation(), property_->getIncrement()};

    QLocale locale = settings_[0]->additionalFields_[0]->locale();
    for (size_t i = 0; i < settings_.size(); i++) {
        for (size_t k = 0; k < vals.size(); ++k) {
            settings_[i]->additionalFields_[k]->setText(
                QStringHelper<BT>::toLocaleString(locale, util::glmcomp(vals[k], i)));
        }
    }
}

template <typename T>
void TemplateMinMaxPropertySettingsWidgetQt<T>::save() {
    hideWidget();
    apply();
}

template <typename T>
void TemplateMinMaxPropertySettingsWidgetQt<T>::cancel() {
    hideWidget();
    reload();
}

template <typename T>
bool TemplateMinMaxPropertySettingsWidgetQt<T>::getVisible() const {
    return isVisible();
}

template <typename T>
void TemplateMinMaxPropertySettingsWidgetQt<T>::hideWidget() {
    property_->deregisterWidget(this);
    QDialog::setVisible(false);
}

template <typename T>
void TemplateMinMaxPropertySettingsWidgetQt<T>::showWidget() {
    property_->registerWidget(this);
    updateFromProperty();
    QDialog::setVisible(true);
}

template <typename T>
void TemplateMinMaxPropertySettingsWidgetQt<T>::updateFromProperty() {
    reload();
}

template <typename T>
void TemplateMinMaxPropertySettingsWidgetQt<T>::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Escape) {
        cancel();
    } else if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        apply();
    }
    QDialog::keyPressEvent(event);
}

}  // namespace inviwo
