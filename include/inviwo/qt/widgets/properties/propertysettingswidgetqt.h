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


#include <inviwo/qt/widgets/inviwoqtwidgetsdefine.h>
// Core
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/properties/propertywidget.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/core/network/networklock.h>
#include <inviwo/qt/widgets/qstringhelper.h>

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

struct SinglePropertySetting {
    SinglePropertySetting(QWidget* widget, std::string label)
        : label_(new QLabel(QString::fromStdString(label), widget)), widget_(widget) {}

    QLabel* label_;
    QWidget* widget_;
    std::vector<QLineEdit*> additionalFields_;

    QLineEdit* addField() {
        QLineEdit* ext = new QLineEdit(widget_);
        ext->setValidator(new QDoubleValidator(widget_));
        additionalFields_.push_back(ext);
        return ext;
    }

    double getFieldAsDouble(int i) {
        if (i >= 0 && i < additionalFields_.size()) {
            QLocale locale = additionalFields_[i]->locale();
            return locale.toDouble(additionalFields_[i]->text().remove(QChar(' ')));
        }
        return DataFLOAT64::minToDouble();
    }
};

class IVW_QTWIDGETS_API PropertySettingsWidgetQt : public QDialog, public PropertyWidget {
    Q_OBJECT
public:
    PropertySettingsWidgetQt(Property* property, QWidget*);
    virtual ~PropertySettingsWidgetQt();

    virtual void updateFromProperty() { reload(); };
    virtual void showWidget() {
        property_->registerWidget(this);
        updateFromProperty();
        QDialog::setVisible(true);
    }
    virtual void hideWidget() {
        property_->deregisterWidget(this);
        QDialog::setVisible(false);
    }

    virtual UsageMode getUsageMode() const { return property_->getUsageMode(); };

    virtual bool getVisible() const { return isVisible(); };
public slots:
    virtual void apply() = 0;
    virtual void save() = 0;
    virtual void reload() = 0;
    virtual void cancel() = 0;

protected:
    virtual void generateWidget() = 0;

    QGridLayout* gridLayout_;
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

        gridLayout_->setContentsMargins(10, 10, 10, 10);
        gridLayout_->setSpacing(10);

        gridLayout_->addWidget(new QLabel("Component", this), 0, 0);
        gridLayout_->addWidget(new QLabel("Min", this), 0, 1);
        gridLayout_->addWidget(new QLabel("Value", this), 0, 2);
        gridLayout_->addWidget(new QLabel("Max", this), 0, 3);
        gridLayout_->addWidget(new QLabel("Increment", this), 0, 4);

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
                QLineEdit* min = settings_[count]->addField();
                QLineEdit* val = settings_[count]->addField();
                QLineEdit* max = settings_[count]->addField();
                QLineEdit* inc = settings_[count]->addField();
                int t = 0;
                gridLayout_->addWidget(settings_[count]->label_, count + 1, t++);
                gridLayout_->addWidget(min, count + 1, t++);
                gridLayout_->addWidget(val, count + 1, t++);
                gridLayout_->addWidget(max, count + 1, t++);
                gridLayout_->addWidget(inc, count + 1, t++);
                count++;
            }
        }

        gridLayout_->addWidget(&btnApply_, count + 1, 0, 1, 1);
        gridLayout_->addWidget(&btnOk_, count + 1, 1, 1, 2);
        gridLayout_->addWidget(&btnCancel_, count + 1, 3, 1, 2);
        gridLayout_->setColumnStretch(2, 2);

        setLayout(gridLayout_);

        reload();
        setWindowTitle(QString::fromStdString(property_->getDisplayName().c_str()));
    }

    virtual void save() {
        hideWidget();
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
                int t = 0;
                util::glmcomp(min, count) =
                    static_cast<BT>(settings_[count]->getFieldAsDouble(t++));
                util::glmcomp(val, count) =
                    static_cast<BT>(settings_[count]->getFieldAsDouble(t++));
                util::glmcomp(max, count) =
                    static_cast<BT>(settings_[count]->getFieldAsDouble(t++));
                util::glmcomp(inc, count) =
                    static_cast<BT>(settings_[count]->getFieldAsDouble(t++));
                count++;
            }
        }
        property_->setInitiatingWidget(this);
        if (min != minOrg) property_->setMinValue(min);
        if (val != valOrg) property_->set(val);
        if (max != maxOrg) property_->setMaxValue(max);
        if (inc != incOrg) property_->setIncrement(inc);
        property_->clearInitiatingWidget();
    }

    virtual void reload() {
        uvec2 components = OrdinalProperty<T>::getDim();
        size_t count = 0;

        T min = property_->getMinValue();
        T val = property_->get();
        T max = property_->getMaxValue();
        T inc = property_->getIncrement();

        QLocale locale = settings_[0]->additionalFields_[0]->locale();

        for (size_t i = 0; i < components.x; i++) {
            for (size_t j = 0; j < components.y; j++) {
                int t = 0;
                settings_[count]->additionalFields_[t++]->setText(
                    QStringHelper<BT>::toLocaleString(locale, util::glmcomp(min, count)));
                settings_[count]->additionalFields_[t++]->setText(
                    QStringHelper<BT>::toLocaleString(locale, util::glmcomp(val, count)));
                settings_[count]->additionalFields_[t++]->setText(
                    QStringHelper<BT>::toLocaleString(locale, util::glmcomp(max, count)));
                settings_[count]->additionalFields_[t++]->setText(
                    QStringHelper<BT>::toLocaleString(locale, util::glmcomp(inc, count)));
                count++;
            }
        }
    }
    virtual void cancel() {
        hideWidget();
        reload();
    }

private:
    OrdinalProperty<T>* property_;
};

template <typename BT, typename T>
class TemplateMinMaxPropertySettingsWidgetQt : public PropertySettingsWidgetQt {
public:
    TemplateMinMaxPropertySettingsWidgetQt(MinMaxProperty<T>* property, QWidget* widget)
        : PropertySettingsWidgetQt(property, widget), property_(property) {
        generateWidget();
    }

    virtual ~TemplateMinMaxPropertySettingsWidgetQt() {}

    typedef glm::detail::tvec2<T, glm::defaultp> V;

    virtual void generateWidget() {
        connect(&btnApply_, SIGNAL(clicked()), this, SLOT(apply()));
        connect(&btnOk_, SIGNAL(clicked()), this, SLOT(save()));
        connect(&btnCancel_, SIGNAL(clicked()), this, SLOT(cancel()));

        gridLayout_->setContentsMargins(10, 10, 10, 10);
        gridLayout_->setSpacing(10);

        gridLayout_->addWidget(new QLabel("Component", this), 0, 0);
        gridLayout_->addWidget(new QLabel("Min Bound", this), 0, 1);
        gridLayout_->addWidget(new QLabel("Start", this), 0, 2);
        gridLayout_->addWidget(new QLabel("End", this), 0, 3);
        gridLayout_->addWidget(new QLabel("Max Bound", this), 0, 4);
        gridLayout_->addWidget(new QLabel("MinSeparation", this), 0, 5);
        gridLayout_->addWidget(new QLabel("Increment", this), 0, 6);

        uvec2 components = MinMaxProperty<T>::getDim();

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
                QLineEdit* rangeMin = settings_[count]->addField();
                QLineEdit* min = settings_[count]->addField();
                QLineEdit* max = settings_[count]->addField();
                QLineEdit* rangeMax = settings_[count]->addField();
                QLineEdit* minSep = settings_[count]->addField();
                QLineEdit* inc = settings_[count]->addField();
                int t = 0;
                gridLayout_->addWidget(settings_[count]->label_, count + 1, t++);
                gridLayout_->addWidget(rangeMin, count + 1, t++);
                gridLayout_->addWidget(min, count + 1, t++);
                gridLayout_->addWidget(max, count + 1, t++); 
                gridLayout_->addWidget(rangeMax, count + 1, t++);
                gridLayout_->addWidget(minSep, count + 1, t++);
                gridLayout_->addWidget(inc, count + 1, t++);
                count++;
            }
        }

        gridLayout_->addWidget(&btnApply_, count + 1, 0, 1, 1);
        gridLayout_->addWidget(&btnOk_, count + 1, 1, 1, 2);
        gridLayout_->addWidget(&btnCancel_, count + 1, 3, 1, 2);
        gridLayout_->setColumnStretch(2, 2);

        setLayout(gridLayout_);

        reload();
        setWindowTitle(QString::fromStdString(property_->getDisplayName().c_str()));
    }

    virtual void save() {
        hideWidget();
        apply();
    }

    virtual void apply() {
        NetworkLock lock;

        uvec2 components = MinMaxProperty<T>::getDim();
        size_t count = 0;

        V range = property_->get();
        T min = range.x;
        T rangeMin = property_->getRangeMin();
        T rangeMax = property_->getRangeMax();
        T max = range.y;
        T minSep = property_->getMinSeparation();
        T inc = property_->getIncrement();

        T minOrg = range.x;
        T rangeMinOrg = property_->getRangeMin();
        T rangeMaxOrg = property_->getRangeMax();
        T maxOrg = range.y;
        T minSepOrg = property_->getMinSeparation();
        T incOrg = property_->getIncrement();

        for (size_t i = 0; i < components.x; i++) {
            for (size_t j = 0; j < components.y; j++) {
                int t = 0;
                util::glmcomp(rangeMin, count) =
                    static_cast<BT>(settings_[count]->getFieldAsDouble(t++));
                util::glmcomp(min, count) =
                    static_cast<BT>(settings_[count]->getFieldAsDouble(t++));
                util::glmcomp(max, count) =
                    static_cast<BT>(settings_[count]->getFieldAsDouble(t++));
                util::glmcomp(rangeMax, count) =
                    static_cast<BT>(settings_[count]->getFieldAsDouble(t++));
                util::glmcomp(minSep, count) =
                    static_cast<BT>(settings_[count]->getFieldAsDouble(t++));
                util::glmcomp(inc, count) =
                    static_cast<BT>(settings_[count]->getFieldAsDouble(t++));
                count++;
            }
        }
        range.x = min;
        range.y = max;
        property_->setInitiatingWidget(this);
        if (min != minOrg || max != maxOrg) property_->set(range);
        if (rangeMin != rangeMinOrg) property_->setRangeMin(rangeMin);
        if (rangeMax != rangeMaxOrg) property_->setRangeMax(rangeMax);
        if (minSep != minSepOrg) property_->setMinSeparation(minSep);
        if (inc != incOrg) property_->setIncrement(inc);
        property_->clearInitiatingWidget();
    }

    virtual void reload() {
        uvec2 components = MinMaxProperty<T>::getDim();
        size_t count = 0;

        V range = property_->get();
        T min = range.x;
        T rangeMin = property_->getRangeMin();
        T rangeMax = property_->getRangeMax();
        T max = range.y;
        T minSep = property_->getMinSeparation();
        T inc = property_->getIncrement();

        QLocale locale = settings_[0]->additionalFields_[0]->locale();

        for (size_t i = 0; i < components.x; i++) {
            for (size_t j = 0; j < components.y; j++) {
                int t = 0;
                settings_[count]->additionalFields_[t++]->setText(
                    QStringHelper<BT>::toLocaleString(locale, util::glmcomp(rangeMin, count)));
                settings_[count]->additionalFields_[t++]->setText(
                    QStringHelper<BT>::toLocaleString(locale, util::glmcomp(min, count)));
                settings_[count]->additionalFields_[t++]->setText(
                    QStringHelper<BT>::toLocaleString(locale, util::glmcomp(max, count)));
                settings_[count]->additionalFields_[t++]->setText(
                    QStringHelper<BT>::toLocaleString(locale, util::glmcomp(rangeMax, count)));
                settings_[count]->additionalFields_[t++]->setText(
                    QStringHelper<BT>::toLocaleString(locale, util::glmcomp(minSep, count)));
                settings_[count]->additionalFields_[t++]->setText(
                    QStringHelper<BT>::toLocaleString(locale, util::glmcomp(inc, count)));
                count++;
            }
        }
    }
    virtual void cancel() {
        hideWidget();
        reload();
    }

private:
    MinMaxProperty<T>* property_;
};

}  // namespace

#endif  // IVW_PROPERTYSETTINGSWIDGETQT_H
