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

#ifndef IVW_PROPERTYSETTINGSWIDGETQT_H
#define IVW_PROPERTYSETTINGSWIDGETQT_H

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
#include <warn/pop>

namespace inviwo {

class IVW_MODULE_QTWIDGETS_API SinglePropertySetting {
public:
    SinglePropertySetting(QWidget* widget, std::string label);

    QLineEdit* addField();
    double getFieldAsDouble(int i);

    QLabel* label_;
    QWidget* widget_;
    std::vector<QLineEdit*> additionalFields_;
};

template <typename T>
class TemplatePropertySettingsWidgetQt : public QDialog, public PropertyWidget {
public:
    using BT = typename util::value_type<T>::type;
    TemplatePropertySettingsWidgetQt(OrdinalProperty<T>* property, QWidget* widget);
    virtual ~TemplatePropertySettingsWidgetQt();

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
    OrdinalProperty<T>* property_;
};

template <typename T>
TemplatePropertySettingsWidgetQt<T>::TemplatePropertySettingsWidgetQt(OrdinalProperty<T>* property,
                                                                      QWidget* widget)
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

    const std::array<QString, 5> labels = {"Component", "Min", "Value", "Max", "Increment"};
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

            for (int k = 0; k < 4; ++k) {
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

    connect(btnApply_, &QPushButton::clicked, this, &TemplatePropertySettingsWidgetQt<T>::apply);
    connect(btnOk_, &QPushButton::clicked, this, &TemplatePropertySettingsWidgetQt<T>::save);
    connect(btnCancel_, &QPushButton::clicked, this, &TemplatePropertySettingsWidgetQt<T>::cancel);

    reload();
    setWindowTitle(QString::fromStdString(property_->getDisplayName().c_str()));
}

template <typename T>
TemplatePropertySettingsWidgetQt<T>::~TemplatePropertySettingsWidgetQt() {
    if (property_) property_->deregisterWidget(this);
}

template <typename T>
void TemplatePropertySettingsWidgetQt<T>::apply() {
    NetworkLock lock(property_);

    std::array<T, 4> vals{property_->getMinValue(), property_->get(), property_->getMaxValue(),
                          property_->getIncrement()};

    const std::array<T, 4> orgVals{vals};

    for (size_t i = 0; i < settings_.size(); i++) {
        for (int k = 0; k < 4; ++k) {
            util::glmcomp(vals[k], i) = static_cast<BT>(settings_[i]->getFieldAsDouble(k));
        }
    }

    static const std::array<void (OrdinalProperty<T>::*)(const T&), 4> setters{
        &OrdinalProperty<T>::setMinValue, &OrdinalProperty<T>::set,
        &OrdinalProperty<T>::setMaxValue, &OrdinalProperty<T>::setIncrement};

    property_->setInitiatingWidget(this);
    for (int k = 0; k < 4; ++k) {
        if (vals[k] != orgVals[k]) (*property_.*setters[k])(vals[k]);
    }
    property_->clearInitiatingWidget();
}

template <typename T>
void TemplatePropertySettingsWidgetQt<T>::reload() {
    std::array<T, 4> vals{property_->getMinValue(), property_->get(), property_->getMaxValue(),
                          property_->getIncrement()};

    QLocale locale = settings_[0]->additionalFields_[0]->locale();
    for (size_t i = 0; i < settings_.size(); i++) {
        for (size_t k = 0; k < vals.size(); ++k) {
            settings_[i]->additionalFields_[k]->setText(
                QStringHelper<BT>::toLocaleString(locale, util::glmcomp(vals[k], i)));
        }
    }
}

template <typename T>
void TemplatePropertySettingsWidgetQt<T>::cancel() {
    hideWidget();
    reload();
}

template <typename T>
void TemplatePropertySettingsWidgetQt<T>::save() {
    hideWidget();
    apply();
}

template <typename T>
bool TemplatePropertySettingsWidgetQt<T>::getVisible() const {
    return isVisible();
}

template <typename T>
void TemplatePropertySettingsWidgetQt<T>::hideWidget() {
    property_->deregisterWidget(this);
    QDialog::setVisible(false);
}

template <typename T>
void TemplatePropertySettingsWidgetQt<T>::showWidget() {
    property_->registerWidget(this);
    updateFromProperty();
    QDialog::setVisible(true);
}

template <typename T>
void TemplatePropertySettingsWidgetQt<T>::updateFromProperty() {
    reload();
}

template <typename T>
void TemplatePropertySettingsWidgetQt<T>::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Escape) {
        cancel();
    } else if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        apply();
    }
    QDialog::keyPressEvent(event);
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
    auto getVal = [&](int index) { return static_cast<BT>(settings_[0]->getFieldAsDouble(index)); };

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

#endif  // IVW_PROPERTYSETTINGSWIDGETQT_H
