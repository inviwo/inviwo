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

#ifndef IVW_PROPERTYSETTINGSWIDGETQT_H
#define IVW_PROPERTYSETTINGSWIDGETQT_H

// Qt
#include <QWidget>
#include <QLineEdit>
#include <QLabel>
#include <QLocale>
#include <QValidator>
#include <QGridLayout>
#include <QPushButton>
#include <QKeyEvent>
#include <QString>
#include <inviwo/qt/widgets/inviwoqtwidgetsdefine.h>
// Core
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/properties/propertywidget.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/qt/widgets/qtwidgetmodule.h>

namespace inviwo {

struct SinglePropertySetting {
    SinglePropertySetting(QWidget* widget, std::string label)
        : label_(new QLabel(QString::fromStdString(label), widget))
        , min_(new QLineEdit(widget))
        , val_(new QLineEdit(widget))
        , max_(new QLineEdit(widget))
        , inc_(new QLineEdit(widget)) {
        min_->setValidator(new QDoubleValidator(widget));
        val_->setValidator(new QDoubleValidator(widget));
        max_->setValidator(new QDoubleValidator(widget));
        inc_->setValidator(new QDoubleValidator(widget));
    }

    QLabel* label_;
    QLineEdit* min_;
    QLineEdit* val_;
    QLineEdit* max_;
    QLineEdit* inc_;

    double getMinAsDouble() const {
        QLocale locale = min_->locale();
        return locale.toDouble(min_->text().remove(QChar(' ')));
    }
    double getValAsDouble() const {
        QLocale locale = val_->locale();
        return locale.toDouble(val_->text().remove(QChar(' ')));
    }
    double getMaxAsDouble() const {
        QLocale locale = max_->locale();
        return locale.toDouble(max_->text().remove(QChar(' ')));
    }
    double getIncAsDouble() const {
        QLocale locale = inc_->locale();
        return locale.toDouble(inc_->text().remove(QChar(' ')));
    }
};

class IVW_QTWIDGETS_API PropertySettingsWidgetQt : public QWidget, public PropertyWidget {
    Q_OBJECT
public:
    PropertySettingsWidgetQt(Property* property, QWidget*);
    virtual ~PropertySettingsWidgetQt();

    virtual void updateFromProperty() { reload(); };
    virtual void showWidget() { QWidget::setVisible(true); }
    virtual void hideWidget() { QWidget::setVisible(false); }

    virtual UsageMode getUsageMode() const { return property_->getUsageMode(); };

    virtual bool getVisible() const { return isVisible(); };
public slots:
    virtual void apply() = 0;
    virtual void save() = 0;
    virtual void reload() = 0;
    virtual void cancel() = 0;

protected:
    virtual void generateWidget() = 0;

    QGridLayout gridLayout_;
    QPushButton btnApply_;
    QPushButton btnOk_;
    QPushButton btnCancel_;
    std::vector<SinglePropertySetting*> settings_;

    void keyPressEvent(QKeyEvent* event);
    virtual void initializeEditorWidgetsMetaData(){};
};

template <typename BT, typename T>
class TemplatePropertySettingsWidgetQt : public PropertySettingsWidgetQt {
public:
    TemplatePropertySettingsWidgetQt(OrdinalProperty<T>* property, QWidget* widget)
        : PropertySettingsWidgetQt(property, widget), property_(property) {
        generateWidget();
    }

    virtual ~TemplatePropertySettingsWidgetQt() {}

    virtual void generateWidget() {
        connect(&btnApply_, SIGNAL(clicked()), this, SLOT(apply()));
        connect(&btnOk_, SIGNAL(clicked()), this, SLOT(save()));
        connect(&btnCancel_, SIGNAL(clicked()), this, SLOT(cancel()));

        gridLayout_.setContentsMargins(10, 10, 10, 10);
        gridLayout_.setSpacing(10);

        gridLayout_.addWidget(new QLabel("Component", this), 0, 0);
        gridLayout_.addWidget(new QLabel("Min", this), 0, 1);
        gridLayout_.addWidget(new QLabel("Value", this), 0, 2);
        gridLayout_.addWidget(new QLabel("Max", this), 0, 3);
        gridLayout_.addWidget(new QLabel("Increment", this), 0, 4);

        uvec2 components = OrdinalProperty<T>::getDim();

        std::string desc[4];
        desc[0] = "x";
        desc[1] = "y";
        desc[2] = "z";
        desc[3] = "w";

        int count = 0;
        for (size_t i = 0; i < components.x; i++) {
            for (size_t j = 0; j < components.y; j++) {
                std::stringstream ss;
                ss << desc[i] << (components.y == 1 ? "" : ", " + desc[j]);
                settings_.push_back(new SinglePropertySetting(this, ss.str()));
                gridLayout_.addWidget(settings_[count]->label_, count + 1, 0);
                gridLayout_.addWidget(settings_[count]->min_, count + 1, 1);
                gridLayout_.addWidget(settings_[count]->val_, count + 1, 2);
                gridLayout_.addWidget(settings_[count]->max_, count + 1, 3);
                gridLayout_.addWidget(settings_[count]->inc_, count + 1, 4);
                count++;
            }
        }

        gridLayout_.addWidget(&btnApply_, count + 1, 0, 1, 1);
        gridLayout_.addWidget(&btnOk_, count + 1, 1, 1, 2);
        gridLayout_.addWidget(&btnCancel_, count + 1, 3, 1, 2);
        gridLayout_.setColumnStretch(2, 2);

        setLayout(&gridLayout_);

        reload();
        setWindowTitle(QString::fromStdString(property_->getDisplayName().c_str()));
    }

    virtual void save() {
        hide();
        apply();
    }

    virtual void apply() {
        NetworkLock lock;

        uvec2 components = OrdinalProperty<T>::getDim();
        size_t count = 0;
        
        T min = property_->getMinValue();
        T val = property_->get();
        T max = property_->getMaxValue();
        T inc = property_->getIncrement();

        T minOrg = property_->getMinValue();
        T valOrg = property_->get();
        T maxOrg = property_->getMaxValue();
        T incOrg = property_->getIncrement();

        for (size_t i = 0; i < components.x; i++) {
            for (size_t j = 0; j < components.y; j++) {
                util::glmcomp(min, count) = static_cast<BT>(settings_[count]->getMinAsDouble());
                util::glmcomp(val, count) = static_cast<BT>(settings_[count]->getValAsDouble());
                util::glmcomp(max, count) = static_cast<BT>(settings_[count]->getMaxAsDouble());
                util::glmcomp(inc, count) = static_cast<BT>(settings_[count]->getIncAsDouble());
                count++;
            }
        }

        if (min != minOrg) property_->setMinValue(min);
        if (val != valOrg) property_->set(val);
        if (max != maxOrg) property_->setMaxValue(max);
        if (inc != incOrg) property_->setIncrement(inc);
    }

    virtual void reload() {
        uvec2 components = OrdinalProperty<T>::getDim();
        size_t count = 0;

        T min = property_->getMinValue();
        T val = property_->get();
        T max = property_->getMaxValue();
        T inc = property_->getIncrement();

        QLocale locale = settings_[0]->min_->locale();

        for (size_t i = 0; i < components.x; i++) {
            for (size_t j = 0; j < components.y; j++) {
                settings_[count]->min_->setText(
                    QStringHelper<BT>::toLocaleString(locale, util::glmcomp(min, count)));
                settings_[count]->val_->setText(
                    QStringHelper<BT>::toLocaleString(locale, util::glmcomp(val, count)));
                settings_[count]->max_->setText(
                    QStringHelper<BT>::toLocaleString(locale, util::glmcomp(max, count)));
                settings_[count]->inc_->setText(
                    QStringHelper<BT>::toLocaleString(locale, util::glmcomp(inc, count)));
                count++;
            }
        }
    }
    virtual void cancel() {
        hide();
        reload();
    }

private:
    OrdinalProperty<T>* property_;
};

}  // namespace

#endif  // IVW_PROPERTYSETTINGSWIDGETQT_H
