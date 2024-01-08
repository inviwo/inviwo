/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2024 Inviwo Foundation
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

#include <inviwo/core/util/glmconvert.h>  // for glm_convert
#include <inviwo/core/util/glmvec.h>      // for dvec4, ivec3, ivec4, dvec3, vec3, vec4

#include <QLineEdit>     // for QLineEdit
#include <QObject>       // for Q_OBJECT, signals, slots
#include <glm/vec4.hpp>  // for operator*

class QEvent;
class QFocusEvent;
class QKeyEvent;
class QWidget;

class QRegularExpressionValidator;

namespace inviwo {

class IVW_MODULE_QTWIDGETS_API ColorLineEdit : public QLineEdit {
    Q_OBJECT
public:
    enum class ColorRepresentation { Integer, FloatingPoint, Hexadecimal };

    ColorLineEdit(QWidget* parent = nullptr);
    virtual ~ColorLineEdit() = default;

    void setColor(ivec3 v, ColorRepresentation rep = ColorRepresentation::Integer);
    void setColor(ivec4 v, ColorRepresentation rep = ColorRepresentation::Integer);
    void setColor(vec3 v, ColorRepresentation rep = ColorRepresentation::FloatingPoint);
    void setColor(vec4 v, ColorRepresentation rep = ColorRepresentation::FloatingPoint);
    void setColor(dvec3 v, ColorRepresentation rep = ColorRepresentation::FloatingPoint);
    void setColor(dvec4 v, ColorRepresentation rep = ColorRepresentation::FloatingPoint);

    template <typename T>
    T getColor() const;

    dvec4 getColor() const;
    bool hasAlpha() const;

    void setRepresentation(ColorRepresentation rep);
    ColorRepresentation getRepresentation() const;

    bool isValid() const;

signals:
    void colorChanged();

public slots:
    void setInvalid(bool invalid = true);

protected:
    virtual void changeEvent(QEvent* event) override;  // use to change validator
    virtual void focusInEvent(QFocusEvent* event) override;
    virtual void focusOutEvent(QFocusEvent* event) override;
    virtual void keyPressEvent(QKeyEvent* event) override;

private:
    void updateText();
    void updateColor();

    void updateRegExp();

    QRegularExpressionValidator* validator_;

    ColorRepresentation representation_ = ColorRepresentation::FloatingPoint;
    dvec4 color_ = dvec4(0.0);
    bool hasAlpha_ = true;

    bool invalid_ = false;
};

template <typename T>
T ColorLineEdit::getColor() const {
    if constexpr (std::is_floating_point_v<typename T::value_type>) {
        return util::glm_convert<T>(color_);
    } else {
        return util::glm_convert<T>(color_ * 255.0);
    }
}

namespace util {

template <typename T>
struct DefaultColorRepresentation {
    static constexpr ColorLineEdit::ColorRepresentation value =
        ColorLineEdit::ColorRepresentation::FloatingPoint;
};

template <>
struct DefaultColorRepresentation<ivec3> {
    static constexpr ColorLineEdit::ColorRepresentation value =
        ColorLineEdit::ColorRepresentation::Integer;
};
template <>
struct DefaultColorRepresentation<ivec4> {
    static constexpr ColorLineEdit::ColorRepresentation value =
        ColorLineEdit::ColorRepresentation::Integer;
};

}  // namespace util

}  // namespace inviwo
