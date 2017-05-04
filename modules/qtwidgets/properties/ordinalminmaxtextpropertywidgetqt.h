/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2017 Inviwo Foundation
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

#ifndef IVW_ORDINALMINMAXTEXTTROPERTYWIDGETQT_H
#define IVW_ORDINALMINMAXTEXTTROPERTYWIDGETQT_H

#include <modules/qtwidgets/qtwidgetsmoduledefine.h>
#include <modules/qtwidgets/inviwoqtutils.h>
#include <modules/qtwidgets/properties/propertywidgetqt.h>
#include <modules/qtwidgets/properties/propertysettingswidgetqt.h>
#include <modules/qtwidgets/editablelabelqt.h>
#include <modules/qtwidgets/ordinaleditorwidget.h>
#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/core/properties/propertyowner.h>
#include <inviwo/core/util/stringconversion.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QAction>
#include <QHBoxLayout>
#include <QMenu>
#include <QSignalBlocker>
#include <warn/pop>

namespace inviwo {


template <typename BT, typename T>
class OrdinalMinMaxTextPropertyWidgetQt : public PropertyWidgetQt {

public:
    OrdinalMinMaxTextPropertyWidgetQt(MinMaxProperty<T>* property);
    virtual ~OrdinalMinMaxTextPropertyWidgetQt();

    virtual void updateFromProperty() override;
    virtual std::unique_ptr<QMenu> getContextMenu() override;

    using V = glm::tvec2<T, glm::defaultp>;
    
protected:
    void updateFromMin();
    void updateFromMax();
    void showSettings();

    TemplateMinMaxPropertySettingsWidgetQt<T>* settingsWidget_;
    EditableLabelQt* label_;
    MinMaxProperty<T>* minMaxProperty_;
    OrdinalEditorWidget<T>* min_;
    OrdinalEditorWidget<T>* max_;
};

using DoubleMinMaxTextPropertyWidgetQt = OrdinalMinMaxTextPropertyWidgetQt<double, double>;
using FloatMinMaxTextPropertyWidgetQt = OrdinalMinMaxTextPropertyWidgetQt<double, float>;
using IntMinMaxTextPropertyWidgetQt = OrdinalMinMaxTextPropertyWidgetQt<int, int>;

template <typename BT, typename T>
OrdinalMinMaxTextPropertyWidgetQt<BT, T>::OrdinalMinMaxTextPropertyWidgetQt(
    MinMaxProperty<T>* property)
    : PropertyWidgetQt(property)
    , settingsWidget_(nullptr)
    , label_(new EditableLabelQt(this, property_))
    , minMaxProperty_(property)
    , min_{new OrdinalEditorWidget<T>()}
    , max_{new OrdinalEditorWidget<T>()} {

    QHBoxLayout* hLayout = new QHBoxLayout();
    setSpacingAndMargins(hLayout);
    hLayout->addWidget(label_);

    QHBoxLayout* textLayout = new QHBoxLayout();
    QWidget* textWidget = new QWidget();
    textWidget->setLayout(textLayout);
    textLayout->setContentsMargins(0, 0, 0, 0);

    QSizePolicy sp = QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    sp.setHorizontalStretch(3);

    textLayout->addWidget(new QLabel("Min:"));
    textLayout->addWidget(min_);
    min_->setSizePolicy(sp);

    textLayout->addWidget(new QLabel("Max:"));
    textLayout->addWidget(max_);
    max_->setSizePolicy(sp);

    hLayout->addWidget(textWidget);
    setLayout(hLayout);

    QSizePolicy textsp = textWidget->sizePolicy();
    textsp.setHorizontalStretch(3);
    textWidget->setSizePolicy(textsp);

    connect(min_, &OrdinalEditorWidget<T>::valueChanged, this,
            &OrdinalMinMaxTextPropertyWidgetQt<BT, T>::updateFromMin);
    connect(max_, &OrdinalEditorWidget<T>::valueChanged, this,
            &OrdinalMinMaxTextPropertyWidgetQt<BT, T>::updateFromMax);

    setFixedHeight(sizeHint().height());
    sp = sizePolicy();
    sp.setVerticalPolicy(QSizePolicy::Fixed);
    setSizePolicy(sp);

    updateFromProperty();
}

template <typename BT, typename T>
OrdinalMinMaxTextPropertyWidgetQt<BT, T>::~OrdinalMinMaxTextPropertyWidgetQt() = default;

template <typename BT, typename T>
void OrdinalMinMaxTextPropertyWidgetQt<BT, T>::updateFromProperty() {
    const V val = minMaxProperty_->get();
    const V range = minMaxProperty_->getRange();
    const T inc = minMaxProperty_->getIncrement();
    const T sep = minMaxProperty_->getMinSeparation();

    QSignalBlocker minBlock(min_);
    QSignalBlocker maxBlock(max_);

    min_->setRange(range.x, range.y - sep);
    max_->setRange(range.x + sep, range.y);

    min_->initValue(val.x);
    max_->initValue(val.y);

    min_->setIncrement(inc);
    max_->setIncrement(inc);
}

template <typename BT, typename T>
void OrdinalMinMaxTextPropertyWidgetQt<BT, T>::updateFromMin() {
    const T min = min_->getValue();
    const T sep = minMaxProperty_->getMinSeparation();
    V range = minMaxProperty_->get();

    if (std::abs(min - range.x) > glm::epsilon<T>()) {
        range.x = min;

        if (range.y - range.x < sep) {
            range.y = range.x + sep;
            QSignalBlocker maxBlock(max_);
            max_->setValue(range.y);
        }

        minMaxProperty_->setInitiatingWidget(this);
        minMaxProperty_->set(range);
        minMaxProperty_->clearInitiatingWidget();
    }
}

template <typename BT, typename T>
void OrdinalMinMaxTextPropertyWidgetQt<BT, T>::updateFromMax() {
    const T max = max_->getValue();
    const T sep = minMaxProperty_->getMinSeparation();
    V range = minMaxProperty_->get();

    if (std::abs(max - range.y) > glm::epsilon<T>()) {
        range.y = max;

        if (range.y - range.x < sep) {
            range.x = range.y - sep;
            QSignalBlocker minBlock(min_);
            min_->setValue(range.x);
        }

        minMaxProperty_->setInitiatingWidget(this);
        minMaxProperty_->set(range);
        minMaxProperty_->clearInitiatingWidget();
    }
}

template <typename BT, typename T>
std::unique_ptr<QMenu> OrdinalMinMaxTextPropertyWidgetQt<BT, T>::getContextMenu() {
    auto menu = PropertyWidgetQt::getContextMenu();
    auto settingsAction = menu->addAction(tr("&Property settings..."));
    settingsAction->setToolTip(
        tr("&Open the property settings dialog to adjust min bound, start, end, max bound, "
           "minSepration and increment values"));

    connect(settingsAction, SIGNAL(triggered()), this, SLOT(showSettings()));
    settingsAction->setEnabled(!property_->getReadOnly());
    settingsAction->setVisible(getApplicationUsageMode() == UsageMode::Development);
    return menu;
}

template <typename BT, typename T>
void OrdinalMinMaxTextPropertyWidgetQt<BT, T>::showSettings() {
    if (!settingsWidget_) {
        settingsWidget_ = new TemplateMinMaxPropertySettingsWidgetQt<T>(minMaxProperty_, this);
    }
    settingsWidget_->showWidget();
}

} // namespace

#endif // IVW_ORDINALMINMAXTEXTTROPERTYWIDGETQT_H

