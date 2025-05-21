/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2025 Inviwo Foundation
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
#include <modules/qtwidgets/ordinalbasewidget.h>
#include <modules/qtwidgets/inviwoqtutils.h>

#include <optional>
#include <limits>
#include <tuple>

#include <QLineEdit>
#include <QByteArray>

class QEvent;
class QMouseEvent;
class QPaintEvent;

namespace inviwo {

struct IVW_MODULE_QTWIDGETS_API NumberWidgetConfig {
    enum Interaction : std::uint8_t { NoDragging, Dragging };

    std::optional<Interaction> interaction = std::nullopt;
    std::optional<std::string_view> prefix = std::nullopt;
    std::optional<std::string_view> postfix = std::nullopt;
    std::optional<bool> barVisible = std::nullopt;
    std::optional<bool> wrapping = std::nullopt;

    static constexpr auto defaultInteraction = Interaction::Dragging;
    static constexpr std::string_view defaultPrefix{};
    static constexpr std::string_view defaultPostfix{};
    static constexpr bool defaultBarVisible = true;
    static constexpr bool defaultWrapping = false;

    friend bool operator==(const NumberWidgetConfig&, const NumberWidgetConfig&) = default;
};

class IVW_MODULE_QTWIDGETS_API BaseNumberWidget : public QLineEdit {
    Q_OBJECT
public:
    enum class PercentageBar : std::uint8_t { Invalid, Regular, Symmetric };

    explicit BaseNumberWidget(QWidget* parent = nullptr);
    explicit BaseNumberWidget(const NumberWidgetConfig& config, QWidget* parent = nullptr);
    BaseNumberWidget(const BaseNumberWidget&) = delete;
    BaseNumberWidget(BaseNumberWidget&&) = delete;
    BaseNumberWidget& operator=(const BaseNumberWidget&) = delete;
    BaseNumberWidget& operator=(BaseNumberWidget&&) = delete;
    virtual ~BaseNumberWidget() = default;

    virtual QSize sizeHint() const override;

    virtual QSize minimumSizeHint() const override;

    void setPrefix(std::string_view prefix);
    const QString& getPrefix() const;
    void setPostfix(std::string_view postfix);
    const QString& getPostfix() const;

    void setWrapping(bool wrapping);
    bool getWrapping() const;

    void setPercentageBarVisibility(bool visible);
    bool getPercentageBarVisibility() const;

    void setInteractionMode(NumberWidgetConfig::Interaction mode);
    NumberWidgetConfig::Interaction getInteractionMode() const;

    void updateText();

    virtual bool event(QEvent* event) override;

signals:
    void valueChanged();

protected:
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void paintEvent(QPaintEvent* event) override;
    virtual void changeEvent(QEvent* event) override;

    virtual bool incrementValue() = 0;
    virtual bool decrementValue() = 0;
    virtual bool applyDragDelta(double deltaSteps) = 0;
    virtual void initDragValue() = 0;
    virtual bool valueFromTextValid(const QString& str) = 0;
    virtual bool updateValueFromText(const QString& str) = 0;
    virtual QString getTextFromValue(bool precise) const = 0;
    virtual std::tuple<std::optional<double>, PercentageBar> getPercentageBar() const = 0;

private:
    enum class FocusAction : std::uint8_t { SetFocus, ClearFocus };
    enum class HoverState : std::uint8_t { Invalid, Center, NegativeInc, PositiveInc };

    QSize calcMinimumSize() const;
    void drawPercentageBar(QPainter& painter, const QRect& rect, bool hover) const;
    QString getPrefixedText() const;
    void updateState(FocusAction action);
    HoverState getHoverState(QPoint mousepos) const;
    void updateHoverState(QPoint mousepos);

    struct InteractionState {
        QPoint previousPos{};
        Qt::KeyboardModifiers modifiers;
        bool dragging = false;
        HoverState hover = HoverState::Invalid;
    };

    bool wrapping_;
    QString prefix_;
    QString postfix_;
    InteractionState state_;
    NumberWidgetConfig::Interaction mode_;
    bool percentageBarVisible_;
    mutable QSize cachedMinimumSizeHint_;
    int minimumWidth_;

    static constexpr Qt::KeyboardModifier increasedStepModifier = Qt::ControlModifier;
    static constexpr Qt::KeyboardModifier decreasedStepModifier = Qt::ShiftModifier;
    static constexpr double increasedStepSize = 10.0;
    static constexpr double decreasedStepSize = 0.1;
};

template <typename T>
class NumberWidget final : public BaseNumberWidget, public OrdinalBaseWidget<T> {
public:
    NumberWidget();
    explicit NumberWidget(const NumberWidgetConfig& config);
    NumberWidget(const NumberWidget&) = delete;
    NumberWidget(NumberWidget&&) = delete;
    NumberWidget& operator=(const NumberWidget&) = delete;
    NumberWidget& operator=(NumberWidget&&) = delete;
    virtual ~NumberWidget() = default;

    // Implements OrdinalBaseWidget
    virtual T getValue() const override;
    virtual void setValue(T value) override;
    virtual void initValue(T value) override;
    virtual void setMinValue(T minValue, ConstraintBehavior cb) override;
    virtual void setMaxValue(T maxValue, ConstraintBehavior cb) override;
    virtual void setIncrement(T increment) override;

protected:
    virtual bool incrementValue() override;
    virtual bool decrementValue() override;
    virtual bool applyDragDelta(double deltaSteps) override;
    virtual void initDragValue() override;
    virtual bool valueFromTextValid(const QString& str) override;
    virtual bool updateValueFromText(const QString& str) override;
    virtual QString getTextFromValue(bool precise) const override;
    virtual std::tuple<std::optional<double>, PercentageBar> getPercentageBar() const override;

    bool updateValue(T value);
    double getUIIncrement() const;

private:
    T value_;
    T minValue_;
    T maxValue_;
    T increment_;
    T initialDragValue_;
    ConstraintBehavior minCB_;
    ConstraintBehavior maxCB_;
};

template <typename T>
inline NumberWidget<T>::NumberWidget()
    : OrdinalBaseWidget<T>{}
    , value_{1}
    , minValue_{0}
    , maxValue_{2}
    , increment_{1}
    , initialDragValue_{0}
    , minCB_{ConstraintBehavior::Editable}
    , maxCB_{ConstraintBehavior::Editable} {}

template <typename T>
inline NumberWidget<T>::NumberWidget(const NumberWidgetConfig& config)
    : BaseNumberWidget{config}
    , OrdinalBaseWidget<T>{}
    , value_{1}
    , minValue_{0}
    , maxValue_{2}
    , increment_{1}
    , initialDragValue_{0}
    , minCB_{ConstraintBehavior::Editable}
    , maxCB_{ConstraintBehavior::Editable} {}

template <typename T>
T NumberWidget<T>::getValue() const {
    return value_;
}
template <typename T>
void NumberWidget<T>::setValue(T value) {
    if (value != value_) {
        value_ = value;
        updateText();
        emit valueChanged();
    }
}
template <typename T>
void NumberWidget<T>::initValue(T value) {
    value_ = value;
    updateText();
}
template <typename T>
void NumberWidget<T>::setMinValue(T minValue, ConstraintBehavior cb) {
    minValue_ = minValue;
    minCB_ = cb;
}
template <typename T>
void NumberWidget<T>::setMaxValue(T maxValue, ConstraintBehavior cb) {
    maxValue_ = maxValue;
    maxCB_ = cb;
}
template <typename T>
void NumberWidget<T>::setIncrement(T increment) {
    increment_ = increment;
}

template <typename T>
bool NumberWidget<T>::incrementValue() {
    const auto uiIncrement = static_cast<T>(getUIIncrement());
    if (maxValue_ - uiIncrement < value_) {
        if (getWrapping()) {
            const T delta = maxValue_ - value_;
            return updateValue(minValue_ + uiIncrement - delta);
        } else {
            return updateValue(maxValue_);
        }
    } else {
        return updateValue(value_ + uiIncrement);
    }
}

template <typename T>
bool NumberWidget<T>::decrementValue() {
    const auto uiIncrement = static_cast<T>(getUIIncrement());
    if (minValue_ + uiIncrement > value_) {
        if (getWrapping()) {
            return updateValue(maxValue_ - uiIncrement + value_ - minValue_);
        } else {
            return updateValue(minValue_);
        }
    } else {
        return updateValue(value_ - uiIncrement);
    }
}

namespace detail {

template <typename T>
T upperBoundedValue(T v, T absDelta, T maxValue, ConstraintBehavior cb) {
    // precondition: delta > 0
    if (cb == ConstraintBehavior::Ignore) {
        if (std::numeric_limits<T>::max() - absDelta < v) {  // check for type overflow
            return std::numeric_limits<T>::max();
        } else {
            return v + absDelta;
        }
    } else if (absDelta > maxValue || maxValue - absDelta < v) {  // check for maxValue overflow
        return maxValue;
    } else {
        return v + absDelta;
    }
}

template <typename T>
T lowerBoundedValue(T v, T absDelta, T minValue, ConstraintBehavior cb) {
    // precondition: delta < 0
    if (cb == ConstraintBehavior::Ignore) {
        if (std::numeric_limits<T>::lowest() + absDelta > v) {  // check for type underflow
            return std::numeric_limits<T>::lowest();
        } else {
            return v - absDelta;
        }
    } else if (minValue + absDelta > v) {  // check for minValue underflow
        return minValue;
    } else {
        return v - absDelta;
    }
}

template <typename T>
T clamp(T v, T minValue, T maxValue, ConstraintBehavior minCB, ConstraintBehavior maxCB) {
    if (minCB != ConstraintBehavior::Ignore && maxCB != ConstraintBehavior::Ignore) {
        return std::clamp(v, minValue, maxValue);
    } else if (minCB != ConstraintBehavior::Ignore) {
        return std::max(v, minValue);
    } else if (maxCB != ConstraintBehavior::Ignore) {
        return std::min(v, maxValue);
    } else {
        return v;
    }
}

}  // namespace detail

template <typename T>
bool NumberWidget<T>::applyDragDelta(double deltaSteps) {
    if (deltaSteps == 0.0) {
        return false;
    }

    const bool positiveDelta = !std::signbit(deltaSteps);

    auto absDelta = static_cast<T>(std::abs(deltaSteps * getUIIncrement()));
    if (getWrapping()) {
        if constexpr (std::is_floating_point_v<T>) {
            absDelta = std::fmod(absDelta, maxValue_ - minValue_);
        } else {
            absDelta = absDelta % (maxValue_ - minValue_);
        }
        if (positiveDelta && maxValue_ - absDelta < initialDragValue_) {
            absDelta -= maxValue_ - minValue_;
        } else if (!positiveDelta && minValue_ + absDelta > initialDragValue_) {
            absDelta = maxValue_ - minValue_ - absDelta;
        }
        if (positiveDelta) {
            return updateValue(initialDragValue_ + absDelta);
        } else {
            return updateValue(initialDragValue_ - absDelta);
        }
    }

    if (positiveDelta) {
        return updateValue(
            detail::upperBoundedValue(initialDragValue_, absDelta, maxValue_, maxCB_));
    } else {
        return updateValue(
            detail::lowerBoundedValue(initialDragValue_, absDelta, minValue_, minCB_));
    }
}

template <typename T>
void NumberWidget<T>::initDragValue() {
    // Need to keep track of the current value when mouse dragging starts to have an absolute delta.
    // Otherwise the incremental delta between two events might be too small for updating integer
    // values.
    initialDragValue_ = value_;
}

template <typename T>
bool NumberWidget<T>::valueFromTextValid(const QString& str) {
    if (auto newValue = utilqt::numericValueFromString<T>(str); newValue) {
        return *newValue == detail::clamp(*newValue, minValue_, maxValue_, minCB_, maxCB_);
    }
    return false;
}

template <typename T>
bool NumberWidget<T>::updateValueFromText(const QString& str) {
    if (auto newValue = utilqt::numericValueFromString<T>(str); newValue) {
        T clamped = detail::clamp(*newValue, minValue_, maxValue_, minCB_, maxCB_);
        bool updated = (clamped != value_);
        value_ = clamped;
        return updated;
    }
    return false;
}

template <typename T>
QString NumberWidget<T>::getTextFromValue(bool precise) const {
    if constexpr (std::is_floating_point_v<T>) {
        if (precise) {
            return utilqt::formatAsNonscientific(value_);
        } else {
            return QString::number(value_, 'f', utilqt::decimals(increment_));
        }
    } else {
        return QString::number(value_);
    }
}

template <typename T>
std::tuple<std::optional<double>, BaseNumberWidget::PercentageBar>
NumberWidget<T>::getPercentageBar() const {
    const auto state = [&]() {
        if (minValue_ == std::numeric_limits<T>::lowest() ||
            maxValue_ == std::numeric_limits<T>::max()) {
            return PercentageBar::Invalid;
        }
        if constexpr (std::is_floating_point_v<T> || std::is_signed_v<T>) {
            return (maxValue_ == -minValue_) ? PercentageBar::Symmetric : PercentageBar::Regular;
        } else {
            return PercentageBar::Regular;
        }
    }();

    std::optional<double> percentage;
    if (state == PercentageBar::Symmetric) {
        percentage = static_cast<double>(value_) / static_cast<double>(maxValue_);
    } else if (state == PercentageBar::Regular) {
        percentage =
            static_cast<double>(value_ - minValue_) / static_cast<double>(maxValue_ - minValue_);
    }
    return std::tuple{percentage, state};
}

template <typename T>
bool NumberWidget<T>::updateValue(T v) {
    if (value_ != v) {
        value_ = v;
        return true;
    }
    return false;
}

template <typename T>
double NumberWidget<T>::getUIIncrement() const {
    if (minValue_ == std::numeric_limits<T>::lowest() ||
        maxValue_ == std::numeric_limits<T>::max()) {
        return static_cast<double>(increment_);
    }
    const double increment =
        static_cast<double>(maxValue_ - minValue_) / static_cast<double>(width());
    if constexpr (std::is_integral_v<T>) {
        return std::max(increment, 1.0);
    } else {
        return increment;
    }
}

}  // namespace inviwo
