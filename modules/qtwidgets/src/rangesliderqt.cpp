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

#include <modules/qtwidgets/rangesliderqt.h>

#include <inviwo/core/util/stringconversion.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QToolTip>
#include <QResizeEvent>
#include <warn/pop>

namespace inviwo {

RangeSliderQt::RangeSliderQt(Qt::Orientation orientation, QWidget *parent, bool showTooltip)
    : QSplitter(orientation, parent)
    , range_{0, 10}
    , value_{0, 10}
    , minSeperation_(0)
    , formatTooltip_{[](int /*handle*/, int pos) { return toString(pos); }} {

    QFrame *left = new QFrame(this);
    QFrame *middle = new QFrame(this);
    QFrame *right = new QFrame(this);

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
    setMinSeparation(minSeperation_);
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

int RangeSliderQt::minSeperation() const { return minSeperation_; }

void RangeSliderQt::setValue(int minVal, int maxVal) {
    setMinValue(minVal);
    setMaxValue(maxVal);
}

void RangeSliderQt::setMinValue(int minVal) {
    if (value_.x != minVal) {
        value_.x = minVal;

        if (value_.x > range_.y - minSeperation_) {
            value_.x = range_.y - minSeperation_;
        }

        if (value_.y - value_.x < minSeperation_) {
            value_.y = value_.x + minSeperation_;
        }

        updateSlidersFromState();
    }
}

void RangeSliderQt::setMaxValue(int maxVal) {
    if (value_.y != maxVal) {
        value_.y = maxVal;
        if (value_.y < range_.x + minSeperation_) {
            value_.y = range_.x + minSeperation_;
        }

        if (value_.y - value_.x < minSeperation_) {
            value_.x = value_.y - minSeperation_;
        }

        updateSlidersFromState();
    }
}

void RangeSliderQt::setRange(int minR, int maxR) {
    setMinRange(minR);
    setMaxRange(maxR);
}

void RangeSliderQt::setMinSeparation(int sep) {
    minSeperation_ = sep;
    QList<int> sizes = QSplitter::sizes();
    int range = sizes[0] + sizes[1] + sizes[2];

    int size = range_.y - range_.x;
    if (size <= 0) {
        return;
    }

    if (orientation() == Qt::Horizontal) {
        widget(1)->setMinimumWidth(range * minSeperation_ / size);
    } else {
        widget(1)->setMinimumHeight(range * minSeperation_ / size);
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

void RangeSliderQt::resizeEvent(QResizeEvent *event) {
    QSplitter::resizeEvent(event);
    setMinSeparation(minSeperation_);
}

void RangeSliderQt::updateStateFromSliders() {
    QList<int> sizes = QSplitter::sizes();
    int range = sizes[0] + sizes[1] + sizes[2];

    int pos1 = sizes[0];
    int pos2 = sizes[0] + sizes[1];

    value_.x = range_.x + (range_.y - range_.x) * pos1 / range;
    value_.y = range_.x + (range_.y - range_.x) * pos2 / range;
}

void RangeSliderQt::updateSlidersFromState() {
    QList<int> sizes = QSplitter::sizes();
    int range = sizes[0] + sizes[1] + sizes[2];

    int size = range_.y - range_.x;
    if (size <= 0) {
        return;
    }

    sizes.clear();
    sizes.append((range * (value_.x - range_.x)) / size);
    sizes.append((range * (value_.y - value_.x)) / size);
    sizes.append((range * (range_.y - value_.y)) / size);

    QSignalBlocker block(this);
    setSizes(sizes);
}

void RangeSliderQt::updateSplitterPosition(int /*pos*/, int /*idx*/) {
    updateStateFromSliders();

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
    updateSplitterPosition(0, 0);
    parentWidget()->repaint();
    repaint();
}

bool RangeSliderQt::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::ToolTip) {
        QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);
        if (obj == handle(1)) {
            QToolTip::showText(helpEvent->globalPos(),
                               QString::fromStdString(formatTooltip_(0, value_.x)));
            return true;
        } else if (obj == handle(2)) {
            QToolTip::showText(helpEvent->globalPos(),
                               QString::fromStdString(formatTooltip_(1, value_.y)));
            return true;
        }

    } else if (obj == widget(1) && event->type() == QEvent::MouseButtonPress && isEnabled()) {
        QMouseEvent *me = static_cast<QMouseEvent *>(event);
        if (me->button() == Qt::LeftButton) {
            lastPos_ = orientation() == Qt::Horizontal ? me->globalX() : me->globalY();
            return true;
        }
    } else if (obj == widget(1) && event->type() == QEvent::MouseMove && isEnabled()) {
        QMouseEvent *me = static_cast<QMouseEvent *>(event);
        if (me->buttons().testFlag(Qt::LeftButton)) {
            const int newPos = orientation() == Qt::Horizontal ? me->globalX() : me->globalY();
            const int delta = newPos - lastPos_;
            lastPos_ = newPos;
            if (delta != 0) {
                moveMiddle(delta);
            }
            me->accept();
            return true;
        }
    } else if (obj == widget(0) && event->type() == QEvent::MouseButtonRelease && isEnabled()) {
        QMouseEvent *me = static_cast<QMouseEvent *>(event);
        if (me->button() == Qt::LeftButton) {
            QList<int> sizes = QSplitter::sizes();
            QList<int> newSizes{me->pos().x(), sizes[1] + (sizes[0] - me->pos().x()), sizes[2]};
            {
                QSignalBlocker block(this);
                QSplitter::setSizes(newSizes);
            }
            // update internal state
            updateSplitterPosition(0, 0);
            me->accept();
            return true;
        }
    } else if (obj == widget(2) && event->type() == QEvent::MouseButtonRelease && isEnabled()) {
        QMouseEvent *me = static_cast<QMouseEvent *>(event);
        if (me->button() == Qt::LeftButton) {
            QList<int> sizes = QSplitter::sizes();
            QList<int> newSizes{sizes[0], sizes[1] + me->pos().x(), sizes[2] - me->pos().x()};
            {
                QSignalBlocker block(this);
                QSplitter::setSizes(newSizes);
            }
            // update internal state
            updateSplitterPosition(0, 0);
            me->accept();
            return true;
        }
    }
    return QObject::eventFilter(obj, event);
}

void RangeSliderQt::setTooltipFormat(std::function<std::string(int, int)> formater) {
    formatTooltip_ = formater;
}

}  // namespace inviwo
