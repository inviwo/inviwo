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

#ifndef IVW_COLORPROPERTYWDIGETQT_H
#define IVW_COLORPROPERTYWDIGETQT_H

#include <modules/qtwidgets/qtwidgetsmoduledefine.h>
#include <modules/qtwidgets/properties/propertywidgetqt.h>
#include <modules/qtwidgets/editablelabelqt.h>
#include <modules/qtwidgets/inviwoqtutils.h>
#include <modules/qtwidgets/properties/colorlineedit.h>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/properties/property.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/metadata/metadata.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QColor>
#include <QHBoxLayout>
#include <QColorDialog>
#include <QSignalBlocker>
#include <QAction>
#include <QActionGroup>
#include <QMenu>
#include <QToolButton>
#include <warn/pop>

namespace inviwo {

template <typename T>
class ColorPropertyWidgetQt : public PropertyWidgetQt {
public:
    ColorPropertyWidgetQt(OrdinalProperty<T>* property);
    virtual ~ColorPropertyWidgetQt() = default;

    virtual void updateFromProperty() override;
    const QColor& getCurrentColor() const;

    virtual std::unique_ptr<QMenu> getContextMenu() override;

private:
    void setPropertyValue();
    void openColorDialog();
    void createColorDialog();
    void offsetColorDialog();
    void updateButton();

    OrdinalProperty<T>* ordinalProperty_;
    QToolButton* btnColor_;
    ColorLineEdit* colorLineEdit_;
    QColorDialog* colorDialog_;
    QColor currentColor_;
    EditableLabelQt* label_;
};

template <typename T>
ColorPropertyWidgetQt<T>::ColorPropertyWidgetQt(OrdinalProperty<T>* property)
    : PropertyWidgetQt(property)
    , ordinalProperty_(property)
    , btnColor_{new QToolButton()}
    , colorLineEdit_{new ColorLineEdit()}
    , colorDialog_{nullptr}
    , currentColor_{}
    , label_{new EditableLabelQt(this, property)} {

    setFocusPolicy(colorLineEdit_->focusPolicy());
    setFocusProxy(colorLineEdit_);

    QHBoxLayout* hLayout = new QHBoxLayout();
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->setSpacing(getSpacing());
    setLayout(hLayout);

    QSizePolicy spBtn = sizePolicy();
    spBtn.setHorizontalPolicy(QSizePolicy::Fixed);
    spBtn.setVerticalPolicy(QSizePolicy::Fixed);
    spBtn.setHorizontalStretch(0);
    btnColor_->setSizePolicy(spBtn);
    btnColor_->setAutoRaise(true);
    btnColor_->setObjectName("ColorButton");
    btnColor_->setFocusPolicy(Qt::ClickFocus);

    connect(btnColor_, &QToolButton::clicked, this, &ColorPropertyWidgetQt::openColorDialog);
    hLayout->addWidget(label_);

    connect(colorLineEdit_, &ColorLineEdit::colorChanged, this,
            [&]() { ordinalProperty_->set(colorLineEdit_->getColor<T>()); });

    {
        QWidget* widget = new QWidget(this);
        QSizePolicy policy = widget->sizePolicy();
        policy.setHorizontalStretch(3);
        widget->setSizePolicy(policy);
        QHBoxLayout* widgetLayout = new QHBoxLayout();
        widget->setLayout(widgetLayout);
        widgetLayout->setContentsMargins(0, 0, 0, 0);
        widgetLayout->setSpacing(getSpacing());

        widgetLayout->addWidget(btnColor_);
        widgetLayout->addWidget(colorLineEdit_);

        hLayout->addWidget(widget);
    }

    setFixedHeight(sizeHint().height());
    QSizePolicy sp = sizePolicy();
    sp.setVerticalPolicy(QSizePolicy::Fixed);
    setSizePolicy(sp);
    updateFromProperty();
}

template <typename T>
void ColorPropertyWidgetQt<T>::createColorDialog() {
    if (!colorDialog_) {
        colorDialog_ = new QColorDialog(this);
#ifdef __APPLE__
        // hide the dialog, due to some Mac issues (OSX Bug workaround)
        colorDialog_->hide();
#endif  // __APPLE__

        colorDialog_->setAttribute(Qt::WA_DeleteOnClose, false);
        colorDialog_->setOption(QColorDialog::ShowAlphaChannel, util::extent<T>::value == 4);
        colorDialog_->setOption(QColorDialog::NoButtons, true);
        colorDialog_->setWindowModality(Qt::NonModal);
        QObject::connect(colorDialog_, &QColorDialog::currentColorChanged, this,
                         &ColorPropertyWidgetQt<T>::setPropertyValue);

        offsetColorDialog();
    }
}

template <typename T>
void ColorPropertyWidgetQt<T>::offsetColorDialog() {
    const static int rightOffset = 200;
    const static int topOffset = 100;
    const static int bottomOffset = 100;
    const static int leftOffset = 200;
    static int offsetX = 0;
    static int offsetY = 0;

    auto mainWindow = utilqt::getApplicationMainWindow();
    QRect mainFrame;
    if (mainWindow) {
        mainFrame = mainWindow->frameGeometry();
    }

    int x = mainFrame.left() + rightOffset;
    int y = mainFrame.top() + topOffset;
    colorDialog_->move(x + offsetX, y + offsetY);
    offsetX += 150;
    offsetY += 50;
    offsetX %= std::max(300, mainFrame.width() - rightOffset - leftOffset - 538);
    offsetY %= std::max(300, mainFrame.height() - topOffset - bottomOffset - 418);
}

namespace detail {

template <typename T>
struct ColorConverter {};

template <>
struct ColorConverter<ivec3> {
    static QColor toQColor(const ivec3& color) {
        auto c = glm::clamp(color, ivec3(0), ivec3(255));
        return QColor(c.r, c.g, c.b);
    }
    static ivec3 toGLM(const QColor& color) {
        return ivec3(color.red(), color.green(), color.blue());
    }
};
template <>
struct ColorConverter<ivec4> {
    static QColor toQColor(const ivec4& color) {
        auto c = glm::clamp(color, ivec4(0), ivec4(255));
        return QColor(c.r, c.g, c.b, c.a);
    }
    static ivec4 toGLM(const QColor& color) {
        return ivec4(color.red(), color.green(), color.blue(), color.alpha());
    }
};
template <>
struct ColorConverter<vec3> {
    static QColor toQColor(const vec3& color) {
        auto c = ivec3{255.0f * glm::clamp(color, vec3(0.0), vec3(1.0))};
        return QColor(c.r, c.g, c.b);
    }
    static vec3 toGLM(const QColor& color) {
        return vec3(color.red(), color.green(), color.blue()) / 255.0f;
    }
};
template <>
struct ColorConverter<vec4> {
    static QColor toQColor(const vec4& color) {
        auto c = ivec4{255.0f * glm::clamp(color, vec4(0.0), vec4(1.0))};
        return QColor(c.r, c.g, c.b, c.a);
    }
    static vec4 toGLM(const QColor& color) {
        return vec4(color.red(), color.green(), color.blue(), color.alpha()) / 255.0f;
    }
};
template <>
struct ColorConverter<dvec3> {
    static QColor toQColor(const dvec3& color) {
        auto c = ivec3{255.0 * glm::clamp(color, dvec3(0.0), dvec3(1.0))};
        return QColor(c.r, c.g, c.b);
    }
    static vec3 toGLM(const QColor& color) {
        return dvec3(color.red(), color.green(), color.blue()) / 255.0;
    }
};
template <>
struct ColorConverter<dvec4> {
    static QColor toQColor(const dvec4& color) {
        auto c = ivec4{255.0 * glm::clamp(color, dvec4(0.0), dvec4(1.0))};
        return QColor(c.r, c.g, c.b, c.a);
    }
    static vec4 toGLM(const QColor& color) {
        return dvec4(color.red(), color.green(), color.blue(), color.alpha()) / 255.0;
    }
};
}  // namespace detail

template <typename T>
void ColorPropertyWidgetQt<T>::updateFromProperty() {
    currentColor_ = detail::ColorConverter<T>::toQColor(ordinalProperty_->get());
    updateButton();

    auto rep = static_cast<ColorLineEdit::ColorRepresentation>(
        ordinalProperty_->template getMetaData<IntMetaData>(
            "representation", static_cast<int>(util::DefaultColorRepresentation<T>::value)));

    colorLineEdit_->setColor(ordinalProperty_->get(), rep);

    if (colorDialog_) {
        QSignalBlocker block{colorDialog_};
        colorDialog_->setWindowTitle(QString::fromStdString(property_->getDisplayName().c_str()));
        colorDialog_->setCurrentColor(currentColor_);
    }
}

template <typename T>
const QColor& ColorPropertyWidgetQt<T>::getCurrentColor() const {
    return currentColor_;
}

template <typename T>
std::unique_ptr<QMenu> ColorPropertyWidgetQt<T>::getContextMenu() {
    auto menu = PropertyWidgetQt::getContextMenu();

    auto representationMenu = menu->addMenu("&Representation");

    auto rep = static_cast<ColorLineEdit::ColorRepresentation>(
        ordinalProperty_->template getMetaData<IntMetaData>(
            "representation", static_cast<int>(util::DefaultColorRepresentation<T>::value)));

    auto actionGroup = new QActionGroup(menu.get());

    auto createAction = [&](const QString& title, ColorLineEdit::ColorRepresentation colorRep) {
        auto action = representationMenu->addAction(title);
        action->setCheckable(true);
        action->setChecked(colorRep == rep);
        connect(action, &QAction::triggered, this, [this, colorRep]() {
            ordinalProperty_->template setMetaData<IntMetaData>("representation",
                                                                static_cast<int>(colorRep));
            colorLineEdit_->setRepresentation(colorRep);
        });
        actionGroup->addAction(action);
    };

    createAction("&Floating Point [0,1]", ColorLineEdit::ColorRepresentation::FloatingPoint);
    createAction("&Integer [0,255]", ColorLineEdit::ColorRepresentation::Integer);
    createAction("&Hex Color Code", ColorLineEdit::ColorRepresentation::Hexadecimal);

    return menu;
}

template <typename T>
void ColorPropertyWidgetQt<T>::setPropertyValue() {
    if (!colorDialog_) return;

    currentColor_ = colorDialog_->currentColor();
    ordinalProperty_->set(detail::ColorConverter<T>::toGLM(currentColor_));
    updateButton();
}

template <typename T>
void ColorPropertyWidgetQt<T>::updateButton() {
    QColor topColor = currentColor_.lighter();
    QColor bottomColor = currentColor_.darker();

    btnColor_->setStyleSheet(QString("QToolButton {"
                                     "    border: 1px solid transparent;"
                                     "    background-color: %1;"
                                     "    border-radius: 3px;"
                                     "    width: 4ex;"
                                     "    height: 4ex;"
                                     "}"
                                     "QToolButton:hover {"
                                     "    border: 1px solid #268bd2;"
                                     "    background-color: qlineargradient(x1:0, \
                                     y1:0, x2:0, y2:1, stop:0 %2, stop:0.1 %1, stop:0.9 %1, \
                                     stop:1 %3);"
                                     "}"
                                     "QToolButton:pressed {"
                                     "    border: 1px solid #268bd2;"
                                     "    background-color: qlineargradient(x1:0, y1:0, x2:0, \
                                     y2:1, stop:1 %2, stop:0.9 %1, stop:0.1 %1, stop:0 %3);"
                                     "}")
                                 .arg(currentColor_.name())
                                 .arg(topColor.name())
                                 .arg(bottomColor.name()));
}

template <typename T>
void ColorPropertyWidgetQt<T>::openColorDialog() {
    createColorDialog();

#ifdef __APPLE__
    colorDialog_->hide();  // OSX Bug workaround
#endif                     // __APPLE__
    updateFromProperty();
    colorDialog_->show();
}

}  // namespace inviwo

#endif  // IVW_COLORPROPERTYWIDGETQT_H
