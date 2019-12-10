/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2019 Inviwo Foundation
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

#ifndef IVW_ORDINALMINMAXPROPERTYWIDGETQT_H
#define IVW_ORDINALMINMAXPROPERTYWIDGETQT_H

#include <modules/qtwidgets/qtwidgetsmoduledefine.h>
#include <modules/qtwidgets/inviwoqtutils.h>
#include <modules/qtwidgets/properties/propertywidgetqt.h>
#include <modules/qtwidgets/properties/propertysettingswidgetqt.h>
#include <inviwo/core/properties/minmaxproperty.h>
#include <modules/qtwidgets/numberlineedit.h>
#include <modules/qtwidgets/editablelabelqt.h>
#include <modules/qtwidgets/rangesliderqt.h>
#include <modules/qtwidgets/inviwoqtutils.h>
#include <inviwo/core/properties/propertyowner.h>
#include <inviwo/core/util/stringconversion.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QAction>
#include <QMenu>
#include <QHBoxLayout>
#include <QSignalBlocker>
#include <warn/pop>

namespace inviwo {

template <typename T, class Enable = void>
class Transformer {
public:
    static T sliderToValue(MinMaxProperty<T>*, int val) { return static_cast<T>(val); };
    static int valueToSlider(MinMaxProperty<T>*, T val) { return static_cast<int>(val); }
    static int sepToSlider(MinMaxProperty<T>*, T sep) { return static_cast<int>(sep); }
    static T spinboxToValue(MinMaxProperty<T>*, double val) { return static_cast<T>(val); };
    static double valueToSpinbox(MinMaxProperty<T>*, T val) { return static_cast<double>(val); }
};

template <typename T>
class Transformer<T, typename std::enable_if<util::is_floating_point<T>::value>::type> {
public:
    static T sliderToValue(MinMaxProperty<T>* p, int val) {
        return (static_cast<T>(val) / steps()) * (p->getRangeMax() - p->getRangeMin()) +
               p->getRangeMin();
    }
    static int valueToSlider(MinMaxProperty<T>* p, T val) {
        return static_cast<int>((val - p->getRangeMin()) / (p->getRangeMax() - p->getRangeMin()) *
                                steps());
    }
    static int sepToSlider(MinMaxProperty<T>* p, T sep) {
        return static_cast<int>(sep / (p->getRangeMax() - p->getRangeMin()) * steps());
    }
    static T spinboxToValue(MinMaxProperty<T>*, double val) { return static_cast<T>(val); }
    static double valueToSpinbox(MinMaxProperty<T>*, T val) { return static_cast<double>(val); }
    static T steps() { return 10000; }
};

template <typename T>
class OrdinalMinMaxPropertyWidgetQt : public PropertyWidgetQt {
public:
    OrdinalMinMaxPropertyWidgetQt(MinMaxProperty<T>* property);
    virtual ~OrdinalMinMaxPropertyWidgetQt() = default;

    virtual void updateFromProperty() override;
    virtual std::unique_ptr<QMenu> getContextMenu() override;

    using V = glm::tvec2<T, glm::defaultp>;

private:
    void updateFromSlider(int valMin, int valMax);
    void updateFromSpinBoxMin(double val);
    void updateFromSpinBoxMax(double val);
    int transformIncrementToSpinnerDecimals();
    void showSettings();

    TemplateMinMaxPropertySettingsWidgetQt<T>* settingsWidget_;
    RangeSliderQt* slider_;
    NumberLineEdit* spinBoxMin_;
    NumberLineEdit* spinBoxMax_;
    EditableLabelQt* label_;
    MinMaxProperty<T>* minMaxProperty_;
};

using DoubleMinMaxPropertyWidgetQt = OrdinalMinMaxPropertyWidgetQt<double>;
using FloatMinMaxPropertyWidgetQt = OrdinalMinMaxPropertyWidgetQt<float>;
using IntSizeTMinMaxPropertyWidgetQt = OrdinalMinMaxPropertyWidgetQt<size_t>;
using Int64MinMaxPropertyWidgetQt = OrdinalMinMaxPropertyWidgetQt<glm::i64>;
using IntMinMaxPropertyWidgetQt = OrdinalMinMaxPropertyWidgetQt<int>;

template <typename T>
OrdinalMinMaxPropertyWidgetQt<T>::OrdinalMinMaxPropertyWidgetQt(MinMaxProperty<T>* property)
    : PropertyWidgetQt(property)
    , settingsWidget_(nullptr)
    , slider_(new RangeSliderQt(Qt::Horizontal, this))
    , spinBoxMin_(new NumberLineEdit(std::is_integral<T>::value, this))
    , spinBoxMax_(new NumberLineEdit(std::is_integral<T>::value, this))
    , label_(new EditableLabelQt(this, property_))
    , minMaxProperty_(property) {

    QHBoxLayout* hLayout = new QHBoxLayout();
    setSpacingAndMargins(hLayout);
    hLayout->addWidget(label_);

    slider_->handle(0)->setFocusProxy(spinBoxMin_);
    slider_->handle(1)->setFocusProxy(spinBoxMax_);

    setFocusPolicy(spinBoxMin_->focusPolicy());
    setFocusProxy(spinBoxMin_);

    QHBoxLayout* hSliderLayout = new QHBoxLayout();
    QWidget* sliderWidget = new QWidget();
    sliderWidget->setLayout(hSliderLayout);
    hSliderLayout->setContentsMargins(0, 0, 0, 0);

    spinBoxMin_->setKeyboardTracking(false);  // don't emit the valueChanged() signal while typing
    spinBoxMin_->setFixedWidth(utilqt::emToPx(this, 4.6));
    hSliderLayout->addWidget(spinBoxMin_);

    hSliderLayout->addWidget(slider_);

    spinBoxMax_->setKeyboardTracking(false);  // don't emit the valueChanged() signal while typing
    spinBoxMax_->setFixedWidth(utilqt::emToPx(this, 4.6));
    hSliderLayout->addWidget(spinBoxMax_);

    hLayout->addWidget(sliderWidget);
    setLayout(hLayout);

    QSizePolicy slidersPol = sliderWidget->sizePolicy();
    slidersPol.setHorizontalStretch(3);
    sliderWidget->setSizePolicy(slidersPol);

    connect(slider_, &RangeSliderQt::valuesChanged, this,
            &OrdinalMinMaxPropertyWidgetQt<T>::updateFromSlider);
    connect(spinBoxMin_,
            static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this,
            &OrdinalMinMaxPropertyWidgetQt<T>::updateFromSpinBoxMin);
    connect(spinBoxMax_,
            static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this,
            &OrdinalMinMaxPropertyWidgetQt<T>::updateFromSpinBoxMax);

    updateFromProperty();
}

template <typename T>
void OrdinalMinMaxPropertyWidgetQt<T>::updateFromProperty() {
    const V val = minMaxProperty_->get();
    const V range = minMaxProperty_->getRange();
    const T inc = minMaxProperty_->getIncrement();
    const T sep = minMaxProperty_->getMinSeparation();

    QSignalBlocker minBlock(spinBoxMin_);
    QSignalBlocker maxBlock(spinBoxMax_);
    QSignalBlocker slideBlock(slider_);

    using F = Transformer<T>;
    const auto p = minMaxProperty_;

    spinBoxMin_->setRange(F::valueToSpinbox(p, range.x), F::valueToSpinbox(p, range.y - sep));
    spinBoxMin_->setSingleStep(F::valueToSpinbox(p, inc));
    spinBoxMin_->setValue(F::valueToSpinbox(p, val.x));
    spinBoxMin_->setDecimals(transformIncrementToSpinnerDecimals());

    spinBoxMax_->setRange(F::valueToSpinbox(p, range.x + sep), F::valueToSpinbox(p, range.y));
    spinBoxMax_->setSingleStep(F::valueToSpinbox(p, inc));
    spinBoxMax_->setValue(F::valueToSpinbox(p, val.y));
    spinBoxMax_->setDecimals(transformIncrementToSpinnerDecimals());

    slider_->setRange(F::valueToSlider(p, range.x), F::valueToSlider(p, range.y));
    slider_->setMinSeparation(F::sepToSlider(p, sep));
    slider_->setValue(F::valueToSlider(p, val.x), F::valueToSlider(p, val.y));
}

template <typename T>
void OrdinalMinMaxPropertyWidgetQt<T>::updateFromSlider(int valMin, int valMax) {
    const T min = Transformer<T>::sliderToValue(minMaxProperty_, valMin);
    const T max = Transformer<T>::sliderToValue(minMaxProperty_, valMax);

    bool modified = false;
    V range = minMaxProperty_->get();

    if (!util::almostEqual(min, range.x)) {
        modified = true;
        range.x = min;
        QSignalBlocker minBlock(spinBoxMin_);
        spinBoxMin_->setValue(Transformer<T>::valueToSpinbox(minMaxProperty_, min));
    }

    if (!util::almostEqual(max, range.y)) {
        modified = true;
        range.y = max;
        QSignalBlocker maxBlock(spinBoxMax_);
        spinBoxMax_->setValue(Transformer<T>::valueToSpinbox(minMaxProperty_, max));
    }

    if (modified) {
        minMaxProperty_->setInitiatingWidget(this);
        minMaxProperty_->set(range);
        minMaxProperty_->clearInitiatingWidget();
    }
}

template <typename T>
void OrdinalMinMaxPropertyWidgetQt<T>::updateFromSpinBoxMin(double minVal) {
    const T min = Transformer<T>::spinboxToValue(minMaxProperty_, minVal);
    const T sep = minMaxProperty_->getMinSeparation();
    V range = minMaxProperty_->get();

    if (!util::almostEqual(min, range.x)) {
        range.x = min;

        if (range.y - range.x < sep) {
            range.y = range.x + sep;
            QSignalBlocker maxBlock(spinBoxMax_);
            QSignalBlocker slideBlock(slider_);
            spinBoxMax_->setValue(Transformer<T>::valueToSpinbox(minMaxProperty_, range.y));
            slider_->setValue(Transformer<T>::valueToSlider(minMaxProperty_, range.x),
                              Transformer<T>::valueToSlider(minMaxProperty_, range.y));
        } else {
            QSignalBlocker slideBlock(slider_);
            slider_->setMinValue(Transformer<T>::valueToSlider(minMaxProperty_, range.x));
        }

        minMaxProperty_->setInitiatingWidget(this);
        minMaxProperty_->set(range);
        minMaxProperty_->clearInitiatingWidget();
    }
}

template <typename T>
void OrdinalMinMaxPropertyWidgetQt<T>::updateFromSpinBoxMax(double maxVal) {
    const T max = Transformer<T>::spinboxToValue(minMaxProperty_, maxVal);
    const T sep = minMaxProperty_->getMinSeparation();
    V range = minMaxProperty_->get();

    if (!util::almostEqual(max, range.y)) {
        range.y = max;

        if (range.y - range.x < sep) {
            range.x = range.y - sep;
            QSignalBlocker minBlock(spinBoxMin_);
            QSignalBlocker slideBlock(slider_);
            spinBoxMin_->setValue(Transformer<T>::valueToSpinbox(minMaxProperty_, range.x));
            slider_->setValue(Transformer<T>::valueToSlider(minMaxProperty_, range.x),
                              Transformer<T>::valueToSlider(minMaxProperty_, range.y));
        } else {
            QSignalBlocker slideBlock(slider_);
            slider_->setMaxValue(Transformer<T>::valueToSlider(minMaxProperty_, range.y));
        }

        minMaxProperty_->setInitiatingWidget(this);
        minMaxProperty_->set(range);
        minMaxProperty_->clearInitiatingWidget();
    }
}

template <typename T>
std::unique_ptr<QMenu> OrdinalMinMaxPropertyWidgetQt<T>::getContextMenu() {
    auto menu = PropertyWidgetQt::getContextMenu();
    auto settingsAction = menu->addAction(tr("&Property settings..."));
    settingsAction->setToolTip(
        tr("&Open the property settings dialog to adjust min bound, start, end, max bound, "
           "minSepration and increment values"));

    connect(settingsAction, &QAction::triggered, this,
            &OrdinalMinMaxPropertyWidgetQt<T>::showSettings);
    settingsAction->setEnabled(!property_->getReadOnly());
    settingsAction->setVisible(getApplicationUsageMode() == UsageMode::Development);
    return menu;
}

template <typename T>
int OrdinalMinMaxPropertyWidgetQt<T>::transformIncrementToSpinnerDecimals() {
    const static QLocale locale;
    double inc = Transformer<T>::valueToSpinbox(minMaxProperty_, minMaxProperty_->getIncrement());
    std::ostringstream buff;
    utilqt::localizeStream(buff);
    buff << inc;
    const std::string str(buff.str());
    auto periodPosition = str.find(locale.decimalPoint().toLatin1());
    if (periodPosition == std::string::npos) {
        return 0;
    } else {
        return static_cast<int>(str.length() - periodPosition - 1);
    }
}

template <typename T>
void OrdinalMinMaxPropertyWidgetQt<T>::showSettings() {
    if (!settingsWidget_) {
        settingsWidget_ = new TemplateMinMaxPropertySettingsWidgetQt<T>(minMaxProperty_, this);
    }
    settingsWidget_->showWidget();
}

}  // namespace inviwo

#endif  // IVW_ORDINALMINMAXPROPERTYWIDGETQT_H
