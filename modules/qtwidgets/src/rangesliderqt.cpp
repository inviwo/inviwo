/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2024 Inviwo Foundation
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

#include <modules/qtwidgets/rangesliderqt.h>

#include <inviwo/core/util/glmvec.h>            // for ivec2
#include <inviwo/core/util/stringconversion.h>  // for toString

#include <algorithm>  // for min
#include <cmath>

#include <QEvent>           // for QEvent, QEvent::MouseButtonRelease, QEven...
#include <QFrame>           // for QFrame
#include <QHelpEvent>       // for QHelpEvent
#include <QList>            // for QList
#include <QMouseEvent>      // for QMouseEvent
#include <QPoint>           // for QPoint
#include <QSignalBlocker>   // for QSignalBlocker
#include <QSizePolicy>      // for QSizePolicy, QSizePolicy::Expanding, QSiz...
#include <QSplitterHandle>  // for QSplitterHandle
#include <QString>          // for QString
#include <QToolTip>         // for QToolTip
#include <QWidget>          // for QWidget
#include <glm/vec2.hpp>     // for vec<>::(anonymous)

class QHelpEvent;
class QMouseEvent;

namespace inviwo {

RangeSliderQt::RangeSliderQt(Qt::Orientation orientation, QWidget* parent, bool showTooltip)
    : QSplitter(orientation, parent)
    , range_{0, 10}
    , value_{0, 10}
    , minSeparation_(0)
    , formatTooltip_{[](int /*handle*/, int pos) { return toString(pos); }} {

    QFrame* left = new QFrame(this);
    QFrame* middle = new QFrame(this);
    QFrame* right = new QFrame(this);

    addWidget(left);
    addWidget(middle);
    addWidget(right);

    // enable QSplitter:hover stylesheet
    // QTBUG-13768 https://bugreports.qt.io/browse/QTBUG-13768
    handle(1)->setAttribute(Qt::WA_Hover);
    handle(2)->setAttribute(Qt::WA_Hover);

    setObjectName("RangeSliderQt");
    setProperty("Vertical", orientation == Qt::Vertical);
    left->setProperty("LeftPart", true);
    left->installEventFilter(this);
    right->setProperty("LeftPart", false);
    right->installEventFilter(this);
    middle->setObjectName("valueArea");
    middle->installEventFilter(this);

    if (orientation == Qt::Horizontal) {
        left->setMinimumWidth(0);
        right->setMinimumWidth(0);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    } else {
        setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);
        left->setMinimumHeight(0);
        right->setMinimumHeight(0);
    }
    setMinSeparation(minSeparation_);
    QSplitter::setChildrenCollapsible(false);
    connect(this, &RangeSliderQt::splitterMoved, this, &RangeSliderQt::updateSplitterPosition);
    updateSlidersFromState();

    if (showTooltip) {
        handle(1)->installEventFilter(this);
        handle(2)->installEventFilter(this);
    }
}

int RangeSliderQt::minValue() const { return value_.x; }

int RangeSliderQt::maxValue() const { return value_.y; }

int RangeSliderQt::minRange() const { return range_.x; }

int RangeSliderQt::maxRange() const { return range_.y; }

int RangeSliderQt::minSeparation() const { return minSeparation_; }

void RangeSliderQt::setValue(int minVal, int maxVal) {
    setMinValue(minVal);
    setMaxValue(maxVal);
}

void RangeSliderQt::setMinValue(int minVal) {
    if (value_.x != minVal) {
        value_.x = minVal;

        if (value_.x > range_.y - minSeparation_) {
            value_.x = range_.y - minSeparation_;
        }

        if (value_.y - value_.x < minSeparation_) {
            value_.y = value_.x + minSeparation_;
        }

        updateSlidersFromState();
    }
}

void RangeSliderQt::setMaxValue(int maxVal) {
    if (value_.y != maxVal) {
        value_.y = maxVal;
        if (value_.y < range_.x + minSeparation_) {
            value_.y = range_.x + minSeparation_;
        }

        if (value_.y - value_.x < minSeparation_) {
            value_.x = value_.y - minSeparation_;
        }

        updateSlidersFromState();
    }
}

void RangeSliderQt::setRange(int minR, int maxR) {
    setMinRange(minR);
    setMaxRange(maxR);
}

void RangeSliderQt::setMinSeparation(int sep) {
    minSeparation_ = sep;
    const int totalRange = getTotalRange();
    const int rangeDelta = range_.y - range_.x;

    if (rangeDelta <= 0) {
        return;
    }

    const int minWidth = totalRange * minSeparation_ / rangeDelta - handle(1)->width();

    if (orientation() == Qt::Horizontal) {
        widget(1)->setMinimumWidth(minWidth);
    } else {
        widget(1)->setMinimumHeight(minWidth);
    }
}

void RangeSliderQt::setMinRange(int minR) {
    if (range_.x != minR) {
        range_.x = minR;
        // ensure that the values are within the updated range
        if (value_.x < range_.x) {
            // updateSlidersFromState() is called in setMinValue
            setMinValue(range_.x);
        } else {
            updateSlidersFromState();
        }
    }
}

void RangeSliderQt::setMaxRange(int maxR) {
    if (range_.y != maxR) {
        range_.y = maxR;
        // ensure that the values are within the updated range
        if (value_.y > range_.y) {
            // updateSlidersFromState() is called in setMinValue
            setMaxValue(range_.y);
        } else {
            updateSlidersFromState();
        }
    }
}

void RangeSliderQt::resizeEvent(QResizeEvent* event) {
    QSplitter::resizeEvent(event);
    setMinSeparation(minSeparation_);
    updateSlidersFromState();
}

int RangeSliderQt::getTotalRange() const {
    const QList<int> sizes = QSplitter::sizes();
    return sizes[0] + sizes[1] + sizes[2] + handle(1)->width();
}

void RangeSliderQt::updateStateFromSliders() {
    const int totalRange = getTotalRange();
    const double rangeDelta = range_.y - range_.x;

    QList<int> sizes = QSplitter::sizes();
    if (totalRange <= 0) {
        // invisible
        return;
    }

    value_.x = range_.x + static_cast<int>(std::lround(sizes[0] * rangeDelta / totalRange));
    value_.y = range_.x + static_cast<int>(std::lround((sizes[0] + sizes[1] + handle(1)->width()) *
                                                       rangeDelta / totalRange));
}

void RangeSliderQt::updateSlidersFromState() {
    const int totalRange = getTotalRange();

    const double rangeDelta = range_.y - range_.x;
    if (rangeDelta <= 0.0) {
        return;
    }

    const QList sizes({
        static_cast<int>(std::lround((totalRange * (value_.x - range_.x)) / rangeDelta)),
        static_cast<int>(std::lround((totalRange * (value_.y - value_.x)) / rangeDelta)) -
            handle(1)->width(),
        static_cast<int>(std::lround((totalRange * (range_.y - value_.y)) / rangeDelta)),
    });

    QSignalBlocker block(this);
    setSizes(sizes);
}

void RangeSliderQt::updateSplitterPosition(int /*pos*/, int /*idx*/) {
    updateStateFromSliders();

    // snap splitter positions to internal state since the slider resolution might be higher
    // than the property datatype or increment permits
    updateSlidersFromState();

    // Emit
    emit valuesChanged(value_.x, value_.y);
}

void RangeSliderQt::moveMiddle(int delta) {
    QList<int> sizes = QSplitter::sizes();
    // ensure not to move middle part further than min/max values
    if (delta > 0) {
        delta = std::min(delta, sizes[2]);
    } else {
        delta = -std::min(-delta, sizes[0]);
    }
    if (delta == 0) {
        return;
    }
    // adjust sizes and modify splitter positions
    sizes[0] += delta;
    sizes[2] -= delta;

    {
        QSignalBlocker block(this);
        QSplitter::setSizes(sizes);
    }
    // update internal state
    updateStateFromSliders();
    emit valuesChanged(value_.x, value_.y);

    parentWidget()->repaint();
    repaint();
}

bool RangeSliderQt::eventFilter(QObject* obj, QEvent* event) {
    if (event->type() == QEvent::ToolTip) {
        const auto* helpEvent = dynamic_cast<QHelpEvent*>(event);
        if (obj == handle(1)) {
            QToolTip::showText(helpEvent->globalPos(),
                               QString::fromStdString(formatTooltip_(0, value_.x)));
            return true;
        } else if (obj == handle(2)) {
            QToolTip::showText(helpEvent->globalPos(),
                               QString::fromStdString(formatTooltip_(1, value_.y)));
            return true;
        }
    } else if (isEnabled()) {
        if (obj == widget(0)) {
            handleGrooveEvent(Groove::Left, event);
        } else if (obj == widget(1)) {
            handleCenterWidgetEvent(event);
        } else if (obj == widget(2)) {
            handleGrooveEvent(Groove::Right, event);
        }
        if (event->isAccepted()) {
            return true;
        }
    }
    return QObject::eventFilter(obj, event);
}

void RangeSliderQt::handleGrooveEvent(Groove groove, QEvent* event) {
    if (event->type() == QEvent::MouseButtonRelease) {
        if (const auto* me = dynamic_cast<QMouseEvent*>(event); me->button() == Qt::LeftButton) {

            auto newSizes = [&]() -> QList<int> {
                const auto sizes = QSplitter::sizes();
                if (groove == Groove::Left) {
                    return {me->pos().x(), sizes[1] + (sizes[0] - me->pos().x()), sizes[2]};
                } else {
                    return {sizes[0], sizes[1] + me->pos().x(), sizes[2] - me->pos().x()};
                }
            }();

            {
                QSignalBlocker block(this);
                QSplitter::setSizes(newSizes);
            }
            // update internal state
            updateSplitterPosition(0, static_cast<int>(groove));
            event->accept();
        }
    }
}

void RangeSliderQt::handleCenterWidgetEvent(QEvent* event) {
    switch (event->type()) {
        case QEvent::MouseButtonPress: {
            if (const auto* me = dynamic_cast<QMouseEvent*>(event);
                me->button() == Qt::LeftButton) {
                lastPos_ =
                    static_cast<int>(orientation() == Qt::Horizontal ? me->globalPosition().x()
                                                                     : me->globalPosition().y());
                event->accept();
            }
            break;
        }
        case QEvent::MouseButtonRelease:
            // snap splitter positions to internal state since the slider resolution might be higher
            // than the property datatype or increment permits
            updateSlidersFromState();
            event->accept();
            break;

        case QEvent::MouseMove: {
            if (const auto* me = dynamic_cast<QMouseEvent*>(event);
                me->buttons().testFlag(Qt::LeftButton)) {
                const auto newPos =
                    static_cast<int>(orientation() == Qt::Horizontal ? me->globalPosition().x()
                                                                     : me->globalPosition().y());
                const int delta = newPos - lastPos_;
                lastPos_ = newPos;
                if (delta != 0) {
                    moveMiddle(delta);
                }
                event->accept();
            }
            break;
        }
        default:
            break;
    }
}

void RangeSliderQt::setTooltipFormat(std::function<std::string(int, int)> formater) {
    formatTooltip_ = std::move(formater);
}

}  // namespace inviwo
