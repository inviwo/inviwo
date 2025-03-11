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

#include <modules/qtwidgets/numberwidget.h>
#include <modules/qtwidgets/inviwoqtutils.h>

#include <QGuiApplication>
#include <QPaintEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QStyleOption>
#include <QStyleOptionFrame>
#include <QPainter>
#include <QSvgRenderer>
#include <QFontMetrics>

namespace inviwo {

BaseNumberWidget::BaseNumberWidget(QWidget* parent) : BaseNumberWidget{{}, {}, parent} {}

BaseNumberWidget::BaseNumberWidget(std::string_view prefix, std::string_view postfix,
                                   QWidget* parent)
    : QLineEdit{parent}
    , wrapping_{false}
    , prefix_{utilqt::toQString(prefix)}
    , postfix_{utilqt::toQString(postfix)} {

    QSizePolicy sp = sizePolicy();
    sp.setHorizontalPolicy(QSizePolicy::Minimum);
    setSizePolicy(sp);

    updateState(FocusAction::ClearFocus);
    setFocusPolicy(Qt::TabFocus);
    setMouseTracking(true);

    QObject::connect(this, &QLineEdit::textEdited, this, [this](const QString& text) {
        if (valueFromTextValid(text)) {
            setProperty("invalid", QVariant{});
        } else {
            setProperty("invalid", true);
        }
        style()->unpolish(this);
        style()->polish(this);
    });
}

void BaseNumberWidget::setPrefix(std::string_view prefix) {
    prefix_ = utilqt::toQString(prefix);
    updateText();
}

const QString& BaseNumberWidget::getPrefix() const { return prefix_; }

void BaseNumberWidget::setPostfix(std::string_view postfix) {
    postfix_ = utilqt::toQString(postfix);
    updateText();
}

const QString& BaseNumberWidget::getPostfix() const { return postfix_; }

void BaseNumberWidget::setWrapping(bool wrapping) { wrapping_ = wrapping; }

bool BaseNumberWidget::getWrapping() const { return wrapping_; }

bool BaseNumberWidget::event(QEvent* event) {
    if (event->type() == QEvent::ReadOnlyChange) {
        updateHoverState(mapFromGlobal(QCursor::pos()));
        QSignalBlocker blocker{this};
        updateText();
    } else if (!isReadOnly() && isEnabled()) {
        if (event->type() == QEvent::FocusIn) {
            updateState(FocusAction::SetFocus);
        } else if (event->type() == QEvent::FocusOut) {
            if (updateValueFromText(text())) {
                emit valueChanged();
            }
            updateState(FocusAction::ClearFocus);
        } else if (event->type() == QEvent::KeyPress) {
            const auto* keyEvent = static_cast<QKeyEvent*>(event);

            if (keyEvent->key() == Qt::Key_Escape) {
                // cancel editing
                QSignalBlocker blocker{this};
                updateText();
                clearFocus();
                return true;
            } else if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
                // commit changes
                clearFocus();
                return true;
            }
        }
    }
    return QLineEdit::event(event);
}

void BaseNumberWidget::mousePressEvent(QMouseEvent* event) {
    if (!hasFocus() && !isReadOnly() && isEnabled() && event->button() == Qt::LeftButton) {
        state_.previousPos = event->pos();
        state_.dragging = false;
        event->accept();
    } else {
        QLineEdit::mousePressEvent(event);
    }
}

void BaseNumberWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (!hasFocus() && !isReadOnly() && isEnabled() && event->button() == Qt::LeftButton) {
        event->accept();
        if (!state_.dragging) {
            switch (state_.hover) {
                case HoverState::PositiveInc:
                    incrementValue();
                    emit valueChanged();
                    update();
                    break;
                case HoverState::NegativeInc:
                    decrementValue();
                    emit valueChanged();
                    update();
                    break;
                case HoverState::Center:
                default:
                    // mouse left click on center and no movement, start editing
                    setFocus(Qt::TabFocusReason);
                    break;
            }
        }
        state_.dragging = false;
    } else {
        QLineEdit::mouseReleaseEvent(event);
    }
}

void BaseNumberWidget::mouseMoveEvent(QMouseEvent* event) {
    updateHoverState(event->pos());

    // ignore dragging as long as movement delta is too small
    if (!hasFocus() && !isReadOnly() && isEnabled() && event->buttons().testFlag(Qt::LeftButton)) {
        event->accept();
        const int delta = (event->pos() - state_.previousPos).x();
        if (!state_.dragging) {
            const int minimalMovement = 3;
            state_.dragging = std::abs(delta) > minimalMovement;
            if (state_.dragging) {
                setCursor(Qt::SizeHorCursor);
                state_.hover = HoverState::Center;
                initDragValue();
            }
        } else {
            double deltaStep = static_cast<double>(delta);
            if (QGuiApplication::queryKeyboardModifiers().testFlag(increasedStepModifier)) {
                deltaStep *= increasedStepSize;
            } else if (QGuiApplication::queryKeyboardModifiers().testFlag(decreasedStepModifier)) {
                deltaStep *= decreasedStepSize;
            }
            applyDelta(deltaStep);
            emit valueChanged();
            update();
        }
    } else {
        QLineEdit::mouseMoveEvent(event);
    }
}

void BaseNumberWidget::paintEvent(QPaintEvent* event) {
    const int borderWidth = 1;
    const int incrementWidth = height() * 2 / 3;
    QStyleOptionFrame panel;
    initStyleOption(&panel);
    const bool hover = panel.state.testFlag(QStyle::State_MouseOver);

    if (!hasFocus() && !isReadOnly() && isEnabled()) {
        constexpr std::string_view arrowLeft{":/svgicons/arrow-left-enabled.svg"};
        constexpr std::string_view arrowRight{":/svgicons/arrow-right-enabled.svg"};

        QPainter painter{this};
        style()->drawPrimitive(QStyle::PE_PanelLineEdit, &panel, &painter, this);
        QRect r = style()->subElementRect(QStyle::SE_LineEditContents, &panel, this);
        painter.setPen(Qt::NoPen);
        painter.setClipRect(r);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

        QRect interiorRect{rect().adjusted(borderWidth, borderWidth, -borderWidth, -borderWidth)};

        // draw percentage bar in the center part
        if (auto percentage = getPercentage(); percentage.has_value()) {
            const QRect centerRect{QPoint{borderWidth, borderWidth},
                                   QPoint{static_cast<int>(interiorRect.width() * *percentage),
                                          height() - 2 * borderWidth}};
            const QBrush percentageBrush{hover ? "#1e70a8" : "#103a57"};
            painter.setBrush(percentageBrush);
            painter.drawRect(centerRect);
        }

        if (hover) {
            QRect arrowRect{interiorRect.topLeft(), QSize{incrementWidth, interiorRect.height()}};

            const QBrush bgActive{"#185987"};

            if (state_.hover == HoverState::NegativeInc) {
                painter.setBrush(bgActive);
                painter.drawRect(arrowRect);
            }

            QSvgRenderer renderer(utilqt::toQString(arrowLeft));
            renderer.setAspectRatioMode(Qt::KeepAspectRatio);
            renderer.render(&painter, arrowRect);

            arrowRect.moveRight(width() - borderWidth);
            if (state_.hover == HoverState::PositiveInc) {
                painter.setBrush(bgActive);
                painter.drawRect(arrowRect);
            }

            renderer.load(utilqt::toQString(arrowRight));
            renderer.setAspectRatioMode(Qt::KeepAspectRatio);
            renderer.render(&painter, arrowRect);
        }

        painter.setFont(font());
        painter.setPen(palette().color(QPalette::Text));
        painter.drawText(event->rect(), Qt::AlignCenter, getPrefixedText());
    } else {
        QLineEdit::paintEvent(event);
    }
}

void BaseNumberWidget::changeEvent(QEvent* event) {
    if (event->type() == QEvent::EnabledChange) {
        updateText();
    }
    QLineEdit::changeEvent(event);
}

void BaseNumberWidget::updateText() {
    const QString str = (isReadOnly() || !isEnabled()) ? getPrefixedText() : getTextFromValue(true);
    if (str != text()) {
        setText(str);
    }
}

QString BaseNumberWidget::getPrefixedText() const {
    return QString("%1%2%3%4")
        .arg(prefix_)
        .arg(prefix_.length() > 0 ? ": " : "")
        .arg(getTextFromValue(false))
        .arg(postfix_);
}

void BaseNumberWidget::updateState(FocusAction action) {
    if (action == FocusAction::SetFocus) {
        // update textual representation
        updateText();
    }
    setProperty("invalid", QVariant{});
    style()->unpolish(this);
    style()->polish(this);
    setAlignment(action == FocusAction::SetFocus ? Qt::AlignLeft : Qt::AlignCenter);

    updateHoverState(mapFromGlobal(QCursor::pos()));
}

auto BaseNumberWidget::getHoverState(QPoint mousepos) const -> HoverState {
    const int incrementWidth = height() * 2 / 3;

    if (hasFocus() || isReadOnly() || !isEnabled()) {
        return HoverState::Invalid;
    }
    if (mousepos.x() < incrementWidth) {
        return HoverState::NegativeInc;
    } else if (mousepos.x() > width() - incrementWidth) {
        return HoverState::PositiveInc;
    } else {
        return HoverState::Center;
    }
}

void BaseNumberWidget::updateHoverState(QPoint mousepos) {
    if (state_.dragging) {
        return;
    }

    if (auto hoverState = getHoverState(mousepos); hoverState != state_.hover) {
        state_.hover = hoverState;
        update();

        switch (state_.hover) {
            case HoverState::Invalid:
                setCursor(Qt::IBeamCursor);
                break;
            case HoverState::Center:
                setCursor(Qt::SizeHorCursor);
                break;
            case HoverState::NegativeInc:
            case HoverState::PositiveInc:
            default:
                unsetCursor();
                break;
        }
    }
}

}  // namespace inviwo
