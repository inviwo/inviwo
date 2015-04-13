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

#include <inviwo/qt/widgets/rangesliderqt.h>

namespace inviwo {


RangeSliderQt::RangeSliderQt(Qt::Orientation orientation, QWidget* parent) 
    : QSplitter(orientation, parent)
    , minSeperation_(0) {

    QFrame* left = new QFrame(this);
    middle_ = new RangeSliderMiddle(this);
    QFrame* right = new QFrame(this);
    addWidget(left);
    addWidget(middle_);
    addWidget(right);

    QObject::connect(middle_, SIGNAL(middleMoved(int)), this, SLOT(middleMoved(int)));

    this->setProperty("Vertical", orientation == Qt::Vertical);
    left->setProperty("LeftPart", true);
    right->setProperty("LeftPart", false);

    middle_->setObjectName("valueArea");
    range_[0] = 0;
    range_[1] = 10;
    value_[0] = 0;
    value_[1] = 10;

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
    connect(this, SIGNAL(splitterMoved(int, int)), this, SLOT(updateSplitterPosition(int, int)));
    updateSlidersFromState();
}

RangeSliderQt::~RangeSliderQt() {}

int RangeSliderQt::minValue() {
    return value_[0];
}

int RangeSliderQt::maxValue() {
    return value_[1];
}

int RangeSliderQt::minRange() {
    return range_[0];
}

int RangeSliderQt::maxRange() {
    return range_[1];
}

int RangeSliderQt::minSeperation() {
    return minSeperation_;
}

void RangeSliderQt::setValue(int minVal, int maxVal) {
    setMinValue(minVal);
    setMaxValue(maxVal);
}

void RangeSliderQt::setMinValue(int minVal) {
    if (value_[0] != minVal) {
        value_[0] = minVal;

        if (value_[0] > range_[1] - minSeperation_) {
            value_[0] = range_[1] - minSeperation_;
        }
        
        if (value_[1] - value_[0] < minSeperation_) {
            value_[1] = value_[0] + minSeperation_;
        }

        updateSlidersFromState();
    }
}

void RangeSliderQt::setMaxValue(int maxVal) {
    if (value_[1] != maxVal) {
        value_[1] = maxVal;
        if (value_[1] < range_[0] + minSeperation_) {
            value_[1] = range_[0] + minSeperation_;
        }

        if (value_[1] - value_[0] < minSeperation_) {
            value_[0] = value_[1] - minSeperation_;
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
    
    int size = range_[1] - range_[0];
    if (size <= 0) {
        return;
    }

    if (QSplitter::orientation() == Qt::Horizontal) {
        middle_->setMinimumWidth(range * minSeperation_/ size);
    } else {
        middle_->setMinimumHeight(range * minSeperation_ / size);
    }
}

void RangeSliderQt::setMinRange(int minR) {
    range_[0] = minR;

    if (minR > value_[0]) {
        setMinValue(minR);
    }
}

void RangeSliderQt::setMaxRange(int maxR) {
    range_[1] = maxR;

    if (maxR < value_[1]) {
        setMaxValue(maxR);
    }
}

void RangeSliderQt::resizeEvent(QResizeEvent* event) {
    QSplitter::resizeEvent(event);
    setMinSeparation(minSeperation_);
}

void RangeSliderQt::updateStateFromSliders() {
    QList<int> sizes = QSplitter::sizes();
    int range = sizes[0] + sizes[1] + sizes[2];

    int pos1 = sizes[0];
    int pos2 = sizes[0] + sizes[1];

    value_[0] = range_[0] + (range_[1]-range_[0]) * pos1 / range;
    value_[1] = range_[0] + (range_[1]-range_[0]) * pos2 / range;
}

void RangeSliderQt::updateSlidersFromState() {
    QList<int> sizes = QSplitter::sizes();
    int range = sizes[0] + sizes[1] + sizes[2];

    int size = range_[1] - range_[0];
    if (size <= 0) {
        return;
    }

    sizes.clear();
    sizes.append((range * (value_[0] - range_[0])) / size);
    sizes.append((range * (value_[1] - value_[0])) / size);
    sizes.append((range * (range_[1] - value_[1])) / size);

    QSplitter::blockSignals(true);
    QSplitter::setSizes(sizes);
    QSplitter::blockSignals(false);
}

//Index 1 = Min, Index 2 = Max
void RangeSliderQt::updateSplitterPosition(int pos, int idx) {
    updateStateFromSliders();

    //Emit
    emit valuesChanged(value_[0], value_[1]);
}

void RangeSliderQt::middleMoved(int delta) {
    QList<int> sizes = QSplitter::sizes();
    // ensure not to move middle part further than min/max values
    if (delta > 0) {
        delta = std::min(delta, sizes[2]);
    }
    else {
        delta = -std::min(-delta, sizes[0]);
    }
    if (delta == 0) {
        return;
    }
    // adjust sizes and modify splitter positions
    sizes[0] += delta;
    sizes[2] -= delta;
    QSplitter::blockSignals(true);
    QSplitter::setSizes(sizes);
    QSplitter::blockSignals(false);

    // update internal state
    this->updateSplitterPosition(0, 0);
}



RangeSliderMiddle::RangeSliderMiddle(QWidget *parent) 
    : QFrame(parent)
    , lastMouseX_(0)
    , drag_(false)
{
}

RangeSliderMiddle::~RangeSliderMiddle() {
}

void RangeSliderMiddle::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        // map to global position since the widget will move
        lastMouseX_ = this->mapToGlobal(event->pos()).x();
        drag_ = true;
    }
}

void RangeSliderMiddle::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        drag_ = false;
    }
}


void RangeSliderMiddle::mouseMoveEvent(QMouseEvent *event) {
    if (drag_) {
        // map to global position since the widget is about to be moved
        int newX = this->mapToGlobal(event->pos()).x();
        int delta = newX - lastMouseX_;
        lastMouseX_ = newX;
        if (delta != 0) {
            emit middleMoved(delta);
        }
    }
}

} // namespace inviwo
