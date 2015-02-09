/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
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

#include <inviwo/qt/widgets/properties/colorpropertywidgetqt.h>

#include <inviwo/core/properties/ordinalproperty.h>
#include <QHBoxLayout>

namespace inviwo {

ColorPropertyWidgetQt::ColorPropertyWidgetQt(Property* property)
    : PropertyWidgetQt(property)
    , property_(property) {

    generateWidget();
    updateFromProperty();
}

ColorPropertyWidgetQt::~ColorPropertyWidgetQt() {
    delete currentColor_;
}

void ColorPropertyWidgetQt::generateWidget() {
    QHBoxLayout* hLayout = new QHBoxLayout();
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->setSpacing(7);
    setLayout(hLayout);
    
    currentColor_ = new QColor();
    colorDialog_ = new QColorDialog(this);
    colorDialog_->hide();
    colorDialog_->setOption(QColorDialog::ShowAlphaChannel, true);
    colorDialog_->setOption(QColorDialog::NoButtons, true);
    colorDialog_->setWindowTitle(QString::fromStdString(property_->getDisplayName().c_str()));
    //colorDialog_->setWindowFlags(Qt::WindowStaysOnTopHint);
    colorDialog_->setWindowModality(Qt::NonModal);
    btnColor_ = new IvwPushButton(this);
    btnColor_->setEnabled(!property_->getReadOnly());

    connect(btnColor_, SIGNAL(clicked()), this, SLOT(openColorDialog()));
    connect(colorDialog_, SIGNAL(currentColorChanged(QColor)), this, SLOT(setPropertyValue()));
    label_ = new EditableLabelQt(this, property_->getDisplayName());
    hLayout->addWidget(label_);
    
    {
        QWidget* widget = new QWidget(this);
        QSizePolicy sliderPol = widget->sizePolicy();
        sliderPol.setHorizontalStretch(3);
        widget->setSizePolicy(sliderPol);
        QGridLayout* vLayout = new QGridLayout();
        widget->setLayout(vLayout);
        vLayout->setContentsMargins(0, 0, 0, 0);
        vLayout->setSpacing(0);
        
        vLayout->addWidget(btnColor_);
        hLayout->addWidget(widget);
    }
    
    
    connect(label_, SIGNAL(textChanged()), this, SLOT(setPropertyDisplayName()));

    
    
    setFixedHeight(sizeHint().height());
    QSizePolicy sp = sizePolicy();
    sp.setVerticalPolicy(QSizePolicy::Fixed);
    setSizePolicy(sp);
}

void ColorPropertyWidgetQt::updateFromProperty() {
    if (dynamic_cast<IntVec3Property*>(property_)) {
        ivec3 colorVector = dynamic_cast<IntVec3Property*>(property_)->get();
        colorVector = glm::clamp(colorVector, ivec3(0), ivec3(255));
        currentColor_->setRgb(colorVector.x, colorVector.y, colorVector.z);
    } else if (dynamic_cast<IntVec4Property*>(property_)) {
        ivec4 colorVector = dynamic_cast<IntVec4Property*>(property_)->get();
        colorVector = glm::clamp(colorVector, ivec4(0), ivec4(255));
        currentColor_->setRgb(colorVector.x, colorVector.y, colorVector.z, colorVector.w);
    } else if (dynamic_cast<FloatVec3Property*>(property_)) {
        vec3 colorVector = dynamic_cast<FloatVec3Property*>(property_)->get();
        colorVector = glm::clamp(colorVector, vec3(0.f), vec3(255.f));
        int xVal = static_cast<int>(colorVector.x*255);
        int yVal = static_cast<int>(colorVector.y*255);
        int zVal = static_cast<int>(colorVector.z*255);
        currentColor_->setRgb(xVal, yVal, zVal);
    } else if (dynamic_cast<FloatVec4Property*>(property_)) {
        vec4 colorVector = dynamic_cast<FloatVec4Property*>(property_)->get();
        colorVector = glm::clamp(colorVector, vec4(0.f), vec4(255.f));
        int xVal = static_cast<int>(colorVector.x*255);
        int yVal = static_cast<int>(colorVector.y*255);
        int zVal = static_cast<int>(colorVector.z*255);
        int wVal = static_cast<int>(colorVector.w*255);
        currentColor_->setRgb(xVal, yVal, zVal, wVal);
    }

    QColor topColor = currentColor_->lighter();
    QColor bottomColor = currentColor_->darker();
    btnColor_->setStyleSheet("QPushButton { background: qlineargradient( \
                                                          x1:0, y1:0, x2:0, y2:1, \
                                                          stop:0 "+topColor.name()+", \
                                                          stop: 0.1 "+currentColor_->name()+", \
                                                          stop:0.9 "+currentColor_->name()+", \
                                                          stop:1 "+bottomColor.name()+"); }");

    colorDialog_->blockSignals(true);
    colorDialog_->setCurrentColor(*currentColor_);
    colorDialog_->blockSignals(false);
}

QColor ColorPropertyWidgetQt::getCurrentColor() {
    return *currentColor_;
}

void ColorPropertyWidgetQt::setPropertyValue() {
    if (dynamic_cast<IntVec3Property*>(property_)) {
        dynamic_cast<IntVec3Property*>(property_)->set(ivec3(colorDialog_->currentColor().red(),
                colorDialog_->currentColor().green(),
                colorDialog_->currentColor().blue()));
    } else if (dynamic_cast<IntVec4Property*>(property_)) {
        dynamic_cast<IntVec4Property*>(property_)->set(ivec4(colorDialog_->currentColor().red(),
                colorDialog_->currentColor().green(),
                colorDialog_->currentColor().blue(),
                colorDialog_->currentColor().alpha()));
    } else if (dynamic_cast<FloatVec3Property*>(property_)) {
        dynamic_cast<FloatVec3Property*>(property_)->set(vec3(static_cast<float>(colorDialog_->currentColor().red())/255,
                static_cast<float>(colorDialog_->currentColor().green())/255,
                static_cast<float>(colorDialog_->currentColor().blue())/255));
    } else if (dynamic_cast<FloatVec4Property*>(property_)) {
        dynamic_cast<FloatVec4Property*>(property_)->set(vec4(static_cast<float>(colorDialog_->currentColor().red())/255,
                static_cast<float>(colorDialog_->currentColor().green())/255,
                static_cast<float>(colorDialog_->currentColor().blue())/255,
                static_cast<float>(colorDialog_->currentColor().alpha())/255));
    }

    QColor topColor = currentColor_->lighter();
    QColor bottomColor = currentColor_->darker();
    btnColor_->setStyleSheet("QPushButton { background: qlineargradient( \
                                            x1:0, y1:0, x2:0, y2:1, \
                                            stop:0 "+topColor.name()+", \
                                            stop: 0.1 "+currentColor_->name()+", \
                                            stop:0.9 "+currentColor_->name()+", \
                                            stop:1 "+bottomColor.name()+"); }");
}

void ColorPropertyWidgetQt::openColorDialog() {
    colorDialog_->hide(); // OSX Bug workaround
    updateFromProperty();
    colorDialog_->show();
}

void ColorPropertyWidgetQt::setPropertyDisplayName() {
    property_->setDisplayName(label_->getText());
}

}//namespace